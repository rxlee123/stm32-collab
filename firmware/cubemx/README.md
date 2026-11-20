# CubeMX files (reference copy)

`main.c` and `main.h` from our STM32CubeIDE project "Autocar Doomsday" (21 Nov 2026).
They are kept here, not in `Core/`, so the PC tests can use their own fake `main.h`.

In a CubeIDE project these live in `Core/Src/main.c` and `Core/Inc/main.h`.
The only lines we added are inside the USER CODE markers:
`#include "robot.h"`, `Robot_Init();` and `Robot_Loop();`.
