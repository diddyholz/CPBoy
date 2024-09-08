#include "current.h"

#include "../../../core/error.h"
#include "../../../helpers/functions.h"
#include "../../../helpers/macros.h"
#include "../../colors.h"
#include "../../components.h"
#include "../../effects.h"
#include "../../font.h"
#include "../../input.h"
#include "../menu.h"
#include <stdlib.h>
#include <string.h>

namespace hhk {
#include <sdk/os/mem.hpp>
}

#define TAB_CURRENT_TITLE "Current"

#define TAB_CUR_ITEM_COUNT 5

#define TAB_CUR_ITEM_FRAMESKIP_INDEX 0
#define TAB_CUR_ITEM_FRAMESKIP_TITLE "Frameskipping"
#define TAB_CUR_ITEM_FRAMESKIP_SUBTITLE "Skips rendering and LCD-Refresh"

#define TAB_CUR_ITEM_INTERL_INDEX 0
#define TAB_CUR_ITEM_INTERL_TITLE "Interlacing"

#define TAB_CUR_ITEM_SPEED_INDEX 1
#define TAB_CUR_ITEM_SPEED_TITLE "Emulation Speed"
#define TAB_CUR_ITEM_SPEED_SUBTITLE "Set the emulation speed target"

#define TAB_CUR_ITEM_OVERCLOCK_INDEX 2
#define TAB_CUR_ITEM_OVERCLOCK_TITLE "Overclock"

#define TAB_CUR_ITEM_PALETTE_INDEX 3
#define TAB_CUR_ITEM_PALETTE_TITLE "Color Palette"
#define TAB_CUR_ITEM_PALETTE_SUBTITLE "Select a palette for this ROM"

#define TAB_CUR_ITEM_QUIT_INDEX 4
#define TAB_CUR_ITEM_QUIT_TITLE "Quit CPBoy"

#define DIALOG_FRAMESKIP_ITEM_COUNT 3
#define DIALOG_FRAMESKIP_WIDTH 200
#define DIALOG_FRAMESKIP_HEIGHT                                                \
  DEBUG_LINE_HEIGHT *DIALOG_FRAMESKIP_ITEM_COUNT + ALERT_CONTENT_OFFSET_Y +    \
      (4 * STD_CONTENT_OFFSET)

#define DIALOG_PALETTE_WIDTH 200

#define DIALOG_SPEED_ITEM_COUNT 2
#define DIALOG_SPEED_WIDTH 200

#define OVERLOCK_WARNING_TEXT                                                  \
  "The CAS will be overclocked by 50%.\n"                                      \
  "This will increase performance at the cost of decreased battery life.\n\n"  \
  "Use at your own risk"

