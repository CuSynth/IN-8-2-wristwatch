#include "work_logic.h"

#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>

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

// uint32_t    led_sequencer   = 0xFFAA00AAul;
uint8_t     current_anode   = 0;
uint8_t     to_show         = 0;
uint8_t     hrs, mins       = 0;
long        supply_voltage  = 0;
uint8_t     anim_set        = 0;

uint16_t    init_delay = 100;
uint32_t    init_timer = 0;
bool        indication = false;
bool        on_power   = false;

struct pt   led_pt;
struct pt   time_pt;
struct pt   pwr_time_pt;
struct pt   vcc_pt;
struct pt   hr_pt;
struct pt   min_pt;


// --------------------------------------------------------

static void get_time() {
    DateTime now = rtc.now();
    mins = now.minute();
    hrs = now.hour();
}

void PreLoop() {

    // Prepare threads
    PT_INIT(&led_pt);
    PT_INIT(&time_pt);
    PT_INIT(&vcc_pt);
    PT_INIT(&hr_pt);
    PT_INIT(&min_pt);
    PT_INIT(&pwr_time_pt);

    // Prepare RTC
    Wire.begin();
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Prepare button
    btn.setDebounce(BTN_DEBOUNCE);        
    btn.setTimeout(BTN_TMT);       
    btn.setClickTimeout(BTN_CL_TMT);  
    btn.setType(LOW_PULL);
    btn.setDirection(NORM_OPEN);
    btn.setTickMode(AUTO);
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

            get_time();

            PT_INIT(&time_pt);
            PT_INIT(&pwr_time_pt);
            PT_INIT(&vcc_pt);

            PT_INIT(&hr_pt);
            PT_INIT(&min_pt);
            PT_INIT(&led_pt);

            to_show = 0;
            mode = INIT;
            anim_set = EEPROM.read(ANIM_SET_ADDR);
            on_power = analogRead(USB_SENS) > 10;
            init_timer = millis();
        }
        else {
            LED_HR_H();
            LED_MIN_H();
            delay(1000);
            prepare_to_sleep();
        }
    }

    // btn.tick();

    switch (mode) {
        case INIT:
            if(btn.isHolded()) {
                if(on_power)
                    mode = PWR_TIME;
                else               
                    mode = SET_H;
                indication = true;
            }
            if(!btn.state() && ((millis() - init_timer) > init_delay)) {
                indication = true;
                mode = TIME;
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
            LED_indication_machine(&led_pt, 0x01);
            
            if(btn.isClick())
                hrs++;
            if(hrs >= 24)
                hrs = 0;
            
            to_show = hrs;
            
            if(btn.isHolded()) {
                LED_HR_L();
                LED_MIN_L();
                mode = SET_M;
                delay(100);
            }

            break;

        case SET_M:
            LED_indication_machine(&led_pt, 0x02);
            
            if(btn.isClick())
                mins++;
            if(mins >= 60)
                mins = 0;

            to_show = mins;

            if(btn.isHolded()) {
                LED_HR_L();
                LED_MIN_L();
                mode = SET_ANIM;
            
                DateTime now = rtc.now();
                rtc.adjust(DateTime(now.year(), now.month(), now.day(), hrs, mins, 0));
            
                delay(100);
            }
            break;

        case SET_ANIM:
            LED_HR_H();
            LED_MIN_H();

            if(btn.isClick())
                anim_set++;
            if(anim_set >= 2)
                anim_set = 0;

            to_show = anim_set;

            if(btn.isHolded()) {
                LED_HR_L();
                LED_MIN_L();
                EEPROM.update(ANIM_SET_ADDR, anim_set);
                prepare_to_sleep();
            }
            break;
        case PWR_TIME:
            if(btn.isHolded()) {
                LED_HR_L();
                LED_MIN_L();
                prepare_to_sleep();
            }

            onpower_time_machine(&vcc_pt);
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
    static struct pt anim_pt;

    PT_BEGIN(pt);
    PT_INIT(&anim_pt);


    if(anim_set == 1)
        PT_WAIT_THREAD(pt, anim_machine(&anim_pt));
    to_show = hrs;
    LED_HR_H();
    PT_DELAY(pt, timer, HR_HOLD_TIME);

    if(anim_set == 1)
        PT_WAIT_THREAD(pt, anim_machine(&anim_pt));
    to_show = mins;
    LED_HR_L();
    LED_MIN_H();
    PT_DELAY(pt, timer, MIN_HOLD_TIME);

    LED_MIN_L();

    PT_YIELD(pt);
    PT_END(pt);
}

PT_THREAD(onpower_time_machine(struct pt *pt)) {
    static uint8_t  submode;
    static uint32_t get_time_timer;
    static uint32_t minute_timer;
    static uint8_t  prev_minute;


    PT_BEGIN(pt);
    get_time_timer = millis();
    submode = 0;

    while (1) {
        if(btn.isClick()) {
            submode ++;
            submode %= 3;
            prev_minute = mins;
            delay(200);
        }

        if((millis() - get_time_timer) > PWR_UPD_TIME_PERIOD) {
            get_time_timer = millis();
            get_time();
        }

        if(submode == 0){
            to_show = hrs;
            LED_HR_H();
            LED_MIN_L();
        }
        else if(submode == 1) {
            to_show = mins;
            LED_HR_L();
            LED_MIN_H();
        }   
        else {
            LED_MIN_L();
            to_show = mins;
            if(prev_minute != mins) {
                prev_minute = mins;
                to_show = hrs;
                LED_HR_H();
                PT_DELAY(pt, minute_timer, PWR_MINUTE_SHOW_TIME);
                LED_HR_L();
            }
        
        }

        PT_YIELD(pt);
    }
    

    PT_END(pt);
}


PT_THREAD(LED_indication_machine(struct pt *pt, uint8_t led)){
    static uint32_t timer;

    PT_BEGIN(pt);

    while (1) {
        if(led & 0x01)
            LED_HR_H();
        else 
            LED_MIN_H();

        PT_DELAY(pt, timer, LED_BLINK_DELAY);
        PT_YIELD(pt);

        if(led & 0x01)
            LED_HR_L();
        else 
            LED_MIN_L();

        PT_DELAY(pt, timer, LED_BLINK_DELAY);
        PT_YIELD(pt);
    }

    PT_END(pt);
}

PT_THREAD(VCC_machine(struct pt *pt, int8_t supply_voltage)) {
    static uint32_t timer;

    PT_BEGIN(pt);
    LED_HR_L();
    LED_MIN_L();

    to_show = supply_voltage;
    PT_DELAY(pt, timer, VCC_HOLD_TIME);

    PT_YIELD(pt);
    PT_END(pt);
}

PT_THREAD(anim_machine(struct pt *pt)) {
    static uint32_t timer;
    static uint8_t  i;
    
    PT_BEGIN(pt);
    
    i=0;
    for(i = 0; i < ANIM_COUNT; ++i) {
        to_show = rand() % ANIM_MAX_NUM;
        
        PT_DELAY(pt, timer, ANIM_DURATION);
        PT_YIELD(pt);
    }

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
