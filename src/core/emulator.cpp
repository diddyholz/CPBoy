#include "emulator.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sdk/calc/calc.hpp>
#include <sdk/os/file.hpp>
#include <sdk/os/input.hpp>
#include <sdk/os/debug.hpp>
#include "controls.h"
#include "cart_ram.h"
#include "error.h"
#include "frametimes.h"
#include "peanut_gb.h"
#include "../cas/display.h"
#include "../cas/cpu/cmt.h"
#include "../cas/cpu/cpg.h"
#include "../cas/cpu/dmac.h"
#include "../cas/cpu/oc_mem.h"
#include "../cas/cpu/stack.h"
#include "../emu_ui/menu/menu.h"
#include "../helpers/macros.h"
#include "../helpers/functions.h"
#include "../helpers/fileio.h"

#define INPUT_NONE      0
#define INPUT_OPEN_MENU 1

#define STACK_PTR_ADDR  (void *)((uint32_t)Y_MEMORY_1 + (0x1000 - 4))

/* Global arrays in OC-Memory */
uint8_t gb_wram[WRAM_SIZE];
uint8_t gb_vram[VRAM_SIZE] __attribute__((section(".oc_mem.x")));
uint8_t gb_oam[OAM_SIZE] __attribute__((section(".oc_mem.y.dma")));
uint8_t gb_hram_io[HRAM_IO_SIZE] __attribute__((section(".oc_mem.y.data")));

uint8_t execution_handle_input(struct gb_s *gb)
{
  uint32_t key1;
  uint32_t key2;

  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Handle Key Input
  getKey(&key1, &key2);

  // Skip this function if no keys are pressed
  if (!Input_IsAnyKeyDown())
  {
    key1 = 0;
    key2 = 0;
  }

  gb->direct.joypad_bits.a =      !((key1 & preferences->controls[GB_KEY_A][0])       | (key2 & preferences->controls[GB_KEY_A][1]));
  gb->direct.joypad_bits.b =      !((key1 & preferences->controls[GB_KEY_B][0])       | (key2 & preferences->controls[GB_KEY_B][1]));
  gb->direct.joypad_bits.start =  !((key1 & preferences->controls[GB_KEY_START][0])   | (key2 & preferences->controls[GB_KEY_START][1]));
  gb->direct.joypad_bits.select = !((key1 & preferences->controls[GB_KEY_SELECT][0])  | (key2 & preferences->controls[GB_KEY_SELECT][1]));
  gb->direct.joypad_bits.up =     !((key1 & preferences->controls[GB_KEY_UP][0])      | (key2 & preferences->controls[GB_KEY_UP][1]));
  gb->direct.joypad_bits.down =   !((key1 & preferences->controls[GB_KEY_DOWN][0])    | (key2 & preferences->controls[GB_KEY_DOWN][1]));
  gb->direct.joypad_bits.left =   !((key1 & preferences->controls[GB_KEY_LEFT][0])    | (key2 & preferences->controls[GB_KEY_LEFT][1]));
  gb->direct.joypad_bits.right =  !((key1 & preferences->controls[GB_KEY_RIGHT][0])   | (key2 & preferences->controls[GB_KEY_RIGHT][1]));

  // Check if menu should be opened
  if (testKey(key1, key2, KEY_NEGATIVE))
  {
    return INPUT_OPEN_MENU;
  }

  return INPUT_NONE;
}

void set_frameskip(struct gb_s *gb, bool enabled, uint8_t amount)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  uint8_t new_amount = clamp(amount, (uint8_t)FRAMESKIP_MIN, (uint8_t)FRAMESKIP_MAX);

  // Check if anything should be changed
  if (
    enabled == preferences->config.frameskip_enabled 
    && new_amount == preferences->config.frameskip_amount
    && enabled == gb->direct.frame_skip 
    && new_amount == (gb->direct.frame_skip_amount + 1) 
  )
  {
    return;
  }

  gb->direct.frame_skip = enabled;
  gb->direct.frame_skip_amount = (new_amount + 1);
  preferences->config.frameskip_enabled = enabled;
  preferences->config.frameskip_amount = new_amount;    

  preferences->file_states.rom_config_changed = true;

  frametime_counter_set(gb);
}

void set_interlacing(struct gb_s *gb, bool enabled)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Check if anything should be changed
  if (
    enabled == preferences->config.interlacing_enabled
    && enabled == gb->direct.interlace
  )
  {
    return;
  }
  
  gb->direct.interlace = enabled;
  preferences->config.interlacing_enabled = enabled;

  preferences->file_states.rom_config_changed = true;
}

void set_emu_speed(struct gb_s *gb, uint16_t percentage){
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  
  uint16_t new_percentage = clamp(percentage, (uint16_t)EMU_SPEED_MIN, (uint16_t)(EMU_SPEED_MAX + EMU_SPEED_STEP));
  
  preferences->config.emulation_speed = new_percentage;
  preferences->file_states.rom_config_changed = true;

  frametime_counter_set(gb);  
}

