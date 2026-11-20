# Firmware

| File | Goes in the CubeIDE project | What it is |
|---|---|---|
| `Core/Inc/robot_config.h` | `Core/Inc` | Every setting you tune (pins, speeds, steering, gaps, servo, route) |
| `Core/Inc/robot.h` | `Core/Inc` | `Robot_Init()` and `Robot_Loop()` for main.c |
| `Core/Src/robot.c` | `Core/Src` | All robot logic |
| `test/` | stays here | PC tests with a fake HAL (`sh test/run_tests.sh`) |
| `cubemx/` | reference only | Our CubeMX `main.c` / `main.h` (the 3 robot lines are inside USER CODE) |

## How the code runs
`Robot_Loop()` runs every 2 ms:
1. Read the buttons (debounced, short and long press).
2. Read the 4 sensors → a 4-bit pattern, left to right.
3. Run the current mode: AUTO (line following), MANUAL (phone letters), SENSOR TEST, MOTOR TEST.
4. Move the servo one small step towards its target (smooth, never sudden).
5. Update the LEDs; print the status line every 0.1 s on USART1.

**AUTO:** line position from −30 to +30, PD steering (`KP`, `KD`). Short line loss = dashed gap (straight for `GAP_MS`), longer = search towards the last side, then stop. 3+ sensors briefly = junction (uses the next `ROUTE_LIST` entry); 3+ sensors for longer than `WIDE_TO_INVERT_MS` = background changed colour, switch between black-line and white-line mode. After `R_FINISH`, brake when the line ends.

**Safety:** sensor and button pins are only read; PA11 is never touched; motors are set off before PWM starts; the servo signal stays off until needed; MANUAL stops if no command arrives for `BT_TIMEOUT_MS`.

## New since v3 (Nov 2026)
- **Bluetooth debug** to Serial Studio (laptop, BLE HC-06, service 0xFFE0 / char 0xFFE1): junction messages `J5 LEFT b=0111`, turn/gap/bend messages, and a short status line every 0.25 s. `BT_DEBUG` 0/1/2.
- **Route words** with distances: `R_INVERT`, `R_BLIND(mm)`, `R_OPEN`, `R_PICKUP(mm)`, `R_DROP(mm)` (cube to Box B), plus a map 1 draft in `robot_config.h`.
- **MOTOR_MIN** (300): lifts small wheel speeds above the stall point, so the inner wheel turns (and reverses) in corners. All other speeds were lowered to match.
- **Corner memory** (`BEND_MEMORY_MS`): line vanishing just after a far sensor saw it = corner, spin to that side.
- **Steering continues in WIDE** (line under 3 sensors no longer stops steering).
- **Safety**: automatic colour swap off (`WIDE_TO_INVERT_MS 0`, use `R_INVERT`), all-black stop (`WIDE_MAX_MS`), soft start (`START_RAMP_MS`), seeking from the start box without counting box edges.
- **SENSOR TEST** counts wide patches (`WIDE #n`) while you push the robot by hand, to map junctions.
- Sensor order is **A12, A15, B3, B4** (left to right).

## Running the tests on a PC
```
cd firmware
sh test/run_tests.sh
```
