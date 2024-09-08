#include "ini.h"

#include "../core/error.h"
#include "../emu_ui/font.h"
#include "../helpers/functions.h"
#include <sdk/os/debug.hpp>
#include <stdint.h>
#include <stdlib.h>

// Use namespace because of conflicting function declarations in mem.hpp and
// string.h
namespace hhk {
#include <sdk/os/mem.hpp>
}

#define TMP_STRING_LEN INI_MAX_VALUE_LEN
#define INI_LINE_TYPE_SECTION 0
#define INI_LINE_TYPE_KEY 1
#define INI_LINE_TYPE_COMMENT 2

void *double_space(void *src, uint32_t *size, size_t elem_size) {
  uint32_t old_size = *size;

  // Allocate double current size
  *size = (*size == 0) ? 1 : *size * 2;

  void *new_space = malloc((*size) * elem_size);

  if (!new_space) {
    char err_info[ERROR_MAX_INFO_LEN];
    char tmp[20];

    strlcpy(err_info, ".ini Parsing: ", sizeof(err_info));
    strlcat(err_info, itoa((*size) * elem_size, tmp, 10), sizeof(err_info));
    strlcat(err_info, "B", sizeof(err_info));

    set_error_i(EMALLOC, err_info);

    return nullptr;
  }

  // Copy old data to new space
  if (old_size != 0) {
    memcpy(new_space, src, old_size * elem_size);
    free(src);
  }

  return new_space;
}

ini_file *ini_parse(const char *ini_string, uint32_t len, ini_file *file) {
  uint8_t char_counter = 0;

  const char *p = ini_string;
  char tmp[TMP_STRING_LEN];

  bool quotes_opened = false;

  uint8_t line_type = INI_LINE_TYPE_KEY;

  ini_section *tmp_section = nullptr;
  ini_key *tmp_key = nullptr;

  // init ini_file
  file->section_count = 0;
  file->sections_size = 0;
  file->sections = nullptr;

  // Go through each character and parse
  for (uint32_t i = 0; *p && i < len; i++, p++) {
    switch (*p) {
    case ';':
    case '#':
      line_type = INI_LINE_TYPE_COMMENT;
      break;

    case '[':
      char_counter = 0;
      break;

    case ']':
      if (line_type == INI_LINE_TYPE_COMMENT) {
        break;
      }

      tmp[char_counter] = '\0';
      char_counter = 0;

      tmp_section = add_section(file, tmp);

      if (!tmp_section) {
        free_ini_file(file);
        return nullptr;
      }

      break;

    case '=':
      if (line_type == INI_LINE_TYPE_COMMENT || !tmp_section) {
        break;
      }

      tmp[char_counter] = '\0';
      char_counter = 0;

      tmp_key = add_key(tmp_section, tmp);

      if (!tmp_key) {
        free_ini_file(file);
        return nullptr;
      }
      break;

    case '"':
      if (!tmp_key) {
        break;
      }

      tmp[char_counter] = '\0';

      if (quotes_opened) {
        tmp_key->value_type = INI_TYPE_STRING;
        tmp_key->value_int = 0;

        strlcpy(tmp_key->value_str, tmp, sizeof(tmp_key->value_str));
      } else {
        quotes_opened = true;
        char_counter = 0;
      }

      char_counter = 0;
      break;

    case '\n':
      // If reached end of line and tmp string is not empty and has key, add the
      // value of the tmp string as integer value to key.
      if (tmp_key && char_counter != 0 && !quotes_opened) {
        tmp[char_counter] = '\0';
        tmp_key->value_type = INI_TYPE_INT;
        tmp_key->value_int = atoi(tmp);
        tmp_key->value_str[0] = '\0';
      }

      tmp_key = nullptr;
      char_counter = 0;
      line_type = INI_LINE_TYPE_KEY;
      quotes_opened = false;
      break;

    case ' ':
      // Only process space when in quotes
      if (!quotes_opened) {
        break;
      }
      // Intentional fallthrough

    default:
      if (char_counter < TMP_STRING_LEN) {
        tmp[char_counter] = *p;
        char_counter++;
      }
      break;
    }
  }

  return file;
}

