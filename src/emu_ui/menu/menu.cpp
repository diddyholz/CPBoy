#include "menu.h"

#include "../../core/controls.h"
#include "../../core/emulator.h"
#include "../../core/error.h"
#include "../../core/peanut_gb_header.h"
#include "../../helpers/fileio.h"
#include "../../helpers/functions.h"
#include "../../helpers/macros.h"
#include "../colors.h"
#include "../components.h"
#include "../effects.h"
#include "../font.h"
#include "../input.h"
#include "tabs/current.h"
#include "tabs/load.h"
#include "tabs/saves.h"
#include "tabs/settings.h"
#include <sdk/calc/calc.hpp>
#include <sdk/os/input.hpp>
#include <sdk/os/lcd.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Use namespace because of conflicting function declarations in mem.hpp and
// string.h
namespace hhk {
#include <sdk/os/mem.hpp>
}

#define MENU_DESCRIPTION_HEIGHT 37
#define MENU_DESCRIPTION_X_OFFSET STD_CONTENT_OFFSET
#define MENU_TAB_HEIGHT 18
#define MENU_TAB_TITLE_OFFSET 2
#define MENU_ITEM_X_OFFSET STD_CONTENT_OFFSET
#define MENU_ITEM_Y_OFFSET STD_CONTENT_OFFSET

#define LOAD_ALERT_WIDTH 150
#define LOAD_ALERT_HEIGHT (DEBUG_LINE_HEIGHT * 3)

#define TAB_COUNT 4

void draw_pause_overlay() {
  darken_screen_area(0, 0, CAS_LCD_WIDTH, LCD_HEIGHT * 2);

  // print game paused text
  print_string_centered("Emulation paused", 0, 128, CAS_LCD_WIDTH, 0,
                        COLOR_WHITE, COLOR_BLACK, 1);
  print_string_centered("Press [(-)] to continue", 0, 148, CAS_LCD_WIDTH, 0,
                        COLOR_WHITE, COLOR_BLACK, 1);
}

void draw_menu_overlay() {
  darken_screen_area(0, LCD_HEIGHT * 2, CAS_LCD_WIDTH,
                     CAS_LCD_HEIGHT - (LCD_HEIGHT * 2));

  // print open menu text
  print_string_centered("Press [(-)] to open menu", 0, 402, CAS_LCD_WIDTH, 0,
                        COLOR_WHITE, COLOR_BLACK, 1);
}

void draw_tab_bar(uint16_t x, uint16_t y, uint16_t width, uint8_t selected_tab,
                  uint8_t tab_count, menu_tab *tabs) {
  const uint16_t tab_width = width / tab_count;

  // Draw background
  draw_rectangle(x, y, width, MENU_TAB_HEIGHT, COLOR_SECONDARY, 0, 0);

  // Draw background for selected tab
  draw_rectangle(x + (selected_tab * tab_width), y, tab_width, MENU_TAB_HEIGHT,
                 COLOR_PRIMARY, 0, 0);

  // Draw tab labels
  for (uint8_t i = 0; i < tab_count; i++) {
    const uint8_t tab_title_len = strlen(tabs[i].title);
    const uint16_t tab_offset = x + (i * tab_width);
    const uint16_t tab_title_offset =
        (tab_width - (tab_title_len * (DEBUG_CHAR_WIDTH - 2))) / 2;

    print_string(tabs[i].title, tab_offset + tab_title_offset,
                 y + MENU_TAB_TITLE_OFFSET, 0, COLOR_WHITE, COLOR_BLACK, true);
  }
}

void draw_tab_description(uint16_t x, uint16_t y, uint16_t width,
                          menu_tab *tab) {
  // Fill description background
  draw_rectangle(x, y, width, MENU_DESCRIPTION_HEIGHT, COLOR_PRIMARY, 0, 0);

  // Search for newline in description
  const char *p;
  for (p = tab->description; *p != '\0' && *p != '\n'; p++) {
  }

  uint8_t y_offset;

  if (*p == '\n') {
    y_offset = (MENU_DESCRIPTION_HEIGHT - (2 * DEBUG_LINE_HEIGHT)) / 2;
  } else {
    y_offset = (MENU_DESCRIPTION_HEIGHT - DEBUG_LINE_HEIGHT) / 2;
  }

  print_string(tab->description, x + MENU_DESCRIPTION_X_OFFSET, y + y_offset, 0,
               COLOR_BLACK, COLOR_BLACK, true);
}

