#include "current.h"

#include "../../../core/error.h"
#include "../../../helpers/macros.h"
#include "../../colors.h"
#include "../../components.h"
#include "../../effects.h"
#include "../../font.h"
#include "../../input.h"
#include "../menu.h"
#include <sdk/os/debug.hpp>
#include <sdk/os/input.hpp>
#include <stdlib.h>
#include <string.h>

namespace hhk {
#include <sdk/os/mem.hpp>
}

#define TAB_SETTINGS_TITLE "Settings"
#define TAB_SETTINGS_DESCRIPTION "Settings"

#define TAB_SETTINGS_ITEM_COUNT 3
#define TAB_SETTINGS_ITEM_PALETTE_INDEX 0
#define TAB_SETTINGS_ITEM_PALETTE_TITLE "Custom Color Palettes"
#define TAB_SETTINGS_ITEM_PALETTE_SUBTITLE "[(-)] to delete"
#define TAB_SETTINGS_ITEM_CONTROLS_INDEX 1
#define TAB_SETTINGS_ITEM_CONTROLS_TITLE "Controls"
#define TAB_SETTINGS_ITEM_CONTROLS_SUBTITLE "Select key to edit"
#define TAB_SETTINGS_ITEM_CREDITS_INDEX 2
#define TAB_SETTINGS_ITEM_CREDITS_TITLE "Credits"
#define TAB_SETTINGS_ITEM_CREDITS_SUBTITLE CPBOY_VERSION
#define TAB_SETTINGS_ITEM_CREDITS_CONTENT                                      \
  "Based on Peanut-GB by deltabeard\n\n"                                       \
  "Source at github.com/diddyholz/CPBoy\n\n"                                   \
  "Contributors:\n"                                                            \
  "diddyholz (Sidney Krombholz)\n"                                             \
  "s3ansh33p (Sean McGinty)"

#define DIALOG_CONTROLS_ITEM_COUNT (GB_KEY_COUNT + 1)
#define DIALOG_CONTROLS_WIDTH 150
#define DIALOG_CONTROLS_HEIGHT                                                 \
  ALERT_CONTENT_OFFSET_Y + (DIALOG_CONTROLS_ITEM_COUNT * DEBUG_LINE_HEIGHT) +  \
      (2 * STD_CONTENT_OFFSET)

#define DIALOG_SELECT_KEY_WIDTH 200
#define DIALOG_SELECT_KEY_HEIGHT STD_CONTENT_OFFSET * 2 + DEBUG_LINE_HEIGHT

#define DIALOG_PALETTE_WIDTH 200
#define DIALOG_PALETTE_HEIGHT                                                  \
  (ALERT_CONTENT_OFFSET_Y - DEBUG_LINE_HEIGHT) +                               \
      (3 * ((2 * STD_CONTENT_OFFSET) + DIALOG_PALETTE_COLOR_PANEL_HEIGHT +     \
            DEBUG_LINE_HEIGHT)) +                                              \
      STD_CONTENT_OFFSET + DEBUG_LINE_HEIGHT
#define DIALOG_PALETTE_COLOR_RECT_HEIGHT 25
#define DIALOG_PALETTE_COLOR_PANEL_HEIGHT                                      \
  (3 * (SLIDER_HANDLE_HEIGHT + DIALOG_PALETTE_SLIDER_SPACING)) +               \
      DIALOG_PALETTE_COLOR_RECT_HEIGHT + STD_CONTENT_OFFSET
#define DIALOG_PALETTE_SLIDER_SPACING 2
#define DIALOG_PALETTE_SELECTED_COLOR COLOR_PRIMARY
#define DIALOG_PALETTE_SELECTED_BORDER_WIDTH 2
#define DIALOG_PALETTE_COLOR_DESC_WIDTH 5 * (DEBUG_CHAR_WIDTH - 2)

#define SELECTKEY_IDLE "Waiting for key ..."

