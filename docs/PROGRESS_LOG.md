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
| Now | Tuning on a test track: sensor height, speed, dashed lines (`GAP_MS 400`), junctions |

## Next steps
1. Fix sensor height (6–8 mm) and set the 4 presets.
2. Tune `BASE_SPEED`, `KP`, `KD` on a test track with curves and a dashed section.
3. Map the practice field's junctions into `ROUTE_LIST`.
4. Set the servo open/closed values; test carrying the cube.
5. Test MANUAL mode with the phone and the Blue Bee.
6. AIROST: add a TCS34725 colour sensor on PB6/PB7 (I2C1).
