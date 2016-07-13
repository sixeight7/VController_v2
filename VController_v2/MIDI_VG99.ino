// ******************************** MIDI messages and functions for the Roland VG-99 and the FC300 ********************************

// Note: when connecting the VG-99 to the RRC connector make sure that you make the following settings:
// 1) Go to SYSTEM / MIDI / PAGE 4 on the VG99
// 2) Switch on RRC -> main (F1)
// 3) Switch off RRC <- main (F2 and F3)
// The reason is that the VG99 sends back patch changes to the VController, which make the system unresponsive.
// I cannot trace this back to the source, but it may be a firmware error on the VG-99.

// ********************************* Section 1: VG99/FC300 SYSEX messages ********************************************

//boolean VG99_FC300_mode = false; // If the FC300 is attached, patch names will be sent in FC300 SYSEX messages

//Sysex messages for the VG-99
#define VG99_REQUEST_CURRENT_PATCH_NAME 0x60000000, 16 //Request 16 bytes for current patch name
#define VG99_REQUEST_PATCH_NUMBER 0x71000100, 2 //Request current patch number

#define VG99_EDITOR_MODE_ON 0x70000100, 0x01 //Gets the VG-99 spitting out lots of sysex data. Does not have to be switched on for the tuner to work on the VG99
#define VG99_EDITOR_MODE_OFF 0x70000100, 0x00
#define VG99_TUNER_ON 0x70000000, 0x01 // Changes the running mode to tuner / multi-mode
#define VG99_TUNER_OFF 0x70000000, 0x00 // Changes the running mode to play
#define VG99_TAP_TEMPO_LED_CC 80 // If you have the D-beam switched off on the VG99, make an assign on the VG99: Source CC #80, momentary, target D BEAM - SELECT - Assignable / off on every patch and you will have a flashing LED. 
// Set to zero and VController will send no CC message. You can also set the cc on the VG99 to V-link sw, but that generates a lot of midi data.

#define VG99_PATCH_CHANGE 0x71000000 //00 00 Patch 001 and 03 0F Patch 400
#define VG99_TEMPO 0x60000015  // Accepts values from 40 bpm - 250 bpm
#define VG99_KEY 0x60000017    // Accepts values from 0 (C) - 11 (B)

#define VG99_COSM_GUITAR_A_SW 0x60003000 // The address of the COSM guitar switch
#define VG99_COSM_GUITAR_B_SW 0x60003800 // The address of the COSM guitar switch
bool VG99_request_onoff = false;

//Sysex messages for FC300 in sysex mode:
//I am trying to fool the VG-99 to believe there is an FC300 attached, but it does not work.
#define FC300_TUNER_ON 0x1002, 0x01 // Does not work for some reason
#define FC300_TUNER_OFF 0x1002, 0x00
#define FC300_SYSEX_MODE 0x1000, 0x01 //Tell the VG-99 we are in sysex mode
#define FC300_NORMAL_MODE 0x1000, 0x00

uint8_t VG99_MIDI_port = 0;
uint8_t VG99_current_assign = 255; // The assign that is being read - set to a high value, because it is not time to read an assign yet.

#define VG99_SYSEX_WATCHDOG_LENGTH 1000 // watchdog for messages (in msec)
unsigned long VG99sysexWatchdog = 0;
boolean VG99_sysex_watchdog_running = false;

#define VG99_SYSEX_DELAY_LENGTH 10 // time between sysex messages (in msec)
unsigned long VG99sysexDelay = 0;

bool VG99_read_assign_target = false;

// VG99 handshake with FC300
// 1. VG99 sends continually F0 41 7F 00 00 1F 11 00 01 7F F7 on RRC port (not on other ports)
// 2. FC300 responds with ???

// ********************************* Section 2: VG99 common MIDI in functions ********************************************

void check_SYSEX_in_VG99(const unsigned char* sxdata, short unsigned int sxlength) {  // Check incoming sysex messages from VG99. Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a VG-99
  if ((sxdata[2] == VG99_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x1C) && (sxdata[6] == 0x12)) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    // Check if it is the patch number
    if (address == 0x71000100) {
      if (VG99_patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        VG99_patch_number = sxdata[11] * 128 + sxdata[12];
        VG99_page_check();
        VG99_do_after_patch_selection();
      }
    }

    // Check if it is the current parameter
    if (address == SP[Current_switch].Address) {
      switch (SP[Current_switch].Type) {
        case VG99_PATCH:
        case VG99_RELSEL:
          for (uint8_t count = 0; count < 16; count++) {
            SP[Current_switch].Label[count] = static_cast<char>(sxdata[count + 11]); //Add ascii character to the SP.Label String
          }
          //update_lcd = Current_switch + 1;
          if (SP[Current_switch].PP_number == VG99_patch_number) {
            VG99_patch_name = SP[Current_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[Current_switch].Label);
          Request_next_switch();
          break;
        case VG99_PARAMETER:
          VG99_read_parameter(sxdata[11], sxdata[12]);
          Request_next_switch();
          break;
        case VG99_ASSIGN:
          if (VG99_read_assign_target == false) VG99_read_current_assign(address, sxdata, sxlength);
          else {
            VG99_read_parameter(sxdata[11], sxdata[12]); //Reading the assign target is equal to readig the parameter
            Request_next_switch();
          }
          break;
      }
    }
    
    // Check if it is the current patch name (address: 0x60, 0x00, 0x00, 0x00)
    if ((sxdata[6] == 0x12) && (address == 0x60000000) ) {
      VG99_patch_name = "";
      for (uint8_t count = 11; count < 28; count++) {
        VG99_patch_name = VG99_patch_name + static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
    }

    // Check if it is the guitar on/off states
    VG99_check_guitar_switch_states(sxdata, sxlength);

    // Check if it is some other stompbox function and copy the status to the right LED
    //VG99_check_stompbox_states(sxdata, sxlength);
  }
}

void check_SYSEX_in_VG99fc(const unsigned char* sxdata, short unsigned int sxlength) { // Check incoming sysex messages from VG99/FC300. Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a VG-99 in FC300 mode.
  if ((sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x20)) {
    uint16_t address = (sxdata[7] << 8) + sxdata[8]; // Make the address 16 bit

    // Check if it is a data request - here we have to fool the VG99 into believing there is an FC300 attached
    // I found the numbers by packet sniffing the communication between the two devices. The addresses are not mentioned in the FC300 manual.

    if (sxdata[6] == 0x11) {
      if (address == 0x0000) { // Request for address 0x0000
        //        VG99_FC300_mode = true; //We are in FC300 mode!!!
        write_VG99fc(0x0000, 0x01, 0x00, 0x00); // Answer with three numbers - 01 00 00
        DEBUGMSG("VG99 request for address 0x0000 answered");
      }
      if (address == 0x0400) { // Request for address 0x0400
        //write_VG99fc(0x0400, 0x03); // Answer with one number - 03
        DEBUGMSG("VG99 request for address 0x0400 answered");
      }
      if (address == 0x0600) { // Request for address 0x0600
        write_VG99fc(0x0600, 0x10, 0x02, 0x08); // Answer with three numbers - 10 02 08
        DEBUGMSG("VG99 request for address 0x0600 answered");
        VG99_fix_reverse_pedals();
      }
    }
  }
}

void check_PC_in_VG99(byte channel, byte program) { // Check incoming PC messages from VG99. Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((Current_MIDI_port == VG99_MIDI_port) && (channel == VG99_MIDI_channel)) { // VG99 outputs a program change
    uint16_t new_patch = (VG99_CC01 * 100) + program;
    if (VG99_patch_number != new_patch) {
      VG99_patch_number = new_patch;
      request_VG99(VG99_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      VG99_page_check();
      VG99_do_after_patch_selection();
    }
  }
}

void VG99_identity_check(const unsigned char* sxdata, short unsigned int sxlength) {

  // Check if it is a VG-99
  if ((sxdata[6] == 0x1C) && (sxdata[7] == 0x02) && (VG99_detected == false)) {
    VG99_detected = true;
    show_status_message("VG-99 detected  ");
    VG99_device_id = sxdata[2]; //Byte 2 contains the correct device ID
    VG99_MIDI_port = Current_MIDI_port; // Set the correct MIDI port for this device
    DEBUGMSG("VG-99 detected on MIDI port " + String(Current_MIDI_port));
    //write_VG99(VG99_EDITOR_MODE_ON); // Put the VG-99 into editor mode - saves lots of messages on the VG99 display, but also overloads the buffer
    //VG99_fix_reverse_pedals();
    request_VG99(VG99_REQUEST_PATCH_NUMBER);
    request_VG99(VG99_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      VG99_do_after_patch_selection();
    load_current_page(true);
  }
}

// ********************************* Section 3: VG99 common MIDI out functions ********************************************

void VG99_check_sysex_delay() { // Will delay if last message was within VG99_SYSEX_DELAY_LENGTH (10 ms)
  while (millis() - VG99sysexDelay <= VG99_SYSEX_DELAY_LENGTH) {}
  VG99sysexDelay = millis();
}

void write_VG99(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, VG99_device_id, 0x00, 0x00, 0x1C, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  VG99_check_sysex_delay();
  if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(14, sysexmessage);
  if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(13, sysexmessage);
  if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(13, sysexmessage);
  if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(13, sysexmessage);
  debug_sysex(sysexmessage, 14, "out(VG99)");
}

void write_VG99(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, VG99_device_id, 0x00, 0x00, 0x1C, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  VG99_check_sysex_delay();
  if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(15, sysexmessage);
  if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(14, sysexmessage);
  if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(14, sysexmessage);
  if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(14, sysexmessage);
  debug_sysex(sysexmessage, 15, "out(VG99)");
}

void write_VG99fc(uint16_t address, uint8_t value) { // VG99 writing to the FC300

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into two bytes: ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[12] = {0xF0, 0x41, FC300_device_id, 0x00, 0x00, 0x020, 0x12, ad[1], ad[0], value, checksum, 0xF7};
  VG99_check_sysex_delay();
  if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(12, sysexmessage);
  if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(11, sysexmessage);
  if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(11, sysexmessage);
  if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(11, sysexmessage);
  debug_sysex(sysexmessage, 12, "out(VG99fc)");
}

void write_VG99fc(uint16_t address, uint8_t value1, uint8_t value2, uint8_t value3) { // VG99 writing to the FC300 - 3 bytes version

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into two bytes: ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[1] + ad[0] + value1 + value2 + value3); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, FC300_device_id, 0x00, 0x00, 0x020, 0x12, ad[1], ad[0], value1, value2, value3, checksum, 0xF7};
  VG99_check_sysex_delay();
  if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(14, sysexmessage);
  if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(13, sysexmessage);
  if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(13, sysexmessage);
  if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(13, sysexmessage);
  debug_sysex(sysexmessage, 14, "out(VG99fc)");
}

