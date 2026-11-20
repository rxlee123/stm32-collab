#ifndef ROBOT_H
#define ROBOT_H

#include "main.h"

/* Call once after all MX_..._Init() functions */
void Robot_Init(void);

/* Call over and over inside while(1) */
void Robot_Loop(void);

/* ---- exposed for testing ---- */
/* Route words for ROUTE_LIST (robot_config.h). Each junction uses the next word.
 * Words with (mm) take a distance in millimetres. */
typedef enum { A_FOLLOW = 0, A_LEFT, A_RIGHT, A_STRAIGHT, A_FINISH,
               A_INVERT, A_BLIND, A_OPEN, A_PICKUP, A_DROP } RouteAct;
typedef struct { uint8_t act; uint16_t mm; } RouteStep;

#define R_FOLLOW      { A_FOLLOW,   0 }   /* keep following the line                       */
#define R_LEFT        { A_LEFT,     0 }   /* spin onto the branch on the left              */
#define R_RIGHT       { A_RIGHT,    0 }   /* spin onto the branch on the right             */
#define R_STRAIGHT    { A_STRAIGHT, 0 }   /* drive straight across, ignore side branches   */
#define R_FINISH      { A_FINISH,   0 }   /* brake when the line ends (Finish Zone)        */
#define R_INVERT      { A_INVERT,   0 }   /* swap black-line / white-line mode HERE        */
#define R_OPEN        { A_OPEN,     0 }   /* open the gate, then like R_STRAIGHT           */
#define R_BLIND(mm)   { A_BLIND,  (mm) }  /* drive straight blind for mm, then find the line */
#define R_PICKUP(mm)  { A_PICKUP, (mm) }  /* drive mm in, close gate, turn round, find line */
#define R_DROP(mm)    { A_DROP,   (mm) }  /* drive mm in, open gate, back out, turn round   */

int  Robot_LineError(uint8_t line_bits, int last_error);   /* -30..+30, 0 = centred */
uint8_t Robot_LineCount(uint8_t line_bits);

#endif
