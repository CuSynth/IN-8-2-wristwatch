#include "work_logic.h"

#include <RTClib.h>
#include <Wire.h>
// #include <avr/power.h>
#include <GyverButton.h>
#include "pt.h"

#include "pins.h"
#include "sleep_logic.h"
   
// --------------------------------------------------------

GButton     btn(BTN_PIN);
RTC_DS3231  rtc;

extern bool sleep_flag;

uint32_t    led_sequencer = 0xFFAA00AAul;
uint8_t     current_anode = 0;
uint8_t     to_show       = 11;
struct pt   led_pt;
struct pt   test_pt;

// --------------------------------------------------------

void PreLoop() {

   // Prepare threads
   PT_INIT(&led_pt);

   // Prepare RTC
   Wire.begin();
   if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }

   // Prepare button
   btn.setDebounce(50);        
   btn.setTimeout(500);       
   btn.setClickTimeout(300);  
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
   noInterrupts();
   TCNT2 = 0;
   OCR2A = 78;
   TCCR2A |= (1 << WGM21);
   TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
   TIMSK2 |= (1 << OCIE2A);
   interrupts();

   // Zzzz..
   sleep_flag = true;
   prepare_to_sleep();
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
         analogWrite(9, pwm_duty);
         delay(100);
      }
   }

   LED_indication_machine(&led_pt);
   test_machine(&test_pt);
}

// --------------------------------------------------------

PT_THREAD(LED_indication_machine(struct pt *pt)){
   static uint32_t timer;
   static uint8_t i;

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


PT_THREAD(test_machine(struct pt *pt)){
   static uint32_t timer;

   PT_BEGIN(pt);

   while(1){


      if(to_show <= 0) {
         prepare_to_sleep();
      }
      --to_show;

      PT_DELAY(pt, timer, 1000);
      // PT_YIELD(pt);
   }
   PT_END(pt);
}

ISR(TIMER2_COMPA_vect) {
   // Todo: refactor

   for(uint8_t i = 0; i < GET_COUNT(cathodes); ++i)  {
      digitalWrite(cathodes[i], LOW);
   }
   
   if(current_anode & 0x1) {
      digitalWrite(ANODE_1, HIGH);
      digitalWrite(ANODE_0, LOW);

      digitalWrite(cathodes[to_show%10], HIGH);
   }
   else {
      digitalWrite(ANODE_1, LOW);
      digitalWrite(ANODE_0, HIGH);

      digitalWrite(cathodes[to_show/10], HIGH);
   }

   current_anode += 1;
   current_anode %= 2;
}
