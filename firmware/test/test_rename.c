/* Test of the one-time Bluetooth rename (BT_NAME) with a fake BLE module.
 * MODULE_TYPE 1 = answers "AT+NAMEname" (HM-10 style), 2 = needs Enter (JDY/BT05),
 * 3 = needs "AT+NAME=name" + Enter (HC-08), 0 = never answers (phone still connected). */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "robot.h"
#include "robot_config.h"
GPIO_TypeDef GA = {0}, GB = {1};
TIM_HandleTypeDef htim2, htim3; UART_HandleTypeDef huart1 = {1}, huart3 = {3};
static uint32_t tick; static uint16_t outA, outB, inA, inB;
static char log_txt[65536], mod_rx[512];
static uint8_t *rx_ptr; static int answered, led_flashes, last_led;
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin){ uint16_t v = p==GPIOA?inA:inB; return (v&pin)?GPIO_PIN_SET:GPIO_PIN_RESET; }
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s){
  uint16_t *o = p==GPIOA?&outA:&outB; if(s)*o|=pin; else *o&=~pin;
  if (p==GPIOB && pin==GPIO_PIN_12) { if (s && !last_led) led_flashes++; last_led = s; } }
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch){ h->pwm_on[ch]=1; return 0; }
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t){ (void)h;(void)t; if(strlen(log_txt)+n<sizeof log_txt-1) strncat(log_txt,(char*)d,n); return 0; }
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n){ (void)h;(void)n; rx_ptr = d; return 0; }
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
static void module_says(const char *s){ for (; *s; s++) { *rx_ptr = (uint8_t)*s; HAL_UART_RxCpltCallback(&huart3); } }
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n){
  if (h == &huart3 && strlen(mod_rx) + n < sizeof mod_rx - 1) strncat(mod_rx, (const char*)d, n);
  HAL_UART_TxCpltCallback(h);
  if (h == &huart3 && !answered) {
    const char *want = MODULE_TYPE==1 ? "AT+NAME" BT_NAME : MODULE_TYPE==2 ? "AT+NAME" BT_NAME "\r\n"
                     : MODULE_TYPE==3 ? "AT+NAME=" BT_NAME "\r\n" : "\x01never";
    if (strstr(mod_rx, want) && (MODULE_TYPE != 1 || strstr(mod_rx, "\r\n") == NULL)) { answered = 1; module_says(MODULE_TYPE==1 ? "OK+Set:" BT_NAME : "OK\r\n"); }
  }
  return 0; }
uint32_t HAL_GetTick(void){ return tick++; }          /* time passes while the robot waits for the answer */
void HAL_Delay(uint32_t ms){ tick += ms; }
static int fails=0;
#define CHECK(c,msg) do{ if(c) printf("  ok   %s\n",msg); else {printf("  FAIL %s\n",msg); fails++;} }while(0)
int main(void){
  printf("Bluetooth rename, fake module type %d:\n", MODULE_TYPE);
  Robot_Init();
  char want[64];
  if (MODULE_TYPE) {
    snprintf(want, sizeof want, "renamed to %s (style %d", BT_NAME, MODULE_TYPE);
    CHECK(strstr(log_txt, want) != NULL, "the module answered OK: rename reported with the style that worked");
    CHECK(led_flashes <= 1, "no failure flashes (only the normal start-up flash)");
  } else {
    CHECK(strstr(log_txt, "rename FAILED") != NULL, "module never answers: FAILED reported on the USB cable");
    CHECK(led_flashes >= 7, "6 fast LED flashes show the failure");
  }
  CHECK(strstr(log_txt, "Autocar ready") != NULL, "robot starts normally afterwards");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
