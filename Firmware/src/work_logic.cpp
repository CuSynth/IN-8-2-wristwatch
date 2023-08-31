#include "work_logic.h"

#include <RTClib.h>
#include <Wire.h>
// #include <avr/power.h>
#include <GyverButton.h>
#include "pt.h"

#include "pins.h"
#include "sleep_logic.h"

// --------------------------------------------------------
//todo:
// ---

// --------------------------------------------------------

GButton     btn(BTN_PIN);
RTC_DS3231  rtc;

extern bool sleep_flag;

modes       mode;

uint32_t    led_sequencer = 0xFFAA00AAul;
uint8_t     current_anode = 0;
uint8_t     to_show       = 0;
uint8_t     hrs, mins     = 0;
long supply_voltage       = 0;

uint16_t    init_delay = 50;
uint32_t    init_timer = 0;
bool        indication = false;

bool        animation = true;

struct pt   led_pt;
struct pt   time_pt;
struct pt   vcc_pt;


// --------------------------------------------------------

void PreLoop() {

    // Prepare threads
    PT_INIT(&led_pt);
    PT_INIT(&time_pt);
    PT_INIT(&vcc_pt);

    // Prepare RTC
    Wire.begin();
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Prepare button
    btn.setDebounce(50);        
    btn.setTimeout(300);       
    btn.setClickTimeout(300);  
    btn.setType(LOW_PULL);
    btn.setDirection(NORM_OPEN);
    btn.setTickMode(MANUAL);
    attachInterrupt(0, wake_up, RISING);

    delay(100);

    // Prepare pins
    pinMode(LED_M, OUTPUT);
    pinMode(LED_H, OUTPUT);
    pinMode(ANODE_0, OUTPUT);
    pinMode(ANODE_1, OUTPUT);
    for(uint8_t i = 0; i <GET_COUNT(cathodes); ++i) {
        pinMode(cathodes[i], OUTPUT);
    }
    digitalWrite(PWM_PIN, LOW);

    // Setup 10 bit and 15.6 kHz freq mode for timer 1.
    TCCR1B = (TCCR1B & 0xF8)|0x01;

    // Setup timer 2 for dynamic indication. 
    // So, ISR will be called every 1ms.
    noInterrupts();
    TCCR2B = 0;
    TCCR2A = 0;
    OCR2A = 8;
    TCCR2A |= (1 << WGM21);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
    TIMSK2 |= (1 << OCIE2A);
    interrupts();

    // Zzzz..
    prepare_to_sleep();
}


long readVcc() {
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

    delay(50); 
    ADCSRA |= _BV(ADSC); 
    while (bit_is_set(ADCSRA,ADSC)); 

    uint8_t low  = ADCL; 
    uint8_t high = ADCH; 

    long result = (high<<8) | low;
    result = 1125300L / result; 
    return result; 
}

void loop_impl() {
    if (sleep_flag) {  
        sleep_flag = false;
        int pwm_duty = 100;

        supply_voltage = readVcc();
        supply_voltage = map(supply_voltage, 3300, 4250, 0, 99);
        if(supply_voltage > 90){pwm_duty = 90;}
        if(supply_voltage < 90){pwm_duty = 100;}
        if(supply_voltage < 70){pwm_duty = 110;}
        if(supply_voltage < 50){pwm_duty = 130;}
        if(supply_voltage < 30){pwm_duty = 140;}
        if(supply_voltage < 15){pwm_duty = 160;}

        if(supply_voltage > 02){
            analogWrite(9, pwm_duty);
            delay(100);

            DateTime now = rtc.now();
            mins = now.minute();
            hrs = now.hour();

            PT_INIT(&led_pt);
            PT_INIT(&time_pt);
            PT_INIT(&vcc_pt);

            to_show = 0;
            mode = INIT;
            init_timer = millis();
        }
        else {
            LED_HR_H();
            LED_MIN_H();
            delay(1000);
            prepare_to_sleep();
        }
    }

    btn.tick();

    switch (mode) {
        case INIT:
            if(btn.isHolded()) {
                mode = SET_H;
                indication = true;
            }
            if(!btn.state() && ((millis() - init_timer) > init_delay)) {
                mode = TIME;
                indication = true;
            }

            break;

        case TIME:
            if(btn.isDouble()) {
                mode = VCC;
            }
            
            if(!PT_SCHEDULE(time_machine(&vcc_pt)))
                prepare_to_sleep();
            break;

        case VCC:
            if(!PT_SCHEDULE(VCC_machine(&time_pt, supply_voltage))) {
                prepare_to_sleep();
            }
            break;

        case SET_H:
            to_show = 2;
            if(!PT_SCHEDULE(LED_indication_machine(&led_pt)))
                prepare_to_sleep();

            break;

        case SET_M:
            break;

        case LOW_BAT:
            break;

        default:
            break;
    }

}

