/* MOTOR_MIN: small speeds lifted above the stall point */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "robot.h"
#include "robot_config.h"
#ifndef START_RAMP_MS
#define START_RAMP_MS 400   /* same default as robot.c */
#endif
GPIO_TypeDef GA, GB;
static int log_inv, log_toolong; static int inverted_for_test(void){ return log_inv; }
TIM_HandleTypeDef htim2, htim3; UART_HandleTypeDef huart1, huart3;
static uint32_t tick; static uint16_t outA, outB, inA, inB; static int fails;
#define CHECK(c,msg) do{ if(c) printf("  ok   %s\n",msg); else {printf("  FAIL %s\n",msg); fails++;} }while(0)
uint32_t HAL_GetTick(void){ return tick; }
void HAL_Delay(uint32_t ms){ tick += ms; }
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s){ uint16_t *o = (p==GPIOA)?&outA:&outB; if(s) *o|=pin; else *o&=~pin; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin){ uint16_t v=(p==GPIOA)?inA:inB; return (v&pin)?GPIO_PIN_SET:GPIO_PIN_RESET; }
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch){ (void)h;(void)ch; return 0; }
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t){ (void)h;(void)t; if(n>10 && strstr((const char*)d,"WHITE line")) log_inv=1; if(n>10 && strstr((const char*)d,"black too long")) log_toolong=1; return 0; }
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n){ (void)h;(void)d;(void)n; return 0; }
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n){ (void)d;(void)n; HAL_UART_TxCpltCallback(h); return 0; }
static void run(int ms){ for(int i=0;i<ms;i++){ tick++; Robot_Loop(); } }
static void press(uint16_t pin){ inA|=pin; run(80); inA&=~pin; run(60); }
static int lift(int v){ return v==0 ? 0 : MOTOR_MIN + v*(PWM_MAX-MOTOR_MIN)/PWM_MAX; }
int main(void){
  Robot_Init(); run(50);
  printf("MOTOR_MIN = %d:\n", MOTOR_MIN);
  CHECK(htim2.ccr[1]==0 && htim2.ccr[4]==0, "stopped: both motors get exactly 0 (no creeping)");
  press(GPIO_PIN_7); press(GPIO_PIN_7); press(GPIO_PIN_7);   /* AUTO -> MANUAL -> SENSOR -> MOTOR TEST */
  press(GPIO_PIN_6); run(300);                                 /* step 1: drive(500, 0) */
  CHECK((int)htim2.ccr[4]==lift(500*LEFT_TRIM/100) && htim2.ccr[1]==0, "left wheel lifted above the stall point, right stays 0");
  run(1000);                                                   /* step 2: drive(0, 500) */
  CHECK((int)htim2.ccr[1]==lift(500*RIGHT_TRIM/100) && htim2.ccr[4]==0, "right wheel lifted, left stays 0");
  CHECK(lift(90) >= MOTOR_MIN && lift(999)==PWM_MAX, "small speed 90 -> at least MOTOR_MIN, full speed stays 999");
  printf("  (asked 90 -> gets %d, asked 170 -> gets %d, asked 410 -> gets %d)\n", lift(90), lift(170), lift(410));
  printf("START_RAMP_MS = %d:\n", START_RAMP_MS);
  run(4000); press(GPIO_PIN_7);                                /* MOTOR TEST -> AUTO */
  inA|=GPIO_PIN_12; inB|=GPIO_PIN_3|GPIO_PIN_4; inA&=~GPIO_PIN_15;  /* only centre-left sees black */
  inA|=GPIO_PIN_6; run(80); inA&=~GPIO_PIN_6; int t0=tick; run(4);
  int early=(int)htim2.ccr[1]; run(START_RAMP_MS); int later=(int)htim2.ccr[1];
  CHECK(early < later && early < MOTOR_MIN + 120, "soft start: wheels start slow and speed up (no jerk)");
  (void)t0; inA|=GPIO_PIN_6; run(80); inA&=~GPIO_PIN_6; run(60);   /* stop */
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);             /* round to MOTOR TEST again */
  printf("Pivot turn (TURN_FWD %d / TURN_BACK %d):\n", TURN_FWD, TURN_BACK);
  run(4000); press(GPIO_PIN_7);                                /* MOTOR TEST -> AUTO */
  inA|=GPIO_PIN_12|GPIO_PIN_15; inB&=~(GPIO_PIN_3|GPIO_PIN_4);      /* 0011: two right sensors black */
  inA|=GPIO_PIN_6; run(80); inA&=~GPIO_PIN_6; run(START_RAMP_MS+300);
  if (TURN_FWD) {
  CHECK((int)htim2.ccr[4] == lift(TURN_FWD*LEFT_TRIM/100) && (outA & GPIO_PIN_2) &&
        (int)htim2.ccr[1] == lift(TURN_BACK*RIGHT_TRIM/100) && (outA & GPIO_PIN_5),
        "0011: left wheel FORWARD (TURN_FWD), right wheel BACKWARDS (TURN_BACK)");
  } else printf("  skipped 0011: pivot is off (TURN_FWD 0)\n");
  inA&=~(GPIO_PIN_12|GPIO_PIN_15); inB|=GPIO_PIN_3|GPIO_PIN_4; run(100);   /* 1100: two left sensors black */
  if (TURN_FWD) {
  CHECK((int)htim2.ccr[1] == lift(TURN_FWD*RIGHT_TRIM/100) && (outA & GPIO_PIN_4) && (outA & GPIO_PIN_1),
        "1100: right wheel FORWARD, left wheel BACKWARDS");
  } else printf("  skipped 1100: pivot is off (TURN_FWD 0)\n");
  inA|=GPIO_PIN_6; run(80); inA&=~GPIO_PIN_6; run(60);              /* stop */
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);
  inA|=GPIO_PIN_7; run(80); inA&=~GPIO_PIN_7; run(60);             /* round to MOTOR TEST again */
  printf("WIDE_TO_INVERT_MS = %d:\n", WIDE_TO_INVERT_MS);
  run(4000);                                                   /* let the motor test finish */
  press(GPIO_PIN_7);                                           /* MOTOR TEST -> AUTO */
  inA&=~(GPIO_PIN_12|GPIO_PIN_15); inB&=~(GPIO_PIN_3|GPIO_PIN_4);   /* LOW = black on this board: all 4 black */
  press(GPIO_PIN_6); run(2000);
  CHECK(WIDE_TO_INVERT_MS || !inverted_for_test(), "0 = off: a long black area does NOT switch to white-line mode");
  CHECK(log_toolong && htim2.ccr[1]==0 && htim2.ccr[4]==0, "all black for 2 s: robot stops, LOST (WIDE_MAX_MS safety)");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
