#include <stdio.h>
#include <string.h>
#include "main.h"
#include "robot.h"
#include "robot_config.h"
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
GPIO_TypeDef GA = {0}, GB = {1};
TIM_HandleTypeDef htim2, htim3; UART_HandleTypeDef huart1 = {1}, huart3 = {3};
static uint32_t tick; static uint16_t outA, outB, inA, inB; static uint8_t *rx_ptr;
static char log_txt[8192];
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin){ uint16_t v = p==GPIOA?inA:inB; return (v&pin)?GPIO_PIN_SET:GPIO_PIN_RESET; }
static uint16_t writtenA, writtenB;
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s){ uint16_t *o = p==GPIOA?&outA:&outB; if(p==GPIOA) writtenA|=pin; else writtenB|=pin; if(s)*o|=pin; else *o&=~pin; }
static int pwm_started_nonzero;
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch){ if(h->ccr[ch]!=0 && h==&htim2) pwm_started_nonzero=1; h->pwm_on[ch]=1; return 0; }
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t){ (void)h;(void)t; if(strlen(log_txt)+n<sizeof log_txt-1) strncat(log_txt,(char*)d,n); return 0; }
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n){ (void)h;(void)n; rx_ptr=d; return 0; }
uint32_t HAL_GetTick(void){ return tick; }
void HAL_Delay(uint32_t ms){ tick += ms; }

/* black bits b0..b3 = sensors S0..S3 (left..right). SENSOR_BLACK_LEVEL=1 -> pin high = black */
static void set_black(int bits){
  if(!SENSOR_BLACK_LEVEL) bits = ~bits & 0xF;   /* board where LOW = black */
  inA &= ~(GPIO_PIN_15|GPIO_PIN_12); inB &= ~(GPIO_PIN_3|GPIO_PIN_4);
  if(bits&1) inA|=GPIO_PIN_15;
  if(bits&2) inA|=GPIO_PIN_12;
  if(bits&4) inB|=GPIO_PIN_3;
  if(bits&8) inB|=GPIO_PIN_4;
}
static void run(uint32_t ms){ for(uint32_t i=0;i<ms;i++){ tick++; Robot_Loop(); } }
static int left(void){ int d=htim2.ccr[4]; int fwd=(outA&GPIO_PIN_2)!=0, rev=(outA&GPIO_PIN_1)!=0; int v=fwd?d:(rev?-d:0); return LEFT_TRIM? v*100/LEFT_TRIM : v; }
static int right(void){ int d=htim2.ccr[1]; int fwd=(outA&GPIO_PIN_4)!=0, rev=(outA&GPIO_PIN_5)!=0; int v=fwd?d:(rev?-d:0); return RIGHT_TRIM? v*100/RIGHT_TRIM : v; }
static void press(uint16_t pin, uint32_t ms){ inA|=pin; run(ms); inA&=~pin; run(60); }
static int fails=0;
#define CHECK(c,msg) do{ if(c) printf("  ok   %s\n",msg); else {printf("  FAIL %s\n",msg); fails++;} }while(0)
static int lastline(const char*key){ return strstr(log_txt,key)!=NULL; }