void request_VG99(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no_of_bytes); // Calculate the Roland checksum
  uint8_t sysexmessage[17] = {0xF0, 0x41, VG99_device_id, 0x00, 0x00, 0x1C, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, 0x00, no_of_bytes, checksum, 0xF7};
  VG99_check_sysex_delay();
  if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(17, sysexmessage);
  if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(16, sysexmessage);
  if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(16, sysexmessage);
  if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(16, sysexmessage);
  debug_sysex(sysexmessage, 17, "out(VG99)");
}

void VG99_request_patch_number() {
  request_VG99(VG99_REQUEST_PATCH_NUMBER);
}

void VG99_request_name() {
  //request_VG99(VG99_REQUEST_PATCH_NAME);
}

void VG99_send_bpm() {
  write_VG99(VG99_TEMPO, (bpm - 40) / 128, (bpm - 40) % 128); // Tempo is modulus 128 on the VG99. And sending 0 gives tempo 40.
}

void VG99_TAP_TEMPO_LED_ON() {
  if ((VG99_TAP_TEMPO_LED_CC > 0) && (VG99_detected)) {
    if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendControlChange(VG99_TAP_TEMPO_LED_CC , 127, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendControlChange(VG99_TAP_TEMPO_LED_CC , 127, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendControlChange(VG99_TAP_TEMPO_LED_CC , 127, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendControlChange(VG99_TAP_TEMPO_LED_CC , 127, VG99_MIDI_channel);
  }
}

void VG99_TAP_TEMPO_LED_OFF() {
  if ((VG99_TAP_TEMPO_LED_CC > 0) && (VG99_detected)) {
    if (VG99_MIDI_port == USBMIDI_PORT) usbMIDI.sendControlChange(VG99_TAP_TEMPO_LED_CC , 0, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI1_PORT) MIDI1.sendControlChange(VG99_TAP_TEMPO_LED_CC , 0, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI2_PORT) MIDI2.sendControlChange(VG99_TAP_TEMPO_LED_CC , 0, VG99_MIDI_channel);
    if (VG99_MIDI_port == MIDI3_PORT) MIDI3.sendControlChange(VG99_TAP_TEMPO_LED_CC , 0, VG99_MIDI_channel);
  }
}
// ********************************* Section 4: VG99 program change ********************************************

//uint8_t VG99_patch_memory = 0;

void VG99_SendProgramChange(uint16_t new_patch) {
  //if (new_patch == VG99_patch_number) VG99_unmute();
  VG99_patch_number = new_patch;

  Send_CC(0, new_patch / 100, VG99_MIDI_channel, VG99_MIDI_port);
  Send_PC(new_patch % 100, VG99_MIDI_channel, VG99_MIDI_port);
  DEBUGMSG("out(VG99) PC" + String(new_patch)); //Debug
  //GP10_mute();
  //GR55_mute();
  VG99_do_after_patch_selection();
}

void VG99_do_after_patch_selection() {
  VG99_request_onoff = false;
  VG99_on = true;
  if (SEND_GLOBAL_TEMPO_AFTER_PATCH_CHANGE == true) VG99_send_bpm();
  Current_patch_number = VG99_patch_number; // the patch name
  Current_device = VG99;
  update_LEDS = true;
  update_main_lcd = true;
  update_switch_lcds = PARAMETERS;
  VG99_request_guitar_switch_states();
  //EEPROM.write(EEPROM_VG99_PATCH_MSB, (VG99_patch_number / 256));
  //EEPROM.write(EEPROM_VG99_PATCH_LSB, (VG99_patch_number % 256));
}

void VG99_relsel_load(uint8_t sw, uint8_t bank_position, uint8_t bank_size) {
  if (VG99_bank_selection_active == false) VG99_bank_number = (VG99_patch_number / bank_size);
  uint16_t new_patch = (VG99_bank_number * bank_size) + bank_position;
  if (new_patch > VG99_PATCH_MAX) new_patch = new_patch - VG99_PATCH_MAX - 1 + VG99_PATCH_MIN;
  VG99_patch_load(sw, new_patch); // Calculate the patch number for this switch
}

void VG99_patch_load(uint8_t sw, uint16_t number) {
  SP[sw].Colour = VG99_PATCH_COLOUR;
  SP[sw].PP_number = number;
  SP[sw].Address = 0x71010000 + (((number * 0x10) / 0x80) * 0x100) + ((number * 0x10) % 0x80); //Calculate the address where the patchname is stored on the VG-99
}
void VG99_patch_select(uint16_t new_patch) {

  //if (my_switch <= bank_size) {
  //if (VG99_bank_selection_active == false) VG99_bank_number = (VG99_patch_number / bank_size); //Reset the bank to current patch
  //uint16_t new_patch = (VG99_bank_number) * bank_size + (my_switch - 1);
  if (new_patch == VG99_patch_number) VG99_select_switch();
  else VG99_SendProgramChange(new_patch);
  VG99_bank_selection_active = false;
  //}
}

void VG99_bank_updown(bool updown, uint16_t bank_size) {
  uint16_t bank_number = (VG99_patch_number / bank_size);

  if (VG99_bank_selection_active == false) {
    VG99_bank_selection_active = true;
    VG99_bank_number = bank_number; //Reset the bank to current patch
    VG99_bank_size = bank_size;
  }
  // Perform bank up:
  if (updown == UP) {
    if (VG99_bank_number >= ((VG99_PATCH_MAX / bank_size) - 1)) VG99_bank_number = (VG99_PATCH_MIN / bank_size); // Check if we've reached the top
    else VG99_bank_number++; //Otherwise move bank up
  }
  // Perform bank down:
  if (updown == DOWN) {
    if ((VG99_bank_number * bank_size) <= VG99_PATCH_MIN) VG99_bank_number = VG99_PATCH_MAX / bank_size; // Check if we've reached the bottom
    else VG99_bank_number--; //Otherwise move bank down
  }

  if (VG99_bank_number == bank_number) VG99_bank_selection_active = false; //Check whether were back to the original bank

  load_current_page(true); //Re-read the patchnames for this bank
}

void VG99_page_check() { // Checks if the current patch is on the page and will reload the page if not
  bool onpage = false;
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    if ((SP[s].Type == VG99_RELSEL) && (SP[s].PP_number == VG99_patch_number)) {
      onpage = true;
      VG99_patch_name = SP[s].Label; // Set patchname correctly
    }
  }
  if (!onpage) load_current_page(true);
}

// ** US-20 simulation
// Selecting and muting the VG99 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the VG99 is selected

void VG99_request_guitar_switch_states() {
  //VG99_select_LED = VG99_PATCH_COLOUR; //Switch the LED on
  request_VG99(VG99_COSM_GUITAR_A_SW, 1);
  request_VG99(VG99_COSM_GUITAR_B_SW, 1);
  VG99_request_onoff = true;
}

void VG99_check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (VG99_request_onoff == true) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    if (address == VG99_COSM_GUITAR_A_SW) {
      VG99_COSM_A_onoff = sxdata[11];  // Store the value
    }

    if (address == VG99_COSM_GUITAR_B_SW) {
      VG99_COSM_B_onoff = sxdata[11];  // Store the value
      VG99_request_onoff = false;
    }
  }
}

