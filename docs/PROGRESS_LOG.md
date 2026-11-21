# Progress log

| Stage | What happened |
|---|---|
| Design | Chose Blue Pill + LM324 comparators + L293D + one servo gate; hand-drew mainboard and sensor board |
| Mainboard | Found a 5 V wire through the PA11 pad (PA11 left unused). Fixed LM324 output pins not wired to the header |
| Sensor board | Fixed IR LEDs with no GND link, a backwards receiver, resistor GND links, a +5V–GND short |
| LM324 | Channel 2 "dead" was really a loose header-socket contact plus a missing wire. All 4 channels work |
| Firmware | Wrote robot.c (AUTO, MANUAL, SENSOR TEST, MOTOR TEST) with PC tests and an ARM build check |
| Buttons | Only worked with the meter attached → CubeMX Pull-down on PA6/PA7, and uploading the newest .elf |
| Motors | MOTOR TEST passes. Left wheel stronger → `LEFT_TRIM 90` |
| 19-20 Nov | Bluetooth debug to Serial Studio; Dabble joystick (iPhone); map 1 route words (INVERT, BLIND, OPEN, PICKUP, DROP) |
| 20 Nov | Fixed swapped sensors (A12 is far left); start box seeking; blue zones read as white |
| 21 Nov | Inner wheel stalled in turns -> `MOTOR_MIN`; corners overshot as "gaps" -> corner memory; steering kept in WIDE; auto colour swap off; soft start; far-right preset re-set. **Line following + corners + dashed line work** |
| 21 Nov (eve) | New battery: re-tuned (BASE 200, MOTOR_MIN 200, KP 90, KD 250). Added an optional pivot turn (`TURN_FWD`/`TURN_BACK`, v3.8p) but it caused left-right wobble, so it is OFF (0). Robot now runs straight and passes the diamond's two side roads, but **gets stuck at the top point of the diamond** (work in progress) |

## Next steps
0. Fix the stop at the top point of the diamond (needs the Serial Studio log from that spot).
1. Measure `SPEED_MM_S` (time 1 m at BASE_SPEED) and `SPIN180_MS`.
2. Map 1: count junctions by hand in SENSOR TEST, fix the `corner?` lines in `ROUTE_LIST`.
3. Test `R_PICKUP` / `R_DROP` with the real cube and gate.
4. AIROST: add a TCS34725 colour sensor on PB6/PB7 (I2C1).
