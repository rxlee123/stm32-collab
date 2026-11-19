/* =====================================================================
 *  robot_config.h  -  every number you may want to change lives here
 *
 *  Autocar (UTM Robocon) + AIROST delivery robot, Blue Pill STM32F103C8
 *  Change a value, rebuild, upload. You never need to edit robot.c for tuning.
 * ===================================================================== */
#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

/* ---------------- LINE SENSORS (LM324 outputs) ----------------
 * Order is LEFT to RIGHT as seen from BEHIND the robot, looking forward.
 * Your drawing is mirrored (its left = robot's right), so CHECK this with
 * the SENSOR TEST mode: cover the robot's far-left sensor, the first
 * digit printed must change.
 */
#define S0_PORT  GPIOA      /* far left     (LM324 pin 8,  A15) */
#define S0_PIN   GPIO_PIN_15
#define S1_PORT  GPIOA      /* centre left  (LM324 pin 14, A12) */
#define S1_PIN   GPIO_PIN_12
#define S2_PORT  GPIOB      /* centre right (LM324 pin 7,  B3)  */
#define S2_PIN   GPIO_PIN_3
#define S3_PORT  GPIOB      /* far right    (LM324 pin 1,  B4)  */
#define S3_PIN   GPIO_PIN_4

/* What pin level means "this sensor sees BLACK"?
 *   1 -> after you swap each channel's sensor/preset wires (LED on black)
 *   0 -> if you leave the board as it is now (LED on white)              */
#define SENSOR_BLACK_LEVEL   0   /* your board: LED + output HIGH on white, so LOW = black */

/* ---------------- MOTORS (L293D) ----------------
 * Motor A = L293D pins 3/6 : EN1 = A3 (TIM2 CH4), IN1 = A2, IN2 = A1
 * Motor B = L293D pins 11/14: EN2 = A0 (TIM2 CH1), IN3 = A4, IN4 = A5
 * Use MOTOR TEST mode, then fix these flags instead of re-soldering.  */
#define MOTOR_A_IS_LEFT      1   /* 1: motor A is the LEFT wheel, 0: it is the right one */
#define MOTOR_LEFT_REVERSED  0   /* 1 if the left wheel spins backwards when it should go forward */
#define MOTOR_RIGHT_REVERSED 0

/* Trim: the two motors are never exactly equal. Lower the STRONGER wheel's
 * value (in %) until the robot drives straight. Keep both at 100 or below. */
#define LEFT_TRIM             90  /* left wheel was stronger on our robot */
#define RIGHT_TRIM           100

#define PWM_MAX              999 /* = TIM2 Counter Period in CubeMX */

/* ---------------- LINE FOLLOWING ----------------
 * Speeds are 0..999 (999 = full power). Start slow, speed up later. */
#define BASE_SPEED           420  /* straight-line speed                         */
#define MAX_SPEED            800  /* no wheel is ever driven faster than this    */
#define KP                   110  /* how hard it steers back to the line         */
#define KD                   260  /* damping: stops it wobbling left-right       */
#define SEARCH_SPEED         380  /* spin speed while looking for a lost line    */

#define GAP_MS               400  /* line missing shorter than this = dashed gap: keep going straight.
                                     Raise it if the robot spins inside dashed-line gaps. */
#define SEARCH_MS           1800  /* then turn towards where the line was, for at most this long      */
#define WIDE_TO_INVERT_MS    300  /* 3+ sensors on "line colour" for longer than this = background
                                     changed (white line on black area, rule 7.3.8). Shorter = junction. */
#define JUNCTION_PASS_MS     120  /* drive straight this long to cross a junction                     */
#define TURN_MAX_MS         1500  /* give up a junction turn after this long                          */

/* ---------------- ROUTE (junction choices) ----------------
 * Each time the robot crosses a junction, it uses the next entry.
 * FOLLOW  = just keep following the line (default)
 * LEFT / RIGHT = turn onto the branch on that side
 * STRAIGHT = go straight across
 * FINISH  = after this junction, stop when the line ends (Finish Zone)
 * Fill this in on the practice field. Unlisted junctions = FOLLOW.     */
#ifndef ROUTE_LIST
#define ROUTE_LIST   { R_FOLLOW }
#endif

/* ---------------- SERVO (one servo only, rule 4.5) ----------------
 * Pulse width in microseconds (TIM3 CH4 on B1, 1 tick = 1 us).
 * Find your own numbers: change in steps of 50 and watch the gate.     */
#define SERVO_OPEN_US        1000
#define SERVO_CLOSED_US      1700
#define SERVO_AUTO_US        SERVO_CLOSED_US /* position during Stage 1 (gate closed to push the cube) */
#define SERVO_STEP_US          15  /* moves at most this much every 10 ms: no "dangerous servo movement" */

/* ---------------- MANUAL MODE (Bluetooth, USART3 9600) ----------------
 * Works with "Bluetooth RC Controller"-style apps:
 * F B L R G I H J S = drive, 0-9 / q = speed, W/w or C/O = gate close/open */
#define MANUAL_SPEED_DEFAULT  700
#define MANUAL_TURN_SPEED     550
#define BT_TIMEOUT_MS         500  /* safety: stop if no command arrives for this long (phone out of
                                      range, app closed). If the car stops by itself while you hold a
                                      button, your app only sends once per press: set this to 0. */

/* ---------------- BUTTONS AND LEDS ----------------
 * Your buttons send 3.3 V when pressed (4.7k pull-down on the board).  */
#define BTN_START_PORT GPIOA
#define BTN_START_PIN  GPIO_PIN_6   /* START / STOP                 */
#define BTN_MODE_PORT  GPIOA
#define BTN_MODE_PIN   GPIO_PIN_7   /* change mode (only when stopped) */
#define LONG_PRESS_MS  1200

#define LED1_PORT GPIOB
#define LED1_PIN  GPIO_PIN_12
#define LED2_PORT GPIOB
#define LED2_PIN  GPIO_PIN_13
#define LED3_PORT GPIOB
#define LED3_PIN  GPIO_PIN_14

/* ---------------- DEBUG ---------------- */
#define DEBUG_PRINT_MS        100   /* print state on USART1 (115200) this often, 0 = never */
#define LOOP_MS                 2   /* control loop period */

#endif
