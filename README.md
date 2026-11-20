# Autocar Robot

A two-wheeled line-following robot built on an STM32 "Blue Pill" for the **UTM Robocon Autocar Challenge** and **AIROST 26/27 (IoT problem 1, dual-mode delivery robot)**.

- **AUTO mode:** follows a 20 mm black line with 4 infrared sensors, handles dashed lines, junctions, dead ends and a white-line-on-black section, and stops in the Finish Zone.
- **MANUAL mode:** driven from an Android phone over Bluetooth (Blue Bee).
- **One servo** opens and closes a gate to carry the cube or ping pong balls.

## How it works

```
4 IR sensor pairs ──► LM324 (4 comparators + presets) ──► Blue Pill (STM32F103C8) ──► L293D ──► 2 TT motors
                                                              │
                              START / MODE buttons ───────────┤
                              Blue Bee (phone) ───────────────┤
                              Servo (gate) ◄──────────────────┘
```

Each sensor's voltage is compared with a preset knob by the LM324, which gives the Blue Pill a clean HIGH or LOW. The code works out where the line is and sets each wheel's speed (PD steering).

## Folder map

| Folder | What is inside |
|---|---|
| [`firmware/`](firmware) | The robot code (`robot.c`, `robot.h`, `robot_config.h`) and PC tests |
| [`docs/SETUP_CUBEMX.md`](docs/SETUP_CUBEMX.md) | Create the STM32CubeIDE project, settings, upload, serial monitor |
| [`docs/USER_GUIDE.md`](docs/USER_GUIDE.md) | Buttons, modes, LEDs, tests, tuning, phone control, competition day |
| [`docs/HARDWARE.md`](docs/HARDWARE.md) | Pin map, circuits, power, known board fixes |
| [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) | Symptoms and fixes, including every fault found so far |
| [`docs/PROGRESS_LOG.md`](docs/PROGRESS_LOG.md) | What was built and fixed so far, next steps |
| [`docs/RULES_SUMMARY.md`](docs/RULES_SUMMARY.md) | The competition rules that shape the design |
| [`hardware/drawings/`](hardware/drawings) | Hand-drawn mainboard and sensor board layouts |
| [`chassis/`](chassis) | Optional 3D-printable chassis (FreeCAD macro + STL files) |

## Quick start

1. Create the CubeIDE project and settings: [`docs/SETUP_CUBEMX.md`](docs/SETUP_CUBEMX.md).
2. Copy `firmware/Core/Inc/*.h` into the project's `Core/Inc` and `firmware/Core/Src/robot.c` into `Core/Src`.
3. Add three lines to `main.c` (inside the USER CODE markers), build, upload.
4. Run **MOTOR TEST**, then **SENSOR TEST**, then tune: [`docs/USER_GUIDE.md`](docs/USER_GUIDE.md).

All tuning numbers live in [`firmware/Core/Inc/robot_config.h`](firmware/Core/Inc/robot_config.h). The values here are starting points; keep your tuned values in this file.

## Status (21 Nov 2026)

| Part | Status |
|---|---|
| Sensor board + LM324 | Working. Order left to right A12, A15, B3, B4. Far-right preset re-set 21 Nov |
| Buttons, LEDs, serial monitor | Working |
| Motors + L293D | Working, `MOTOR_MIN` 300 so slow wheels don't stall |
| Line following, corners, dashed line | **Working on the test track** (robot.c v3.8) |
| Bluetooth debug to Serial Studio | Working |
| Phone control (Dabble joystick, iPhone) | Working |
| Map 1 route list | Draft in `robot_config.h`; junction counts to check on the real field |
| Cube pickup / Box B drop | Written and PC-tested, not yet tried on the robot |

## Tests

The logic can be checked on a PC without the robot (steering, gaps, corners, junctions, route words, white-line mode, cube/box steps, Bluetooth, motor power, start-up safety):

```
cd firmware
sh test/run_tests.sh
```
