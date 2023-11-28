#ifndef __WORK_LOGIC__ 
#define __WORK_LOGIC__

#include <Arduino.h>
#include "pt.h"

#define GET_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

#define BTN_DEBOUNCE    50
#define BTN_TMT         500
#define BTN_CL_TMT      300
#define ANIM_SET_ADDR   0x0F

// --------------------------------------------------------

#define PWR_UPD_TIME_PERIOD  1000
#define PWR_MINUTE_SHOW_TIME 5000


#define HR_HOLD_TIME    1500
#define MIN_HOLD_TIME   1500
#define VCC_HOLD_TIME   1000

#define ANIM_MAX_NUM    100
#define ANIM_DURATION   15
#define ANIM_COUNT      12

#define LED_BLINK_DELAY 400


// --------------------------------------------------------

enum modes {
  INIT,
  TIME, 
  PWR_TIME,
  VCC,
  SET_H,
  SET_M,
  SET_ANIM
};

// --------------------------------------------------------

void PreLoop() ;
void loop_impl();
void wake_up();
void cath_to_state(uint8_t cath, bool state);

// --------------------------------------------------------

PT_THREAD(      onpower_time_machine(struct pt *pt));
PT_THREAD(      time_machine(struct pt *pt));
PT_THREAD(      VCC_machine(struct pt *pt, int8_t supply_voltage));

PT_THREAD(      hr_set_machine(struct pt *pt));
PT_THREAD(      min_set_machine(struct pt *pt));
PT_THREAD(      LED_indication_machine(struct pt *pt, uint8_t led));

PT_THREAD(      anim_machine(struct pt *pt));

#endif
