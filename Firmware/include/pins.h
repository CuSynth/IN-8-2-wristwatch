#define LED_H  A0
#define LED_M  3

#define BTN_PIN 2
#define PWM_PIN 9

#define ANODE_0  SCK
#define ANODE_1  5

#define CATH_0  4
#define CATH_1  A1
#define CATH_2  A2
#define CATH_3  MISO
#define CATH_4  MOSI
#define CATH_5  10
#define CATH_6  8
#define CATH_7  7
#define CATH_8  6
#define CATH_9  A3

const char cathodes[10] = {CATH_0, CATH_1, CATH_2,
                           CATH_3, CATH_4, CATH_5,
                           CATH_6, CATH_7, CATH_8, 
                           CATH_9 };