void draw_controls_alert(emu_preferences *preferences, uint8_t selected_item) {
  uint32_t position =
      draw_alert_box(TAB_SETTINGS_ITEM_CONTROLS_TITLE,
                     TAB_SETTINGS_ITEM_CONTROLS_SUBTITLE, DIALOG_CONTROLS_WIDTH,
                     DIALOG_CONTROLS_HEIGHT, COLOR_MENU_BG, COLOR_PRIMARY);

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);

  // Draw every controls option
  for (uint8_t i = 0; i < GB_KEY_COUNT; i++) {
    char title[14];

    // Draw controls title
    switch (i) {
    case GB_KEY_A:
      strcpy(title, GB_KEY_TEXT_A);
      break;
    case GB_KEY_B:
      strcpy(title, GB_KEY_TEXT_B);
      break;
    case GB_KEY_START:
      strcpy(title, GB_KEY_TEXT_START);
      break;
    case GB_KEY_SELECT:
      strcpy(title, GB_KEY_TEXT_SELECT);
      break;
    case GB_KEY_UP:
      strcpy(title, GB_KEY_TEXT_UP);
      break;
    case GB_KEY_DOWN:
      strcpy(title, GB_KEY_TEXT_DOWN);
      break;
    case GB_KEY_LEFT:
      strcpy(title, GB_KEY_TEXT_LEFT);
      break;
    case GB_KEY_RIGHT:
      strcpy(title, GB_KEY_TEXT_RIGHT);
      break;

    default:
      break;
    }

    print_string(title, dialog_x + ALERT_CONTENT_OFFSET_X,
                 dialog_y + ALERT_CONTENT_OFFSET_Y + (i * DEBUG_LINE_HEIGHT), 0,
                 COLOR_WHITE, COLOR_BLACK, true);

    // Draw button title
    switch (preferences->controls[i][0]) {
    case KEY_SHIFT:
      strcpy(title, CAS_KEY_TEXT_SHIFT);
      break;
    case KEY_CLEAR:
      strcpy(title, CAS_KEY_TEXT_CLEAR);
      break;
    case KEY_BACKSPACE:
      strcpy(title, CAS_KEY_TEXT_BACKSPACE);
      break;
    case KEY_LEFT:
      strcpy(title, CAS_KEY_TEXT_LEFT);
      break;
    case KEY_RIGHT:
      strcpy(title, CAS_KEY_TEXT_RIGHT);
      break;
    case KEY_Z:
      strcpy(title, CAS_KEY_TEXT_Z);
      break;
    case KEY_POWER:
      strcpy(title, CAS_KEY_TEXT_POWER);
      break;
    case KEY_DIVIDE:
      strcpy(title, CAS_KEY_TEXT_DIVIDE);
      break;
    case KEY_MULTIPLY:
      strcpy(title, CAS_KEY_TEXT_MULTIPLY);
      break;
    case KEY_SUBTRACT:
      strcpy(title, CAS_KEY_TEXT_SUBSTRACT);
      break;
    case KEY_ADD:
      strcpy(title, CAS_KEY_TEXT_ADD);
      break;
    case KEY_EXE:
      strcpy(title, CAS_KEY_TEXT_EXE);
      break;
    case KEY_EXP:
      strcpy(title, CAS_KEY_TEXT_EXP);
      break;
    case KEY_3:
      strcpy(title, CAS_KEY_TEXT_3);
      break;
    case KEY_6:
      strcpy(title, CAS_KEY_TEXT_6);
      break;
    case KEY_9:
      strcpy(title, CAS_KEY_TEXT_9);
      break;

    default:
      break;
    }

    switch (preferences->controls[i][1]) {
    case KEY_KEYBOARD:
      strcpy(title, CAS_KEY_TEXT_KEYBOARD);
      break;
    case KEY_UP:
      strcpy(title, CAS_KEY_TEXT_UP);
      break;
    case KEY_DOWN:
      strcpy(title, CAS_KEY_TEXT_DOWN);
      break;
    case KEY_EQUALS:
      strcpy(title, CAS_KEY_TEXT_EQUALS);
      break;
    case KEY_X:
      strcpy(title, CAS_KEY_TEXT_X);
      break;
    case KEY_Y:
      strcpy(title, CAS_KEY_TEXT_Y);
      break;
    case KEY_LEFT_BRACKET:
      strcpy(title, CAS_KEY_TEXT_LEFT_BRACKET);
      break;
    case KEY_RIGHT_BRACKET:
      strcpy(title, CAS_KEY_TEXT_RIGHT_BRACKET);
      break;
    case KEY_COMMA:
      strcpy(title, CAS_KEY_TEXT_COMMA);
      break;
    case KEY_NEGATIVE:
      strcpy(title, CAS_KEY_TEXT_NEGATIVE);
      break;
    case KEY_0:
      strcpy(title, CAS_KEY_TEXT_0);
      break;
    case KEY_DOT:
      strcpy(title, CAS_KEY_TEXT_DOT);
      break;
    case KEY_1:
      strcpy(title, CAS_KEY_TEXT_1);
      break;
    case KEY_2:
      strcpy(title, CAS_KEY_TEXT_2);
      break;
    case KEY_4:
      strcpy(title, CAS_KEY_TEXT_4);
      break;
    case KEY_5:
      strcpy(title, CAS_KEY_TEXT_5);
      break;
    case KEY_7:
      strcpy(title, CAS_KEY_TEXT_7);
      break;
    case KEY_8:
      strcpy(title, CAS_KEY_TEXT_8);
      break;

    default:
      break;
    }

    if (!(preferences->controls[i][0]) && !(preferences->controls[i][1])) {
      strcpy(title, CAS_KEY_TEXT_NONE);
    }

    print_string_centered(
        title,
        dialog_x +
            (DIALOG_CONTROLS_WIDTH -
             (ALERT_CONTENT_OFFSET_X + (10 * (DEBUG_CHAR_WIDTH - 2)) + 2)),
        dialog_y + ALERT_CONTENT_OFFSET_Y + (i * DEBUG_LINE_HEIGHT),
        (DEBUG_CHAR_WIDTH - 2) * 10, 0, COLOR_WHITE,
        (selected_item == i) ? COLOR_SELECTED : COLOR_BLACK, true);
  }

  print_string_centered(
      " OK ", dialog_x,
      dialog_y + ALERT_CONTENT_OFFSET_Y + (GB_KEY_COUNT * DEBUG_LINE_HEIGHT) +
          STD_CONTENT_OFFSET,
      DIALOG_CONTROLS_WIDTH, 0, COLOR_WHITE,
      (selected_item == 8) ? COLOR_SELECTED : COLOR_BLACK, true);
}

