/****************************************************************************
**
** Copyright (C) 2015 Catrinus Feddema
** All rights reserved.
** This file is part of "VController" teensy software.
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

// Manages the 128 bytes EEPROM of Teensy LC

#include <EEPROM.h>

// ************************ Settings you probably won't have to change ***********************
// Define EEPROM addresses
#define EEPROM_GP10_PATCH_NUMBER 0
#define EEPROM_GP10_ALWAYS_ON 1
#define EEPROM_GR55_PATCH_MSB 2
#define EEPROM_GR55_PATCH_LSB 3
#define EEPROM_GR55_ALWAYS_ON 4
#define EEPROM_VG99_PATCH_MSB 5
#define EEPROM_VG99_PATCH_LSB 6
#define EEPROM_VG99_ALWAYS_ON 7
#define EEPROM_CURRENT_PAGE 8
#define EEPROM_BPM 9
#define EEPROM_VC_ON 10 //Is the unit on or off
#define EEPROM_MEMORY_BASE_ADDRESS 20 // Store stompbox LEDs from address 10 and higher
#define EEPROM_MEMORY_LOCATION_SIZE 6

void setup_eeprom()
{
  // Read data from EEPROM memory
  // read_eeprom_common_data();
}

void main_eeprom()
{

}

void read_eeprom_common_data() {
  GP10_patch_number = EEPROM.read(EEPROM_GP10_PATCH_NUMBER);
  GP10_always_on = EEPROM.read(EEPROM_GP10_ALWAYS_ON);
  GR55_patch_number = (EEPROM.read(EEPROM_GR55_PATCH_MSB) * 256) + EEPROM.read(EEPROM_GR55_PATCH_LSB);
  GR55_always_on = EEPROM.read(EEPROM_GR55_ALWAYS_ON);
  VG99_patch_number = (EEPROM.read(EEPROM_VG99_PATCH_MSB) * 256) + EEPROM.read(EEPROM_VG99_PATCH_LSB);
  VG99_always_on = EEPROM.read(EEPROM_VG99_ALWAYS_ON);
  Current_page = EEPROM.read(EEPROM_CURRENT_PAGE);
  bpm = EEPROM.read(EEPROM_BPM);
}

void write_eeprom_common_data() {
  EEPROM.write(EEPROM_GP10_PATCH_NUMBER, GP10_patch_number);
  EEPROM.write(EEPROM_GP10_ALWAYS_ON, GP10_always_on);
  EEPROM.write(EEPROM_GR55_PATCH_MSB, (GR55_patch_number / 256));
  EEPROM.write(EEPROM_GR55_PATCH_LSB, (GR55_patch_number % 256));
  EEPROM.write(EEPROM_GR55_ALWAYS_ON, GR55_always_on);
  EEPROM.write(EEPROM_VG99_PATCH_MSB, (VG99_patch_number / 256));
  EEPROM.write(EEPROM_VG99_PATCH_LSB, (VG99_patch_number % 256));
  EEPROM.write(EEPROM_VG99_ALWAYS_ON, VG99_always_on);
  EEPROM.write(EEPROM_CURRENT_PAGE, Current_page);
  EEPROM.write(EEPROM_BPM, bpm);
}

// Store data in memory location
void store_memory(uint8_t number) {
  uint8_t location = EEPROM_MEMORY_BASE_ADDRESS + (number * EEPROM_MEMORY_LOCATION_SIZE);
  EEPROM.write(location, GP10_patch_number); // Store GP10 patch number (0 - 99)
  EEPROM.write(location + 1, GR55_patch_number % 256); // Store GR55 patch number LSB (2 extra bits needed - patches go from 0 - 657)
  bool GR55_patchnumber_bit1 = (GR55_patch_number & 0x100); // Temporary remember the first bit
  bool GR55_patchnumber_bit2 = (GR55_patch_number & 0x200); // Temporary remember the second bit
  EEPROM.write(location + 2, VG99_patch_number % 256); // Store VG99 patch number LSB (1 extra bit needed - patches go from 0 - 399)
  bool VG99_patchnumber_bit1 = (VG99_patch_number & 0x100); // Temporary remember the first bit
  EEPROM.write(location + 3, bpm); // Store the bpm
  uint8_t booleans1 = (GR55_patchnumber_bit1) | (GR55_patchnumber_bit2 << 1) | (VG99_patchnumber_bit1 << 2) | (GP10_on << 3) | (GP10_always_on << 4) | (GR55_on << 5) | (GR55_always_on << 6) | (VG99_on << 7);
  uint8_t booleans2 = (GP10_COSM_onoff) | (GP10_nrml_pu_onoff << 1) | (GR55_synth1_onoff << 2) | (GR55_synth2_onoff << 3) | (GR55_COSM_onoff << 4) | (GR55_nrml_pu_onoff << 5) | (VG99_COSM_A_onoff << 6) | (VG99_COSM_B_onoff << 7);
  EEPROM.write(location + 4, booleans1); // Store the boolean values
  EEPROM.write(location + 5, booleans2); // Store the boolean values
  //show_status_message("Memory " + String(number + 1) + " stored ");
  DEBUGMSG("write: B" + String(booleans1, BIN) + ", B" + String(booleans2, BIN));
}

// Store data in memory location
void read_memory(uint8_t number) {
  uint8_t location = EEPROM_MEMORY_BASE_ADDRESS + (number * EEPROM_MEMORY_LOCATION_SIZE);
  uint8_t booleans1 = EEPROM.read(location + 4); // Read the boolean values
  uint8_t booleans2 = EEPROM.read(location + 5); // Read the boolean values

  bool GR55_patchnumber_bit1 = (booleans1 & 1);
  bool GR55_patchnumber_bit2 = (booleans1 & 2);
  bool VG99_patchnumber_bit1 = (booleans1 & 4);
  GP10_on = (booleans1 & 8);
  GP10_always_on = (booleans1 & 16);
  GR55_on = (booleans1 & 32);
  GR55_always_on = (booleans1 & 64);
  VG99_on = (booleans1 & 128);

  GP10_COSM_onoff = (booleans2 & 1);
  GP10_nrml_pu_onoff = (booleans2 & 2) >> 1;
  GR55_synth1_onoff = (booleans2 & 4) >> 2;
  GR55_synth2_onoff = (booleans2 & 8) >> 3;
  GR55_COSM_onoff = (booleans2 & 16) >> 4;
  GR55_nrml_pu_onoff = (booleans2 & 32) >> 5;
  VG99_COSM_A_onoff = (booleans2 & 64) >> 6;
  VG99_COSM_B_onoff = (booleans2 & 128) >> 7;

  DEBUGMSG("read: B" + String(booleans1, BIN) + ", B" + String(booleans2, BIN));
  
  bpm = EEPROM.read(location + 3);
  VG99_patch_number = EEPROM.read(location + 2) + (VG99_patchnumber_bit1 << 8);
  GR55_patch_number = EEPROM.read(location + 1) + (GR55_patchnumber_bit1 << 8) + (GR55_patchnumber_bit2 << 9);
  GP10_patch_number = EEPROM.read(location);
}

