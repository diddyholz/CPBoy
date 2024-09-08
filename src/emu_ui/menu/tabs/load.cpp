#include "load.h"

#include "../../../core/error.h"
#include "../../../helpers/fileio.h"
#include "../../../helpers/functions.h"
#include "../../../helpers/macros.h"
#include "../../colors.h"
#include "../../components.h"
#include "../../effects.h"
#include "../../font.h"
#include "../../input.h"
#include "../menu.h"
#include <sdk/os/input.hpp>
#include <stdlib.h>
#include <string.h>

namespace hhk {
#include <sdk/os/mem.hpp>
}

#define TAB_LOAD_TITLE "Load"
#define TAB_LOAD_ITEM_COUNT 20

int32_t action_set_romfile(menu_item *item, gb_s *gb) {
  emu_preferences *prefs = (emu_preferences *)gb->direct.priv;

  strlcpy(prefs->current_filename, item->title,
          sizeof(prefs->current_filename));

  return MENU_LOAD_NEW;
}

menu_tab *prepare_tab_load(menu_tab *tab) {
  strlcpy(tab->title, TAB_LOAD_TITLE, sizeof(tab->title));

  char files[TAB_LOAD_ITEM_COUNT][MAX_FILENAME_LEN];

  tab->item_count =
      find_files(DIRECTORY_ROM "\\*" EXTENSION_ROM, files, TAB_LOAD_ITEM_COUNT);
  tab->items =
      (menu_item *)malloc(tab->item_count * sizeof(menu_item));

  if (!(tab->items)) {
    set_error(EMALLOC);
    return nullptr;
  }

  for (uint8_t i = 0; i < tab->item_count; i++) {
    tab->items[i].disabled = false;
    strlcpy(tab->items[i].title, files[i], sizeof(tab->items[i].title));
    tab->items[i].value[0] = '\0';
    tab->items[i].action = action_set_romfile;
  }

  char tmp[5];

  strlcpy(tab->description, "Detected ", sizeof(tab->description));
  strlcat(tab->description, itoa(tab->item_count, tmp, 10),
          sizeof(tab->description));
  strlcat(tab->description, " ROMs in " DIRECTORY_ROM "\\",
          sizeof(tab->description));

  return tab;
}
