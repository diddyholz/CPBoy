#include "components.h"

#include "../emu_ui/colors.h"
#include "../emu_ui/effects.h"
#include "../emu_ui/font.h"
#include "../emu_ui/input.h"
#include "../helpers/macros.h"
#include <sdk/calc/calc.hpp>
#include <sdk/os/debug.hpp>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Use namespace because of conflicting function declarations in mem.hpp and
// string.h
namespace hhk {
#include <sdk/os/mem.hpp>
}

#define CAS_LCD_WIDTH 320
#define CAS_LCD_HEIGHT 528

#define SLIDER_HANDLE_WIDTH 4
#define SLIDER_TRACK_HEIGHT 2
#define SLIDER_Y_OFFSET 1

#define TEXT_ALERT_MIN_WIDTH 100
#define TEXT_ALERT_MAX_CHAR 40
#define TEXT_ALERT_MAX_WIDTH                                                   \
  ((2 * ALERT_CONTENT_OFFSET_X) +                                              \
   ((DEBUG_CHAR_WIDTH - 2) * TEXT_ALERT_MAX_CHAR))
#define TEXT_ALERT_MIN_HEIGTH (STD_CONTENT_OFFSET * 4) + (DEBUG_LINE_HEIGHT * 2)
#define TEXT_ALERT_MAX_HEIGHT (CAS_LCD_HEIGHT - 60)

#define MAX_ALERT_MESSAGE_LEN 300

int string_split_newline(char *dest, size_t len, const char *src,
                         uint16_t max_chars) {
  const char *last_space = nullptr;
  const char *prev = src;

  uint16_t line = 1;

  // Clear destination buffer
  memset(dest, 0, len);

  for (const char *p = src; *p != '\0'; p++) {
    // Check if there is still enough space to hold current line
    if (len < (size_t)(p - prev)) {
      return line;
    }

    // Check if line has reached max_chars
    if ((p - prev) == (max_chars - 1)) {
      // Check if space was found
      if (!last_space) {
        last_space = p;
      }

      last_space++;

      // Write line to destination buffer when max chars is reached
      strncat(dest, prev, last_space - prev);
      strcat(dest, "\n");

      len -= (last_space - prev);
      prev = last_space;
      last_space = nullptr;
      line++;
      continue;
    }

    if (*p == ' ') {
      last_space = p;
    } else if (*p == '\n') {
      last_space = p + 1;
      strncat(dest, prev, last_space - prev);

      len -= (last_space - prev);
      prev = last_space;
      last_space = nullptr;
      line++;
      continue;
    }
  }

  // Handle last line
  strlcat(dest, prev, len);

  return line;
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                    uint16_t color, uint16_t border, uint16_t border_color) {
  uint16_t max_x = x + width;
  uint16_t max_y = y + height;
  uint16_t border_left = x + border;
  uint16_t border_right = max_x - border;
  uint16_t border_top = y + border;
  uint16_t border_bottom = max_y - border;

  for (uint16_t iy = y; iy < max_y; iy++) {
    for (uint16_t ix = x; ix < max_x; ix++) {
      if (iy >= CAS_LCD_HEIGHT || ix >= CAS_LCD_WIDTH)
        continue;

      uint16_t pixel_color = color;

      // Check if on border
      if (ix < border_left || ix >= border_right || iy < border_top ||
          iy >= border_bottom) {
        pixel_color = border_color;
      }

      vram[(iy * CAS_LCD_WIDTH) + ix] = pixel_color;
    }
  }
}

void draw_slider(uint16_t x, uint16_t y, uint16_t width, uint16_t track_color,
                 uint16_t handle_color, uint16_t min_value, uint16_t max_value,
                 uint16_t value) {
  const uint16_t track_width = width - SLIDER_HANDLE_WIDTH;

  // Calculate handle position (Fixed point arithmetic)
  const uint16_t handle_offset =
      ((((value - min_value) * 0x10000) / (max_value - min_value)) *
       track_width) /
      0x10000;

  // Draw track
  draw_rectangle(x + (SLIDER_HANDLE_WIDTH / 2),
                 y + (SLIDER_HANDLE_HEIGHT / 2) - (SLIDER_TRACK_HEIGHT / 2) +
                     SLIDER_Y_OFFSET,
                 track_width, SLIDER_TRACK_HEIGHT, track_color, 0, 0);

  // Draw handle
  draw_rectangle(x + handle_offset, y + SLIDER_Y_OFFSET, SLIDER_HANDLE_WIDTH,
                 SLIDER_HANDLE_HEIGHT, handle_color, 0, 0);
}

