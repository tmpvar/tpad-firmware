/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include <LUFA/Common/Common.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/Misc/TerminalCodes.h>

#if (ARCH == ARCH_AVR8)
	#include <LUFA/Drivers/Peripheral/Serial.h>
	#include <LUFA/Drivers/Peripheral/TWI.h>
	#include <LUFA/Drivers/Peripheral/ADC.h>
	#include <LUFA/Drivers/Peripheral/SPI.h>
	#include <LUFA/Drivers/Peripheral/SerialSPI.h>
#elif (ARCH == ARCH_XMEGA)
	#include <LUFA/Drivers/Peripheral/Serial.h>
	#include <LUFA/Drivers/Peripheral/SPI.h>
	#include <LUFA/Drivers/Peripheral/SerialSPI.h>
	
	#include <LUFA/Platform/XMEGA/ClockManagement.h>
#elif (ARCH == ARCH_UC3)
	#include <LUFA/Platform/UC3/ClockManagement.h>
	#include <LUFA/Platform/UC3/InterruptManagement.h>
#endif