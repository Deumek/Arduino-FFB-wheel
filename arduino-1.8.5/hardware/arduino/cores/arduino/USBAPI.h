/*
  USBAPI.h
  Copyright (c) 2005-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __USBAPI__
#define __USBAPI__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

#include "Arduino.h"

// This definitions is useful if you want to reduce the EP_SIZE to 16
// at the moment only 64 and 16 as EP_SIZE for all EPs are supported except the control endpoint
#ifndef USB_EP_SIZE
#define USB_EP_SIZE 64
#endif

#if defined(USBCON)

#include "USBDesc.h"
#include "USBCore.h"

//================================================================================
//================================================================================
//  Low level API

typedef struct
{
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint16_t wIndex;
	uint16_t wLength;
} USBSetup;

//================================================================================
//================================================================================
//	USB

#define EP_TYPE_CONTROL				(0x00)
#define EP_TYPE_BULK_IN				((1<<EPTYPE1) | (1<<EPDIR))
#define EP_TYPE_BULK_OUT			(1<<EPTYPE1)
#define EP_TYPE_INTERRUPT_IN		((1<<EPTYPE1) | (1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_INTERRUPT_OUT		((1<<EPTYPE1) | (1<<EPTYPE0))
#define EP_TYPE_ISOCHRONOUS_IN		((1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_ISOCHRONOUS_OUT		(1<<EPTYPE0)

class USBDevice_
{
public:
	USBDevice_();
	bool configured();

	void attach();
	void detach();	// Serial port goes down too...
	void poll();
	bool wakeupHost(); // returns false, when wakeup cannot be processed

	bool isSuspended();

  b8 (*HID_Setup_Callback) (USBSetup& setup);
  void (*HID_ReceiveReport_Callback) (uint8_t *data, uint16_t len);
};
extern USBDevice_ USBDevice;

//================================================================================
//================================================================================
//	Serial over CDC (Serial1 is the physical port)

struct ring_buffer;

#ifndef SERIAL_BUFFER_SIZE
#if ((RAMEND - RAMSTART) < 1023)
#define SERIAL_BUFFER_SIZE 16
#else
#define SERIAL_BUFFER_SIZE 64
#endif
#endif
#if (SERIAL_BUFFER_SIZE>256)
#error Please lower the CDC Buffer size
#endif

class Serial_ : public Stream
{
private:
	int peek_buffer;
public:
	Serial_() { peek_buffer = -1; };
	void begin(unsigned long);
	void begin(unsigned long, uint8_t);
	void end(void);

	virtual int available(void);
	virtual int peek(void);
	virtual int read(void);
  	virtual void accept(void);
	virtual int availableForWrite(void);
	virtual void flush(void);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t*, size_t);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	operator bool();

	volatile uint8_t _rx_buffer_head;
	volatile uint8_t _rx_buffer_tail;
	unsigned char _rx_buffer[SERIAL_BUFFER_SIZE];

	// This method allows processing "SEND_BREAK" requests sent by
	// the USB host. Those requests indicate that the host wants to
	// send a BREAK signal and are accompanied by a single uint16_t
	// value, specifying the duration of the break. The value 0
	// means to end any current break, while the value 0xffff means
	// to start an indefinite break.
	// readBreak() will return the value of the most recent break
	// request, but will return it at most once, returning -1 when
	// readBreak() is called again (until another break request is
	// received, which is again returned once).
	// This also mean that if two break requests are received
	// without readBreak() being called in between, the value of the
	// first request is lost.
	// Note that the value returned is a long, so it can return
	// 0-0xffff as well as -1.
	int32_t readBreak();

	// These return the settings specified by the USB host for the
	// serial port. These aren't really used, but are offered here
	// in case a sketch wants to act on these settings.
	uint32_t baud();
	uint8_t stopbits();
	uint8_t paritytype();
	uint8_t numbits();
	bool dtr();
	bool rts();
	enum {
		ONE_STOP_BIT = 0,
		ONE_AND_HALF_STOP_BIT = 1,
		TWO_STOP_BITS = 2,
	};
	enum {
		NO_PARITY = 0,
		ODD_PARITY = 1,
		EVEN_PARITY = 2,
		MARK_PARITY = 3,
		SPACE_PARITY = 4,
	};

};
extern Serial_ Serial;

//================================================================================
//================================================================================
// Joystick
// Implemented in HID.cpp

class Joystick_
{
  public:
    Joystick_();
    void send_8 (int8_t x, uint8_t y, uint8_t throttle, uint8_t buttons);
    void send_10 (int16_t x, uint16_t y, uint16_t z, uint8_t buttons);
    void send_12 (int16_t x, uint16_t y, uint16_t z, uint8_t buttons);
    void send_16_12_12 (int16_t x, uint16_t y, uint16_t z, uint8_t buttons);
    void send_16_16_12 (int16_t x, uint16_t y, uint16_t z, uint16_t buttons);
    void send_16(int16_t x, uint16_t y, uint16_t z, uint8_t buttons);
    void send_16_10_10_10(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint16_t buttons);
    void send_16_12_12_12(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint32_t buttons);
    void send_16_10_18(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint32_t buttons);
    void send_16_16_10_10_12(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint32_t buttons); // milos, added this one
    void send_16_16_12_12_32(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint32_t buttons); // milos, added this one
    void send_16_16_12_12_12_28(uint16_t x, uint16_t y, uint16_t z, uint16_t rx, uint16_t ry, uint32_t buttons); // milos, added this one
    void send_16_16_12_12_12_12_32(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint16_t ry, uint16_t rz, uint32_t buttons); // milos, added this one
    // DEBUG 2 axis to H-Shifter
    void send_16_8_32(int16_t x, uint16_t y, uint16_t z, uint16_t rx, uint16_t sx, uint16_t sy, uint32_t buttons);
};
extern Joystick_ Joystick;

#define HAVE_CDCSERIAL

//================================================================================
//================================================================================
//	Mouse

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

class Mouse_
{
  private:
    uint8_t _buttons;
    void buttons(uint8_t b);
  public:
    Mouse_(void);
    void begin(void);
    void end(void);
    void click(uint8_t b = MOUSE_LEFT);
    void move(signed char x, signed char y, signed char wheel = 0);
    void press(uint8_t b = MOUSE_LEFT);		// press LEFT by default
    void release(uint8_t b = MOUSE_LEFT);	// release LEFT by default
    bool isPressed(uint8_t b = MOUSE_LEFT);	// check LEFT by default
};
extern Mouse_ Mouse;

//================================================================================
//================================================================================
//	Keyboard

#define KEY_LEFT_CTRL		0x80
#define KEY_LEFT_SHIFT		0x81
#define KEY_LEFT_ALT		0x82
#define KEY_LEFT_GUI		0x83
#define KEY_RIGHT_CTRL		0x84
#define KEY_RIGHT_SHIFT		0x85
#define KEY_RIGHT_ALT		0x86
#define KEY_RIGHT_GUI		0x87

#define KEY_UP_ARROW		0xDA
#define KEY_DOWN_ARROW		0xD9
#define KEY_LEFT_ARROW		0xD8
#define KEY_RIGHT_ARROW		0xD7
#define KEY_BACKSPACE		0xB2
#define KEY_TAB				0xB3
#define KEY_RETURN			0xB0
#define KEY_ESC				0xB1
#define KEY_INSERT			0xD1
#define KEY_DELETE			0xD4
#define KEY_PAGE_UP			0xD3
#define KEY_PAGE_DOWN		0xD6
#define KEY_HOME			0xD2
#define KEY_END				0xD5
#define KEY_CAPS_LOCK		0xC1
#define KEY_F1				0xC2
#define KEY_F2				0xC3
#define KEY_F3				0xC4
#define KEY_F4				0xC5
#define KEY_F5				0xC6
#define KEY_F6				0xC7
#define KEY_F7				0xC8
#define KEY_F8				0xC9
#define KEY_F9				0xCA
#define KEY_F10				0xCB
#define KEY_F11				0xCC
#define KEY_F12				0xCD

//	Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

class Keyboard_ : public Print
{
  private:
    KeyReport _keyReport;
    void sendReport(KeyReport* keys);
  public:
    Keyboard_(void);
    void begin(void);
    void end(void);
    virtual size_t write(uint8_t k);
    virtual size_t press(uint8_t k);
    virtual size_t release(uint8_t k);
    virtual void releaseAll(void);
};
extern Keyboard_ Keyboard;

//================================================================================
//================================================================================
//	HID 'Driver'

int		HID_GetInterface(uint8_t* interfaceNum);
int		HID_GetDescriptor(int i);
b8		HID_Setup(USBSetup& setup);
void	HID_SendReport(uint8_t id, const void* data, int len);
u8		HID_ReportAvailable();
s16	 	HID_ReceiveReport(void* data, int len);

//================================================================================
//================================================================================
//	MSC 'Driver'

int		MSC_GetInterface(uint8_t* interfaceNum);
int		MSC_GetDescriptor(int i);
bool	MSC_Setup(USBSetup& setup);
bool	MSC_Data(uint8_t rx,uint8_t tx);

//================================================================================
//================================================================================
//	CSC 'Driver'

int		CDC_GetInterface(uint8_t* interfaceNum);
int		CDC_GetDescriptor(int i);
bool	CDC_Setup(USBSetup& setup);

//================================================================================
//================================================================================

#define TRANSFER_PGM		0x80
#define TRANSFER_RELEASE	0x40
#define TRANSFER_ZERO		0x20

int USB_SendControl(uint8_t flags, const void* d, int len);
int USB_RecvControl(void* d, int len);
int USB_RecvControlLong(void* d, int len);

uint8_t	USB_Available(uint8_t ep);
uint8_t USB_SendSpace(uint8_t ep);
int USB_Send(uint8_t ep, const void* data, int len);	// blocking
int USB_Recv(uint8_t ep, void* data, int len);		// non-blocking
int USB_Recv(uint8_t ep);							// non-blocking
void USB_Flush(uint8_t ep);

#endif

#endif /* if defined(USBCON) */
