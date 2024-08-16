#pragma once

#include <stdint.h>
#include <sdk/calc/calc.hpp>
#include "../core/peanut_gb.h"

#define DEFAULT_GB_KEY_A_0      KEY_EXE
#define DEFAULT_GB_KEY_A_1      0
#define DEFAULT_GB_KEY_B_0      KEY_ADD
#define DEFAULT_GB_KEY_B_1      0
#define DEFAULT_GB_KEY_START_0  KEY_CLEAR
#define DEFAULT_GB_KEY_START_1  0
#define DEFAULT_GB_KEY_SELECT_0 KEY_SHIFT
#define DEFAULT_GB_KEY_SELECT_1 0
#define DEFAULT_GB_KEY_UP_0     0
#define DEFAULT_GB_KEY_UP_1     KEY_UP
#define DEFAULT_GB_KEY_DOWN_0   0
#define DEFAULT_GB_KEY_DOWN_1   KEY_DOWN
#define DEFAULT_GB_KEY_LEFT_0   KEY_LEFT
#define DEFAULT_GB_KEY_LEFT_1   0
#define DEFAULT_GB_KEY_RIGHT_0  KEY_RIGHT
#define DEFAULT_GB_KEY_RIGHT_1  0

#define CAS_KEY_TEXT_SHIFT          "[SHIFT]"
#define CAS_KEY_TEXT_CLEAR          "[CLEAR]"
#define CAS_KEY_TEXT_BACKSPACE      "[<--]"
#define CAS_KEY_TEXT_LEFT           "[LEFT]"
#define CAS_KEY_TEXT_RIGHT          "[RIGHT]"
#define CAS_KEY_TEXT_Z              "[Z]"
#define CAS_KEY_TEXT_POWER          "[^]"
#define CAS_KEY_TEXT_DIVIDE         "[/]"
#define CAS_KEY_TEXT_MULTIPLY       "[*]"
#define CAS_KEY_TEXT_SUBSTRACT      "[-]"
#define CAS_KEY_TEXT_ADD            "[+]"
#define CAS_KEY_TEXT_EXE            "[EXE]"
#define CAS_KEY_TEXT_EXP            "[EXP]"
#define CAS_KEY_TEXT_3              "[3]"
#define CAS_KEY_TEXT_6              "[6]"
#define CAS_KEY_TEXT_9              "[9]"
#define CAS_KEY_TEXT_KEYBOARD       "[KEYBOARD]"
#define CAS_KEY_TEXT_UP             "[UP]"
#define CAS_KEY_TEXT_DOWN           "[DOWN]"
#define CAS_KEY_TEXT_EQUALS         "[=]"
#define CAS_KEY_TEXT_X              "[X]"
#define CAS_KEY_TEXT_Y              "[Y]"
#define CAS_KEY_TEXT_LEFT_BRACKET   "[(]"
#define CAS_KEY_TEXT_RIGHT_BRACKET  "[)]"
#define CAS_KEY_TEXT_COMMA          "[,]"
#define CAS_KEY_TEXT_NEGATIVE       "[-]"
#define CAS_KEY_TEXT_0              "[0]"
#define CAS_KEY_TEXT_DOT            "[.]"
#define CAS_KEY_TEXT_1              "[1]"
#define CAS_KEY_TEXT_2              "[2]"
#define CAS_KEY_TEXT_4              "[4]"
#define CAS_KEY_TEXT_5              "[5]"
#define CAS_KEY_TEXT_7              "[7]"
#define CAS_KEY_TEXT_8              "[8]"
#define CAS_KEY_TEXT_NONE           "NONE"

#define GB_KEY_TEXT_A      "A"
#define GB_KEY_TEXT_B      "B"
#define GB_KEY_TEXT_START  "START"
#define GB_KEY_TEXT_SELECT "SELECT"
#define GB_KEY_TEXT_UP     "UP"
#define GB_KEY_TEXT_DOWN   "DOWN"
#define GB_KEY_TEXT_LEFT   "LEFT"
#define GB_KEY_TEXT_RIGHT  "RIGHT"

#define GB_KEY_COUNT  8

#define GB_KEY_A      0
#define GB_KEY_B      1
#define GB_KEY_START  2
#define GB_KEY_SELECT 3
#define GB_KEY_UP     4
#define GB_KEY_DOWN   5
#define GB_KEY_LEFT   6
#define GB_KEY_RIGHT  7

typedef uint32_t emu_controls[GB_KEY_COUNT][2];

uint8_t load_controls(struct gb_s *gb);
uint8_t save_controls(struct gb_s *gb);