void set_overclock(struct gb_s *gb, bool enabled)
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Check if anything should be changed
  if (preferences->config.overclock_enabled == enabled)
  {
    return;
  }

  preferences->config.overclock_enabled = enabled;
  preferences->file_states.rom_config_changed = true;

  cpg_set_pll_mul((enabled)? PLL_MUL_48 : CPG_PLL_MUL_DEFAULT);
  frametime_counter_set(gb);
}

// Draws scanline into framebuffer.
void lcd_draw_line(struct gb_s *gb, const uint32_t pixels[160],
  const uint_fast8_t line)
{
	emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Wait for previous DMA to complete
  dma_wait(DMAC_CHCR_0);

  if (unlikely(line == 0))
  {
    prepare_gb_lcd();
  }

  // When emulator will be paused, render a full frame in vram
  if (unlikely(preferences->emulator_paused))
  {
    for (uint16_t i = 0; i < LCD_WIDTH; i++)
    {
      *(uint32_t *)&vram[(i * 2) + ((line * 2) * CAS_LCD_WIDTH)] = pixels[i];
      *(uint32_t *)&vram[(i * 2) + (((line * 2) + 1) * CAS_LCD_WIDTH)] = pixels[i];
    }

    return;
  }

  // Initialize DMA settings
  dmac_chcr tmp_chcr = { .raw = 0 };
  tmp_chcr.TS_0 = SIZE_32_0;
  tmp_chcr.TS_1 = SIZE_32_1;
  tmp_chcr.DM   = DAR_FIXED_SOFT;
  tmp_chcr.SM   = SAR_INCREMENT;
  tmp_chcr.RS   = AUTO;
  tmp_chcr.TB   = CYCLE_STEAL;
  tmp_chcr.RPT  = RELOAD_SAR_TCR;
  tmp_chcr.DE   = 1;

  DMAC_CHCR_0->raw = 0;
#if PEANUT_FULL_GBC_SUPPORT
	if (gb->cgb.cgbMode)
	{
		uint32_t pixels_cgb[160];
		for (unsigned int x = 0; x < LCD_WIDTH; x++)
		{
			 pixels_cgb[x]=gb->cgb.fixPalette[pixels[x]];
		} 
		*DMAC_SAR_0   = (uint32_t)pixels_cgb; 
	}
	else {
#endif
  *DMAC_SAR_0   = (uint32_t)pixels;                            // P4 Area (OC-Memory) => Physical address is same as virtual
#if PEANUT_FULL_GBC_SUPPORT
	}
#endif
  *DMAC_DAR_0   = (uint32_t)SCREEN_DATA_REGISTER & 0x1FFFFFFF; // P2 Area => Physical address is virtual with 3 ms bits cleared
  *DMAC_TCR_0   = (CAS_LCD_WIDTH * 2) / 32 * 2;                // (Pixels per line * bytes per pixel) / dmac operation bytes * 2 lines      
  *DMAC_TCRB_0  = ((CAS_LCD_WIDTH * 2 / 32) << 16) 
    | (CAS_LCD_WIDTH * 2 / 32);

  // Start Channel 0
  DMAC_CHCR_0->raw = tmp_chcr.raw;
}

// Handles an error reported by the emulator. The emulator context may be used
// to better understand why the error given in gb_err was reported.
// TODO: Correctly implement this
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
{
  switch(gb_err)
  {
    case GB_INVALID_OPCODE:
      Debug_Printf(0, 0, false, 0, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
        val,
        gb->cpu_reg.pc.reg - 1,
        gb->cpu_reg.sp);
      break;

    // Ignoring non fatal errors.
    case GB_INVALID_WRITE:
    case GB_INVALID_READ:
      Debug_Printf(0, 0, false, 0, "IO-Operation failed");
      return;

    default:
      Debug_Printf(0, 0, false, 0, "Unknown error while executing");
      break;
  }
  
  for (uint8_t i = 0; i < 100; i++) 
  {
    LCD_Refresh();	
  }

  return;
}

uint8_t prepare_emulator(struct gb_s *gb, emu_preferences *preferences)
{
  enum gb_init_error_e gb_ret;

  // Initialise emulator context
  gb_ret = gb_init(gb, &gb_error, preferences, gb_wram, gb_vram, gb_oam, gb_hram_io, preferences->rom);
  
  // Add ROM name to preference struct
  gb_get_rom_name(gb, preferences->current_rom_name);

  switch(gb_ret)
  {
    case GB_INIT_NO_ERROR:
      break;

    case GB_INIT_CARTRIDGE_UNSUPPORTED:
      set_error(EEMUCARTRIDGE);
      return 1;

    case GB_INIT_INVALID_CHECKSUM:
      // Ignore invalid checksum
      break;

    default:
      set_error(EEMUGEN);
      return 1;
  }

  // Init gameboy rtc (Just zero everything)
  struct tm time;
  time.tm_sec = 0;
  time.tm_min = 0;
  time.tm_hour = 0;
  time.tm_yday = 0;

  gb_set_rtc(gb, &time);

  // Initialise lcd stuff 
  gb_init_lcd(gb, &lcd_draw_line);

  // Load cart save
  load_cart_ram(gb);
  gb_set_cram(gb, preferences->cart_ram);

  // Load user configs
  load_rom_config(gb);
  load_controls(gb);

  preferences->file_states.controls_changed = false;
  preferences->file_states.rom_config_changed = false;
  
  if (load_palettes(gb))
  {
    return 1;
  }

  // load_savestates();

  // check if loaded palette is out of range
  if (preferences->config.selected_palette >= preferences->palette_count) 
  {
    // Reset to default palette
    preferences->config.selected_palette = 0;
  }

  // Set default flags
  preferences->emulator_paused = false;

  return 0;
}

