#include "stm32f4xx.h"                  // Device header
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "queue.h"                      // ARM.FreeRTOS::RTOS:Core

#define LED_DELAY							1000 
#define LED_ARRAY_LENGTH						8
#define m									3

static TaskHandle_t xTaskBlueLED, xTaskArrayLED;

void delay(const int constr)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(constr);
	xLastWakeTime = xTaskGetTickCount();
	
	vTaskDelayUntil(&xLastWakeTime, xPeriod); 
};

void InitHardware(void)
{
	// Enable clock for GPIO port D
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	
	// PD15 output mode
	GPIOD->MODER |= (1 << 30);	
	
	// Enable clock for GPIO port A
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// PA0-PA7 output mode
	for(uint16_t i = 0; i < LED_ARRAY_LENGTH; i++)
		GPIOA->MODER |= (1 << (i*2));
};

void vTaskArrayLED(void *pvParameters)
{
	// State for LED array
	uint16_t state = 0;
	
	vTaskPrioritySet(NULL, 1);
	
	while(1) {
		for(uint16_t i = 0; i < m; i++) {
			delay(LED_DELAY);
			switch(state) {
				case 0:
					GPIOA->ODR &= 0;
					GPIOA->ODR |= (1 << 3) | (1 << 4);
					state++;
					break;
				case 1:
					GPIOA->ODR &= 0;
					GPIOA->ODR |= (1 << 2) | (1 << 5);
					state++;
					break;
				case 2:
					GPIOA->ODR &= 0;
					GPIOA->ODR |= (1 << 1) | (1 << 6);
					state++;
					break;
				case 3:
					GPIOA->ODR &= 0;
					GPIOA->ODR |= (1 << 0) | (1 << 7);
					state = 0;
					break;
			};
		};
		
		vTaskDelay(5);
	};
};

void vTaskBlueLED(void *pvParameters)
{
	vTaskPrioritySet(NULL, 0);
	
	while(1) {
		uint16_t i = 0;
		for(; i < m*3; i++) {
			delay(LED_DELAY);
			GPIOD->ODR |= (1 << 15);
			delay(LED_DELAY);
			GPIOD->ODR &= ~(0xFFFFFFFF&(1 << 15));
		};
		
		xTaskCreate(vTaskArrayLED, "ArrayLED",
			configMINIMAL_STACK_SIZE, NULL, uxTaskPriorityGet(NULL),
			&xTaskArrayLED);
		
		for(; i < m*7; i++) {
			delay(LED_DELAY);
			GPIOD->ODR |= (1 << 15);
			delay(LED_DELAY);
			GPIOD->ODR &= ~(0xFFFFFFFF&(1 << 15));
		};
		
		vTaskDelete(xTaskArrayLED);
	};
};

int main(void)
{
	InitHardware();
	
	xTaskCreate(vTaskBlueLED, "BlueLED",
		configMINIMAL_STACK_SIZE, NULL, 2, &xTaskBlueLED);
	
	vTaskStartScheduler();
	
	while(1);
};