int main(void){
  printf("Line error:\n");
  CHECK(Robot_LineError(0x6,0)==0, "0110 -> 0 (centred)");
  CHECK(Robot_LineError(0x2,0)==-10 && Robot_LineError(0x4,0)==10, "0100/0010 -> -10/+10");
  CHECK(Robot_LineError(0x1,0)==-30 && Robot_LineError(0x8,0)==30, "1000/0001 -> -30/+30");
  CHECK(Robot_LineError(0x3,0)==-20, "1100 -> -20");
  CHECK(Robot_LineError(0x0,17)==17, "no line keeps last error");

  htim2.ccr[1]=777; htim2.ccr[4]=777;   /* pretend garbage in the timer before init */
  Robot_Init(); run(50);
  printf("Safety at power-up:\n");
  CHECK(!pwm_started_nonzero, "motor PWM is set to 0 BEFORE it is switched on");
  CHECK(left()==0 && right()==0, "motors are off after start-up");
  CHECK(!htim3.pwm_on[4], "servo signal stays off until it is needed");
  printf("Start:\n");
  set_black(0x6); press(GPIO_PIN_6, 80); run(20);
  CHECK(lastline("AUTO: GO"), "START button starts AUTO");
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "centred line: both wheels equal");
  CHECK(htim3.pwm_on[4] && htim3.ccr[4]==SERVO_AUTO_US, "servo goes to the AUTO position");
  printf("Steering:\n");
  set_black(0x2); run(10);
  CHECK(left()<right(), "line on the left: left wheel slower (turns left)");
  set_black(0x8); run(10);
  CHECK(left()>right(), "line far right: right wheel slower (turns right)");
  set_black(0x6); run(30);
  printf("Gap and search:\n");
  set_black(0x0); run(100);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "short gap (dashed line): drive straight");
  set_black(0x6); run(20);
  set_black(0x4); run(20);         /* last seen on the right */
  set_black(0x0); run(GAP_MS+200);
  CHECK(left()>0 && right()<0, "line lost on the right: spins right to look for it");
  set_black(0x6); run(20);
  CHECK(left()>0 && right()>0, "found again: following");
  printf("Junction vs colour change:\n");
  set_black(0xF); run(60); set_black(0x6); run(20);
  int j=0; { char*p=strstr(log_txt,"junc="); (void)p; }
  CHECK(!lastline("following the WHITE line"), "short wide patch = junction, not a colour change");
  set_black(0xF); run(WIDE_TO_INVERT_MS+100);
  CHECK(lastline("following the WHITE line"), "long black area = switch to white-line mode");
  set_black(0x9); run(20);        /* black outside, white line under the centre two */
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "white line centred: drives straight");
  set_black(0xB); run(10);        /* white only under S2 -> line slightly right */
  CHECK(left()>right(), "white line to the right: turns right");
  set_black(0x0); run(WIDE_TO_INVERT_MS+100);       /* all white: background back to white */
  CHECK(lastline("Background is WHITE again"), "back on white background: black-line mode");
  (void)j;
  printf("Stop / lost:\n");
  set_black(0x6); run(20);
  press(GPIO_PIN_6, 80);
  CHECK(lastline("AUTO: stopped") && left()==0 && right()==0, "START again stops the robot");
  press(GPIO_PIN_6, 80); set_black(0x0); run(GAP_MS+SEARCH_MS+200);
  CHECK(lastline("LOST") , "line gone for too long: stops and reports LOST");
  printf("Manual mode:\n");
  press(GPIO_PIN_7, 80);
  CHECK(lastline("MODE -> MANUAL"), "MODE button switches to MANUAL when stopped");
  *rx_ptr='F'; HAL_UART_RxCpltCallback(&huart3); run(5);
  CHECK(left()==MANUAL_SPEED_DEFAULT && right()==MANUAL_SPEED_DEFAULT, "'F' drives forward");
  *rx_ptr='L'; HAL_UART_RxCpltCallback(&huart3); run(5);
  CHECK(left()<0 && right()>0, "'L' spins left");
  *rx_ptr='S'; HAL_UART_RxCpltCallback(&huart3); run(5);
  CHECK(left()==0 && right()==0, "'S' stops");
  *rx_ptr='O'; HAL_UART_RxCpltCallback(&huart3); run(150);
  CHECK(htim3.ccr[4]>SERVO_OPEN_US && htim3.ccr[4]<SERVO_CLOSED_US, "gate is moving gradually, not jumping");
  run(450);
  CHECK(htim3.ccr[4]==SERVO_OPEN_US, "'O' opens the gate smoothly");
  printf("Bluetooth safety:\n");
  *rx_ptr='F'; HAL_UART_RxCpltCallback(&huart3); run(100);
  CHECK(left()>0, "driving while commands arrive");
  run(BT_TIMEOUT_MS+50);
  CHECK(!BT_TIMEOUT_MS || (left()==0 && right()==0), "no command for a while (phone gone): car stops itself");
  printf("Pins the code must never drive:\n");
  CHECK(!(writtenA & (GPIO_PIN_15|GPIO_PIN_12)) && !(writtenB & (GPIO_PIN_3|GPIO_PIN_4)), "sensor pins A15 A12 B3 B4 are only READ, never driven");
  CHECK(!(writtenA & (1u<<11)), "A11 (has 5 V on it) is never touched");
  CHECK(!(writtenA & (GPIO_PIN_6|GPIO_PIN_7)), "button pins A6 A7 are only READ");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
