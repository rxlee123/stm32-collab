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
#define S0_PORT  GPIOA      /* far left     (A12) */
#define S0_PIN   GPIO_PIN_12
#define S1_PORT  GPIOA      /* centre left  (A15) - restored 26 Nov: 20:39 & 20:52 logs showed 1010/0101 (never 1100/0011) */
#define S1_PIN   GPIO_PIN_15
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
#define LEFT_TRIM            100  /* left wheel was stronger on our robot */
#define RIGHT_TRIM           100

#define PWM_MAX              999 /* = TIM2 Counter Period in CubeMX */

/* Minimum motor power. A TT motor through the L293D doesn't turn below roughly 300,
 * it just hums. Every speed that isn't 0 is lifted into MOTOR_MIN..999, so slow
 * wheels (inner wheel in a turn, gentle steering) still turn, forwards or backwards.
 * Find it: raise it until a wheel always turns in sharp corners; if the robot
 * jerks or is too fast in gentle steering, lower it. 0 = off.                */
#ifndef MOTOR_MIN
#define MOTOR_MIN            200
#endif

/* ---------------- LINE FOLLOWING ----------------
 * Speeds are 0..999 (999 = full power). Start slow, speed up later. */
#define BASE_SPEED           300  /* 30% of MAX_SPEED (999); PWM ~440 with MOTOR_MIN 200 */
#define MAX_SPEED            999  /* no wheel is ever driven faster than this    */
#define KP                    60  /* smooth steering for BASE_SPEED 300 (e=10 -> 180/300, e=20 -> 60/300) */
#define KD                    80  /* toned down so returning to 0110 after a bend doesn't kick off the line */
#define D_MS                  80  /* how long the damping acts after the line moves (ms). 0 = no damping */
#define CORNER_SLOW           60  /* slow both wheels in bends (e=10 -> base 240, e=20 -> base 180) */
#define SEARCH_SPEED         230  /* toned down turning/spin speed for smoother 90 deg turns */
#define SHARP_OUTER          280  /* sharp bend (only an outer sensor sees the line): outer wheel forward */
#define SHARP_INNER          0    /* 0 = stop inner wheel (no harsh reverse kick) */
#define TURN_FWD             0   /* pivot: outer wheel forward  */
#define TURN_BACK            0   /* pivot: inner wheel backwards *//* ...and inner wheel BACKWARDS at this speed. Raise both = sharper turns */

#define GAP_MS               600  /* line missing shorter than this = dashed gap: keep going straight.
                                     Raise it if the robot spins inside dashed-line gaps. */
#define BEND_MEMORY_MS       150  /* line vanishes within this long after a FAR sensor saw it = a corner:
                                     spin towards it at once (0 = off)                              */
#define BEND_BACK             70  /* toned down corner spin backwards drift */
#define SEARCH_MS           1800  /* then turn towards where the line was, for at most this long      */
#define SWEEP_MS             300  /* after a long gap: look this long one way, then twice as long the other */
#define SEEK_MS             3000  /* "seeking": after START in the start box, and after R_BLIND, R_INVERT,
                                     R_PICKUP, R_DROP: drive straight up to this long to reach the next line */