void draw_select_key_alert() {
  uint32_t position =
      draw_alert_box(nullptr, nullptr, DIALOG_SELECT_KEY_WIDTH,
                     DIALOG_SELECT_KEY_HEIGHT, COLOR_MENU_BG, COLOR_PRIMARY);

  uint16_t dialog_x = ALERT_GET_X(position);
  uint16_t dialog_y = ALERT_GET_Y(position);

  print_string_centered(SELECTKEY_IDLE, dialog_x, dialog_y + STD_CONTENT_OFFSET,
                        DIALOG_SELECT_KEY_WIDTH, 0, COLOR_WHITE, COLOR_BLACK,
                        true);
}

void draw_edit_color_panel(uint16_t x, uint16_t y, uint16_t width,
                           uint16_t *colors, uint8_t selected_item,
                           uint8_t selected_color) {
  const uint16_t color_rect_width = width / 4;

  // Draw color selection rectangles
  for (uint8_t i = 0; i < 4; i++) {
    draw_rectangle(x + (i * color_rect_width), y, color_rect_width,
                   DIALOG_PALETTE_COLOR_RECT_HEIGHT, colors[i],
                   (selected_color == i) * DIALOG_PALETTE_SELECTED_BORDER_WIDTH,
                   (selected_item == 0) ? DIALOG_PALETTE_SELECTED_COLOR
                                        : COLOR_WHITE);
  }

  const char *color_descriptions[] = {"Red", "Green", "Blue"};

  const uint8_t color_max_vals[] = {
      31, // Red (5-bit)
      63, // Green (6-bit)
      31  // Blue (5-bit)
  };

  const uint16_t color_vals[] = {RGB565_TO_R(colors[selected_color]),
                                 RGB565_TO_G(colors[selected_color]),
                                 RGB565_TO_B(colors[selected_color])};

  // Draw sliders and description
  for (uint8_t i = 0; i < 3; i++) {
    print_string(
        color_descriptions[i], x,
        y + DIALOG_PALETTE_COLOR_RECT_HEIGHT + STD_CONTENT_OFFSET +
            (i * (SLIDER_HANDLE_HEIGHT + DIALOG_PALETTE_SLIDER_SPACING)),
        0, COLOR_WHITE, COLOR_BLACK, true);

    draw_slider(
        x + DIALOG_PALETTE_COLOR_DESC_WIDTH + STD_CONTENT_OFFSET,
        y + DIALOG_PALETTE_COLOR_RECT_HEIGHT + STD_CONTENT_OFFSET +
            (i * (SLIDER_HANDLE_HEIGHT + DIALOG_PALETTE_SLIDER_SPACING)),
        width - (DIALOG_PALETTE_COLOR_DESC_WIDTH + STD_CONTENT_OFFSET),
        SLIDER_STD_TRACK_COLOR,
        (selected_item == (i + 1)) ? COLOR_PRIMARY : COLOR_WHITE, 0,
        color_max_vals[i], color_vals[i]);
  }
}

