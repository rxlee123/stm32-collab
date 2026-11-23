#include <stdio.h>
#include <string.h>
#include "main.h"
#include "robot.h"
#include "robot_config.h"
GPIO_TypeDef GA = {0}, GB = {1};
TIM_HandleTypeDef htim2, htim3; UART_HandleTypeDef huart1 = {1}, huart3 = {3};
static uint32_t tick; static uint16_t outA, outB, inA, inB;
static char log_txt[262144];
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin){ uint16_t v = p==GPIOA?inA:inB; return (v&pin)?GPIO_PIN_SET:GPIO_PIN_RESET; }
static uint16_t writtenA, writtenB;
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s){ uint16_t *o = p==GPIOA?&outA:&outB; if(p==GPIOA) writtenA|=pin; else writtenB|=pin; if(s)*o|=pin; else *o&=~pin; }
static int pwm_started_nonzero;
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch){ if(h->ccr[ch]!=0 && h==&htim2) pwm_started_nonzero=1; h->pwm_on[ch]=1; return 0; }
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t){ (void)h;(void)t; if(strlen(log_txt)+n<sizeof log_txt-1) strncat(log_txt,(char*)d,n); return 0; }
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n){ (void)h;(void)n;(void)d; return 0; }
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n){ (void)d;(void)n; HAL_UART_TxCpltCallback(h); return 0; }
uint32_t HAL_GetTick(void){ return tick; }
void HAL_Delay(uint32_t ms){ tick += ms; }
static void set_black(int bits){
  if(!SENSOR_BLACK_LEVEL) bits = ~bits & 0xF;   /* board where LOW = black */
  inA &= ~(GPIO_PIN_15|GPIO_PIN_12); inB &= ~(GPIO_PIN_3|GPIO_PIN_4);
  if(bits&1) { if(S0_PORT==GPIOA) inA|=S0_PIN; else inB|=S0_PIN; }
  if(bits&2) { if(S1_PORT==GPIOA) inA|=S1_PIN; else inB|=S1_PIN; }
  if(bits&4) { if(S2_PORT==GPIOA) inA|=S2_PIN; else inB|=S2_PIN; }
  if(bits&8) { if(S3_PORT==GPIOA) inA|=S3_PIN; else inB|=S3_PIN; }
}
static void run(uint32_t ms){ for(uint32_t i=0;i<ms;i++){ tick++; Robot_Loop(); } }
static int left(void){ int d=htim2.ccr[4]; int fwd=(outA&GPIO_PIN_2)!=0, rev=(outA&GPIO_PIN_1)!=0; int v=fwd?d:(rev?-d:0); return LEFT_TRIM? v*100/LEFT_TRIM : v; }
static int right(void){ int d=htim2.ccr[1]; int fwd=(outA&GPIO_PIN_4)!=0, rev=(outA&GPIO_PIN_5)!=0; int v=fwd?d:(rev?-d:0); return RIGHT_TRIM? v*100/RIGHT_TRIM : v; }
static int fails=0;
#define CHECK(c,msg) do{ if(c) printf("  ok   %s\n",msg); else {printf("  FAIL %s\n",msg); fails++;} }while(0)
int main(void){
  Robot_Init(); run(50);
  set_black(0x6); inA|=GPIO_PIN_6; run(80); inA&=~GPIO_PIN_6; run(60);
  printf("Route { LEFT, FINISH }:\n");
  set_black(0x7); run(50); set_black(0x6); run(4);   /* junction with a branch on the left */
  CHECK(left()<0 && right()>0, "junction 1 = LEFT: spins left");
  set_black(0x0); run(100);                         /* centre leaves the old line */
  CHECK(left()<0 && right()>0, "still turning while between lines");
  set_black(0x2); run(4);                           /* centre-left finds the new branch */
  CHECK(strstr(log_txt,"turn done")!=NULL && right()>0, "found the branch: following again (no longer spinning)");
  set_black(0x6); run(50);
  set_black(0xE); run(50); set_black(0x6); run(20);  /* junction 2 = FINISH (armed) */
  CHECK(left()>0 && right()>0, "junction 2 = FINISH: keeps following until the line ends");
  set_black(0x0); run(GAP_MS+30);
  CHECK(strstr(log_txt,"FINISH: stopped")!=NULL, "line ends after FINISH: robot brakes and stops");
  run(100);
  CHECK(left()==0 && right()==0, "stays stopped in the Finish Zone");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