void VG99_select_switch() {
  Current_device = VG99;
  if (VG99_on) {
    VG99_always_on_toggle();
  }
  else {
    VG99_unmute();
    //GP10_mute();
    //GR55_mute();
    //if (mode != MODE_VG99_VG99_COMBI) show_status_message(VG99_patch_name); // Show the correct patch name
  }
}

void VG99_always_on_toggle() {
  if (US20_emulation_active) {
    VG99_always_on = !VG99_always_on; // Toggle VG99_always_on
    if (VG99_always_on) {
      VG99_unmute();
      show_status_message("VG99 always ON");
    }
    else {
      //VG99_mute();
      show_status_message("VG99 can be muted");
    }
  }
}


void VG99_unmute() {
  VG99_on = true;
  //VG99_select_LED = VG99_PATCH_COLOUR; //Switch the LED on
  //write_VG99(VG99_COSM_GUITAR_A_SW, VG99_COSM_A_onoff); // Switch COSM guitar on
  //write_VG99(VG99_COSM_GUITAR_B_SW, VG99_COSM_B_onoff); // Switch normal pu on
  VG99_SendProgramChange(VG99_patch_number); //Just sending the program change will put the sound back on
}

void VG99_mute() {
  if ((US20_emulation_active) && (!VG99_always_on)) {
    VG99_mute_now();
  }
}

void VG99_mute_now() {
  VG99_on = false;
  //  VG99_select_LED = VG99_OFF_COLOUR; //Switch the LED off
  write_VG99(VG99_COSM_GUITAR_A_SW, 0x00); // Switch COSM guitar off
  write_VG99(VG99_COSM_GUITAR_B_SW, 0x00); // Switch normal pu off
}

// ********************************* Section 5: VG99 parameter and assign control ********************************************

// Procedures for the VG99_PARAMETER and VG99_ASSIGN commands

// Procedures for the VG99_PARAMETER:
// 1. Load in SP array - in load_current_page(true)
// 2. Request parameter state - in Request_current_switch()
// 3. Read parameter state - VG99_read_parameter() below
// 4. Press switch - VG99_parameter_press() below - also calls VG99_check_update_label()
// 5. Release switch - VG99_parameter_release() below - also calls VG99_check_update_label()

struct VG99_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address; // The address of the parameter
  char Name[17]; // The name for the label
  uint8_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
};

//typedef struct stomper Stomper;
#define VG99_FX_COLOUR 255 // Just a colour number to pick the colour from the VG99_FX_colours table
#define VG99_FX_TYPE_COLOUR 254 //Another number for the MFX type
#define VG99_POLYFX_COLOUR 253 // Just a colour number to pick the colour from the VG99_POLY_FX_colours table
#define VG99_POLYFX_TYPE_COLOUR 252 //Another number for the MOD type

