// snes_controller.h

#ifndef _SNES_CONTROLLER_h
#define _SNES_CONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// BIT DEFINITIONS
#define SNES_B       0x01
#define SNES_Y       0x02
#define SNES_SELECT  0x04
#define SNES_START   0x08
#define SNES_UP      0x10
#define SNES_DOWN    0x20
#define SNES_LEFT    0x40
#define SNES_RIGHT   0x80
#define SNES_A       0x100
#define SNES_X       0x200
#define SNES_L       0x400
#define SNES_R       0x800

#endif

