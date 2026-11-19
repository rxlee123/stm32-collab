# Firmware

| File | Goes in the CubeIDE project | What it is |
|---|---|---|
| `Core/Inc/robot_config.h` | `Core/Inc` | Every setting you tune (pins, speeds, steering, gaps, servo, route) |
| `Core/Inc/robot.h` | `Core/Inc` | `Robot_Init()` and `Robot_Loop()` for main.c |
| `Core/Src/robot.c` | `Core/Src` | All robot logic |
| `test/` | stays here | PC tests with a fake HAL |

## How the code runs
`Robot_Loop()` runs every 2 ms:
1. Read the buttons (debounced, short and long press).
2. Read the 4 sensors → a 4-bit pattern, left to right.
3. Run the current mode: AUTO (line following), MANUAL (phone letters), SENSOR TEST, MOTOR TEST.
4. Move the servo one small step towards its target (smooth, never sudden).
5. Update the LEDs; print the status line every 0.1 s on USART1.

**AUTO:** line position from −30 to +30, PD steering (`KP`, `KD`). Short line loss = dashed gap (straight for `GAP_MS`), longer = search towards the last side, then stop. 3+ sensors briefly = junction (uses the next `ROUTE_LIST` entry); 3+ sensors for longer than `WIDE_TO_INVERT_MS` = background changed colour, switch between black-line and white-line mode. After `R_FINISH`, brake when the line ends.

**Safety:** sensor and button pins are only read; PA11 is never touched; motors are set off before PWM starts; the servo signal stays off until needed; MANUAL stops if no command arrives for `BT_TIMEOUT_MS`.

## Running the tests on a PC
```
gcc -std=c99 -Itest -ICore/Inc test/test.c Core/Src/robot.c -o test/run && ./test/run
gcc -std=c99 -Itest -ICore/Inc "-DROUTE_LIST={ R_LEFT, R_FINISH }" test/test_route.c Core/Src/robot.c -o test/run_route && ./test/run_route
```