void draw_edit_palette_alert(palette *pal, uint8_t selected_item,
                             uint8_t *selected_colors) {
  uint32_t position =
      draw_alert_box(pal->name, nullptr, DIALOG_PALETTE_WIDTH,
                     DIALOG_PALETTE_HEIGHT, COLOR_MENU_BG, COLOR_PRIMARY);

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);
  const uint16_t content_y =
      dialog_y + ALERT_CONTENT_OFFSET_Y - DEBUG_LINE_HEIGHT;

  const char *titles[] = {"OBJ0", "OBJ1", "Background"};

  // Print all titles and selection panels
  for (uint8_t i = 0; i < 3; i++) {
    print_string(titles[i], dialog_x + ALERT_CONTENT_OFFSET_X,
                 content_y +
                     (i * (DIALOG_PALETTE_COLOR_PANEL_HEIGHT +
                           DEBUG_LINE_HEIGHT + (2 * STD_CONTENT_OFFSET))),
                 0, COLOR_WHITE, COLOR_BLACK, true);

    draw_edit_color_panel(
        dialog_x + ALERT_CONTENT_OFFSET_X,
        content_y + DEBUG_LINE_HEIGHT +
            (i * (DIALOG_PALETTE_COLOR_PANEL_HEIGHT + DEBUG_LINE_HEIGHT +
                  (2 * STD_CONTENT_OFFSET))),
        DIALOG_PALETTE_WIDTH - 2 * (STD_CONTENT_OFFSET), pal->data[i],
        selected_item - (i * 4), selected_colors[i]);
  }

  print_string_centered(
      " OK ", dialog_x,
      content_y + (3 * (DIALOG_PALETTE_COLOR_PANEL_HEIGHT + DEBUG_LINE_HEIGHT +
                        (2 * STD_CONTENT_OFFSET))),
      DIALOG_PALETTE_WIDTH, 0, COLOR_WHITE,
      (selected_item == 12) ? COLOR_SELECTED : COLOR_BLACK, true);
}