void draw_frameskip_alert(emu_preferences *preferences, uint8_t selected_item) {
  uint32_t position =
      draw_alert_box(TAB_CUR_ITEM_FRAMESKIP_TITLE,
                     TAB_CUR_ITEM_FRAMESKIP_SUBTITLE, DIALOG_FRAMESKIP_WIDTH,
                     DIALOG_FRAMESKIP_HEIGHT, COLOR_MENU_BG, COLOR_PRIMARY);

  char tmp[4];

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);

  const uint16_t slider_offset =
      ALERT_CONTENT_OFFSET_X + (7 * DEBUG_CHAR_WIDTH);
  const uint16_t slider_width = DIALOG_FRAMESKIP_WIDTH - slider_offset -
                                (4 * DEBUG_CHAR_WIDTH) - STD_CONTENT_OFFSET;

  // Draw frameskip enabled state
  print_string_centered(
      (preferences->config.frameskip_enabled) ? "Enabled" : "Disabled",
      dialog_x, dialog_y + ALERT_CONTENT_OFFSET_Y, DIALOG_FRAMESKIP_WIDTH, 0,
      (preferences->config.frameskip_enabled) ? COLOR_SUCCESS : COLOR_DANGER,
      (selected_item == 0) ? COLOR_SELECTED : COLOR_BLACK, true);

  // Draw amount slider
  print_string("Frames", dialog_x + ALERT_CONTENT_OFFSET_X,
               dialog_y + ALERT_CONTENT_OFFSET_Y + DEBUG_LINE_HEIGHT +
                   STD_CONTENT_OFFSET,
               0, COLOR_WHITE, COLOR_BLACK, true);

  draw_slider(dialog_x + slider_offset,
              dialog_y + ALERT_CONTENT_OFFSET_Y + DEBUG_LINE_HEIGHT +
                  STD_CONTENT_OFFSET,
              slider_width, SLIDER_STD_TRACK_COLOR,
              (selected_item == 1) ? COLOR_PRIMARY : COLOR_WHITE, FRAMESKIP_MIN,
              FRAMESKIP_MAX, preferences->config.frameskip_amount);

  print_string_centered(itoa(preferences->config.frameskip_amount, tmp, 10),
                        dialog_x + slider_offset + slider_width +
                            STD_CONTENT_OFFSET + STD_CONTENT_OFFSET,
                        dialog_y + ALERT_CONTENT_OFFSET_Y + DEBUG_LINE_HEIGHT +
                            STD_CONTENT_OFFSET,
                        (DEBUG_CHAR_WIDTH - 2) * 4, 0, COLOR_WHITE, COLOR_BLACK,
                        true);

  // Print ok button
  print_string_centered(
      "OK", dialog_x + ALERT_CONTENT_OFFSET_X,
      dialog_y + ALERT_CONTENT_OFFSET_Y + (2 * DEBUG_LINE_HEIGHT) +
          (3 * STD_CONTENT_OFFSET),
      DIALOG_FRAMESKIP_WIDTH - (2 * ALERT_CONTENT_OFFSET_X), 0, COLOR_WHITE,
      (selected_item == 2) ? COLOR_SELECTED : COLOR_BLACK, true);
}

void draw_speed_alert(emu_preferences *preferences, uint8_t selected_item) {
  uint16_t dialog_height = ALERT_CONTENT_OFFSET_Y + (2 * DEBUG_LINE_HEIGHT) +
                           (3 * STD_CONTENT_OFFSET);

  uint32_t position = draw_alert_box(
      TAB_CUR_ITEM_SPEED_TITLE, TAB_CUR_ITEM_SPEED_SUBTITLE, DIALOG_SPEED_WIDTH,
      dialog_height, COLOR_MENU_BG, COLOR_PRIMARY);

  char tmp[9];

  if (preferences->config.emulation_speed == EMU_SPEED_MAX + EMU_SPEED_STEP) {
    strlcpy(tmp, "Unlocked", sizeof(tmp));
  } else {
    itoa(preferences->config.emulation_speed, tmp, 10);
    strlcat(tmp, "%", sizeof(tmp));
  }

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);

  const uint16_t slider_offset =
      ALERT_CONTENT_OFFSET_X + (6 * DEBUG_CHAR_WIDTH);
  const uint16_t slider_width = DIALOG_FRAMESKIP_WIDTH - slider_offset -
                                (9 * DEBUG_CHAR_WIDTH) - STD_CONTENT_OFFSET;

  // Draw amount slider
  print_string("Speed", dialog_x + ALERT_CONTENT_OFFSET_X,
               dialog_y + ALERT_CONTENT_OFFSET_Y, 0, COLOR_WHITE, COLOR_BLACK,
               true);

  draw_slider(dialog_x + slider_offset, dialog_y + ALERT_CONTENT_OFFSET_Y,
              slider_width, SLIDER_STD_TRACK_COLOR,
              (selected_item == 0) ? COLOR_PRIMARY : COLOR_WHITE, EMU_SPEED_MIN,
              EMU_SPEED_MAX + EMU_SPEED_STEP,
              preferences->config.emulation_speed);

  print_string_centered(tmp,
                        dialog_x + slider_offset + slider_width +
                            STD_CONTENT_OFFSET + STD_CONTENT_OFFSET,
                        dialog_y + ALERT_CONTENT_OFFSET_Y,
                        (DEBUG_CHAR_WIDTH - 2) * 8, 0, COLOR_WHITE, COLOR_BLACK,
                        true);

  // Print ok button
  print_string_centered(
      "OK", dialog_x + ALERT_CONTENT_OFFSET_X,
      dialog_y + ALERT_CONTENT_OFFSET_Y + DEBUG_LINE_HEIGHT +
          (2 * STD_CONTENT_OFFSET),
      DIALOG_FRAMESKIP_WIDTH - (2 * ALERT_CONTENT_OFFSET_X), 0, COLOR_WHITE,
      (selected_item == 1) ? COLOR_SELECTED : COLOR_BLACK, true);
}