uint32_t draw_alert_box(const char *title, const char *subtitle, uint16_t width,
                        uint16_t height, uint16_t background, uint16_t border) {
  const uint16_t x_pos = (CAS_LCD_WIDTH - width) / 2;
  const uint16_t y_pos = (CAS_LCD_HEIGHT - height) / 2;

  draw_rectangle(x_pos, y_pos, width, height, background, 1, border);

  // Print title
  if (title) {
    print_string_centered(title, x_pos, y_pos + STD_CONTENT_OFFSET, width, 0,
                          border, COLOR_BLACK, true);
  }

  // Print subtitle
  if (subtitle) {
    print_string_centered(subtitle, x_pos,
                          y_pos + STD_CONTENT_OFFSET + DEBUG_LINE_HEIGHT, width,
                          0, COLOR_DISABLED, COLOR_BLACK, true);
  }

  // Return x and y pos as uint32_t
  return ((uint32_t)y_pos << 16) | ((uint32_t)x_pos & 0xFFFF);
}

void ok_alert(const char *title, const char *subtitle, const char *text,
              uint16_t foreground, uint16_t background, uint16_t border) {
  // Backup LCD and darken background
  uint16_t *lcd_backup = (uint16_t *)malloc(
      CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));

  if (lcd_backup) {
    memcpy(lcd_backup, vram, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    darken_screen_area(0, 0, CAS_LCD_WIDTH, CAS_LCD_HEIGHT);
  }

  const uint16_t text_len = strlen(text);

  // Calculate alert size
  uint16_t width =
      text_len * (DEBUG_CHAR_WIDTH - 2) + (ALERT_CONTENT_OFFSET_X * 2);
  uint16_t height =
      TEXT_ALERT_MIN_HEIGTH + ((subtitle) ? DEBUG_LINE_HEIGHT : 0);

  uint8_t lines = 1;

  if (width < TEXT_ALERT_MIN_WIDTH) {
    width = TEXT_ALERT_MIN_WIDTH;
  }

  char message[MAX_ALERT_MESSAGE_LEN];
  lines =
      string_split_newline(message, sizeof(message), text, TEXT_ALERT_MAX_CHAR);

  // Calculate height when above max characters per line
  if (width > TEXT_ALERT_MAX_WIDTH) {
    width = TEXT_ALERT_MAX_WIDTH;
    height += (lines)*DEBUG_LINE_HEIGHT;
  }

  uint32_t position =
      draw_alert_box(title, subtitle, width, height, background, border);

  uint16_t x = ALERT_GET_X(position);
  uint16_t y = ALERT_GET_Y(position);

  print_n_string(
      message, 0, x + ALERT_CONTENT_OFFSET_X,
      y + ALERT_CONTENT_OFFSET_Y - ((subtitle) ? 0 : DEBUG_LINE_HEIGHT),
      x + width - ALERT_CONTENT_OFFSET_X, 0, foreground, background, true);

  // Print ok button
  print_string_centered(
      " OK ", x + ALERT_CONTENT_OFFSET_X,
      y + ALERT_CONTENT_OFFSET_Y +
          ((lines - ((subtitle) ? 0 : 1)) * DEBUG_LINE_HEIGHT) +
          STD_CONTENT_OFFSET,
      width - (2 * ALERT_CONTENT_OFFSET_X), 0, foreground, COLOR_SELECTED,
      false);

  LCD_Refresh();

  // Wait for ok loop
  uint32_t key1;
  uint32_t key2;

  // handle controls
  wait_input_release();

  do {
    getKey(&key1, &key2);
  } while (!testKey(key1, key2, KEY_EXE));

  // Wait until EXE button is released
  wait_input_release();

  // Close alert
  if (lcd_backup) {
    memcpy(vram, lcd_backup, CAS_LCD_HEIGHT * CAS_LCD_WIDTH * sizeof(uint16_t));
    free(lcd_backup);
  }
}
