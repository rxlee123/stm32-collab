#ifndef ROBOT_H
#define ROBOT_H

#include "main.h"

/* Call once after all MX_..._Init() functions */
void Robot_Init(void);

/* Call over and over inside while(1) */
void Robot_Loop(void);

/* ---- exposed for testing ---- */
typedef enum { R_FOLLOW = 0, R_LEFT, R_RIGHT, R_STRAIGHT, R_FINISH } RouteAction;

int  Robot_LineError(uint8_t line_bits, int last_error);   /* -30..+30, 0 = centred */
uint8_t Robot_LineCount(uint8_t line_bits);

#endif
