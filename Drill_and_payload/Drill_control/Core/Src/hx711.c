// hx711.c
#include "hx711.h"

GPIO_TypeDef *hx711_dout_port;
uint16_t hx711_dout_pin;
GPIO_TypeDef *hx711_sck_port;
uint16_t hx711_sck_pin;

long gAoffset;
long gBoffset;

float gAscale;
float gBscale;
uint8_t gAgain;
uint8_t gBgain;

void hx711_Init(GPIO_TypeDef *data_port, uint16_t data_pin, GPIO_TypeDef *clk_port, uint16_t clk_pin)
{
	hx711_dout_port = data_port;
	hx711_dout_pin = data_pin;
	hx711_sck_port = clk_port;
	hx711_sck_pin = clk_pin;

	GPIO_InitTypeDef  gpio = {0};
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pin = clk_pin;
	HAL_GPIO_Init(clk_port, &gpio);
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	gpio.Pin = data_pin;
	HAL_GPIO_Init(data_port, &gpio);

}


uint8_t hx711_IsReady()
{
    return HAL_GPIO_ReadPin(hx711_dout_port, hx711_dout_pin) == GPIO_PIN_RESET;

}

void waitReady()
{
	while (!hx711_IsReady())
	{
		HAL_Delay(0);
	}
}

void setScale(float Ascale, float Bscale)
{
	gAscale = Ascale;
	gBscale = Bscale;
}

void setGain(uint8_t Ag, uint8_t Bg)
{
	if (Ag == 128) gAgain = 1;
	if (Ag == 64) gAgain = 3;

	gBgain = 2;
}

void setOffset(long offset, uint8_t channel)
{
	if (channel == CHANNEL_A) gAoffset = offset;
	else gBoffset = offset;
}

uint8_t shiftIn(uint8_t bitOrder)
{
	uint8_t value = 0;

	for (uint8_t i=0; i<8; i++)
	{
		HAL_GPIO_WritePin(hx711_sck_port, hx711_sck_pin, SET);
		if (bitOrder == 0)
		{
			value |= HAL_GPIO_ReadPin(hx711_dout_port, hx711_dout_pin) << i;
		}
		else
		{
			value |= HAL_GPIO_ReadPin(hx711_dout_port, hx711_dout_pin) << (7-i);
		}
		HAL_GPIO_WritePin(hx711_sck_port, hx711_sck_pin, RESET);
	}
	return value;
}

long read(uint8_t channel)
{
	waitReady();
	unsigned long value = 0;
	uint8_t data[3] = {0};
	uint8_t filler = 0x00;

	noInterrupts();

	data[2] = shiftIn(1);
	data[1] = shiftIn(1);
	data[0] = shiftIn(1);

	uint8_t gain = 0;
	if (channel == 0)
		gain = gAgain;
	else gain = gBgain;

	for (unsigned int i = 0; i<gain; i++)
	{
		HAL_GPIO_WritePin(hx711_sck_port, hx711_sck_pin, SET);
		HAL_GPIO_WritePin(hx711_sck_port, hx711_sck_pin, RESET);
	}

	interrupts();

	if (data[2] & 0x80)
	{
		filler = 0xFF;
	}
	else
	{
		filler = 0x00;
	}

	value = ( (unsigned long) (filler) << 24
			| (unsigned long) (data[2]) << 16
			| (unsigned long) (data[1]) << 8
			| (unsigned long) (data[0]) );

	return (long) (value);
}


long readAverage(int8_t times, uint8_t channel)
{
	long sum = 0;
	for (int8_t i = 0; i < times; i++)
	{
		sum += read(channel);
		HAL_Delay(0);
	}
	return sum / times;
}


double getValue(int8_t times, uint8_t channel)
{
	long offset = 0;
	if (channel == CHANNEL_A)
		offset = gAoffset;
	else
		offset = gBoffset;

	return readAverage(times, channel) - offset;
}

void tare(uint8_t times, uint8_t channel)
{
	read(channel);
	double sum = readAverage(times, channel);
	setOffset(sum, channel);
}

void tareAll(uint8_t times)
{
	tare(times, CHANNEL_A);
	tare(times, CHANNEL_B);
}


float getWeight(int8_t times, uint8_t channel)
{
	read(channel);
	float scale = 0;

	if (channel == CHANNEL_A)
		scale = gAscale;
	else
		scale = gBscale;

	return getValue(times, channel) / scale;
}
