void draw_palettes_alert(palette *user_palettes, uint8_t user_palette_count,
                         uint8_t selected_item) {
  uint16_t dialog_height = ALERT_CONTENT_OFFSET_Y +
                           ((user_palette_count + 2) * DEBUG_LINE_HEIGHT) +
                           (2 * STD_CONTENT_OFFSET);

  uint32_t position = draw_alert_box(
      TAB_SETTINGS_ITEM_PALETTE_TITLE, TAB_SETTINGS_ITEM_PALETTE_SUBTITLE,
      DIALOG_PALETTE_WIDTH, dialog_height, COLOR_MENU_BG, COLOR_PRIMARY);

  const uint16_t dialog_x = ALERT_GET_X(position);
  const uint16_t dialog_y = ALERT_GET_Y(position);

  // Draw every palette. Begin from 1 to skip default
  for (uint8_t i = 0; i < user_palette_count; i++) {
    print_string_centered(
        user_palettes[i].name, dialog_x + ALERT_CONTENT_OFFSET_X,
        dialog_y + ALERT_CONTENT_OFFSET_Y + (i * DEBUG_LINE_HEIGHT),
        DIALOG_PALETTE_WIDTH - (2 * ALERT_CONTENT_OFFSET_X), 0, COLOR_WHITE,
        (selected_item == i) ? COLOR_SELECTED : COLOR_BLACK, true);
  }

  // Print Create New button
  print_string_centered(
      " Create New ", dialog_x,
      dialog_y + ALERT_CONTENT_OFFSET_Y +
          (user_palette_count * DEBUG_LINE_HEIGHT) + STD_CONTENT_OFFSET,
      DIALOG_PALETTE_WIDTH, 0, COLOR_WHITE,
      (selected_item == user_palette_count) ? COLOR_SELECTED : COLOR_BLACK,
      true);

  print_string_centered(
      " OK ", dialog_x,
      dialog_y + ALERT_CONTENT_OFFSET_Y +
          ((user_palette_count + 1) * DEBUG_LINE_HEIGHT) + STD_CONTENT_OFFSET,
      DIALOG_PALETTE_WIDTH, 0, COLOR_WHITE,
      (selected_item == (user_palette_count + 1)) ? COLOR_SELECTED
                                                  : COLOR_BLACK,
      true);
}