void free_emulator(struct gb_s *gb)
{
  emu_preferences *prefs = (emu_preferences *)gb->direct.priv;

  free(prefs->rom);
  free(prefs->cart_ram);
  free(prefs->palettes);
}

uint8_t close_rom(struct gb_s *gb)
{
  emu_preferences *prefs = (emu_preferences *)gb->direct.priv;
  uint8_t return_code = save_cart_ram(gb);

  if (prefs->file_states.rom_config_changed)
  {
    return_code |= save_rom_config(gb);
  }    

  free_emulator(gb);

  return return_code;
}

uint8_t execute_rom(struct gb_s *gb) 
{
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  frametime_counter_set(gb);

  for (;;)
  {
    frametime_counter_start();

    // Handle rtc
    if (unlikely(gb->display.frame_count % 24 == 0))
    {
      gb_tick_rtc(gb);
    }

    void *tmp_stack_ptr_bak = get_stack_ptr(); 
    set_stack_ptr(STACK_PTR_ADDR);

    // Run CPU until next frame
    gb_run_frame(gb);

    set_stack_ptr(tmp_stack_ptr_bak);

    frametime_counter_wait(gb);

    // Check if pause menu should be displayed
    if (unlikely(preferences->emulator_paused && gb->direct.frame_drawn))
    {
      uint8_t menu_code = emulation_menu(gb, false);

      if (menu_code != MENU_CLOSED)
      {
        return menu_code;
      }

      preferences->emulator_paused = false;

      LCD_Refresh();
    }

    // Handle input
    if (unlikely(execution_handle_input(gb) == INPUT_OPEN_MENU))
    {
      // Do not actually open the menu, but render another frame for gb preview first
      preferences->emulator_paused = true;
    }
  }
}

uint8_t run_emulator(struct gb_s *gb, emu_preferences *prefs) 
{
  bool exit_emulator = false;

  // Show load menu
  switch (load_menu(prefs))
  {
    case MENU_EMU_QUIT:
      return 0;

    case MENU_CRASH:
      return 1;
    
    default:
      break;
  }

  // Emulator rom load, execute and unload loop
  while (!exit_emulator)
  {
    draw_load_alert();
    LCD_Refresh();

    if (load_rom(prefs) != 0)
    {
      return 1;
    }

    if (prepare_emulator(gb, prefs) != 0)
    {
      return 1;
    }    
    
    // Render preview of menu
    fillScreen(0x0000);
    emulation_menu(gb, true);
    LCD_Refresh();
    
    switch (execute_rom(gb))
    {
      case MENU_CRASH:
        // Try to normaly close the rom, but this may fail
        close_rom(gb);
        return 1;

      case MENU_EMU_QUIT:
        exit_emulator = true;

      default:
        break;
    }

    if (close_rom(gb) != 0)
    {
      return 1;
    }
  }

  if (prefs->file_states.controls_changed)
  {
    if (save_controls(gb) != 0)
    {
      return 1;
    }
  }    
  
  return 0;
}

uint8_t load_rom(emu_preferences *prefs)
{
  char rom_filename[MAX_FILENAME_LEN] = DIRECTORY_ROM "\\";
  strncat(rom_filename, prefs->current_filename, MAX_FILENAME_LEN - 1);
  rom_filename[MAX_FILENAME_LEN - 1] = '\0';

  size_t rom_size;
	
	if (get_file_size(rom_filename, &rom_size))
  {
    return 1;
  }

	// dynamically allocate space for rom in heap
	prefs->rom = (uint8_t *)malloc(rom_size);

	// check if pointer to rom is no nullptr
	if (!prefs->rom)
  {
    char err_info[ERROR_MAX_INFO_LEN];
    char tmp[20];

    strlcpy(err_info, "GB ROM: ", sizeof(err_info));
    strlcat(err_info, itoa(rom_size, tmp, 10), sizeof(err_info));
    strlcat(err_info, "B", sizeof(err_info));
    
    set_error_i(EMALLOC, err_info);
		return 1;
  }

  if(read_file(rom_filename, prefs->rom, rom_size) != 0)
  {
    return 1;
  }

  return 0;
}