// Make sure you edit the VG99_NUMBER_OF_STOMPS when adding new parameters
#define VG99_NUMBER_OF_PARTS 8
#define VG99_NUMBER_OF_PARAMETERS 28
const PROGMEM VG99_parameter_struct VG99_parameters[VG99_NUMBER_OF_PARTS][VG99_NUMBER_OF_PARAMETERS] = {
  { // part 0: 0000 - 1000 Tunings
    {0x0017, "KEY", 0, FX_PITCH_COLOUR}, // Parameter number: 0
    {0x0015, "BPM", 0, FX_DELAY_COLOUR},
    {0x0024, "FC AMP CTL1", 0, FX_AMP_COLOUR},
    {0x0025, "FC AMP CTL2", 0, FX_AMP_COLOUR},
    {0x0D1E, "D BM SELECT", 0, FX_PITCH_COLOUR},
    {0x0D1F, "D BM PITCH TYP", 0, FX_PITCH_COLOUR}, // Parameter number: 5
    {0x0D20, "D BM T-ARM CH", 0, FX_PITCH_COLOUR},
    {0x0D21, "D BM T-ARM TYP", 0, FX_PITCH_COLOUR},
    {0x0D26, "D BM FREEZE CH", 0, VG99_STOMP_COLOUR},
    {0x0D29, "D BM FRZ(A) LVL", 0, VG99_STOMP_COLOUR},
    {0x0D2D, "D BM FRZ(B) LVL", 0, VG99_STOMP_COLOUR}, // Parameter number: 10
    {0x0D2F, "D BM FILTER CH", 0, FX_FILTER_COLOUR},
    {0x0D30, "D BM FLTR TYPE", 0, FX_FILTER_COLOUR},
    {0x0D34, "D BM FILTR LVL", 0, FX_FILTER_COLOUR},
    {0x0D35, "RIBBON SELECT", 0, VG99_STOMP_COLOUR},
    {0x0D36, "RBBN T-ARM CH", 0, FX_PITCH_COLOUR}, // Parameter number: 15
    {0x0D37, "RBBN T-ARM TYPE", 0, FX_PITCH_COLOUR},
    {0x0D3C, "RBBN FILTER CH", 0, FX_FILTER_COLOUR},
    {0x0D3D, "RBBN FILTER TYP", 0, FX_FILTER_COLOUR},
    {0x0D41, "RBBN FILTER LVL", 0, FX_FILTER_COLOUR}
  },

  { // part 1: 1000 - 2000 Alt tuning parameters
    {0x1001, "[A]TU", 5, FX_PITCH_COLOUR}, // Parameter number: 30
    {0x1002, "[A]TUNING TYPE", 105, FX_PITCH_COLOUR},
    {0x1007, "[A]BEND SW", 0, FX_PITCH_COLOUR},
    {0x1004, "[A]12STRING SW", 0, FX_PITCH_COLOUR},
    {0x1003, "[A]DETUNE SW", 0, FX_PITCH_COLOUR},
    {0x1005, "[A]HMNY", 3, FX_PITCH_COLOUR}, // Parameter number: 35
    {0x1006, "[A]HARMO", 103, FX_PITCH_COLOUR},
    {0x1009, "[B]TU", 5, FX_PITCH_COLOUR},
    {0x100A, "[B]TUNING TYPE", 0, FX_PITCH_COLOUR},
    {0x100F, "[B]BEND SW", 0, FX_PITCH_COLOUR},
    {0x100C, "[B]12STRING SW", 0, FX_PITCH_COLOUR}, // Parameter number: 40
    {0x100B, "[B]DETUNE SW", 0, FX_PITCH_COLOUR},
    {0x100D, "[B]HMNY", 3, FX_PITCH_COLOUR},
    {0x100E, "[B]HARMO", 0, FX_PITCH_COLOUR}
  },

  { // part 2: 2000 - 3000 Common parameters
    {0x2007, "DYNA SW", 0, VG99_STOMP_COLOUR}, // Parameter number: 60
    {0x2008, "DYNA TYPE", 0, VG99_STOMP_COLOUR},
    {0x2014, "[A]MIXER PAN", 0, VG99_STOMP_COLOUR},
    {0x2015, "[A]MIXER LVL", 0, VG99_STOMP_COLOUR},
    {0x2019, "[A]MIX SW", 0, VG99_STOMP_COLOUR},
    {0x201A, "[B]MIXER PAN", 0, VG99_STOMP_COLOUR}, // Parameter number: 65
    {0x201B, "[B]MIXER LVL", 0, VG99_STOMP_COLOUR},
    {0x201F, "[B]MIX SW", 0, VG99_STOMP_COLOUR},
    {0x202F, "TOTAL EQ SW", 0, VG99_STOMP_COLOUR},
    {0x2013, "A/B BAL", 0, VG99_STOMP_COLOUR},
    {0x2000, "PATCH LEVEL", 0, VG99_STOMP_COLOUR}, // Parameter number: 70
    {0x2020, "D/R REVERB SW", 0, FX_REVERB_COLOUR},
    {0x2021, "D/R REVRB TYPE", 0, FX_REVERB_COLOUR},
    {0x2022, "D/R REVRB TIME", 0, FX_REVERB_COLOUR},
    {0x2023, "D/R RVRB PREDY", 0, FX_REVERB_COLOUR},
    {0x2024, "D/R RVRB LWCUT", 0, FX_REVERB_COLOUR}, // Parameter number: 75
    {0x2025, "D/R RVRB HICUT", 0, FX_REVERB_COLOUR},
    {0x2026, "D/R REVRB DENS", 0, FX_REVERB_COLOUR},
    {0x2027, "D/R REVERB LVL", 0, FX_REVERB_COLOUR},
    {0x2028, "D/R DELAY SW", 0, FX_DELAY_COLOUR},
    {0x2029, "D/R DELAY TIME", 0, FX_DELAY_COLOUR}, // Parameter number: 80
    {0x202B, "D/R DLAY FDBCK", 0, FX_DELAY_COLOUR},
    {0x202C, "D/R DLAY HICUT", 0, FX_DELAY_COLOUR},
    {0x202D, "D/R DLAY LEVEL", 0, FX_DELAY_COLOUR}
  },

  { // part 3: 3000 - 4000 Guitar parameters
    {0x3000, "[A]COSM GTR SW", 0, FX_GTR_COLOUR}, // Parameter number: 90
    {0x3001, "[A]MODEL TYPE", 0, FX_GTR_COLOUR},
    {0x301B, "[A]E.GTR TYPE", 0, FX_GTR_COLOUR},
    {0x301E, "[A]PU SEL", 0, FX_GTR_COLOUR},
    {0x301F, "[A]E.GTR VOL", 0, FX_GTR_COLOUR},
    {0x3022, "[A]E.GTR TONE", 0, FX_FILTER_COLOUR}, // Parameter number: 95
    {0x3045, "[A]AC TYPE", 0, FX_GTR_COLOUR},
    {0x3046, "[A]BODY TYPE", 0, FX_GTR_COLOUR},
    {0x3069, "[A]BASS TYPE", 0, FX_GTR_COLOUR},
    {0x3400, "[A]SYNTH TYPE", 0, FX_GTR_COLOUR},
    {0x3002, "[A]GTR EQ SW", 0, FX_FILTER_COLOUR}, // Parameter number: 100
    {0x3018, "[A]COSM LEVEL", 0, FX_GTR_COLOUR},
    {0x301A, "[A]NORM PU LEVEL", 0, FX_GTR_COLOUR},
    {0x3457, "[A]NS SW", 0, FX_GTR_COLOUR},
    {0x3800, "[B]COSM GTR SW", 0, FX_GTR_COLOUR},
    {0x3801, "[B]MODEL TYPE", 0, FX_GTR_COLOUR}, // Parameter number: 105
    {0x381B, "[B]E.GTR TYPE", 0, FX_GTR_COLOUR},
    {0x381E, "[B]PU SEL", 0, FX_GTR_COLOUR},
    {0x381F, "[B]E.GTR VOL", 0, FX_GTR_COLOUR},
    {0x3822, "[B]E.GTR TONE", 0, FX_FILTER_COLOUR},
    {0x3845, "[B]AC TYPE", 0, FX_GTR_COLOUR}, // Parameter number: 110
    {0x3846, "[B]BODY TYPE", 0, FX_GTR_COLOUR},
    {0x3869, "[B]BASS TYPE", 0, FX_GTR_COLOUR},
    {0x3C00, "[B]SYNTH TYPE", 0, FX_GTR_COLOUR},
    {0x3802, "[B]GTR EQ SW", 0, FX_FILTER_COLOUR},
    {0x3818, "[B]COSM LEVEL", 0, FX_GTR_COLOUR}, // Parameter number: 115
    {0x381A, "[B]NORM PU LEVEL", 0, FX_GTR_COLOUR},
    {0x3C57, "[B]NS SW", 0, FX_FILTER_COLOUR}
  },

  { // part 4: 4000 - 5000 Poly FX
    {0x4000, "POLY FX CHAN", 0, FX_FILTER_COLOUR}, // Parameter number: 120
    {0x4001, "POLY", 6, VG99_POLYFX_COLOUR},
    {0x4002, "POLY FX TYPE", 106, VG99_POLYFX_TYPE_COLOUR},
    {0x4003, "POLY COMP TYPE", 0, FX_FILTER_COLOUR},
    {0x4004, "POLY COMP SUSTN", 0, FX_FILTER_COLOUR},
    {0x4005, "POLY COMP ATTACK", 0, FX_FILTER_COLOUR}, // Parameter number: 125
    {0x4006, "POLY COMP THRSH", 0, FX_FILTER_COLOUR},
    {0x4007, "POLY COMP REL", 0, FX_FILTER_COLOUR},
    {0x4008, "POLY COMP TONE", 0, FX_FILTER_COLOUR},
    {0x4009, "POLY COMP LEVEL", 0, FX_FILTER_COLOUR},
    {0x400A, "POLY COMP BAL", 0, FX_FILTER_COLOUR}, // Parameter number: 130
    {0x400B, "POLY DIST MODE", 0, FX_DIST_COLOUR},
    {0x400C, "POLY DIST DRIVE", 0, FX_DIST_COLOUR},
    {0x400D, "POLY D HIGH CUT", 0, FX_DIST_COLOUR},
    {0x400E, "POLY D POLY BAL", 0, FX_DIST_COLOUR},
    {0x400F, "POLY D DRIVE BAL", 0, FX_DIST_COLOUR}, // Parameter number: 135
    {0x4010, "POLY DIST LEVEL", 0, FX_DIST_COLOUR},
    {0x402F, "POLY SG RISETIME", 0, FX_MODULATE_COLOUR},
    {0x4030, "POLY SG SENS", 0, FX_MODULATE_COLOUR},
  },

  { // part 5: 5000 - 6000 FX and amps chain A
    {0x502B, "[A]COMP SW", 0, FX_FILTER_COLOUR}, // Parameter number: 150
    {0x502C, "[A]COMP TYPE", 0, FX_FILTER_COLOUR},
    {0x5033, "[A]OD", 1, FX_DIST_COLOUR}, // Check VG99_odds_type table (1)
    {0x5034, "[A]OD / DS TYPE", 0, FX_DIST_COLOUR},
    {0x503F, "[A]WAH SW", 0, FX_FILTER_COLOUR},
    {0x5040, "[A]WAH TYPE", 0, FX_FILTER_COLOUR}, // Parameter number: 155
    {0x5048, "[A]EQ SW", 0, FX_FILTER_COLOUR},
    {0x5054, "[A]DELAY SW", 0, FX_DELAY_COLOUR},
    {0x5055, "[A]DELAY TYPE", 0, FX_DELAY_COLOUR},
    {0x506D, "[A]CHORUS SW", 0, FX_MODULATE_COLOUR},
    {0x506E, "[A]CHORUS MODE", 0, FX_MODULATE_COLOUR}, // Parameter number: 160
    {0x5075, "[A]REVERB SW", 0, FX_REVERB_COLOUR},
    {0x5076, "[A]REVERB TYPE", 0, FX_REVERB_COLOUR},
    {0x5400, "[A]M1", 2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2)
    {0x5401, "[A]M1 TYPE", 102, VG99_FX_TYPE_COLOUR},
    {0x5800, "[A]M2", 2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2) // Parameter number: 165
    {0x5801, "[A]M2 TYPE", 102, VG99_FX_TYPE_COLOUR},
    {0x507E, "[A]NS SW", 0, FX_FILTER_COLOUR},
    {0x500D, "[A]AMP", 4, FX_AMP_COLOUR},
    {0x500E, "[A]AMP TYPE", 104, FX_AMP_COLOUR},
    {0x500F, "[A]AMP GAIN", 0, FX_AMP_COLOUR}, // Parameter number: 170
    {0x5015, "[A]AMP BRIGHT", 0, FX_AMP_COLOUR},
    {0x5016, "[A]AMP GAIN SW", 0, FX_AMP_COLOUR},
    {0x5017, "[A]AMP(SOLO) SW", 0, FX_AMP_COLOUR},
    {0x5019, "[A]AMP SP TYPE", 0, FX_AMP_COLOUR}
  },

  { // part 6: 6000 - 7000 FX and amps chain B
    {0x602B, "[B]COMP SW", 0, FX_FILTER_COLOUR}, // Parameter number: 180
    {0x602C, "[B]COMP TYPE", 0, FX_FILTER_COLOUR},
    {0x6033, "[B]OD", 1, FX_DIST_COLOUR}, // Check VG99_odds_type table (1)
    {0x6034, "[B]OD / DS TYPE", 0, FX_DIST_COLOUR},
    {0x603F, "[B]WAH SW", 0, FX_FILTER_COLOUR},
    {0x6040, "[B]WAH TYPE", 0, FX_FILTER_COLOUR}, // Parameter number: 185
    {0x6048, "[B]EQ SW", 0, FX_FILTER_COLOUR},
    {0x6054, "[B]DELAY SW", 0, FX_DELAY_COLOUR},
    {0x6055, "[B]DELAY TYPE", 0, FX_DELAY_COLOUR},
    {0x606D, "[B]CHORUS SW", 0, FX_MODULATE_COLOUR},
    {0x606E, "[B]CHORUS MODE", 0, FX_MODULATE_COLOUR}, // Parameter number: 190
    {0x6075, "[A]REVERB SW", 0, FX_REVERB_COLOUR},
    {0x6076, "[A]REVERB TYPE", 0, FX_REVERB_COLOUR},
    {0x6400, "[B]M1", 2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2)
    {0x6401, "[B]M1 TYPE", 102, VG99_FX_TYPE_COLOUR},
    {0x6800, "[B]M2", 2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2) // Parameter number: 195
    {0x6801, "[B]M2 TYPE", 102, VG99_FX_TYPE_COLOUR},
    {0x607E, "[B]NS SW", 0, FX_FILTER_COLOUR},
    {0x600D, "[B]AMP", 4, FX_AMP_COLOUR}, // Sublist amps
    {0x600E, "[B]AMP TYPE", 0, FX_AMP_COLOUR},
    {0x600F, "[B]AMP GAIN", 0, FX_AMP_COLOUR}, // Parameter number: 200
    {0x6015, "[B]AMP BRIGHT", 0, FX_AMP_COLOUR},
    {0x6016, "[B]AMP GAIN SW", 0, FX_AMP_COLOUR},
    {0x6017, "[B]AMP(SOLO) SW", 0, FX_AMP_COLOUR},
    {0x6019, "[B]AMP SP TYPE", 0, FX_AMP_COLOUR}
  },

  { // part 7: 7000 - 8000 Special functions
    {0x7600, "[A]BEND", 0, FX_PITCH_COLOUR}, // Parameter number: 210
    {0x7601, "[B]BEND", 0, FX_PITCH_COLOUR},
    {0x7602, "DB T-ARM CONTROL", 0, FX_PITCH_COLOUR},
    {0x7603, "DB T-ARM SW", 0, FX_PITCH_COLOUR},
    {0x7604, "DB FREEZE SW", 0, VG99_STOMP_COLOUR},
    {0x7606, "DB FILTER CONTRL", 0, FX_FILTER_COLOUR}, // Parameter number: 215
    {0x7607, "DB FILTER SW", 0, FX_FILTER_COLOUR},
    {0x7608, "RB T-ARM CONTROL", 0, FX_PITCH_COLOUR},
    {0x7609, "RB T-ARM SW", 0, FX_PITCH_COLOUR},
    {0x760A, "RB FILTER CONTRL", 0, FX_FILTER_COLOUR},
    {0x760B, "RB FILTER SW", 0, FX_FILTER_COLOUR}, // Parameter number: 220
    {0x760C, "[A]FX DLY REC", 0, FX_DELAY_COLOUR},
    {0x760D, "[A]FX DLY STOP", 0, FX_DELAY_COLOUR},
    {0x760E, "[B]FX DLY REC", 0, FX_DELAY_COLOUR},
    {0x760F, "[B]FX DLY STOP", 0, FX_DELAY_COLOUR},
    {0x7611, "BPM TAP", 0, FX_DELAY_COLOUR}, // Parameter number: 225
    {0x7610, "V-LINK SW", 0, FX_AMP_COLOUR},
    {0x7F7F, "OFF", 0, 0},
  }
};

