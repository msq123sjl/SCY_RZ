#ifndef GPIO_DRIVERS_H
#define GPIO_DRIVERS_H
#include "api/em335x_drivers.h"

#define    SWITCH_OUT1   GPIO6
#define    SWITCH_OUT2   GPIO7
#define    SWITCH_OUT3   GPIO8
#define    SWITCH_OUT4   GPIO9
#define    SWITCH_OUT5   GPIO16
#define    SWITCH_OUT6   GPIO17

#define    SWITCH_IN1      GPIO10
#define    SWITCH_IN2      GPIO11
#define    SWITCH_IN3      GPIO12
#define    SWITCH_IN4      GPIO13
#define    SWITCH_IN5      GPIO14
#define    SWITCH_IN6      GPIO15

#define    SPI_CS               GPIO31


int  GPIO_OutEnable(int fd, unsigned int dwEnBits);
int  GPIO_OutDisable(int fd, unsigned int dwDisBits);
int  GPIO_OutSet(int fd, unsigned int dwSetBits);
int  GPIO_OutClear(int fd, unsigned int dwClearBits);
int  GPIO_PinState(int fd, unsigned int* pPinState);
int  InitGPIO();
void SwitchOut_On(int ch);
void SwitchOut_Off(int ch);
bool GetSwitchStatus(int ch);
bool WDT_Feed();
extern int Power_Change;
extern int Power_Old;
extern int Power_New;


#endif // GPIO_DRIVERS_H
