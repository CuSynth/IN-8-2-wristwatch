#include "work_logic.h"

#include <RTClib.h>
#include <Wire.h>
// #include <avr/power.h>
#include <GyverButton.h>
#include "pt.h"

#include "pins.h"
#include "sleep_logic.h"
   
// --------------------------------------------------------

uint32_t    led_sequencer = 0xFFAA00AAul;
GButton btn(BTN_PIN);
RTC_DS3231 rtc;

extern bool sleep_flag;

struct pt   led_pt;

// --------------------------------------------------------

void PreLoop() {

   PT_INIT(&led_pt);

   Wire.begin();
   if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }

   btn.setDebounce(50);        
   btn.setTimeout(500);       
   btn.setClickTimeout(300);  
   btn.setType(LOW_PULL);
   btn.setDirection(NORM_OPEN);
   btn.setTickMode(AUTO);
   attachInterrupt(0, wake_up, RISING);

   delay(100);

   pinMode(LED_M, OUTPUT);
   pinMode(LED_H, OUTPUT);
   pinMode(ANODE_0, OUTPUT);
   pinMode(ANODE_1, OUTPUT);
   for(uint8_t i = 0; i <GET_COUNT(cathodes); ++i) {
      pinMode(cathodes[i], OUTPUT);
   }
  
   digitalWrite(PWM_PIN, LOW);

   sleep_flag = true;

   TCCR1B = (TCCR1B & 0xF8)|0x01;   // Setup 10 bit and 15.6 kHz freq mode for timer 1.
   prepare_to_sleep();              // Zzzz..
}


long readVcc() {
// todo: refactor

  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

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

      long volts = readVcc();
      volts = map(volts, 3300, 4250, 0, 99);
      if(volts > 90){pwm_duty = 90;}
      if(volts < 90){pwm_duty = 100;}
      if(volts < 70){pwm_duty = 110;}
      if(volts < 50){pwm_duty = 130;}
      if(volts < 30){pwm_duty = 140;}
      if(volts < 15){pwm_duty = 160;}

      if(volts > 02){
         // analogWrite(9, pwm_duty);
         delay(100);
      }

      analogWrite(PWM_PIN, 90);
      digitalWrite(cathodes[0], HIGH);    // Show 00 for test
      digitalWrite(ANODE_0, HIGH);
      digitalWrite(ANODE_1, HIGH);
   }


   LED_indication_machine(&led_pt);
}

// --------------------------------------------------------

PT_THREAD(LED_indication_machine(struct pt *pt)){
   static uint32_t timer;
   static uint8_t i;
   static uint8_t ctr = 0;

   PT_BEGIN(pt);

   while(1){
      for(i = 0; i < 32; i ++){
         digitalWrite(LED_M, (led_sequencer >> i) & 0x01ul);      
         PT_DELAY(pt, timer, 200);

      }
      PT_YIELD(pt);
   }
   PT_END(pt);
}