#define VG99_NO_OF_SUBLISTS 7
#define VG99_SIZE_OF_SUBLISTS 49
const PROGMEM char VG99_sublists[VG99_NO_OF_SUBLISTS][VG99_SIZE_OF_SUBLISTS][8] = {
  // Sublist 1 from the "Control assign target" table on page 57 of the VG99 MIDI impementation guide
  { "BOOST", "BLUES", "CRUNCH", "NATURAL", "TURBO", "FAT OD", "OD - 1", "TSCREAM", "WARM OD", "DIST",
    "MILD DS", "DRIVE", "RAT", "GUV DS", "DST + ", "SOLID", "MID DS", "STACK", "MODERN", "POWER", "R - MAN",
    "METAL", "HVY MTL", "LEAD", "LOUD", "SHARP", "MECHA", "60 FUZZ", "OCTFUZZ", "BIGMUFF", "CUSTOM"
  },

  // Sublist 2 from the "FX mod type" table on page 71 of the VG99 MIDI impementation guide
  { "COMPRSR", "LIMITER", "T. WAH", "AUTOWAH", "T_WAH", "---", "TREMOLO", "PHASER", //00 - 07
    "FLANGER", "PAN", "VIB", "UNI - V", "RINGMOD", "SLOW GR", "DEFRET", "", //08 - 0F
    "FEEDBKR", "ANTI FB", "HUMANZR", "SLICER", "---", "SUB EQ", "HARMO", "PITCH S", //10 - 17
    "P. BEND", "OCTAVE", "ROTARY", "2x2 CHS", "---", "---", "---", "---", //18 - 1F
    "S DELAY"
  },

  // Sublist 3 from the "Harmony" table on page 56 of the VG99 MIDI impementation guide
  { "-2oct", "-14th", "-13th", "-12th", "-11th", "-10th", "-9th",
    "-1oct", "-7th", "-6th", "-5th", "-4th", "-3rd", "-2nd", "TONIC",
    "+2nd", "+3rd", "+4th", "+5th", "+6th", "+7th", "+1oct", "+9th", "+10th", "+11th",
    "+12th", "+13th", "+14th", "+2oct", "USER"
  },

  // Sublist 4 from the "COSM AMP" table on page 71 of the VG99 MIDI impementation guide
  { "JC-120", "JC WRM", "JC JZZ", "JC FLL", "JC BRT", "CL TWN", "PRO CR", "TWEED", "WRM CR", "CRUNCH",
    "BLUES", "WILD C", "C STCK", "VXDRIV", "VXLEAD", "VXCLN", "MTCH D", "MTCH F", "MTCH L", "BGLEAD",
    "BGDRIV", "BRHYTM", "SMOOTH", "BGMILD", "MS1959", "MS1959", "MS1959", "MS HI", "MS PWR", "RF-CLN",
    "RF-RAW", "RF-VT1", "RF-MN1", "RF-VT2", "RF-MN1", "T-CLN", "T-CNCH", "T-LEAD", "T-EDGE", "SLDANO",
    "HI-DRV", "HI-LD", "HI-HVY", "5150", "MODERN", "M LEAD", "CUSTOM", "BASS V", "BASS M"
  },

  // Sublist 5 from page 17 of the VG99 MIDI impementation guide
  { "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "D-MODAL", "-1 STEP", "-2 STEP", "BARITON", "NASHVL", "-1 OCT", "+1 OCT", "USER" },

  // Sublist 6 for poly FX
  { "COMPR", "DISTORT", "OCTAVE", "SLOW GR"}
};

const PROGMEM uint8_t VG99_FX_colours[33] = {
  FX_FILTER_COLOUR, // Colour for "COMPRSR"
  FX_FILTER_COLOUR, // Colour for "LIMITER"
  FX_FILTER_COLOUR, // Colour for "T. WAH"
  FX_FILTER_COLOUR, // Colour for "AUTOWAH"
  FX_FILTER_COLOUR, // Colour for "T_WAH"
  0, // Colour for "GUITAR SIM" - not implemented in VG99
  FX_MODULATE_COLOUR, // Colour for "TREMOLO"
  FX_MODULATE_COLOUR, // Colour for "PHASER"
  FX_MODULATE_COLOUR, // Colour for "FLANGER"
  FX_MODULATE_COLOUR, // Colour for "PAN"
  FX_MODULATE_COLOUR, // Colour for "VIB"
  FX_MODULATE_COLOUR, // Colour for "UNI - V"
  FX_MODULATE_COLOUR, // Colour for "RINGMOD"
  FX_MODULATE_COLOUR, // Colour for "SLOW GR"
  FX_DIST_COLOUR, // Colour for "DEFRET"
  VG99_STOMP_COLOUR, // Colour for ""
  FX_FILTER_COLOUR, // Colour for "FEEDBKR"
  FX_FILTER_COLOUR, // Colour for "ANTI FB"
  FX_FILTER_COLOUR, // Colour for "HUMANZR"
  FX_FILTER_COLOUR, // Colour for "SLICER"
  0, // Colour for "SITAR" - not implemented in VG99
  FX_FILTER_COLOUR, // Colour for "SUB EQ"
  FX_PITCH_COLOUR, // Colour for "HARMO"
  FX_PITCH_COLOUR, // Colour for "PITCH S"
  FX_PITCH_COLOUR, // Colour for "P. BEND"
  FX_PITCH_COLOUR, // Colour for "OCTAVE"
  FX_MODULATE_COLOUR, // Colour for "ROTARY"
  FX_MODULATE_COLOUR, // Colour for "2x2 CHORUS"
  0, // Colour for "AUTO RIFF" - not implemented in VG99
  0, // Colour for "GUITAR SYNTH" - not implemented in VG99
  0, // Colour for "AC. PROC" - not implemented in VG99
  0, // Colour for "SOUND HOLD"  - not implemented in VG99
  FX_DELAY_COLOUR // Colour for "S DELAY"
};

const PROGMEM uint8_t VG99_polyFX_colours[4] = {
  FX_FILTER_COLOUR, // Colour for "COMPR"
  FX_DIST_COLOUR, // Colour for "DISTORT"
  FX_PITCH_COLOUR, // Colour for "OCTAVE"
  FX_FILTER_COLOUR // Colour for "SLOW GR"
};

// Toggle VG99 stompbox parameter
void VG99_parameter_press(uint8_t Sw, uint8_t Cmd, uint8_t number) {

  uint8_t part = number / 30; // Split the parameter number in part and index
  uint8_t index = number % 30;

  // Send sysex MIDI command to VG-99
  uint8_t value = 0;
  if (SP[Sw].State == 1) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value1;
  if (SP[Sw].State == 2) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value2;
  if (SP[Sw].State == 3) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value3;
  if (SP[Sw].State == 4) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value4;
  if (SP[Sw].State == 5) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value5;
  write_VG99(0x60000000 + VG99_parameters[part][index].Address, value);

  // Show message
  VG99_check_update_label(Sw, value);
  show_status_message(SP[Sw].Label);

  load_current_page(false); // To update the other switch states, we re-load the current page
}

void VG99_parameter_release(uint8_t Sw, uint8_t Cmd, int8_t number) {

  uint8_t part = number / 30; // Split the parameter number in part and index
  uint8_t index = number % 30;

  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    write_VG99(0x60000000 + VG99_parameters[part][index].Address, Page[Current_page].Switch[Sw].Cmd[Cmd].Value1);

    load_current_page(false); // To update the other switch states, we re-load the current page
  }
}

