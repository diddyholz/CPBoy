#include "saves.h"

#include "../../../core/error.h"
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

#define TAB_SAVES_TITLE "Saves"

int32_t dummy_function(menu_item *item, gb_s *gb) { return 0; }

menu_tab *prepare_tab_saves(menu_tab *tab, emu_preferences *preferences) {
  // Description for "Saves" tab
  strcpy(tab->title, TAB_SAVES_TITLE);
  strcpy(tab->description, "Description");

  tab->item_count = 1;
  tab->items = (menu_item *)malloc(1 * sizeof(menu_item));

  if (!tab->items) {
    set_error(EMALLOC);
    return nullptr;
  }

  tab->items[0].disabled = false;
  strcpy(tab->items[0].title, "DUMMY");
  tab->items[0].value[0] = '\0';
  tab->items[0].action = dummy_function;

  return tab;
}
