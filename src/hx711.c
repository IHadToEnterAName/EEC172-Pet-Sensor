#include "hx711.h"

long HX711_Tare(int times)
{
	long weight;
    unsigned int uiGPIOPort;
    unsigned char pucGPIOPin;

    GPIO_IF_GetPortNPin(CLK_GPIO_00,&uiGPIOPort,&pucGPIOPin);
    GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 1);

    GPIO_IF_GetPortNPin(CLK_GPIO_00,&uiGPIOPort,&pucGPIOPin);
    GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 0);

    weight = averageValue(times);
    setOffset(weight);
	setScale(853.3f);
	return weight;
}

long averageValue(int times)
{
	long data = 0;
    long average = 0;
    int i;

	for (i = 0; i < times; i++)
	{
		data = getValue();
		average += data;
	}

	data = average / times;
	return data;
}

long getValue(void)
{
	long data = 0;
    unsigned int uiGPIOPort;
    unsigned char pucGPIOPin;
	int i;

	// Wait till HX711 is ready: Data is low
	GPIO_IF_GetPortNPin(DT_GPIO_07,&uiGPIOPort,&pucGPIOPin);
	while(GPIO_IF_Get(DT_GPIO_07,uiGPIOPort,pucGPIOPin));

	// Read Channel A Gain 128 (24 bits)
	for (i = 0; i<24; i++)
	{

		// Set clock
		GPIO_IF_GetPortNPin(CLK_GPIO_00,&uiGPIOPort,&pucGPIOPin);
		GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 1);

		// Read Bit
		GPIO_IF_GetPortNPin(DT_GPIO_07,&uiGPIOPort,&pucGPIOPin);
		if(GPIO_IF_Get(DT_GPIO_07,uiGPIOPort,pucGPIOPin))
		{
			data = (data | 1);
		}
			data = (data << 1);

		// Clear Clock
		GPIO_IF_GetPortNPin(CLK_GPIO_00,&uiGPIOPort,&pucGPIOPin);
		GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 0);
	}

	// Set next conversionto Channel A Gain 128
    GPIO_IF_GetPortNPin(CLK_GPIO_00,&uiGPIOPort,&pucGPIOPin);
    GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 1);
    GPIO_IF_Set(CLK_GPIO_00, uiGPIOPort, pucGPIOPin, 0);

    return data;
}

void setOffset(long offset)
{
	_offset = offset;
    return;
}

void setScale(float scale)
{
	_scale = scale;
    return;
}

long getGram(int times)
{
	long data;

	data = labs( (averageValue(times) - _offset) / _scale);
	return data;
}
