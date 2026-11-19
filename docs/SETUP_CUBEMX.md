# Setting up the STM32CubeIDE project

Start a **new** project. A project with FreeRTOS enabled never reaches the main `while(1)` loop, so the robot code would never run.

## 1. Create the project
1. **File → New → STM32 Project**, Part Number **STM32F103C8T6**, name it (ours: `Autocar Doomsday`), Finish.

## 2. Settings (Pinout & Configuration)

| Where | Setting | Why |
|---|---|---|
| System Core → RCC | HSE: **Crystal/Ceramic Resonator** | Accurate 8 MHz clock |
| System Core → SYS | Debug: **Serial Wire** | Frees PA15, PB3, PB4 for the sensors. Set this first |
| Clock Configuration | HCLK **72 MHz** | Full speed; timer numbers assume it |
| PA15, PA12, PB3, PB4 | **GPIO_Input, no pull** | LM324 outputs (sensors). Never outputs |
| PA6, PA7 | **GPIO_Input, Pull-down** | START / MODE buttons. The pull-down makes the release reliable |
| PA1, PA2, PA4, PA5 | **GPIO_Output, level Low** | Motor direction |
| PB12, PB13, PB14 | **GPIO_Output, level Low** | Status LEDs |
| TIM2 | Internal clock, **CH1 + CH4 PWM**, Prescaler **71**, Period **999**, Pulse 0 | Motor speed on **PA0** and **PA3** (1 kHz) |
| TIM3 | Internal clock, **CH4 PWM**, Prescaler **71**, Period **19999**, Pulse 0 | Servo on **PB1** (50 Hz, 1 count = 1 µs) |
| USART1 | Asynchronous, **115200** | USB-serial cable on PA9 (TX) / PA10 (RX) |
| USART3 | Asynchronous, **9600**, NVIC: **USART3 global interrupt** on | Blue Bee on PB10 / PB11 |
| PA11 | **Leave unused** | A 5 V wire passes through its pad |
| Project Manager → Code Generator | Do **not** tick "Set all free pins as analog" | |

Check on the chip picture that TIM2_CH1 = PA0, TIM2_CH4 = PA3, TIM3_CH4 = PB1. Save (Ctrl+S) and generate code.

## 3. Add the robot code
Drag the **files** (not the folder) into the project: `robot.c` → `Core/Src`, `robot.h` and `robot_config.h` → `Core/Inc`, choose **Copy files**.

In `main.c`, write only between the USER CODE markers (CubeMX deletes anything else when it regenerates):

```c
/* USER CODE BEGIN Includes */
#include "robot.h"
/* USER CODE END Includes */

  /* USER CODE BEGIN 2 */
  Robot_Init();
  /* USER CODE END 2 */

    /* USER CODE BEGIN 3 */
    Robot_Loop();
  }
  /* USER CODE END 3 */
```

Build with the hammer: **0 errors**. `implicit declaration of Robot_Init` = the `#include` line is missing or outside the markers.

## 4. Upload (USB-serial cable, BOOT0 method)
Cable TX → PA10, RX → PA9, **GND → GND**.

1. Disconnect the serial monitor (only one program can use the COM port).
2. BOOT0 = 1, press reset.
3. STM32CubeProgrammer: UART, your COM port, 115200, parity **Even** → Connect.
4. Erase & Programming → **browse again** to the newest `Debug/<project>.elf` (check its time) → Start → wait for "verified".
5. BOOT0 = 0, press reset.

**Check the new code really arrived:** the start-up message says `Autocar ready v2`. An old `.elf` uploads without any error, so always check.

## 5. Serial monitor
In CubeIDE: Console → **Open Console ▾** (last toolbar icon) → **Command Shell Console** → Serial Port → New → your COM port, 115200, 8N1. Or PuTTY (Serial, COM port, 115200).

Each line every 0.1 s:

```
AUTO   IDLE   black=0110 inv=0 err=  0 L=   0 R=   0 junc=0 btn=00
```

`black` = sensors left to right (1 = black), `inv` = white-line mode, `err` = line position, `L/R` = wheel speeds, `junc` = junctions counted, `btn` = START and MODE as the chip reads them.
