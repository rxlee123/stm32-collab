# Troubleshooting

Measure with the black probe on GND and the red probe on the point you test. A multimeter showing a single "1" means open (no connection). The beep test does not pass through resistors or LEDs: use Ω mode or diode mode for those.

| Symptom | Likely cause | Fix |
|---|---|---|
| Chip leg and its solder joint read different voltages | Leg not touching the header socket | Bend legs slightly outward and re-seat; better, a DIP socket |
| LM324 output changes but its LED stays dark | Missing wire or bad joint in output → resistor → LED → GND | Measure each joint; solder the gap |
| IR LED does not glow purple on a phone camera | Short (about 0.1 V across), no GND link (both legs at 5 V), or backwards | Fix the link or flip it; good = 1.2–2 V across |
| Sensor voltage does not change between white and black | IR LED off, receiver not conducting, receiver aimed wrong, wrong height | Phone camera check, torch test, point both straight down, 6–8 mm high |
| Sensor reads 4–5 V on both surfaces | Receiver backwards, 4.7 kΩ not linked to GND, or seeing its own LED | Diode-test receiver, link resistor, tape around receiver |
| An indicator LED never changes | Knob set outside the sensor's range | Set knob halfway between white and black readings |
| Whole sensor board dead, 7805 hot | Short between +5V and GND (often a new solder link touching the other rail) | Ω check +5V to GND; knife between the rails |
| Button only works with the multimeter attached | Pin not pulled low on release | Enable Pull-down on PA6/PA7 in CubeMX and upload; check the 4.7 kΩ |
| A code change has no effect | An old `.elf` was uploaded | Re-browse to the newest `.elf`; check `Autocar ready v2` on start-up |
| `implicit declaration of Robot_Init` | `#include "robot.h"` missing or outside USER CODE markers | Put it between `USER CODE BEGIN Includes` / `END Includes` |
| Serial monitor shows nothing or garbage | No GND wire, TX/RX swapped, wrong baud, or BOOT0 on 1 | Cable TX → PA10, RX → PA9, GND → GND, 115200, BOOT0 = 0 |
| Wheels don't move but `L=`/`R=` show numbers | Battery 2 off or its − not on GND, L293D power/GND missing | L293D pin 8 = battery voltage, pin 16 = 5 V, pins 4/5/12/13 = 0 V |
| One wheel stronger, robot drifts | Normal motor difference | Lower that wheel's `*_TRIM` |
| Robot spins inside dashed-line gaps | Gap lasts longer than `GAP_MS` | Raise `GAP_MS` (400–600) |
| Robot takes the wrong branch | No route set for that junction | Fill `ROUTE_LIST`; slow down near junctions |

## Lessons from building this robot
1. Most faults were **loose contacts or missing links**, not broken parts. Measure both ends of every connection before replacing a chip.
2. After every soldering session, do the **+5V-to-GND Ω check** before powering on.
3. Never let the Blue Pill drive a pin that another chip also drives. The sensor pins are inputs only.
4. Always confirm a new upload really arrived (version text on the serial monitor).
