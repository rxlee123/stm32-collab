# Hardware

Hand-drawn layouts: [mainboard](../hardware/drawings/mainboard.png), [sensor board](../hardware/drawings/sensor_board.png).

## Boards
- **Mainboard:** donut board, about 15 × 9 cm. Blue Pill in female headers, LM324 in a pin-header socket, L293D, LM7805, buttons, LEDs.
- **Sensor board:** about 9 × 7.3 cm, 4 IR pairs, 6-pin header (GND, 3, 5, 12, 10, +5V = LM324 pin numbers).
- **Caster:** CY-15A ball caster (21 mm tall, holes 40 mm apart), or a smooth rounded skid.

## Power
- **Battery 1 → 1N4007 → 330 µF → LM7805 → 5 V:** Blue Pill (5V pin), LM324, L293D logic (pin 16), sensor board, servo. Needs about 7.7 V or more in.
- **Battery 2 → L293D pin 8:** motors only. Its − must join the common GND.

## Circuits
**Sensor pair (×4)**
```
+5V ── 330 Ω ── IR LED ── GND            (IR LED: about 1.2–2 V across it when working)
+5V ── receiver ──(signal)── 4.7 kΩ ── GND
```
More reflected light → higher signal, so **white reads higher than black**.

**Comparator (×4, LM324):** sensor on + input, preset wiper on − input, output → indicator LED + Blue Pill pin. Output HIGH (about 3.5 V) when sensor > preset, so the LEDs light on **white**. LM324 pin 4 = 5 V, pin 11 = GND.

**Buttons:** press → 3.3 V on PA6 / PA7; 4.7 kΩ pull-down + CubeMX internal pull-down.

## Pin map (Blue Pill STM32F103C8)

| Pin | Connected to | Mode |
|---|---|---|
| PA15, PA12, PB3, PB4 | LM324 outputs (ch 3, 4, 2, 1) | Input, read only |
| PA6 / PA7 | START / MODE | Input, pull-down |
| PA3 / PA0 | L293D EN1 / EN2 | TIM2 CH4 / CH1 PWM |
| PA2, PA1 | L293D IN1, IN2 (motor A) | Output |
| PA4, PA5 | L293D IN3, IN4 (motor B) | Output |
| PB1 | Servo | TIM3 CH4 PWM |
| PB12–PB14 | Status LEDs | Output |
| PB10 / PB11 | Blue Bee | USART3 9600 |
| PA9 / PA10 | USB-serial cable | USART1 115200 |
| PB6 / PB7 | Free, reserved for a TCS34725 colour sensor (AIROST) | I2C1 |
| PA11 | 5 V wire through its pad | Never use |
| Free | PB0, PB5, PB15, PA8, PC14, PC15 | |

## Known board fixes and recommendations
- The **LM324 pin-header socket grips poorly**: a leg and its solder joint can read different voltages. Replace it with a 14-pin DIP socket.
- Recommended: 1 kΩ in series between each LM324 output and its Blue Pill pin (after the LED), so a pin mistake can never damage either chip.
- Recommended: cut the Blue Pill header's PA11 leg so the 5 V wire can never reach the chip.
- Add a GND pin beside the PA9/PA10 upload pins.
- Optional: 10 kΩ from PA0 and PA3 to GND keeps the motors off during the first milliseconds of power-up.