char *ini_write(ini_file *file, char *ini_string, uint32_t len) {
  uint32_t remaining_len = len;

  ini_string[0] = '\0';

  // Go through each section, write the corresponding header and its keys
  for (uint32_t i = 0; i < file->section_count; i++) {
    ini_section *section = &(file->sections[i]);

    // Write header
    strncat(ini_string, "[", remaining_len);
    ini_string[len - 1] = '\0';

    if (remaining_len < 1) {
      break;
    }

    remaining_len -= 1;

    strncat(ini_string, section->name, remaining_len);
    ini_string[len - 1] = '\0';

    if (remaining_len < strlen(section->name)) {
      break;
    }

    remaining_len -= strlen(section->name);

    strncat(ini_string, "]\n", remaining_len);
    ini_string[len - 1] = '\0';

    if (remaining_len < 2) {
      break;
    }

    remaining_len -= 2;

    // Write keys
    for (uint32_t j = 0; j < section->key_count; j++) {

      ini_key *key = &(section->keys[j]);

      strncat(ini_string, key->name, remaining_len);
      ini_string[len - 1] = '\0';

      if (remaining_len < strlen(key->name)) {
        break;
      }

      remaining_len -= strlen(key->name);

      strncat(ini_string, "=", remaining_len);
      ini_string[len - 1] = '\0';

      if (remaining_len < 1) {
        break;
      }

      remaining_len--;

      // Write actual value
      if (key->value_type == INI_TYPE_INT) {
        char tmp[12];
        itoa(key->value_int, tmp, 10);

        strncat(ini_string, tmp, remaining_len);
        ini_string[len - 1] = '\0';

        if (remaining_len < strlen(tmp)) {
          break;
        }

        remaining_len -= strlen(tmp);
      } else {
        strncat(ini_string, "\"", remaining_len);
        ini_string[len - 1] = '\0';

        if (remaining_len < 1) {
          break;
        }

        remaining_len -= 1;

        strncat(ini_string, key->value_str, remaining_len);
        ini_string[len - 1] = '\0';

        if (remaining_len < strlen(key->value_str)) {
          break;
        }

        remaining_len -= strlen(key->value_str);

        strncat(ini_string, "\"", remaining_len);
        ini_string[len - 1] = '\0';

        if (remaining_len < 1) {
          break;
        }

        remaining_len -= 1;
      }

      strncat(ini_string, "\n", remaining_len);
      ini_string[len - 1] = '\0';

      if (remaining_len < 1) {
        break;
      }

      remaining_len -= 1;
    }
  }

  return ini_string;
}

// Searches a section by name in a section array and returns a pointer if found
ini_section *find_section(const ini_file *file, const char *name) {
  if (!file) {
    return nullptr;
  }

  for (uint32_t i = 0; i < file->section_count; i++) {
    if (strncmp(file->sections[i].name, name, TMP_STRING_LEN) == 0) {
      return &(file->sections[i]);
    }
  }

  return nullptr;
}

// Searches a key by name in a keys array and returns a pointer if found
ini_key *find_key(const ini_section *section, const char *name) {
  if (!section) {
    return nullptr;
  }

  for (uint32_t i = 0; i < section->key_count; i++) {
    if (strncmp(section->keys[i].name, name, TMP_STRING_LEN) == 0) {
      return &(section->keys[i]);
    }
  }

  return nullptr;
}

void free_ini_file(ini_file *file) {
  for (uint32_t i = 0; i < file->section_count; i++) {
    free(file->sections[i].keys);
    file->sections->keys = nullptr;
  }

  free(file->sections);
  file->sections = nullptr;
}

ini_key *add_key(ini_section *section, const char *name, uint8_t value_type,
                 uint32_t value_int, const char *value_str) {
  if (!section) {
    return nullptr;
  }

  ini_key *tmp_key = find_key(section, name);

  // If key could not be found, create a new key in the keys array.
  // We first check if new space needs to be allocated. If so, we will double
  // the available space.
  if (!tmp_key) {
    if (section->key_count == section->keys_size) {
      section->keys = (ini_key *)double_space(
          section->keys, &(section->keys_size), sizeof(ini_key));

      if (!section->keys) {
        return nullptr;
      }
    }

    tmp_key = &(section->keys[section->key_count]);
    section->key_count++;

    // Set values for newly created key
    tmp_key->value_type = value_type;
    tmp_key->value_int = value_int;

    // Use valuestr if it was specified, else fill it with 0s
    if (value_str) {
      strlcpy(tmp_key->value_str, value_str, sizeof(tmp_key->value_str));
    } else {
      memset(tmp_key->value_str, 0, sizeof(tmp_key->value_str));
    }

    // Set name of key
    strlcpy(tmp_key->name, name, sizeof(tmp_key->name));
  }

  return tmp_key;
}

ini_section *add_section(ini_file *file, const char *name) {
  if (!file) {
    return nullptr;
  }

  ini_section *tmp_section = find_section(file, name);

  // If section could not be found, create a new section in the sections array.
  // We first check if new space needs to be allocated. If so, we will double
  // the available space.
  if (!tmp_section) {
    if (file->section_count == file->sections_size) {
      file->sections = (ini_section *)double_space(
          file->sections, &(file->sections_size), sizeof(ini_section));

      if (!file->sections) {
        return nullptr;
      }
    }

    tmp_section = &(file->sections[file->section_count]);
    file->section_count++;

    // Set default values for newly created section
    tmp_section->key_count = 0;
    tmp_section->keys_size = 0;
    tmp_section->keys = nullptr;
    strlcpy(tmp_section->name, name, sizeof(tmp_section->name));
  }

  return tmp_section;
}
