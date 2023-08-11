#include <Arduino.h>
#include "pt.h"

#define GET_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

// --------------------------------------------------------

void PreLoop() ;
void loop_impl();
void wake_up();

// --------------------------------------------------------

PT_THREAD(LED_indication_machine(struct pt *pt));