int32_t select_key_alert(uint32_t *key) {
  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  draw_select_key_alert();

  LCD_Refresh();

  wait_input_release();

  while (!Input_IsAnyKeyDown()) {
  }

  // Save the pressed key to the controls array
  // Do this in a loop because the key buffer might
  // still be empty even when a key is pressed
  do {
    getKey(&(key[0]), &(key[1]));
  } while (!(key[0] | key[1]));

  // Close alert
  if (lcd_backup) {
    memcpy(vram, lcd_backup, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    free(lcd_backup);
  }

  wait_input_release();

  return 0;
}

int32_t controls_alert(struct gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)gb->direct.priv;

  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  uint8_t selected_item = 0;

  // Create horizontal items count
  uint8_t h_items_count[DIALOG_CONTROLS_ITEM_COUNT];

  for (uint8_t i = 0; i < DIALOG_CONTROLS_ITEM_COUNT; i++) {
    h_items_count[i] = 1;
  }

  // Create horizontal items pointer
  uint8_t *selected_h_items[DIALOG_CONTROLS_ITEM_COUNT];

  for (uint8_t i = 0; i < DIALOG_CONTROLS_ITEM_COUNT; i++) {
    selected_h_items[i] = nullptr;
  }

  // Rendering and input handling
  for (;;) {
    draw_controls_alert(preferences, selected_item);

    uint8_t ret = process_input(selected_h_items, &selected_item, h_items_count,
                                DIALOG_CONTROLS_ITEM_COUNT, nullptr, false);

    // Check which controls key was clicked
    if (ret == INPUT_PROC_EXECUTE) {
      // Check if OK button was pressed
      if (selected_item == GB_KEY_COUNT) {
        break;
      }

      select_key_alert(preferences->controls[selected_item]);

      preferences->file_states.controls_changed = true;
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

int32_t edit_palette_alert(palette *pal) {
  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  uint8_t selected_item = 0;
  uint8_t selected_colors[3] = {0};
  uint8_t prev_selected_colors[3] = {0};
  uint8_t colors[3][3] = {0};

  // Create horizontal item count array
  uint8_t h_items_count[] = {
      4, 32, 64, 32, // OBJ0
      4, 32, 64, 32, // OBJ1
      4, 32, 64, 32, // OBJ2
      1,             // OK
  };

  // Create horizontal items pointer array
  uint8_t *selected_h_items[] = {
      &selected_colors[0],
      &colors[0][0],
      &colors[0][1],
      &colors[0][2], // OBJ0
      &selected_colors[1],
      &colors[1][0],
      &colors[1][1],
      &colors[1][2], // OBJ1
      &selected_colors[2],
      &colors[2][0],
      &colors[2][1],
      &colors[2][2], // OBJ2
      nullptr        // OK
  };

  // Get colors
  for (uint8_t i = 0; i < 3; i++) {
    colors[i][0] = RGB565_TO_R(pal->data[i][selected_colors[i]]);
    colors[i][1] = RGB565_TO_G(pal->data[i][selected_colors[i]]);
    colors[i][2] = RGB565_TO_B(pal->data[i][selected_colors[i]]);
  }

  // Rendering and input handling
  for (;;) {
    draw_edit_palette_alert(pal, selected_item, selected_colors);

    // Check if palette or add new was selected
    if (process_input(selected_h_items, &selected_item, h_items_count, 13,
                      nullptr, false) == INPUT_PROC_EXECUTE) {
      // Check if OK button was selected
      if (selected_item == 12) {
        break;
      }
    }

    // Check if color was switched and needs to be refetched
    if (memcmp(prev_selected_colors, selected_colors,
               sizeof(selected_colors)) != 0) {
      // Get colors
      for (uint8_t i = 0; i < 3; i++) {
        colors[i][0] = RGB565_TO_R(pal->data[i][selected_colors[i]]);
        colors[i][1] = RGB565_TO_G(pal->data[i][selected_colors[i]]);
        colors[i][2] = RGB565_TO_B(pal->data[i][selected_colors[i]]);
      }

      memcpy(prev_selected_colors, selected_colors, sizeof(selected_colors));
    } else {
      // Set colors
      for (uint8_t i = 0; i < 3; i++) {
        pal->data[i][selected_colors[i]] =
            RGB_TO_RGB565((uint16_t)colors[i][0], (uint16_t)colors[i][1],
                          (uint16_t)colors[i][2]);
      }
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

int32_t palettes_alert(struct gb_s *gb) {
  emu_preferences *preferences = (emu_preferences *)(gb->direct.priv);
  palette *user_palettes;
  uint8_t user_palette_count = get_user_palettes(&user_palettes, gb);

  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  uint8_t selected_item = 0;

  // Create horizontal item count array
  uint8_t h_items_count[user_palette_count + 2];

  for (uint8_t i = 0; i < user_palette_count + 2; i++) {
    h_items_count[i] = 1;
  }

  // Create horizontal items pointer array
  uint8_t *selected_h_items[user_palette_count + 2];

  for (uint8_t i = 0; i < user_palette_count + 2; i++) {
    selected_h_items[i] = nullptr;
  }

  // Rendering and input handling
  for (;;) {
    draw_palettes_alert(user_palettes, user_palette_count, selected_item);

    uint8_t ret;

    // Check if palette or add new was selected
    if ((ret = process_input(selected_h_items, &selected_item, h_items_count,
                             user_palette_count + 2, nullptr, false)) !=
        INPUT_PROC_NONE) {
      if (ret == INPUT_PROC_CLOSE) {
        if (selected_item < user_palette_count) {
          delete_palette(gb, selected_item);
          user_palette_count = get_user_palettes(&user_palettes, gb);

          // Restore background as the window gets smaller
          if (lcd_backup) {
            memcpy(vram, lcd_backup,
                   CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
            darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
          }
        }
      } else if (ret == INPUT_PROC_EXECUTE) {
        // Check if OK was pressed
        if (selected_item == user_palette_count + 1) {
          break;
        }

        // Check if Create New was pressed
        if (selected_item == user_palette_count) {
          uint8_t ret = create_palette(gb);

          if (ret == ERR_MAX_PALETTE_REACHED) {
            ok_alert("ERROR", "Could not create palette",
                     "You have already created the maximum of " XSTR(
                         MAX_PALETTE_COUNT) " palettes!",
                     COLOR_WHITE, COLOR_MENU_BG, COLOR_PRIMARY);

            continue;
          } else if (ret != 0) {
            return MENU_CRASH;
          }

          user_palette_count = get_user_palettes(&user_palettes, gb);
        }

        edit_palette_alert(&(user_palettes[selected_item]));
        save_palette(&(user_palettes[selected_item]), selected_item);
      }
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

int32_t action_edit_palettes(menu_item *item, gb_s *gb) {
  return palettes_alert(gb);
}

int32_t action_edit_controls(menu_item *item, gb_s *gb) {
  return controls_alert(gb);
}

int32_t action_show_credits(menu_item *item, gb_s *gb) {
  ok_alert(TAB_SETTINGS_ITEM_CREDITS_TITLE, TAB_SETTINGS_ITEM_CREDITS_SUBTITLE,
           TAB_SETTINGS_ITEM_CREDITS_CONTENT, COLOR_WHITE, COLOR_MENU_BG,
           COLOR_PRIMARY);

  return 0;
}

menu_tab *prepare_tab_settings(menu_tab *tab, emu_preferences *preferences) {
  // Description and title for "Settings" tab
  strcpy(tab->title, TAB_SETTINGS_TITLE);
  strcpy(tab->description, TAB_SETTINGS_DESCRIPTION);

  tab->item_count = TAB_SETTINGS_ITEM_COUNT;
  tab->items =
      (menu_item *)malloc(TAB_SETTINGS_ITEM_COUNT * sizeof(menu_item));

  if (!tab->items) {
    set_error(EMALLOC);
    return nullptr;
  }

  // Disabled state for each item
  tab->items[TAB_SETTINGS_ITEM_PALETTE_INDEX].disabled = false;
  tab->items[TAB_SETTINGS_ITEM_CONTROLS_INDEX].disabled = false;
  tab->items[TAB_SETTINGS_ITEM_CREDITS_INDEX].disabled = false;

  // Title for each item
  strcpy(tab->items[TAB_SETTINGS_ITEM_PALETTE_INDEX].title,
         TAB_SETTINGS_ITEM_PALETTE_TITLE);
  strcpy(tab->items[TAB_SETTINGS_ITEM_CONTROLS_INDEX].title,
         TAB_SETTINGS_ITEM_CONTROLS_TITLE);
  strcpy(tab->items[TAB_SETTINGS_ITEM_CREDITS_INDEX].title,
         TAB_SETTINGS_ITEM_CREDITS_TITLE);

  // Value for each item
  tab->items[TAB_SETTINGS_ITEM_PALETTE_INDEX].value[0] = '\0';
  tab->items[TAB_SETTINGS_ITEM_CONTROLS_INDEX].value[0] = '\0';
  tab->items[TAB_SETTINGS_ITEM_CREDITS_INDEX].value[0] = '\0';

  // Action for each item
  tab->items[TAB_SETTINGS_ITEM_PALETTE_INDEX].action = action_edit_palettes;
  tab->items[TAB_SETTINGS_ITEM_CONTROLS_INDEX].action = action_edit_controls;
  tab->items[TAB_SETTINGS_ITEM_CREDITS_INDEX].action = action_show_credits;

  return tab;
}
