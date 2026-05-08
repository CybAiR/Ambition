// hx711.h
#ifndef HX711_H
#define HX711_H

#include "main.h"
#include<string.h>
#include "stdbool.h"
#include "stdint.h"

#define CHANNEL_A 0
#define CHANNEL_B 1
#define interrupts() __enable_irq()
#define noInterrupts() __disable_irq()

extern GPIO_TypeDef *hx711_dout_port;
extern uint16_t hx711_dout_pin;
extern GPIO_TypeDef *hx711_sck_port;
extern uint16_t hx711_sck_pin;

extern long gAoffset;
extern long gBoffset;

extern float gAscale;
extern float gBscale;
extern uint8_t gAgain;
extern uint8_t gBgain;

void hx711_Init(GPIO_TypeDef *data_port, uint16_t data_pin, GPIO_TypeDef *clk_port, uint16_t clk_pin);
void setScale(float AScale, float BScale);
void setGain(uint8_t AGain, uint8_t BGain);

uint8_t  hx711_IsReady(void);

void tare(uint8_t times, uint8_t channel);
void tareAll(uint8_t times);
float getWeight(int8_t times, uint8_t channel);

void setOffset(long offset, uint8_t channel);
uint8_t shiftIn(uint8_t bit_count);
void waitReady();
long read(uint8_t channel);
long readAverage(int8_t times, uint8_t channel);
double getValue(int8_t times, uint8_t channel);



#endif

