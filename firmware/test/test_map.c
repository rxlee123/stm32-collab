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
static char bt_txt[262144]; static int bt_n;
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n){ for(int i=0;i<n && bt_n<(int)sizeof bt_txt-1;i++) bt_txt[bt_n++]=d[i]?d[i]:'.'; HAL_UART_TxCpltCallback(h); return 0; }
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

static int has(const char *k){ return strstr(log_txt,k)!=NULL; }
static int fwd_equal(void){ return left()>0 && left()==right(); }
static void press(uint16_t pin, uint32_t ms){ inA|=pin; run(ms); inA&=~pin; run(60); }
#define MS(mm) ((mm)*1000/SPEED_MM_S)
int main(void){
  Robot_Init(); run(50);
  set_black(0x6); press(GPIO_PIN_6, 80); run(200);
  printf("Route { BLIND(100), OPEN, PICKUP(50), LEFT, DROP(50), INVERT, FINISH }:\n");
  /* J1 BLIND: acts the moment the wide patch starts */
  set_black(0xF); run(4);
  CHECK(has("J1 BLIND"), "J1 BLIND starts at once (no waiting for the patch to end)");
  set_black(0x5); run(100); CHECK(fwd_equal(), "blind: straight even on random black bricks");
  set_black(0x0); run(MS(100)); CHECK(has("blind part done"), "blind part ends after its distance");
  set_black(0x0); run(300); CHECK(fwd_equal(), "then drives straight looking for the line");
  set_black(0xF); run(20);  CHECK(has("line found") && !has("J2"), "first line after a blind part is NOT counted");
  set_black(0x6); run(300);
  /* J2 OPEN */
  set_black(0x7); run(30); set_black(0x6); run(5);
  CHECK(has("J2 OPEN") && fwd_equal(), "J2 OPEN: drives straight across");
  run(120); set_black(0xF); run(20); set_black(0x6); run(20);   /* messy edge of the same junction */
  CHECK(!has("J3"), "a 2nd wide patch right after a junction is NOT counted again (hold-off)");
  run(800); CHECK(htim3.ccr[4]==SERVO_OPEN_US, "J2 OPEN: gate is open");
  /* J3 PICKUP */
  set_black(0xF); run(4); CHECK(has("J3 PICKUP") && fwd_equal(), "J3 PICKUP: drives in");
  set_black(0x0); run(MS(50)+10); CHECK(left()==0 && right()==0 && has("closing gate"), "stops and closes the gate");
  run(GATE_MS+10); CHECK(left()>0 && right()<0, "then turns round");
  run(SPIN180_MS); CHECK(fwd_equal(), "then drives straight to find the line");
  set_black(0x6); run(100); CHECK(!has("J4"), "line found after turning round is not a junction");
  run(800); CHECK(htim3.ccr[4]==SERVO_CLOSED_US, "gate stays closed on the cube");
  /* J4 LEFT */
  set_black(0x7); run(30); set_black(0x6); run(4);
  CHECK(has("J4 LEFT") && left()<0 && right()>0, "J4 LEFT: spins left");
  set_black(0x0); run(60); set_black(0x2); run(4); CHECK(has("turn done"), "finds the branch");
  set_black(0x6); run(300);
  run(300);
  /* J5 DROP */
  set_black(0xF); run(4); CHECK(has("J5 DROP") && fwd_equal(), "J5 DROP: drives into the box");
  set_black(0x0); run(MS(50)+10); CHECK(has("opening gate") && left()==0, "stops and opens the gate");
  run(GATE_MS+10); CHECK(left()<0 && right()<0, "backs out");
  run(MS(50)); CHECK(left()>0 && right()<0, "turns round");
  run(SPIN180_MS); CHECK(fwd_equal(), "drives straight to find the line");
  set_black(0x6); run(300);
  /* J6 INVERT */
  set_black(0xF); run(4); CHECK(has("J6 INVERT") && has("now following a WHITE line"), "J6 INVERT: white-line mode at once");
  set_black(0x9); run(100); CHECK(fwd_equal(), "follows the white line in the middle");
  set_black(0xB); run(10); CHECK(left()>right(), "white line to the right: turns right");
  set_black(0x9); run(300);
  /* J7 FINISH (white line, so a wide patch is all-white) */
  set_black(0x0); run(30); set_black(0x9); run(20); CHECK(has("J7 FINISH"), "J7 FINISH armed");
  set_black(0xF); run(MS(FINISH_MM)+30);
  CHECK(has("FINISH: stopped") && left()==0 && right()==0, "brakes FINISH_MM after the line ends");
  printf("Phone messages:\n");
  CHECK(strstr(bt_txt,"J4 LEFT")!=NULL && strstr(bt_txt,"turn done")!=NULL, "junction and turn messages reach the phone");
  CHECK(strstr(bt_txt,"j=")!=NULL, "status line reaches the phone");
  printf("SENSOR TEST counter:\n");
  press(GPIO_PIN_7,80); press(GPIO_PIN_7,80);   /* AUTO -> MANUAL -> SENSOR */
  set_black(0x6); run(200); set_black(0x7); run(30); set_black(0x6); run(200); set_black(0xF); run(30); set_black(0x6); run(200);
  CHECK(has("WIDE #1") && has("WIDE #2") && left()==0, "pushing by hand counts wide patches, motors off");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
