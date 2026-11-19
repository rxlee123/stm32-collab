# User guide

## Before every power-on
- [ ] Power off before plugging or unplugging any chip, board or cable.
- [ ] After any soldering: measure **+5V to GND in Ω mode** on both boards. Some kΩ = fine, under 10 Ω = short.
- [ ] LM324 notch at the pin 1 end, every leg inside its socket hole.
- [ ] BOOT0 on 0.
- [ ] First run after a code change: wheels lifted off the table.
- [ ] After a minute, touch the 7805 and the L293D: warm is fine, too hot to hold = switch off.

## Buttons, modes and LEDs
The robot powers up in AUTO, stopped, motors off.

| Button | Press | Action |
|---|---|---|
| START (PA6) | short | AUTO: go / stop. MOTOR TEST: run. MANUAL: stop motors |
| START | hold 1.2 s, stopped | Reset the route counter (restart from the Start Zone) |
| MODE (PA7) | short, stopped | AUTO → MANUAL → SENSOR TEST → MOTOR TEST |

| B12 | B13 | B14 | Meaning |
|---|---|---|---|
| on | off | off | AUTO, ready |
| blinking | off | off | AUTO, following a black line |
| blinking | off | on | AUTO, following a white line on black |
| blinking | blinking | blinking | Line lost, stopped |
| on | on | on | Finished |
| off | on | off | MANUAL |
| off | off | on | SENSOR TEST |
| on | off | on | MOTOR TEST |

## First tests
**MOTOR TEST** (wheels lifted): left forward, right forward, both forward, spin right, 1 s each.

| Seen | Change in `robot_config.h` |
|---|---|
| Step 1 moves the right wheel | `MOTOR_A_IS_LEFT 0` |
| A wheel turns backwards | `MOTOR_LEFT_REVERSED 1` / `MOTOR_RIGHT_REVERSED 1` |
| Robot drifts when driving straight | Lower the stronger wheel's `LEFT_TRIM` / `RIGHT_TRIM` in steps of 5 |

**SENSOR TEST:** on white, `black=0000`. Black tape under the far-left sensor only → `1000`, then `0100`, `0010`, `0001`. Wrong order → swap the `S0`–`S3` lines. All inverted → flip `SENSOR_BLACK_LEVEL`.

## Tuning
1. **Sensor height:** test 4, 6, 8, 10 mm; pick the height with the biggest white/black difference (usually 6–8 mm). Fix the board there.
2. **Presets:** for each channel set the knob pin halfway between the sensor pin's white and black readings, below 3.5 V.

| Channel | Sensor (+) | Knob (−) | Output | Blue Pill |
|---|---|---|---|---|
| 1 | 3 | 2 | 1 | PB4 |
| 2 | 5 | 6 | 7 | PB3 |
| 3 | 10 | 9 | 8 | PA15 |
| 4 | 12 | 13 | 14 | PA12 |

3. **Line following** (black tape on white paper):

| Setting | Start | Raise if | Lower if |
|---|---|---|---|
| `BASE_SPEED` | 350–420 | it follows well | it overshoots curves |
| `MAX_SPEED` | 800 (max 999) | you need more top speed | the L293D gets too hot |
| `KP` | 110 | it misses curves | it wobbles |
| `KD` | 260 | it wobbles | it jerks |
| `GAP_MS` | 400 | it spins inside dashed-line gaps | it drives off at real corners |
| `WIDE_TO_INVERT_MS` | 300 | junctions or striped areas switch it to white-line mode (B14 lights) | it is slow to notice the black area |

Keep `BASE_SPEED` well below `MAX_SPEED` so steering has room.

4. **Servo:** in MANUAL send `O` / `C`; adjust `SERVO_OPEN_US` / `SERVO_CLOSED_US` in steps of 50.
5. **Route:** on the practice field note `junc=` at each decision point (corners count too) and fill `ROUTE_LIST`, e.g. `{ R_FOLLOW, R_STRAIGHT, R_LEFT, R_FINISH }`.

## Manual control (phone)
Android only (the Blue Bee uses classic Bluetooth serial). Pair (PIN 1234 or 0000), use an app such as "Bluetooth RC Controller", press MODE until B13.

| Letter | Action |
|---|---|
| F / B | forward / backward |
| L / R | spin left / right |
| G / I / H / J | forward-left / forward-right / back-left / back-right |
| S | stop |
| 0–9, q | speed (q = full) |
| W or C / w or O | close / open gate |

No letter for 0.5 s → the car stops (`BT_TIMEOUT_MS`; set 0 if your app sends once per press).

## Competition day
**Stage 1:** load final settings (no code changes during a match). In set-up, check B12 on, hold START 1.2 s if the robot was run before, place it in the Start Zone. Press START only on the judge's signal. Retry: raise hand, "RETRY!", press START to stop, carry to the restart point, "START!", press START when approved.

**Stage 2:** switch to MANUAL, connect the phone, test once. Drive out within 15 s of the start signal. Never cross into the opponent's area; move the servo calmly.
