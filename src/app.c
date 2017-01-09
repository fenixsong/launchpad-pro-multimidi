/******************************************************************************
 
 Copyright (c) 2015, Focusrite Audio Engineering Ltd.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of Focusrite Audio Engineering Ltd., nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 *****************************************************************************/

//______________________________________________________________________________
//
// Headers
//______________________________________________________________________________

#include "app.h"

//______________________________________________________________________________
//
// This is where the fun is!  Add your code to the callbacks below to define how
// your app behaves.
//
// In this example, we store all button states in an array, which we sometimes
// save to flash.
//______________________________________________________________________________

#define BUTTON_COUNT 100
#define PAD_COUNT 64

static u8 g_Buttons[BUTTON_COUNT] = {0};

static const u16 *g_ADC = 0;

//______________________________________________________________________________

void app_surface_event(u8 type, u8 index, u8 value)
{
	switch (type)
	{
		case  TYPEPAD:
		{
			return;
			
			
		}
		break;

		case TYPESETUP:
		{
			if (value)
			{
				// save button states to flash (reload them by power cycling the hardware!)
				hal_write_flash(0, g_Buttons, BUTTON_COUNT);
			}
		}
		break;
	}
}

//______________________________________________________________________________

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
	// example - MIDI interface functionality for USB "MIDI" port -> DIN port
	if (port == USBMIDI)
	{
		hal_send_midi(DINMIDI, status, d1, d2);
	}

	// // example -MIDI interface functionality for DIN -> USB "MIDI" port port
	if (port == DINMIDI)
	{
		hal_send_midi(USBMIDI, status, d1, d2);
	}
}

//______________________________________________________________________________

void app_sysex_event(u8 port, u8 * data, u16 count)
{
	// example - respond to UDI messages?
}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
	return;
	
    // example - send poly aftertouch to MIDI ports
	hal_send_midi(USBMIDI, POLYAFTERTOUCH | 0, index, value);

    // example - set LED to white, brightness in proportion to pressure
	hal_plot_led(TYPEPAD, index, value/2, value/2, value/2);
}
	
//______________________________________________________________________________

void app_cable_event(u8 type, u8 value)
{
    // example - light the Setup LED to indicate cable connections
	if (type == MIDI_IN_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, 0, value, 0); // green
	}
	else if (type == MIDI_OUT_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, value, 0, 0); // red
	}
}

//______________________________________________________________________________

static const u8 ADC_MAP[] =
{
	11, 51, 12, 52, 13, 53, 14, 54,
	15, 55, 16, 56, 17 ,57, 18, 58,
	21, 61, 22, 62, 23, 63, 24, 64,
	25, 65, 26, 66, 27, 67, 28, 68,
	31, 71, 32, 72, 33, 73, 34, 74,
	35, 75, 36, 76, 37, 77, 38, 78,
	41, 81, 42, 82, 43, 83, 44, 84,
	45, 85, 46, 86, 47, 87, 48, 88,
};


void app_timer_event()
{
	// example - send MIDI clock at 125bpm
#define TICK_MS 20
	
	static u8 ms = TICK_MS;
	
	if (++ms >= TICK_MS)
	{
		ms = 0;
		
		// send a clock pulse up the USB
		hal_send_midi(USBMIDI, MIDITIMINGCLOCK, 0, 0);
	}
	
	// example: render raw ADC data as LEDs
	for (int i=0; i < PAD_COUNT; ++i)
	{
		// raw adc values are 12 bit, but LEDs are 6 bit.
		// so saturate into r;g;b for a rainbow effect to show pressure
		u16 r =0;
		u16 g =0;
		u16 b =0;
		
		u16 x = g_ADC[i];
		
		x = (3 * MAXLED * x) >> 12;
		
		if (x < MAXLED)
		{
			r = x;
		}
		else if (x > MAXLED && x < (2*MAXLED))
		{
			r = MAXLED - x;
			g = x - MAXLED;
		}
		else
		{
			g = MAXLED - x;
			b = x - MAXLED;
		}
		
		hal_plot_led(TYPEPAD, ADC_MAP[i], r, g, b);
	}
}

//______________________________________________________________________________

void app_init(const u16 *adc_raw)
{
	
	// example - store off the raw ADC frame pointer for later use
	g_ADC = adc_raw;
}
