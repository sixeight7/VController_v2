/****************************************************************************
**
** Copyright (C) 2015 Catrinus Feddema
** All rights reserved.
** This file is part of "VController v2" teensy software.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
****************************************************************************/

// Documentation on the bottom of this page

void setup() {
  setup_LED_control(); //Should be first, to reduce startup flash of LEDs
  setup_eeprom();
  setup_LCD_control();
  setup_debug();
  setup_switch_check();
  setup_switch_control();
  setup_page();
  setup_MIDI_common();
}

void loop() {
  main_switch_check(); // Check for switches pressed
  main_switch_control(); //If switch is pressed, take the configured action
  main_LED_control(); //Check update of LEDs
  main_LCD_control(); //Check update of displays
  main_MIDI_common(); //Read MIDI ports
  main_page(); // Check update of entire page
}

bool debug_active = true; // Switch debugging on or off here

void setup_debug() {
  if (debug_active) {
    Serial.begin(115200);
    delay(3000); // Wait until the serial communication is ready
    Serial.println("VController v2 started...");
  }
}

#define DEBUGMSG if (debug_active) Serial.println //Use this command for debug purposes

/****** Documentation VController V2 *********
VController version 2.

Hardware;
- 16 momentary switches connected in keypad matrix
- 16 neopixel LEDs
- 12 LCD displays for first 12 switches + main LCD display - all 16x2 character displays
- Teensy 3.1 (or Teensy LC with limited number of devices)
- 3 MIDI input/outputs and MIDI over USB
- Extra EEPROM Flash memory: 24LC512

Software features:
- Patch and parameter control for Boss GP-10, Roland GR-55, Roland VG-99 and Zoom G3
- Patchnames are read from devices
- Parameter states and assigns are read from devices
- Programmable pages of switches. There can be programmed on the Config page

Software structure:
- Initialization of LCDs, LEDs and MIDI ports
- Configuration of the switches is done in the page[] array on the Config page.
- On startup, detection of devices, page change or patch change the current page is loaded in the SP[] array (void load_page)
- After the page is loaded, the patchnames and parameter states are read (void Request_first_switch, Request_next_switch and Request_current_switch)
- Main loop checks for switches being pressed (Switch_check/Switch Control), midi received and update of LCD and LED states.

*/

