/* Fake HAL so robot.c can be compiled and tested on a PC */
#ifndef FAKE_MAIN_H
#define FAKE_MAIN_H
#include <stdint.h>
typedef struct { int id; } GPIO_TypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;
typedef struct { int id; uint32_t ccr[5]; int pwm_on[5]; } TIM_HandleTypeDef;
typedef struct { int id; } UART_HandleTypeDef;
extern GPIO_TypeDef GA, GB;
#define GPIOA (&GA)
#define GPIOB (&GB)
#define GPIO_PIN_1 (1u<<1)
#define GPIO_PIN_2 (1u<<2)
#define GPIO_PIN_3 (1u<<3)
#define GPIO_PIN_4 (1u<<4)
#define GPIO_PIN_5 (1u<<5)
#define GPIO_PIN_6 (1u<<6)
#define GPIO_PIN_7 (1u<<7)
#define GPIO_PIN_12 (1u<<12)
#define GPIO_PIN_13 (1u<<13)
#define GPIO_PIN_14 (1u<<14)
#define GPIO_PIN_15 (1u<<15)
#define TIM_CHANNEL_1 1
#define TIM_CHANNEL_4 4
#define __HAL_TIM_SET_COMPARE(h, ch, v) ((h)->ccr[(ch)] = (uint32_t)(v))
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s);
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch);
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t);
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n);
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h);
#define HAL_OK 0
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t ms);
#endif
