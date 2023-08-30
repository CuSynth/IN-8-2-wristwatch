#include <Arduino.h>
#include "pt.h"

#define GET_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))
enum modes {
  INIT,
  TIME, 
  VCC,
  SET_H,
  SET_M,
  SET_ANIM,
  LOW_BAT
};

// --------------------------------------------------------

void PreLoop() ;
void loop_impl();
void wake_up();
void cath_to_state(uint8_t cath, bool state);

// --------------------------------------------------------

PT_THREAD(      anim_machine(struct pt *pt));
PT_THREAD(      time_machine(struct pt *pt));

PT_THREAD(      LED_indication_machine(struct pt *pt));
PT_THREAD(      indicator_machine(struct pt *pt));
