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
| 21 Nov (night) | **v3.8q:** the all-black safety stop fired on 3 sensors (0111) at the diamond top: now only on 1111. **v3.8r:** KD never worked (the derivative lasted one 2 ms loop): the last change of the line position is now kept for `D_MS` (80 ms), so the side-to-side swinging is braked; no damping while 3+ sensors see black. **v3.8s:** corner memory is no longer erased by a short `0110` right after a wide patch (robot drove straight past a corner). Result: **no more wobble, corners and the diamond top work.** Next problem: stuck at the small corner near the dashed line |
| 22 Nov (night) | **v3.8u** junction filter (`JUNCTION_MIN_MS` 20 ms). **v3.8v** damping only on a real swing (two steps the same way), no kicks on flicker or after a spin. **v3.8w** corner turn spins while drifting backwards (`BEND_BACK` 100), because the sensors sit close to the wheel axle and a spin on the spot missed every line. Sensor presets narrowed. Tuned: KP 70, KD 350, CORNER_SLOW 30, SEARCH_SPEED 200, SHARP_OUTER 400, SHARP_INNER 0. Result: runs from the start to the cross junction before the white-line section. Still wobbles; many fake junctions on long runs |
| 22 Nov (early morning) | **v3.8w2** = v3.8w + optional one-time Bluetooth rename: `#define BT_NAME "Doomsday01"` in robot_config.h makes the robot send `AT+NAME...` at start-up (3 command styles tried); the module was renamed to **Doomsday01** (laptop shows it; the iPhone kept showing the old name until restarted). Remove the line again after renaming. Driving is the same as v3.8w. Competition on 22 Nov |

## Next steps
0. Reduce the wobble; then R_LEFT at the cross before the white-line section and R_INVERT into the black square.
1. Measure `SPEED_MM_S` (time 1 m at BASE_SPEED) and `SPIN180_MS`.
2. Map 1: count junctions by hand in SENSOR TEST, fix the `corner?` lines in `ROUTE_LIST`.
3. Test `R_PICKUP` / `R_DROP` with the real cube and gate.
4. AIROST: add a TCS34725 colour sensor on PB6/PB7 (I2C1).