void VG99_read_parameter(uint8_t byte1, uint8_t byte2) { //Read the current VG99 parameter
  SP[Current_switch].Target_byte1 = byte1;
  SP[Current_switch].Target_byte2 = byte2;

  uint8_t part = SP[Current_switch].PP_number / 30; // Split the parameter number in part and index
  uint8_t index = SP[Current_switch].PP_number % 30;

  // Set the status
  SP[Current_switch].State = 0;
  if (SP[Current_switch].Type == VG99_PARAMETER) {
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value5) SP[Current_switch].State = 5;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value4) SP[Current_switch].State = 4;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value3) SP[Current_switch].State = 3;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value2) SP[Current_switch].State = 2;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value1) SP[Current_switch].State = 1;
  }
  else { // It must be a VG99_ASSIGN
    if (byte1 == SP[Current_switch].Assign_min) SP[Current_switch].State = 2;
    if (byte1 == SP[Current_switch].Assign_max) SP[Current_switch].State = 1;
  }

  // Set the colour
  uint8_t my_colour = VG99_parameters[part][index].Colour;

  //Check for special colours:
  switch (my_colour) {
    case VG99_FX_COLOUR:
      SP[Current_switch].Colour = VG99_FX_colours[byte2]; //MFX type read in byte2
      break;
    case VG99_FX_TYPE_COLOUR:
      SP[Current_switch].Colour = VG99_FX_colours[byte1]; //MFX type read in byte1
      break;
    case VG99_POLYFX_COLOUR:
      SP[Current_switch].Colour = VG99_polyFX_colours[byte2]; //MOD type read in byte2
      break;
    case VG99_POLYFX_TYPE_COLOUR:
      SP[Current_switch].Colour = VG99_polyFX_colours[byte1]; //MOD type read in byte1
      break;
    default:
      SP[Current_switch].Colour =  my_colour;
      break;
  }

  // Set the display message
  String msg = VG99_parameters[part][index].Name;
  if ((VG99_parameters[part][index].Sublist > 0) && (VG99_parameters[part][index].Sublist < 100)) { // Check if a sublist exists
    String type_name = VG99_sublists[VG99_parameters[part][index].Sublist - 1][byte2];
    msg = msg + " (" + type_name + ")";
  }
  if (VG99_parameters[part][index].Sublist > 100) { // Check if state needs to be read
    uint8_t sublist_no = (VG99_parameters[part][index].Sublist - 101);
    String type_name = VG99_sublists[sublist_no][byte1];
    msg = msg + ":" + type_name;
  }
  //Copy it to the display name:
  set_label(Current_switch, msg);
  //update_lcd = Current_switch + 1;
}

