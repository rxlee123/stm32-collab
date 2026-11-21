#include <stdio.h>
#include <string.h>
#include "main.h"
#include "robot.h"
#include "robot_config.h"
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
GPIO_TypeDef GA = {0}, GB = {1};
TIM_HandleTypeDef htim2, htim3; UART_HandleTypeDef huart1 = {1}, huart3 = {3};
static uint32_t tick; static uint16_t outA, outB, inA, inB; static uint8_t *rx_ptr;
static char log_txt[262144];
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin){ uint16_t v = p==GPIOA?inA:inB; return (v&pin)?GPIO_PIN_SET:GPIO_PIN_RESET; }
static uint16_t writtenA, writtenB;
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s){ uint16_t *o = p==GPIOA?&outA:&outB; if(p==GPIOA) writtenA|=pin; else writtenB|=pin; if(s)*o|=pin; else *o&=~pin; }
static int pwm_started_nonzero;
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch){ if(h->ccr[ch]!=0 && h==&htim2) pwm_started_nonzero=1; h->pwm_on[ch]=1; return 0; }
static uint8_t bt_tx[16384]; static int bt_tx_n;
int HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t){ (void)t;
  if(h==&huart3){ for(int i=0;i<n && bt_tx_n<(int)sizeof bt_tx;i++) bt_tx[bt_tx_n++]=d[i]; return 0; }
  if(strlen(log_txt)+n<sizeof log_txt-1) strncat(log_txt,(char*)d,n);
  return 0; }
int HAL_UART_Transmit_IT(UART_HandleTypeDef *h, const uint8_t *d, uint16_t n){
  for(int i=0;i<n && bt_tx_n<(int)sizeof bt_tx;i++) bt_tx[bt_tx_n++]=d[i];
  HAL_UART_TxCpltCallback(h); return 0; }
static int bt_has(const uint8_t *pat, int n){ for(int i=0;i+n<=bt_tx_n;i++) if(!memcmp(bt_tx+i,pat,n)) return 1; return 0; }
int HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n){ (void)h;(void)n; rx_ptr=d; return 0; }
uint32_t HAL_GetTick(void){ return tick; }
void HAL_Delay(uint32_t ms){ tick += ms; }