#ifndef WIDE_TO_INVERT_MS
#define WIDE_TO_INVERT_MS      0  /* AUTOMATIC colour swap: 3+ sensors on "line colour" longer than this
                                     = background changed. 0 = OFF (use R_INVERT in the route instead,
                                     safer: driving off the track can't flip the colour). Try 500 to use it. */
#endif
#ifndef JUNCTION_PASS_MS
#if AIROST_MODE
#define JUNCTION_PASS_MS     220  /* AIROST: clear the 19 mm left ball branch cleanly even when angled */
#else
#define JUNCTION_PASS_MS     120  /* drive straight this long to cross a junction                     */
#endif
#endif
#define TURN_MAX_MS         2000  /* give up a junction turn after this long                          */
#define SEEK_SETTLE_MS       150  /* after seeking (start box, blind parts...): junctions count only after
                                     the line has been followed this long (box edges are not junctions) */
/* JUNCTION_MIN_MS and JUNCTION_GAP_MS are defined below (AIROST_MODE uses longer values
 * to ignore wide-tape straight-line wobbles and space out the 20 cm Ball 1/2/3 junctions). */

/* ---------------- DISTANCES ----------------
 * R_BLIND(mm), R_PICKUP(mm), R_DROP(mm) and FINISH_MM are distances in millimetres.
 * The robot turns them into time using SPEED_MM_S, so MEASURE IT:
 * time how long the robot takes to drive 1 metre in AUTO at BASE_SPEED, then
 * SPEED_MM_S = 1000 / seconds  (e.g. 1 m in 3.3 s -> 300)                  */
#define SPEED_MM_S           300  /* how fast the robot drives at BASE_SPEED, in mm per second  */
#define FINISH_MM            150  /* after R_FINISH: drive this far past the end of the line, then brake */
#define SPIN180_MS           900  /* spin time for a half turn at SEARCH_SPEED (R_PICKUP, R_DROP) */
#define GATE_MS              600  /* wait this long for the gate to open/close                   */

/* ---------------- ROUTE (junction choices) ----------------
 * Each time the robot crosses a junction (3+ sensors on the line), it uses the next word.
 *   R_FOLLOW      keep following the line (corners)
 *   R_LEFT/RIGHT  spin onto the branch on that side
 *   R_STRAIGHT    drive straight across, ignore side branches
 *   R_FINISH      brake FINISH_MM after the line ends (Finish Zone)
 *   R_INVERT      swap black-line / white-line mode right here
 *   R_OPEN        open the gate, then like R_STRAIGHT
 *   R_BLIND(mm)   drive straight mm with the sensors ignored, then find the line
 *   R_PICKUP(mm)  drive mm in, close the gate on the cube, turn round, find the line
 *   R_DROP(mm)    drive mm into the box, open the gate, back out, turn round, find the line
 * Junctions after the end of the list = R_FOLLOW.
 * After START, R_BLIND, R_INVERT, R_PICKUP and R_DROP the robot drives straight
 * to the next line; that line is NOT counted as a junction.
 * Check the junction numbers with SENSOR TEST or the J-messages on the phone.   */
#ifndef ROUTE_LIST
/* MAP 1 draft (Box B). Lines marked "corner?" may not count on your robot:
 * if the J-numbers on the phone don't match, delete or add those R_FOLLOWs.
 * For a simple test track use:  #define ROUTE_LIST { R_FOLLOW }                */
#define ROUTE_LIST { R_FOLLOW  } /* J1: fake one on the first straight */ \
                      /* J5: far point -> main line         */
#endif

/* ---------------- SERVO (one servo only, rule 4.5) ----------------
 * Pulse width in microseconds (TIM3 CH4 on B1, 1 tick = 1 us).
 * Find your own numbers: change in steps of 50 and watch the gate.     */
#define SERVO_OPEN_US        1000
#define SERVO_CLOSED_US      1700
#define SERVO_AUTO_US        SERVO_CLOSED_US /* position during Stage 1 (gate closed to push the cube) */
#define SERVO_STEP_US          15  /* moves at most this much every 10 ms: no "dangerous servo movement" */
#ifndef SERVO_IDLE_OFF_MS
#if AIROST_MODE
#define SERVO_IDLE_OFF_MS     400  /* stop PWM pulses (CCR=0) this long after the servo reaches its target:
                                      prevents continuous stall current from browning out the 5 V rail / Bluetooth.
                                      0 = keep holding PWM on continuously. */
#else
#define SERVO_IDLE_OFF_MS       0  /* legacy Autocar mode: keep PWM holding the heavy cube */
#endif
#endif

/* ---------------- MANUAL MODE (Bluetooth, USART3 9600) ----------------
 * Dabble app (iPhone/Android) gamepad: joystick or arrows drive, TRIANGLE close gate, CROSS open,
 * CIRCLE faster, SQUARE slower, SELECT stop.
 * Letters also work (Serial Studio, PuTTY, "Bluetooth RC Controller"-style apps):
 * F B L R G I H J S = drive, 0-9 / q = speed, W/w or C/O = gate close/open */
/* ONE-TIME RENAME of the Bluetooth module (many teams have "HC-06"/"BT05"...). Remove the // ,
 * pick a name (letters/digits, no spaces, max 12), upload, start the robot with NO phone
 * connected. The USB serial monitor says if it worked (6 fast LED flashes = failed).
 * Then switch the robot off/on, put the // back and upload again. The name stays saved.  */
// #define BT_NAME            "Doomsday01"

#define MANUAL_SPEED_DEFAULT  700
#define MANUAL_TURN_SPEED     550
#define BT_TIMEOUT_MS         500  /* safety: stop if no command arrives for this long (phone out of
                                      range, app closed). If the car stops by itself while you hold a
                                      button, your app only sends once per press: set this to 0. */
