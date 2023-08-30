#include <sleep_logic.h>
#include <work_logic.h>

#include "pins.h"

bool            sleep_flag;
extern modes    mode;
extern bool     indication;
// --------------------------------------------------------

void prepare_to_sleep() {
    digitalWrite(PWM_PIN, 0);   // Turn everything off.
    A0_TO_L();
    A1_TO_L();
    LED_HR_L();
    LED_MIN_L();

    ALL_CATH_L();

    sleep_flag = true;
    mode = INIT;
    indication = false;
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void wake_up() {
    // digitalWrite(LED_H, HIGH);
}