void VG99_check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint8_t par_no = SP[Sw].PP_number;
  if (par_no != NOT_FOUND) {
    uint8_t part = par_no / 30; // Split the parameter number in part and index
    uint8_t index = par_no % 30;

    if (VG99_parameters[part][index].Sublist > 100) { // Check if state needs to be read
      clear_label(Sw);
      // Set the display message
      String msg = VG99_parameters[part][index].Name;
      uint8_t sublist_no = (VG99_parameters[part][index].Sublist - 101);
      String type_name = VG99_sublists[sublist_no][value];
      msg = msg + ":" + type_name;

      //Copy it to the display name:
      set_label(Sw, msg);
      //update_lcd = Current_switch + 1;
    }
  }
}

// **** VG99 assigns
// VG99 has 16 general assigns, which we can control with a cc-MIDI message.
// Also we can control 8 FC300 assigns, which need a special sysex message to function
// If you set the cc number to 1-8, the VController will control the FC300 CTL 1-8 !!!

// Procedures for VG99_ASSIGN:
// 1. Load in SP array - VG99_assign_load() below
// 2. Request - VG99_request_current_assign()
// 3. Read assign - VG99_read_current_assign() - also requests parameter state
// 4. Read parameter state - VG99_read_parameter() above
// 5. Press switch - VG99_assign_press() below
// 6. Release switch - VG99_assign_release() below

#define VG99_NUMBER_OF_ASSIGNS 24
const PROGMEM uint32_t VG99_assign_address[VG99_NUMBER_OF_ASSIGNS] = {
  0x60007000, 0x6000701C, 0x60007038, 0x60007054, 0x60007100, 0x6000711C, 0x60007138, 0x60007154, // CTRL ASSIGN 1 - 8
  0x60007200, 0x6000721C, 0x60007238, 0x60007254, 0x60007300, 0x6000731C, 0x60007338, 0x60007354, // CTRL ASSIGN 9 - 16
  0x60000600, 0x60000614, 0x60000628, 0x6000063C, 0x60000650, 0x60000664, 0x60000678, 0x6000070C // FC300 ASSIGN 1 - 8
};

const PROGMEM uint16_t FC300_CTL[8] = {0x2100, 0x2101, 0x2402, 0x2102, 0x2403, 0x2103, 0x2404, 0x2104 };

void VG99_assign_press(uint8_t Sw) { // Switch set to VG99_ASSIGN is pressed

  // Send cc MIDI command to VG-99. If cc is 1 - 8, send the FC300 CTL sysex code
  uint8_t cc_number = SP[Sw].Trigger;
  if ((cc_number >= 1) && (cc_number <= 8)) write_VG99fc(FC300_CTL[cc_number - 1], 127);
  else Send_CC(cc_number, 127, VG99_MIDI_channel, VG99_MIDI_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    VG99_check_update_label(Sw, value);
  }
  show_status_message(SP[Sw].Label);

  if (SP[Sw].Assign_on) load_current_page(false); // To update the other switch states, we re-load the current page
}