void draw_menu_items(uint16_t x, uint16_t y, uint16_t width,
                     uint8_t selected_item, menu_tab *tab) {
  for (uint8_t i = 0; i < tab->item_count; i++) {
    menu_item *current_item = &(tab->items[i]);

    // Draw background for selected item
    if (selected_item == i) {
      draw_rectangle(x, y + (i * DEBUG_LINE_HEIGHT), width, DEBUG_LINE_HEIGHT,
                     COLOR_SELECTED, 0, 0);
    }

    // Draw title
    print_string(current_item->title, x + MENU_ITEM_X_OFFSET,
                 y + (i * DEBUG_LINE_HEIGHT), 0,
                 (current_item->disabled) ? COLOR_DISABLED : COLOR_WHITE,
                 COLOR_BLACK, true);

    // Draw value if it exists
    if (current_item->value[0] != '\0') {
      const uint8_t value_len = strlen(current_item->value);
      const uint16_t value_pos = x + width - MENU_ITEM_X_OFFSET -
                                 (value_len * (DEBUG_CHAR_WIDTH - 2)) - 1;

      print_string(current_item->value, value_pos, y + (i * DEBUG_LINE_HEIGHT),
                   0, current_item->value_color, COLOR_BLACK, true);
    }
  }
}

void draw_menu(menu *menu) {
  const uint16_t bottom_bar_pos =
      menu->y_pos + menu->height - MENU_TAB_HEIGHT - 1;

  // Fill menu area
  draw_rectangle(menu->x_pos, menu->y_pos, menu->width, menu->height,
                 menu->background, 0, 0);

  draw_tab_bar(menu->x_pos, bottom_bar_pos, menu->width, menu->selected_tab,
               menu->tab_count, menu->tabs);

  draw_tab_description(menu->x_pos, menu->y_pos, menu->width,
                       &(menu->tabs[menu->selected_tab]));

  draw_menu_items(
      menu->x_pos, menu->y_pos + MENU_DESCRIPTION_HEIGHT + MENU_ITEM_Y_OFFSET,
      menu->width, menu->selected_item, &(menu->tabs[menu->selected_tab]));
}

menu *prepare_load_menu_info(menu *menu) {
  menu->x_pos = 0;
  menu->y_pos = 0;
  menu->width = CAS_LCD_WIDTH;
  menu->height = CAS_LCD_HEIGHT;
  menu->background = COLOR_MENU_BG;
  menu->selected_tab = 0;
  menu->selected_item = 0;
  menu->tab_count = 1;
  menu->tabs = (menu_tab *)malloc(sizeof(menu_tab));

  if (!menu->tabs) {
    set_error(EMALLOC);
    return nullptr;
  }

  if (!prepare_tab_load(&(menu->tabs[0]))) {
    return nullptr;
  }

  return menu;
}

menu *prepare_menu_info(menu *menu, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  menu->x_pos = 0;
  menu->y_pos = LCD_HEIGHT * 2;
  menu->width = CAS_LCD_WIDTH;
  menu->height = CAS_LCD_HEIGHT - (LCD_HEIGHT * 2);
  menu->background = COLOR_MENU_BG;
  menu->selected_tab = 0;
  menu->selected_item = 0;
  menu->tab_count = 4;
  menu->tabs = (menu_tab *)malloc(menu->tab_count * sizeof(menu_tab));

  if (!menu->tabs) {
    set_error(EMALLOC);
    return nullptr;
  }

  if (!prepare_tab_current(&(menu->tabs[0]), preferences)) {
    return nullptr;
  }

  if (!prepare_tab_saves(&(menu->tabs[1]), preferences)) {
    return nullptr;
  }

  if (!prepare_tab_load(&(menu->tabs[2]))) {
    return nullptr;
  }

  if (!prepare_tab_settings(&(menu->tabs[3]), preferences)) {
    return nullptr;
  }

  return menu;
}

void cleanup_menu_info(menu *menu) {
  // Free the items for each tab
  for (uint8_t i = 0; i < menu->tab_count; i++) {
    free(menu->tabs[i].items);
  }

  // Free the tabs array
  free(menu->tabs);
}

uint8_t load_menu(emu_preferences *prefs) {
  // Prepare the menu
  menu load_menu;

  prepare_load_menu_info(&load_menu);

  uint8_t return_code;

  wait_input_release();

  // Menu loop
  for (;;) {
    draw_menu(&load_menu);
    LCD_Refresh();

    // Create horizontal items count array
    uint8_t h_items_count[load_menu.tabs[load_menu.selected_tab].item_count];

    for (uint8_t i = 0; i < load_menu.tabs[load_menu.selected_tab].item_count;
         i++) {
      h_items_count[i] = 1;
    }

    // Create horizontal items pointer array
    uint8_t
        *selected_h_items[load_menu.tabs[load_menu.selected_tab].item_count];

    for (uint8_t i = 0; i < load_menu.tabs[load_menu.selected_tab].item_count;
         i++) {
      selected_h_items[i] = &load_menu.selected_tab;
    }

    // Process input
    // This will move the selected item and selected tab and check if the menu
    // was closed or an action should be executed
    return_code = process_input(
        selected_h_items, &(load_menu.selected_item), h_items_count,
        load_menu.tabs[load_menu.selected_tab].item_count,
        load_menu.tabs[load_menu.selected_tab].items, true);

    // Check if something should be executed or the menu was closed
    if (return_code == INPUT_PROC_EXECUTE) {
      // Create dummy gb struct
      gb_s dummy_gb;
      dummy_gb.direct.priv = prefs;

      // Execute action
      return_code = load_menu.tabs[load_menu.selected_tab]
                        .items[load_menu.selected_item]
                        .action(&(load_menu.tabs[load_menu.selected_tab]
                                      .items[load_menu.selected_item]),
                                &dummy_gb);

      // Close the menu if something went wrong in the action
      if (return_code != 0) {
        break;
      }
    } else if (return_code == INPUT_PROC_CLOSE) {
      return_code = MENU_EMU_QUIT;
      break;
    }
  }

  cleanup_menu_info(&load_menu);

  // Wait for input to be released
  wait_input_release();

  return return_code;
}

