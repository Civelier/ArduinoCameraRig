/*
 Name:		I2CRemoting.h
 Created:	10/2/2021 4:50:08 PM
 Author:	civel
 Editor:	http://www.visualmicro.com
*/

#ifndef _I2CRemoting_h
#define _I2CRemoting_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "CommandRegister.h"

/*
* Master:
* Host, has address, needs to initiate the connection with clock. Needs to send requests to subs.
* Subs have an interrupt pin to request data transfer.
*/


#endif