void VG99_assign_release(uint8_t Sw) { // Switch set to VG99_ASSIGN is released

  // Send cc MIDI command to VG-99. If cc is 1 - 8, send the FC300 CTL sysex code
  uint8_t cc_number = SP[Sw].Trigger;
  if ((cc_number >= 1) && (cc_number <= 8)) write_VG99fc(FC300_CTL[cc_number - 1], 0);
  else Send_CC(cc_number, 0, VG99_MIDI_channel, VG99_MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) SP[Sw].State = 2; // Switch state off
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) load_current_page(false); // To update the other switch states, we re-load the current page
  }
}

void VG99_fix_reverse_pedals() {
  // Pedal 3,5 and 7 are reversed. But by operating them once the VG99 remembers the settings
  write_VG99fc(FC300_CTL[3], 0x7F); // Press CTL-3
  write_VG99fc(FC300_CTL[3], 0x00); // Release CTL-3
  write_VG99fc(FC300_CTL[5], 0x7F); // Press CTL-5
  write_VG99fc(FC300_CTL[5], 0x00); // Release CTL-5
  write_VG99fc(FC300_CTL[7], 0x7F); // Press CTL-7
  write_VG99fc(FC300_CTL[7], 0x00); // Release CTL-7
}

void VG99_assign_load(uint8_t sw, uint8_t assign_number, uint8_t my_trigger) { // Switch set to VG99_ASSIGN is loaded in SP array
  SP[sw].Trigger = my_trigger; //Save the cc_number / FC300 pedal number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void VG99_request_current_assign() {
  uint8_t index = SP[Current_switch].Assign_number - 1;  //index should be between 0 and 7
  SP[Current_switch].Address = VG99_assign_address[index];
  if (index < VG99_NUMBER_OF_ASSIGNS) {
    DEBUGMSG("Request assign " + String(index + 1));
    VG99_read_assign_target = false;
    request_VG99(SP[Current_switch].Address, 14);  //Request 14 bytes for the VG99 assign
  }
  else Request_next_switch(); // Wrong assign number given in Config - skip it
}

void VG99_read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool assign_on, found;
  String msg;
  uint8_t assign_switch = sxdata[11];
  uint16_t assign_target = (sxdata[12] << 8) + sxdata[13];
  uint16_t assign_target_min = (sxdata[14] << 8) + sxdata[15];
  uint16_t assign_target_max = (sxdata[16] << 8) + sxdata[17];
  uint8_t assign_latch = sxdata[18];
  uint8_t assign_source = sxdata[24]; // As sxdata[23] is always 0, we will not bother reading it.

  // Check for valid assign. We have three options
  // 1) CTL assign with FC300 CTL as source
  // 2) CTL assign with cc09 - cc31 or cc64 - cc95 as source
  // 3) FC300 CTL1-8 assign with fixed source
  uint8_t my_trigger = SP[Current_switch].Trigger;
  if ((my_trigger >= 1) && (my_trigger <= 8)) my_trigger = my_trigger + 14; // Trigger if FC300 CTRL 1 - 8. Add 14 to match the VG99 implemntation of these sources (0x0F - 0x16)
  else if ((my_trigger >= 9) && (my_trigger <= 31)) my_trigger = my_trigger + 24; // Trigger is cc09 - cc31 Add 24 to match the VG99 implemntation of these sources (0x19 - 0x37)
  else if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 8; // Trigger is cc64 - cc95 Add 24 to match the VG99 implemntation of these sources (0x38 - 0x57)

  if (SP[Current_switch].Assign_number <= 16) assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source
  else assign_on = (assign_switch == 0x01); // Assign is FC300 CTL1-8 type, so we do not need to check the source

  DEBUGMSG("VG-99 Assign_switch: 0x" + String(assign_switch, HEX));
  DEBUGMSG("VG-99 Assign_target 0x:" + String(assign_target, HEX));
  DEBUGMSG("VG-99 Assign_min: 0x" + String(assign_target_min, HEX));
  DEBUGMSG("VG-99 Assign_max: 0x" + String(assign_target_max, HEX));
  DEBUGMSG("VG-99 Assign_source: 0x" + String(assign_source, HEX));
  DEBUGMSG("VG-99 Assign_latch: 0x" + String(assign_latch, HEX));
  DEBUGMSG("VG-99 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  if (assign_on) {
    SP[Current_switch].Assign_on = true; // Switch the pedal off
    SP[Current_switch].Latch = assign_latch;

    // Allow for VG-99 assign min and max swap. Is neccesary, because the VG-99 will not save a parameter in the on-state, unless you swap the assign min and max values
    if (assign_target_max > assign_target_min) {
      // Store values the normal way around
      SP[Current_switch].Assign_max = assign_target_max;
      SP[Current_switch].Assign_min = assign_target_min;
    }
    else {
      // Reverse the values
      SP[Current_switch].Assign_max = assign_target_min;
      SP[Current_switch].Assign_min = assign_target_max;
    }

    // Request the target - on the VG99 target and the address of the target are directly related
    SP[Current_switch].Address = 0x60000000 + assign_target;

    // We have to check the table to find the location of the target there
    found = VG99_target_lookup(assign_target); // Lookup the address of the target in the VG99_Parameters array

    DEBUGMSG("Request target of assign " + String(SP[Current_switch].Assign_number) + ": " + String(SP[Current_switch].Address, HEX));
    if (found) {
      VG99_read_assign_target = true;
      request_VG99((SP[Current_switch].Address), 2);
    }
    else {
      SP[Current_switch].PP_number = NOT_FOUND;
      SP[Current_switch].Colour = VG99_STOMP_COLOUR;
      // Set the Label
      if (SP[Current_switch].Assign_number <= 16) msg = "CC#" + String(SP[Current_switch].Trigger) + " (ASGN" + String(SP[Current_switch].Assign_number) + ")";
      else msg = "FC300 ASGN" + String(SP[Current_switch].Assign_number - 16);
      set_label(Current_switch, msg);
      Request_next_switch();
    }

  }
  else { // Assign is off
    SP[Current_switch].Assign_on = false; // Switch the pedal off
    SP[Current_switch].State = 0; // Switch the stompbox off
    SP[Current_switch].Latch = MOMENTARY; // Make it momentary
    SP[Current_switch].Colour = VG99_STOMP_COLOUR; // Set the on colour to default
    // Set the Label
    if (SP[Current_switch].Assign_number <= 16) msg = "CC#" + String(SP[Current_switch].Trigger);
    else msg = "--"; //"FC300 ASGN" + String(SP[Current_switch].Assign_number - 16);
    set_label(Current_switch, msg);
    Request_next_switch();
  }
}

bool VG99_target_lookup(uint16_t target) {

  // Lookup in VG99_parameter array
  uint8_t part = (target / 0x1000); // As the array is divided in addresses by 1000, it is easy to find the right part
  bool found = false;
  for (uint8_t i = 0; i < VG99_NUMBER_OF_PARAMETERS; i++) {
    if (VG99_parameters[part][i].Address == 0) break; //Break the loop if there is no more useful data
    if (target == VG99_parameters[part][i].Address) { //Check is we've found the right target
      SP[Current_switch].PP_number = (part * 30) + i; // Save the index number
      found = true;
    }
  }
  return found;
}