#define MANUAL_SPEED_STEP     100  /* Dabble CIRCLE / SQUARE: faster / slower by this much per press */
#define DABBLE_TIMEOUT_MS       0  /* Dabble app safety stop, like BT_TIMEOUT_MS. 0 = off (the car stops
                                      when you let go). Watch dab= on the serial monitor while you
                                      hold the joystick still: if it keeps counting up, the app sends
                                      all the time, and you can set this to 500 for extra safety. */

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
#define BT_DEBUG                2   /* messages to the phone (Serial Studio): 0 = off,
                                       1 = events only (J-numbers, turns, gaps, LOST...),
                                       2 = events + a short status line every BT_STATUS_MS */
#define BT_STATUS_MS          250   /* how often the short status line goes to the phone */
#define LOOP_MS                 2   /* control loop period */

/* ---------------- AIROST MODE & TCS34725 COLOUR SENSOR ----------------
 * AIROST_MODE = 1: side-mounted TCS34725 on I2C1 (PB6 SCL / PB7 SDA, 100 kHz, LED always ON)
 *   detects Red -> ball 1, Blue -> ball 2, Green -> ball 3 on Figure 1 and auto-switches to MANUAL.
 * AIROST_MODE = 0: original UTM Robocon Autocar ROUTE_LIST mode (I2C1 not required). */
#ifndef AIROST_MODE
#define AIROST_MODE            1
#endif
#ifndef JUNCTION_MIN_MS
#define JUNCTION_MIN_MS       20   /* 20 ms catches 30-35 ms real left junctions at BASE_SPEED 300 (26 Nov 20:24 log) */
#endif
#ifndef JUNCTION_GAP_MS
#if AIROST_MODE
#define JUNCTION_GAP_MS      250   /* debounce after AS_CROSS clears a ball branch (220+250=470 ms << 20 cm spacing) */
#else
#define JUNCTION_GAP_MS      150
#endif
#endif
#ifndef AIROST_RIGHT_CORNERS
#define AIROST_RIGHT_CORNERS   2   /* Figure 1 has two 90 deg right corners before Ball 1 (0 = disable gating) */
#endif
#ifndef RIGHT_CORNER_MIN_MS
#define RIGHT_CORNER_MIN_MS  250   /* sharp right turn (0001 / BEND right) must accumulate this long to count (real 90 deg corner is 700-1800 ms) */
#endif
#ifndef RIGHT_CORNER_GAP_MS
#define RIGHT_CORNER_GAP_MS 1500   /* minimum gap (ms) after exiting a right corner before next corner counts */
#endif
#ifndef TBAR_MIN_MS
#define TBAR_MIN_MS           20   /* on a ball branch, require all 4 sensors (1111) this long for end T-bar */
#endif
#ifndef COLOR_POLL_MS
#define COLOR_POLL_MS         30   /* non-blocking poll period (ms); ATIME = 24 ms continuous integration */
#endif
#ifndef COLOR_I2C_TIMEOUT_MS
#define COLOR_I2C_TIMEOUT_MS   5   /* short I2C timeout (ms) so the 2 ms IR loop is never blocked */
#endif
#ifndef COLOR_MIN_CLEAR
#define COLOR_MIN_CLEAR       40   /* lowered 80 -> 40 so dark navy blue paper is not rejected as black */
#endif
#ifndef COLOR_R_SCALE
#define COLOR_R_SCALE         72   /* white-balance trims (%) calibrated from 17:09 white mat (rn=335 gn=331 bn=327) */
#endif
#ifndef COLOR_G_SCALE
#define COLOR_G_SCALE         98
#endif
#ifndef COLOR_B_SCALE
#define COLOR_B_SCALE        165   /* white-balance B trim (%) matching white mat */
#endif
#ifndef COLOR_DOM_PCT
#define COLOR_DOM_PCT        115   /* kept for backwards compatibility; classifier now uses COLOR_REF_* hue rays */
#endif
/* Chromaticity references {cr, cg, cb} (per-mille of rn+gn+bn), calibrated from 26 Nov logs + blue screenshot.
 * Classifier measures distance from WHITE (dW >= COLOR_MIN_DIST) and hue-ray cosine (>= COLOR_MIN_COS). */