// --------------------------------------------------------
// void a(){};
// void b(){};
// void c(){};

// void(*funcs[])() = {a, b, c};
// funcs[1]() == b()

// todo: use void(*cath_to_low[])() and void(*cath_to_high[])() ?
// Both cases use only one func call !
void cath_to_state(uint8_t cath, bool state) {
    switch (cath) {
        case 0:
            if(state)         
                CATH_0_H();
            else
                CATH_0_L();
            break;

        case 1:
            if(state) 
                CATH_1_H();
            else 
                CATH_1_L();
            break;

        case 2:
            if(state)
                CATH_2_H();
            else
                CATH_2_L();
            break;

        case 3:
            if(state)
                CATH_3_H();        
            else    
                CATH_3_L();
            break;

        case 4:
            if(state)
                CATH_4_H();
            else
                CATH_4_L();
            break;

        case 5:
            if(state)
                CATH_5_H();
            else 
                CATH_5_L();
            break;

        case 6:
            if(state)
                CATH_6_H();
            else
                CATH_6_L();
            break;

        case 7:
            if(state)
                CATH_7_H();
            else
                CATH_7_L();
            break;

        case 8:
            if(state)
                CATH_8_H();
            else
                CATH_8_L();
            break;

        case 9:
            if(state)
                CATH_9_H();
            else
                CATH_9_L();
            break;

        default:
        break;
    }
}

// --------------------------------------------------------

PT_THREAD(time_machine(struct pt *pt)) {
    static uint32_t timer;

    PT_BEGIN(pt);

    to_show = hrs;
    LED_HR_H();
    PT_DELAY(pt, timer, 1000);

    to_show = mins;
    LED_HR_L();
    LED_MIN_H();
    PT_DELAY(pt, timer, 1000);

    LED_MIN_L();

    PT_YIELD(pt);
    PT_END(pt);
}


PT_THREAD(LED_indication_machine(struct pt *pt)){
    static uint32_t timer;
    static uint8_t i;

    PT_BEGIN(pt);
    LED_HR_H();

    for(i = 0; i < 32; i ++) {
        digitalWrite(LED_M, (led_sequencer >> i) & 0x01ul);    
        PT_DELAY(pt, timer, 200);
    }
    PT_YIELD(pt);

    LED_HR_L();
    PT_END(pt);
}

PT_THREAD(VCC_machine(struct pt *pt, int8_t supply_voltage)) {
    static uint32_t timer;

    PT_BEGIN(pt);
    LED_HR_L();
    LED_MIN_L();

    to_show = supply_voltage;
    PT_DELAY(pt, timer, 1000);

    PT_YIELD(pt);
    PT_END(pt);
}

// --------------------------------------------------------

ISR(TIMER2_COMPA_vect) {
    static uint8_t  cntr = 0;
    
    if(cntr == 0 && indication) {
        A0_TO_H();
        cath_to_state(to_show/10, 1);
    }

    if(cntr == 1 && indication){
        A0_TO_L();
        ALL_CATH_L();
    }

    if(cntr == 5 && indication) {
        A1_TO_H();
        cath_to_state(to_show%10, 1);
    }

    if(cntr == 6 && indication) {
        A1_TO_L();
        ALL_CATH_L();
    }

    cntr +=  1;
    cntr %= 10;
}
