/**
 * MIT License
 *
 * Copyright (c) 2023 Sidney Krombholz 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include "cas/bootstrap.h"
#include "core/emulator.h"
#include "core/error.h"

APP_NAME("CPBoy")
APP_DESCRIPTION("A Gameboy (DMG) emulator. Forked from PeanutGB by deltabeard.")
APP_AUTHOR("diddyholz")
APP_VERSION(CPBOY_VERSION)

gb_s main_gb __attribute__((section(".oc_mem.y.data")));
emu_preferences main_preferences __attribute__((section(".oc_mem.y.data")));
 
int main() 
{
  calcInit();

  if (setup_cas())
  {
    error_crash_alert(get_error_string(errno));
    goto end;
  }

  if (run_emulator(&main_gb, &main_preferences) != 0) 
  {
    // Error handling
    error_crash_alert(get_error_string(errno));
  }

end:
  restore_cas(); 
  calcEnd();
  return 0;
}