#ifndef COLOR_REF_WHITE
#define COLOR_REF_WHITE      336, 332, 332
#endif
#ifndef COLOR_REF_RED
#define COLOR_REF_RED        564, 181, 255
#endif
#ifndef COLOR_REF_BLUE
#define COLOR_REF_BLUE       269, 352, 379   /* R=849 G=816 B=521 C=2047 -> dW=84, cos=996 */
#endif
#ifndef COLOR_REF_GREEN
#define COLOR_REF_GREEN      285, 419, 296
#endif
#ifndef COLOR_MIN_DIST
#define COLOR_MIN_DIST        40   /* min chromaticity distance from white mat (white noise ~1..5, real blue ~84) */
#endif
#ifndef COLOR_MIN_COS
#define COLOR_MIN_COS        580   /* min hue-ray cosine (out of 1000) to the nearest colour reference */
#endif
#ifndef COLOR_LOCK_SAMPLES
#define COLOR_LOCK_SAMPLES     4   /* consecutive matching samples (~120 ms) required to lock a colour */
#endif
#ifndef AIROST_NEEDED_COLORS
#define AIROST_NEEDED_COLORS   1   /* 1 = Basic (1 square), up to 3 = Advanced (3 squares) */
#endif
#ifndef ARRIVE_MM
#define ARRIVE_MM              0   /* optional creep distance (mm) at ball T-bar before braking -> MANUAL */
#endif
#ifndef TURN_MIN_MS
#define TURN_MIN_MS          120   /* minimum spin time (ms) in AS_TURN before accepting a new line */
#endif

/* ---- Ball turn: simple timed 90 deg left spin, then MANUAL ----
 * At the target Ball junction the robot spins left for TURN90_MS (sensors ignored), brakes and
 * switches to MANUAL. TUNE IT: in MANUAL send the letter 'T' (Serial Studio / BT terminal):
 * the robot does the same spin. Raise TURN90_MS if it turns less than 90 deg, lower it if more. */
#ifndef TURN90_SPEED
#define TURN90_SPEED         SEARCH_SPEED  /* spin speed for the 90 deg ball turn */
#endif
#ifndef TURN90_MS
#define TURN90_MS            550   /* spin time for 90 deg at TURN90_SPEED (tune with 'T' in MANUAL) */
#endif
#ifndef TURN_ADVANCE_MS
#define TURN_ADVANCE_MS        0   /* optional: drive straight this long before the spin so the wheels sit
                                      on the branch (0 = spin right where the junction was seen) */
#endif

/* ---- Ball zone junction counting (after RIGHT CORNER #2) ---- */
#ifndef BALL_SETTLE_MS
#define BALL_SETTLE_MS        80   /* after corner #2: line centred (no outer sensor) this long = "BALL ZONE armed".
                                      From then on EVERY wide patch is a Ball junction, even if S3 came first */
#endif
#ifndef BALL_PULSE_MAX_MS
#define BALL_PULSE_MAX_MS      0   /* in the Ball zone, also count a 2-sensor left pulse (1100 / 1010, robot
                                      off-centre) lasting JUNCTION_MIN_MS..this as a Ball junction.
                                      0 = only LOG it ("2-sensor left pulse"). Try 100 if logs show real
                                      Ball junctions appearing only as 2-sensor pulses. */
#endif

#endif