uint8_t emulation_menu(struct gb_s *gb, bool preview_only) {
  // Backup gb frame and display pause overlay
  uint16_t *gb_frame_backup =
      (uint16_t *)malloc(LCD_HEIGHT * LCD_WIDTH * sizeof(uint16_t));

  if (gb_frame_backup) {
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
      for (uint16_t x = 0; x < LCD_WIDTH; x++) {
        gb_frame_backup[(y * LCD_WIDTH) + x] =
            vram[((y * 2) * CAS_LCD_WIDTH) + (x * 2)];
      }
    }
  }

  draw_pause_overlay();

  // Prepare the menu
  menu emulation_menu;

  if (prepare_menu_info(&emulation_menu, gb) == nullptr) {
    return MENU_CRASH;
  }

  int32_t return_code = MENU_CLOSED;

  if (!preview_only) {
    wait_input_release();
  }

  // Menu loop
  for (;;) {
    draw_menu(&emulation_menu);

    if (preview_only) {
      break;
    }

    LCD_Refresh();

    // Create horizontal items count array
    uint8_t h_items_count[emulation_menu.tabs[emulation_menu.selected_tab]
                              .item_count];

    for (uint8_t i = 0;
         i < emulation_menu.tabs[emulation_menu.selected_tab].item_count; i++) {
      h_items_count[i] = TAB_COUNT;
    }

    // Create horizontal items pointer array
    uint8_t *selected_h_items[emulation_menu.tabs[emulation_menu.selected_tab]
                                  .item_count];

    for (uint8_t i = 0;
         i < emulation_menu.tabs[emulation_menu.selected_tab].item_count; i++) {
      selected_h_items[i] = &(emulation_menu.selected_tab);
    }

    // Process input
    // This will move the selected item and selected tab and check if the menu
    // was closed or an action should be executed
    return_code = process_input(
        selected_h_items, &(emulation_menu.selected_item), h_items_count,
        emulation_menu.tabs[emulation_menu.selected_tab].item_count,
        emulation_menu.tabs[emulation_menu.selected_tab].items, true);

    // Check if something should be executed or the menu was closed
    if (return_code == INPUT_PROC_EXECUTE) {
      // Execute action
      return_code =
          emulation_menu.tabs[emulation_menu.selected_tab]
              .items[emulation_menu.selected_item]
              .action(&(emulation_menu.tabs[emulation_menu.selected_tab]
                            .items[emulation_menu.selected_item]),
                      gb);

      // Close the menu if something went wrong in the action
      if (return_code != 0) {
        break;
      }
    } else if (return_code == INPUT_PROC_CLOSE) {
      return_code = MENU_CLOSED;
      break;
    }
  }

  if (!preview_only) {
    wait_input_release();
  }

  cleanup_menu_info(&emulation_menu);

  // restore lcd
  if (gb_frame_backup) {
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
      for (uint16_t x = 0; x < LCD_WIDTH; x++) {
        vram[((y * 2) * (LCD_WIDTH * 2)) + (x * 2)] =
            gb_frame_backup[y * LCD_WIDTH + x];
        vram[((y * 2) * (LCD_WIDTH * 2)) + (x * 2) + 1] =
            gb_frame_backup[y * LCD_WIDTH + x];
        vram[(((y * 2) + 1) * (LCD_WIDTH * 2)) + (x * 2)] =
            gb_frame_backup[y * LCD_WIDTH + x];
        vram[(((y * 2) + 1) * (LCD_WIDTH * 2)) + (x * 2) + 1] =
            gb_frame_backup[y * LCD_WIDTH + x];
      }
    }

    free(gb_frame_backup);
  }

  draw_menu_overlay();

  return return_code;
}

void draw_load_alert(void) {
  darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);

  uint32_t pos =
      draw_alert_box(nullptr, nullptr, LOAD_ALERT_WIDTH, LOAD_ALERT_HEIGHT,
                     COLOR_MENU_BG, COLOR_PRIMARY);

  print_string_centered("Loading ...", ALERT_GET_X(pos),
                        ALERT_GET_Y(pos) + DEBUG_LINE_HEIGHT, LOAD_ALERT_WIDTH,
                        0, COLOR_WHITE, COLOR_BLACK, true);
}
