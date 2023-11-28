#ifndef __PINS__ 
#define __PINS__

#define BTN_PIN 2
#define PWM_PIN 9

// --------------------------------------------------------

#define USB_SENS    A7

#define LED_H       A0
#define LED_HR_H()  PORTC |= (1 << 0)
#define LED_HR_L()  PORTC &= ~(1 << 0)

#define LED_M       3
#define LED_MIN_H() PORTD |= (1 << 3)
#define LED_MIN_L() PORTD &= ~(1 << 3)


#define ANODE_0     SCK
#define A0_TO_H()   PORTB |= (1 << 5)
#define A0_TO_L()   PORTB &= ~(1 << 5)

#define ANODE_1     5
#define A1_TO_H()   PORTD |= (1 << 5)
#define A1_TO_L()   PORTD &= ~(1 << 5)

#define CATH_0      4
#define CATH_0_H()  PORTD |= (1 << 4)
#define CATH_0_L()  PORTD &= ~(1 << 4)

#define CATH_1      A1
#define CATH_1_H()  PORTC |= (1 << 1)
#define CATH_1_L()  PORTC &= ~(1 << 1)

#define CATH_2      A2
#define CATH_2_H()  PORTC |= (1 << 2)
#define CATH_2_L()  PORTC &= ~(1 << 2)

#define CATH_3      MISO
#define CATH_3_H()  PORTB |= (1 << 4)
#define CATH_3_L()  PORTB &= ~(1 << 4)

#define CATH_4      MOSI
#define CATH_4_H()  PORTB |= (1 << 3)
#define CATH_4_L()  PORTB &= ~(1 << 3)

#define CATH_5      10
#define CATH_5_H()  PORTB |= (1 << 2)
#define CATH_5_L()  PORTB &= ~(1 << 2)

#define CATH_6      8
#define CATH_6_H()  PORTB |= (1 << 0)
#define CATH_6_L()  PORTB &= ~(1 << 0)

#define CATH_7      7
#define CATH_7_H()  PORTD |= (1 << 7)
#define CATH_7_L()  PORTD &= ~(1 << 7)

#define CATH_8      6
#define CATH_8_H()  PORTD |= (1 << 6)
#define CATH_8_L()  PORTD &= ~(1 << 6)

#define CATH_9      A3
#define CATH_9_H()  PORTC |= (1 << 3)
#define CATH_9_L()  PORTC &= ~(1 << 3)

#define ALL_CATH_L() { \
                    CATH_0_L(); \
                    CATH_1_L(); \
                    CATH_2_L(); \
                    CATH_3_L(); \
                    CATH_4_L(); \
                    CATH_5_L(); \
                    CATH_6_L(); \
                    CATH_7_L(); \
                    CATH_8_L(); \
                    CATH_9_L(); \
                    }

#define ALL_CATH_H() { \
                    CATH_0_H(); \
                    CATH_1_H(); \
                    CATH_2_H(); \
                    CATH_3_H(); \
                    CATH_4_H(); \
                    CATH_5_H(); \
                    CATH_6_H(); \
                    CATH_7_H(); \
                    CATH_8_H(); \
                    CATH_9_H(); \
                    }

const char cathodes[10] = {CATH_0, CATH_1, CATH_2,
                           CATH_3, CATH_4, CATH_5,
                           CATH_6, CATH_7, CATH_8, 
                           CATH_9 };

#endif
