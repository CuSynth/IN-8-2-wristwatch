#include <sleep_logic.h>
#include <work_logic.h>

#include "pins.h"

bool sleep_flag;

// --------------------------------------------------------

void prepare_to_sleep() {
    digitalWrite(PWM_PIN, 0);   // Turn everything off.
    digitalWrite(ANODE_0, 0);
    digitalWrite(ANODE_1, 0);
    digitalWrite(LED_H, 0);
    digitalWrite(LED_M, 0);

    for(uint8_t i = 0; i < GET_COUNT(cathodes); ++i)  {
        digitalWrite(cathodes[i], LOW);
    }
  
    sleep_flag = true;
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void wake_up() {
    digitalWrite(LED_H, HIGH);
}
