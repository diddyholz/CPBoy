#include "frametimes.h"

#include "emulator.h"
#include "peanut_gb.h"
#include "preferences.h"
#include "../cas/cpu/cmt.h"
#include "../cas/cpu/cpg.h"

#define FRAME_TARGET 60

void frametime_counter_set(struct gb_s *gb)
{
  emu_preferences *pref = (emu_preferences *)gb->direct.priv;
  uint8_t frameskip = (pref->config.frameskip_enabled)?
    pref->config.frameskip_amount + 1 : 1;

  uint32_t ticks = (CMT_TICKS_PER_SEC / FRAME_TARGET) * frameskip;
  
  const uint16_t speed_perc = (pref->config.emulation_speed == 0)? 100 : pref->config.emulation_speed;
  const uint32_t default_pll = CPG_PLL_MUL_DEFAULT + 1;
  const uint32_t current_pll = CPG_FRQCRA->STC + 1;
  
  // Modify ticks per frame based on currently selected PLL multiplier
  ticks = (ticks * current_pll) / default_pll;

  cmt_set((ticks * 100) / speed_perc, MODE_ONE_SHOT, REQUEST_DISABLE);
}

void frametime_counter_start()
{
  cmt_start();
}

void frametime_counter_wait(struct gb_s *gb)
{
  emu_preferences *pref = (emu_preferences *)gb->direct.priv;

  if (!gb->direct.frame_drawn)
  {
    return;
  }
  
  if (pref->config.emulation_speed == (EMU_SPEED_MAX + EMU_SPEED_STEP))
  {
    return;
  }

  cmt_wait();
}