void draw_palette_selection_alert(emu_preferences *preferences,
                                  uint8_t selected_item) {
  uint16_t dialog_height = ALERT_CONTENT_OFFSET_Y +
                           (preferences->palette_count * DEBUG_LINE_HEIGHT) +
                           STD_CONTENT_OFFSET;

  uint32_t position = draw_alert_box(
      TAB_CUR_ITEM_PALETTE_TITLE, TAB_CUR_ITEM_PALETTE_SUBTITLE,
      DIALOG_PALETTE_WIDTH, dialog_height, COLOR_MENU_BG, COLOR_PRIMARY);

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);

  // Draw every palette
  for (uint8_t i = 0; i < preferences->palette_count; i++) {
    print_string_centered(
        preferences->palettes[i].name, dialog_x + ALERT_CONTENT_OFFSET_X,
        dialog_y + ALERT_CONTENT_OFFSET_Y + (i * DEBUG_LINE_HEIGHT),
        DIALOG_PALETTE_WIDTH - (2 * ALERT_CONTENT_OFFSET_X), 0, COLOR_WHITE,
        (selected_item == i) ? COLOR_SELECTED : COLOR_BLACK, true);
  }
}

int32_t frameskip_alert(struct gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  bool frameskip_enabled = preferences->config.frameskip_enabled;
  uint8_t frameskip_amount;

  uint8_t selected_item = 0;

  // Create horizontal items count
  static const uint8_t h_items_count[DIALOG_FRAMESKIP_ITEM_COUNT] = {
      1, FRAMESKIP_MAX, 1};

  // Create horizontal items pointer
  uint8_t *selected_h_items[DIALOG_FRAMESKIP_ITEM_COUNT] = {
      nullptr, &frameskip_amount, nullptr};

  // Rendering and input handling
  for (;;) {
    frameskip_amount = preferences->config.frameskip_amount - FRAMESKIP_MIN;

    draw_frameskip_alert(preferences, selected_item);

    // Check if OK button or toggle was pressed
    if (process_input(selected_h_items, &selected_item, h_items_count,
                      DIALOG_FRAMESKIP_ITEM_COUNT, nullptr,
                      false) == INPUT_PROC_EXECUTE) {
      if (selected_item == 0) {
        TOGGLE(frameskip_enabled);
      } else if (selected_item == 2) {
        break;
      }
    }

    // Apply frameskip to emulator
    set_frameskip(gb, frameskip_enabled, frameskip_amount + FRAMESKIP_MIN);

    LCD_Refresh();
  }

  // Close alert
  if (lcd_backup) {
    memcpy(vram, lcd_backup, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    free(lcd_backup);
  }

  return 0;
}

int32_t emu_speed_alert(struct gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  uint8_t emu_speed;
  uint8_t selected_item = 0;

  // Create horizontal items count
  static const uint8_t h_items_count[DIALOG_SPEED_ITEM_COUNT] = {
      (EMU_SPEED_MAX + EMU_SPEED_STEP) / EMU_SPEED_STEP, 1};

  // Create horizontal items pointer
  uint8_t *selected_h_items[DIALOG_SPEED_ITEM_COUNT] = {&emu_speed, nullptr};

  // Rendering and input handling
  for (;;) {
    emu_speed =
        (preferences->config.emulation_speed - EMU_SPEED_MIN) / EMU_SPEED_STEP;

    draw_speed_alert(preferences, selected_item);

    // Check if OK button or toggle was pressed
    if (process_input(selected_h_items, &selected_item, h_items_count,
                      DIALOG_SPEED_ITEM_COUNT, nullptr,
                      false) == INPUT_PROC_EXECUTE) {
      if (selected_item == 1) {
        break;
      }
    }

    // Apply speed to emulator
    set_emu_speed(gb, (emu_speed * EMU_SPEED_STEP) + EMU_SPEED_MIN);

    LCD_Refresh();
  }

  // Close alert
  if (lcd_backup) {
    memcpy(vram, lcd_backup, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    free(lcd_backup);
  }

  return 0;
}

int32_t palette_selection_alert(struct gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  uint8_t selected_item = 0;

  // Create horizontal item count array
  uint8_t h_items_count[preferences->palette_count];

  for (uint8_t i = 0; i < preferences->palette_count; i++) {
    h_items_count[i] = 1;
  }

  // Create horizontal items pointer array
  uint8_t *selected_h_items[preferences->palette_count];

  for (uint8_t i = 0; i < preferences->palette_count; i++) {
    selected_h_items[i] = nullptr;
  }

  // Rendering and input handling
  for (;;) {
    draw_palette_selection_alert(preferences, selected_item);

    // Check if palette was selected
    if (process_input(selected_h_items, &selected_item, h_items_count,
                      preferences->palette_count, nullptr,
                      false) == INPUT_PROC_EXECUTE) {
      // Do not do anything if the same palette is selected
      if (preferences->config.selected_palette == selected_item) {
        break;
      }

      preferences->file_states.rom_config_changed = true;
      preferences->config.selected_palette = selected_item;
      break;
    }

    LCD_Refresh();
  }

  // Close alert
  if (lcd_backup) {
    memcpy(vram, lcd_backup, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    free(lcd_backup);
  }

  return 0;
}

int32_t action_frameskip_selection(menu_item *item, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  int32_t return_code = frameskip_alert(gb);

  // Update item value text and color
  strlcpy(item->value,
          (preferences->config.frameskip_enabled) ? "Enabled (" : "Disabled",
          sizeof(item->value));
  item->value_color =
      (preferences->config.frameskip_enabled) ? COLOR_SUCCESS : COLOR_DANGER;

  // Append number of frames to skip when enabled
  if (preferences->config.frameskip_enabled) {
    char tmp[4];

    strlcat(item->value, itoa(preferences->config.frameskip_amount, tmp, 10),
            sizeof(item->value));
    strlcat(item->value, ")", sizeof(item->value));
  }

  return return_code;
}

int32_t action_speed_selection(menu_item *item, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;
  int32_t return_code = emu_speed_alert(gb);

  // Update item value text and color
  item->value_color = COLOR_SUCCESS;

  if (preferences->config.emulation_speed == EMU_SPEED_MAX + EMU_SPEED_STEP) {
    strlcpy(item->value, "Unlocked", sizeof(item->value));
  } else {
    itoa(preferences->config.emulation_speed, item->value, 10);
    strlcat(item->value, "%", sizeof(item->value));
  }

  return return_code;
}

int32_t action_interlacing_selection(menu_item *item, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Toggle interlacing
  set_interlacing(gb, !preferences->config.interlacing_enabled);

  // Update item value text and color
  strlcpy(item->value,
          (preferences->config.interlacing_enabled) ? "Enabled" : "Disabled",
          sizeof(item->value));
  item->value_color =
      (preferences->config.interlacing_enabled) ? COLOR_SUCCESS : COLOR_DANGER;

  return 0;
}

int32_t action_overclock_selection(menu_item *item, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Show warning when enabling overclock
  if (!preferences->config.overclock_enabled) {
    ok_alert(TAB_CUR_ITEM_OVERCLOCK_TITLE, nullptr, OVERLOCK_WARNING_TEXT,
             COLOR_WHITE, COLOR_MENU_BG, COLOR_PRIMARY);
  }

  // Toggle overclock
  set_overclock(gb, !preferences->config.overclock_enabled);

  // Update item value text and color
  strlcpy(item->value,
          (preferences->config.overclock_enabled) ? "Enabled" : "Disabled",
          sizeof(item->value));
  item->value_color =
      (preferences->config.overclock_enabled) ? COLOR_SUCCESS : COLOR_DANGER;

  return 0;
}

int32_t action_palette_selection(menu_item *item, gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  uint8_t ret = palette_selection_alert(gb);

  // Update item value
  strlcpy(item->value,
          preferences->palettes[preferences->config.selected_palette].name,
          sizeof(item->value));

  return ret;
}

int32_t action_quit_emulator(menu_item *item, gb_s *gb) {
  return MENU_EMU_QUIT;
}

menu_tab *prepare_tab_current(menu_tab *tab, emu_preferences *preferences) {
  char filename[TAB_DESCR_MAX_FILENAME_LENGTH + 1];

  // Description and title for "Settings" tab
  strlcpy(tab->title, TAB_CURRENT_TITLE, sizeof(tab->title));
  strlcpy(tab->description, "Current ROM: ", sizeof(tab->description));
  strlcat(tab->description, preferences->current_rom_name,
          sizeof(tab->description));
  strlcat(tab->description, "\nFilename: ", sizeof(tab->description));

  // If filename is too long, copy only first TAB_DESCR_MAX_FILENAME_LENGTH
  // chars
  if (strlen(preferences->current_filename) > TAB_DESCR_MAX_FILENAME_LENGTH) {
    strlcpy(filename, preferences->current_filename,
            TAB_DESCR_MAX_FILENAME_LENGTH - 4);
    strlcat(filename, " ...", sizeof(filename));
  } else {
    strlcpy(filename, preferences->current_filename, sizeof(filename));
  }

  strlcat(tab->description, filename, sizeof(tab->description));

  tab->item_count = TAB_CUR_ITEM_COUNT;
  tab->items =
      (menu_item *)malloc(TAB_CUR_ITEM_COUNT * sizeof(menu_item));

  if (!tab->items) {
    set_error(EMALLOC);
    return nullptr;
  }

  // Disabled state for each item
  tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].disabled = false;
  tab->items[TAB_CUR_ITEM_SPEED_INDEX].disabled = false;
  tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].disabled = false;
  // tab->items[TAB_CUR_ITEM_INTERL_INDEX].disabled = false;
  tab->items[TAB_CUR_ITEM_PALETTE_INDEX].disabled = false;
  tab->items[TAB_CUR_ITEM_QUIT_INDEX].disabled = false;

  // Title for each item
  strlcpy(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].title,
          TAB_CUR_ITEM_FRAMESKIP_TITLE,
          sizeof(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].title));
  strlcpy(tab->items[TAB_CUR_ITEM_SPEED_INDEX].title, TAB_CUR_ITEM_SPEED_TITLE,
          sizeof(tab->items[TAB_CUR_ITEM_SPEED_INDEX].title));
  strlcpy(tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].title,
          TAB_CUR_ITEM_OVERCLOCK_TITLE,
          sizeof(tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].title));
  // strlcpy(tab->items[TAB_CUR_ITEM_INTERL_INDEX].title,
  // TAB_CUR_ITEM_INTERL_TITLE,
  // sizeof(tab->items[TAB_CUR_ITEM_INTERL_INDEX].title));
  strlcpy(tab->items[TAB_CUR_ITEM_PALETTE_INDEX].title,
          TAB_CUR_ITEM_PALETTE_TITLE,
          sizeof(tab->items[TAB_CUR_ITEM_PALETTE_INDEX].title));
  strlcpy(tab->items[TAB_CUR_ITEM_QUIT_INDEX].title, TAB_CUR_ITEM_QUIT_TITLE,
          sizeof(tab->items[TAB_CUR_ITEM_QUIT_INDEX].title));

  // Value for each item
  strlcpy(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value,
          (preferences->config.frameskip_enabled) ? "Enabled (" : "Disabled",
          sizeof(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value));

  if (preferences->config.frameskip_enabled) {
    char tmp[4];

    itoa(preferences->config.frameskip_amount, tmp, 10);
    strlcat(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value, tmp,
            sizeof(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value));
    strlcat(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value, ")",
            sizeof(tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value));
  }

  if (preferences->config.emulation_speed == EMU_SPEED_MAX + EMU_SPEED_STEP) {
    strlcpy(tab->items[TAB_CUR_ITEM_SPEED_INDEX].value, "Unlocked",
            sizeof(tab->items[TAB_CUR_ITEM_SPEED_INDEX].value));
  } else {
    itoa(preferences->config.emulation_speed,
         tab->items[TAB_CUR_ITEM_SPEED_INDEX].value, 10);
    strlcat(tab->items[TAB_CUR_ITEM_SPEED_INDEX].value, "%",
            sizeof(tab->items[TAB_CUR_ITEM_SPEED_INDEX].value));
  }

  // strlcpy(tab->items[TAB_CUR_ITEM_INTERL_INDEX].value,
  //   (preferences->config.interlacing_enabled)? "Enabled" : "Disabled",
  //   sizeof(tab->items[TAB_CUR_ITEM_INTERL_INDEX].value));
  strlcpy(tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].value,
          (preferences->config.overclock_enabled) ? "Enabled" : "Disabled",
          sizeof(tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].value));
  strlcpy(tab->items[TAB_CUR_ITEM_PALETTE_INDEX].value,
          preferences->palettes[preferences->config.selected_palette].name,
          sizeof(tab->items[TAB_CUR_ITEM_PALETTE_INDEX].value));
  tab->items[TAB_CUR_ITEM_QUIT_INDEX].value[0] = '\0';

  // Value color for each item
  tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].value_color =
      (preferences->config.frameskip_enabled) ? COLOR_SUCCESS : COLOR_DANGER;
  // tab->items[TAB_CUR_ITEM_INTERL_INDEX].value_color =
  //   (preferences->config.interlacing_enabled)? COLOR_SUCCESS : COLOR_DANGER;
  tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].value_color =
      (preferences->config.overclock_enabled) ? COLOR_SUCCESS : COLOR_DANGER;
  tab->items[TAB_CUR_ITEM_SPEED_INDEX].value_color = COLOR_SUCCESS;
  tab->items[TAB_CUR_ITEM_PALETTE_INDEX].value_color = COLOR_SUCCESS;

  // Action for each item
  tab->items[TAB_CUR_ITEM_FRAMESKIP_INDEX].action = action_frameskip_selection;
  tab->items[TAB_CUR_ITEM_SPEED_INDEX].action = action_speed_selection;
  // tab->items[TAB_CUR_ITEM_INTERL_INDEX].action =
  // action_interlacing_selection;
  tab->items[TAB_CUR_ITEM_OVERCLOCK_INDEX].action = action_overclock_selection;
  tab->items[TAB_CUR_ITEM_PALETTE_INDEX].action = action_palette_selection;
  tab->items[TAB_CUR_ITEM_QUIT_INDEX].action = action_quit_emulator;

  return tab;
}