/* black bits b0..b3 = sensors S0..S3 (left..right). SENSOR_BLACK_LEVEL=1 -> pin high = black */
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
static void press(uint16_t pin, uint32_t ms){ inA|=pin; run(ms); inA&=~pin; run(60); }
static int fails=0;
#define CHECK(c,msg) do{ if(c) printf("  ok   %s\n",msg); else {printf("  FAIL %s\n",msg); fails++;} }while(0)
static void bt_send(const uint8_t *b, int n){ for(int k=0;k<n;k++){ *rx_ptr=b[k]; HAL_UART_RxCpltCallback(&huart3); } }
static void pad(uint8_t fn, uint8_t buttons, uint8_t v){ uint8_t f[8]={0xFF,0x01,fn,0x01,0x02,buttons,v,0x00}; bt_send(f,8); }
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
  printf("Start box:\n");
  press(GPIO_PIN_6, 80);                       /* stop */
  set_black(0x0); run(20); press(GPIO_PIN_6, 80);   /* START while on all white (inside the start box) */
  run(GAP_MS + 2*SWEEP_MS);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "START on white: drives straight, no spinning");
  set_black(0xF); run(40); set_black(0x6); run(200);   /* crosses the box edge, then finds the track line */
  CHECK(left()>0 && right()>0 && !lastline("no line found"), "reaches the box edge and follows the line");
  CHECK(!strstr(log_txt,"J1 "), "start box edge is not counted as a junction");
  press(GPIO_PIN_6, 80); press(GPIO_PIN_6, 1300);  /* stop, reset route */
  set_black(0x0); run(20); press(GPIO_PIN_6, 80); run(300);   /* START in the box again */
  set_black(0x6); run(30); set_black(0x7); run(40); set_black(0x6); run(300);  /* line first, then the box edge */
  CHECK(!strstr(log_txt,"J1 "), "line seen, then box edge 30 ms later: still not counted");
  set_black(0x7); run(40); set_black(0x6); run(50);
  CHECK(strstr(log_txt,"J1 ")!=NULL, "a junction after following the line steadily IS counted");
  press(GPIO_PIN_6, 80);                       /* stop */
  set_black(0x0); run(20); press(GPIO_PIN_6, 80); run(SEEK_MS + 100);
  CHECK(lastline("no line found while driving straight") && left()==0 && right()==0, "no line within START_STRAIGHT_MS: stops safely");
  set_black(0x6); run(20); press(GPIO_PIN_6, 80); run(20);   /* START again from LOST, on the line: running */
  printf("Steering:\n");
  set_black(0x2); run(10);
  CHECK(left()<right(), "line on the left: left wheel slower (turns left)");
  set_black(0x8); run(10);
  CHECK(left()>right(), "line far right: right wheel slower (turns right)");
  set_black(0x6); run(30);
  set_black(0x6); run(300);
  { int ok=1; for(int i=0;i<6;i++){ set_black(0xC); run(40); if(!(left()>right())) ok=0; set_black(0xE); run(40); if(!(left()>right())) ok=0; }
    CHECK(ok, "your log: line flicks 0011/0111 on a right curve -> keeps turning right, also in WIDE"); }
  set_black(0x6); run(300);
  printf("Gap and search:\n");
  set_black(0x0); run(100);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "short gap (dashed line): drive straight");
  set_black(0x6); run(20);
  set_black(0x4); run(20);         /* small wobble to the right on a dashed straight */
  set_black(0x0); run(GAP_MS-50);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "wobble then dashed gap: still straight (no wrong turn)");
  set_black(0x6); run(20);
  set_black(0x4); run(20);         /* dash ends a bit to the right (centre-right sensor only) */
  set_black(0x0); run(GAP_MS-50);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "dash ends slightly off-centre: still a gap, no spin");
  set_black(0x0); run(4*SWEEP_MS-100);
  set_black(0x6); run(300);
  set_black(0x6); run(20); set_black(0x4); run(20); set_black(0xC); run(30); set_black(0x0); run(30);
  CHECK(left()>0 && right()<0 && lastline("BEND right"), "your log: 0010 -> 0011 -> gone = corner, spins right at once");
  set_black(0x6); run(300);
  set_black(0x4); run(20); set_black(0xE); run(40); set_black(0xC); run(4); set_black(0x0); run(30);
  CHECK(lastline("BEND right") && left()>0 && right()<0, "your new log: corner counted as J, then line gone = spins right");
  set_black(0x6); run(300);
  set_black(0xF); run(40); set_black(0x0); run(30);
  CHECK(left()==BASE_SPEED && right()==BASE_SPEED, "T-junction (both sides at once), then line gone: no guess, straight");
  set_black(0x6); run(300);
  set_black(0x8); run(20);         /* sharp bend: line runs out under the far-right sensor */
  CHECK(left()>0 && right()<=0, "only the far-right sensor sees the line: sharp right (inner wheel stops or reverses)");
  set_black(0x0); run(30);
  CHECK(left()>0 && right()<0, "line left from the right side: spins right AT ONCE, no straight run");
  CHECK(-right() > left(), "corner turn drifts backwards (BEND_BACK): inner wheel back faster than outer forward");
  set_black(0x8); run(10); set_black(0xC); run(10);   /* spinning right: the line comes back in from the right */
  set_black(0x6); run(20);
  CHECK(left()<right(), "found again in the middle while spinning right: short counter-steer brakes the spin (KD)");
  run(D_MS + 10);
  CHECK(left()>0 && right()>0, "found again: following");
  { set_black(0x6); run(300); log_txt[0]=0;
    set_black(0xE); run(40); set_black(0x6); run(20); set_black(0x0); run(30);   /* your log 21 Nov: wide, 0110, gone */
    CHECK(strstr(log_txt,"BEND right")!=NULL && left()>0 && right()<0,
          "corner: 0111, then centred 0110 for a moment, then gone -> BEND right (not GAP)"); }
  { set_black(0x6); run(400); log_txt[0]=0;
    set_black(0xE); run(10); set_black(0x6); run(50);        /* 10 ms flicker of 0111 */
    CHECK(strstr(log_txt,"not counted")!=NULL && strstr(log_txt,"J")==NULL, "3 sensors for only 10 ms: a flicker, NOT counted as a junction");
    set_black(0x6); run(400); log_txt[0]=0;
    set_black(0xE); run(40); set_black(0x6); run(20);        /* 40 ms: a real junction */
    CHECK(strstr(log_txt,"J")!=NULL, "3 sensors for 40 ms: counted as a junction"); }
  { int ok=1; set_black(0x6); run(300);
    for (int i=0;i<8;i++){ set_black(0x4); run(30); if(left()<0||right()<0) ok=0; set_black(0x6); run(30); if(left()<0||right()<0) ok=0; }
    CHECK(ok, "your log 22 Nov: line flickers 0110 <-> 0010 on a sensor edge -> no wheel reverses (no kicks)"); }
  { set_black(0x6); run(300); set_black(0x1); run(30); set_black(0x0); run(100); set_black(0x6); run(6);
    CHECK(left()>=0 && right()>=0, "your log 22 Nov: line found centred after a spin -> no L=950 R=-550 kick"); }
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
  press(GPIO_PIN_6, 80); set_black(0x0); run(GAP_MS+4*SWEEP_MS+SEARCH_MS+200);
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
  printf("Dabble app gamepad:\n");
  CHECK(bt_tx_n>=5 && bt_tx[0]==0xFF && bt_tx[1]==0x00 && bt_tx[2]==0x01, "start-up: says hello to the Dabble app");
  bt_tx_n=0;
  { uint8_t q[]={0xFF,0x00,0x03,0x01,0x02,0x01,0x00,0x00}; bt_send(q,sizeof q); run(5); }
  { static const uint8_t rep[]={0xFF,0x00,0x03,0x01,0x04,0x05,0x01,0x05,0x02,0x00};
    CHECK(bt_has(rep,sizeof rep), "app asks 'which board?': robot answers"); }
  pad(0x01,0x00,0x01); run(5);
  CHECK(left()==MANUAL_SPEED_DEFAULT && right()==MANUAL_SPEED_DEFAULT, "UP arrow: drives forward");
  run(BT_TIMEOUT_MS+300);
  CHECK(DABBLE_TIMEOUT_MS || left()>0, "holding UP with no new message: keeps driving (app sends on change)");
  pad(0x01,0x00,0x04); run(5);
  CHECK(left()<0 && right()>0, "LEFT arrow: spins left");
  pad(0x01,0x00,0x00); run(5);
  CHECK(left()==0 && right()==0, "arrow let go: stops");
  pad(0x02,0x00,(6<<3)|7); run(5);
  CHECK(left()==MANUAL_SPEED_DEFAULT && right()==MANUAL_SPEED_DEFAULT, "joystick pushed fully UP: full forward");
  pad(0x02,0x00,(6<<3)|3); run(5);
  CHECK(left()>0 && left()<MANUAL_SPEED_DEFAULT/2+10 && left()==right(), "joystick pushed a little UP: slow forward");
  pad(0x02,0x00,(0<<3)|7); run(5);
  CHECK(left()>0 && right()<0, "joystick fully RIGHT: spins right");
  pad(0x02,0x00,(3<<3)|7); run(5);
  CHECK(left()>right() && right()>0, "joystick up-right: forward, curving right");
  pad(0x02,0x00,(18<<3)|7); run(5);
  CHECK(left()<0 && right()<0, "joystick fully DOWN: backwards");
  pad(0x02,0x00,0x00); run(5);
  CHECK(left()==0 && right()==0, "joystick released: stops");
  pad(0x02,0x04,0x00); run(700);
  CHECK(htim3.ccr[4]==SERVO_CLOSED_US, "TRIANGLE: closes the gate");
  pad(0x02,0x00,0x00); pad(0x02,0x10,0x00); run(700);
  CHECK(htim3.ccr[4]==SERVO_OPEN_US, "CROSS: opens the gate");
  pad(0x02,0x00,0x00); pad(0x02,0x08,0x00); pad(0x02,0x00,(6<<3)|7); run(5);
  CHECK(left()==MANUAL_SPEED_DEFAULT+MANUAL_SPEED_STEP, "CIRCLE: faster by one step");
  pad(0x02,0x02,(6<<3)|7); run(5);
  CHECK(left()==0 && right()==0, "SELECT held: stops even with the joystick pushed");
  { uint8_t f[]={0xFF,0x01,0x02,0x01,0x02,0x00,'F',0x00}; bt_send(f,sizeof f); run(5); }
  CHECK(!(left()==right() && left()>0), "an 'F' byte INSIDE an app message is not read as a letter");
  pad(0x02,0x00,0x00); run(5);
  *rx_ptr='F'; HAL_UART_RxCpltCallback(&huart3); run(5);
  CHECK(left()>0 && right()>0, "letters still work next to the app");
  *rx_ptr='S'; HAL_UART_RxCpltCallback(&huart3); run(5);
  press(GPIO_PIN_7, 80);                               /* leave MANUAL */
  pad(0x01,0x00,0x01); *rx_ptr='F'; HAL_UART_RxCpltCallback(&huart3); run(20);
  CHECK(left()==0 && right()==0, "outside MANUAL the app and letters cannot drive");
  press(GPIO_PIN_7, 80); press(GPIO_PIN_7, 80); press(GPIO_PIN_7, 80); run(20);   /* back round to MANUAL */
  CHECK(lastline("MODE -> MANUAL") && left()==0 && right()==0, "back in MANUAL: old messages do not suddenly drive");
  printf("Bluetooth debug:\n");
#if MOTOR_MIN == 0
  CHECK(1, "(MOTOR_MIN tests run in test_motor_min)");
#endif
  CHECK(bt_has((const uint8_t*)"MODE -> ",8), "messages are also sent to the phone");
  CHECK(bt_has((const uint8_t*)" b=",3) && bt_has((const uint8_t*)" j=",3), "short status line is sent to the phone");
  printf("Pins the code must never drive:\n");
  CHECK(!(writtenA & (GPIO_PIN_15|GPIO_PIN_12)) && !(writtenB & (GPIO_PIN_3|GPIO_PIN_4)), "sensor pins A15 A12 B3 B4 are only READ, never driven");
  CHECK(!(writtenA & (1u<<11)), "A11 (has 5 V on it) is never touched");
  CHECK(!(writtenA & (GPIO_PIN_6|GPIO_PIN_7)), "button pins A6 A7 are only READ");
  printf("\n%s (%d failed)\n", fails? "SOME TESTS FAILED":"ALL TESTS PASSED", fails);
  return fails;
}
