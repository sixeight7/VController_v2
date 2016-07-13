// ******************************** MIDI messages and functions for the Roland GR-55 ********************************
uint8_t GR55_MIDI_port = 0;
// ********************************* Section 1: GR55 SYSEX messages ********************************************

//Sysex messages for Roland GR-55
#define GR55_REQUEST_MODE 0x18000000, 1 //Is 00 in guitar mode, 01 in bass mode (Gumtown in town :-)
#define GR55_REQUEST_CURRENT_PATCH_NAME 0x18000001, 16 //Request 16 bytes for current patch name
#define GR55_REQUEST_PATCH_NUMBER 0x01000000, 2 //Request current patch number
#define GR55_CTL_LED_ON 0x18000011, 0x01
#define GR55_CTL_LED_OFF 0x18000011, 0x00

#define GR55_TEMPO 0x1800023C  // Accepts values from 40 bpm - 250 bpm

#define GR55_SYNTH1_SW 0x18002003 // The address of the synth1 switch
#define GR55_SYNTH2_SW 0x18002103 // The address of the synth1 switch
#define GR55_COSM_GUITAR_SW 0x1800100A // The address of the COSM guitar switch
#define GR55_NORMAL_PU_SW 0x18000232 // The address of the COSM guitar switch

#define GR55_SYSEX_DELAY_LENGTH 10 // time between sysex messages (in msec)
unsigned long GR55sysexDelay = 0;

bool GR55_request_onoff = false;
bool GR55_read_assign_target = false;

// ********************************* Section 2: GR55 common MIDI in functions ********************************************

void check_SYSEX_in_GR55(const unsigned char* sxdata, short unsigned int sxlength) {  // Check incoming sysex messages from GR55. Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a GR-55
  if ((sxdata[2] == GR55_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x53) && (sxdata[6] == 0x12)) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    // Check if it is the patch number
    if (address == 0x01000000) {
      if (GR55_patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        GR55_patch_number = sxdata[11] * 128 + sxdata[12];
        if (GR55_patch_number > 2047) GR55_patch_number = GR55_patch_number - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
        GR55_page_check();
        GR55_do_after_patch_selection();
      }
    }

    // Check if it is the current parameter
    if (address == SP[Current_switch].Address) {
      switch (SP[Current_switch].Type) {
        case GR55_PATCH:
        case GR55_RELSEL:
          for (uint8_t count = 0; count < 16; count++) {
            SP[Current_switch].Label[count] = static_cast<char>(sxdata[count + 11]); //Add ascii character to the SP.Label String
          }
          //update_lcd = Current_switch + 1;
          if (SP[Current_switch].PP_number == GR55_patch_number) {
            GR55_patch_name = SP[Current_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[Current_switch].Label);
          Request_next_switch();
          break;
        case GR55_PARAMETER:
          GR55_read_parameter(sxdata[11], sxdata[12]);
          Request_next_switch();
          break;
        case GR55_ASSIGN:
          if (GR55_read_assign_target == false) GR55_read_current_assign(address, sxdata, sxlength);
          else {
            GR55_read_parameter(sxdata[11], sxdata[12]); //Reading the assign target is equal to readig the parameter
            Request_next_switch();
          }
          break;
      }
    }
    
    // Check if it is the patch name (address: 0x18, 0x00, 0x00, 0x01)
    if ((sxdata[6] == 0x12) && (address == 0x18000001) ) {
      GR55_patch_name = "";
      for (uint8_t count = 11; count < 27; count++) {
        GR55_patch_name = GR55_patch_name + static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
    }

    // Check if it is the guitar on/off states
    GR55_check_guitar_switch_states(sxdata, sxlength);

    // Check if it is some other stompbox function and copy the status to the right LED
    //GR55_check_stompbox_states(sxdata, sxlength);
  }
}


void check_PC_in_GR55(byte channel, byte program) { // Check incoming PC messages from GR55. Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((Current_MIDI_port == GR55_MIDI_port) && (channel == GR55_MIDI_channel)) { // GR55 outputs a program change
    uint16_t new_patch = (GR55_CC01 * 128) + program;
    if (GR55_patch_number != new_patch) {
      GR55_patch_number = new_patch;
      if (GR55_patch_number > 2047) GR55_patch_number = GR55_patch_number - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
      request_GR55(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch  
      GR55_page_check();
      GR55_do_after_patch_selection();
    }
  }
}

void GR55_identity_check(const unsigned char* sxdata, short unsigned int sxlength) {

  // Check if it is a GR-55
  if ((sxdata[6] == 0x53) && (sxdata[7] == 0x02) && (GR55_detected == false)) {
    GR55_detected = true;
    show_status_message("GR-55 detected ");
    GR55_device_id = sxdata[2]; //Byte 2 contains the correct device ID
    GR55_MIDI_port = Current_MIDI_port; // Set the correct MIDI port for this device
    DEBUGMSG("GR-55 detected on MIDI port" + String(Current_MIDI_port));
    request_GR55(GR55_REQUEST_PATCH_NUMBER);
    request_GR55(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch  
      GR55_do_after_patch_selection();
    load_current_page(true);
  }
}

// ********************************* Section 3: GR55 common MIDI out functions ********************************************

void GR55_check_sysex_delay() { // Will delay if last message was within GR55_SYSEX_DELAY_LENGTH (10 ms)
  while (millis() - GR55sysexDelay <= GR55_SYSEX_DELAY_LENGTH) {}
  GR55sysexDelay = millis();
}

void write_GR55(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, GR55_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  GR55_check_sysex_delay();
  if (GR55_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(14, sysexmessage);
  if (GR55_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(13, sysexmessage);
  if (GR55_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(13, sysexmessage);
  if (GR55_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(13, sysexmessage);
  debug_sysex(sysexmessage, 14, "out(GR55)");
}

void write_GR55(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, GR55_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  GR55_check_sysex_delay();
  if (GR55_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(15, sysexmessage);
  if (GR55_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(14, sysexmessage);
  if (GR55_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(14, sysexmessage);
  if (GR55_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(14, sysexmessage);
  debug_sysex(sysexmessage, 15, "out(GR55)");
}

void request_GR55(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no_of_bytes); // Calculate the Roland checksum
  uint8_t sysexmessage[17] = {0xF0, 0x41, GR55_device_id, 0x00, 0x00, 0x53, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, 0x00, no_of_bytes, checksum, 0xF7};
  GR55_check_sysex_delay();
  if (GR55_MIDI_port == USBMIDI_PORT) usbMIDI.sendSysEx(17, sysexmessage);
  if (GR55_MIDI_port == MIDI1_PORT) MIDI1.sendSysEx(16, sysexmessage);
  if (GR55_MIDI_port == MIDI2_PORT) MIDI2.sendSysEx(16, sysexmessage);
  if (GR55_MIDI_port == MIDI3_PORT) MIDI3.sendSysEx(16, sysexmessage);
  debug_sysex(sysexmessage, 17, "out(GR55)");
}

void GR55_request_patch_number() {
  request_GR55(GR55_REQUEST_PATCH_NUMBER);
}

void GR55_request_name() {
  //request_GR55(GR55_REQUEST_PATCH_NAME);
}

void GR55_send_bpm() {
  write_GR55(GR55_TEMPO, bpm / 16, bpm % 16); // Tempo is modulus 16. It's all so very logical. NOT.
}

// ********************************* Section 4: GR55 program change ********************************************

uint8_t GR55_patch_memory = 0;

void GR55_SendProgramChange(uint16_t new_patch) {

  if (new_patch == GR55_patch_number) GR55_unmute();
  GR55_patch_number = new_patch;

  uint16_t GR55_patch_send = 0; // Temporary value
  if (GR55_patch_number > 296) {
    GR55_patch_send = GR55_patch_number + 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will recreate it.
  }
  else {
    GR55_patch_send = GR55_patch_number;
  }

  Send_CC(0, GR55_patch_send / 128, GR55_MIDI_channel, GR55_MIDI_port);
  Send_PC(GR55_patch_send % 128, GR55_MIDI_channel, GR55_MIDI_port);
  DEBUGMSG("out(GR55) PC" + String(new_patch)); //Debug
  //GP10_mute();
  //VG99_mute();
  GR55_do_after_patch_selection();
}

void GR55_do_after_patch_selection() {
  GR55_request_onoff = false;
  GR55_on = true;
  if (SEND_GLOBAL_TEMPO_AFTER_PATCH_CHANGE == true) GR55_send_bpm();
  Current_patch_number = GR55_patch_number; // the patch name
  Current_device = GR55;
  update_LEDS = true;
  update_main_lcd = true;
  update_switch_lcds = PARAMETERS;
  GR55_request_guitar_switch_states();
  //EEPROM.write(EEPROM_GR55_PATCH_MSB, (GR55_patch_number / 256));
  //EEPROM.write(EEPROM_GR55_PATCH_LSB, (GR55_patch_number % 256));
}

void GR55_relsel_load(uint8_t sw, uint8_t bank_position, uint8_t bank_size) {
  if (GR55_bank_selection_active == false) GR55_bank_number = (GR55_patch_number / bank_size);
  uint16_t new_patch = (GR55_bank_number * bank_size) + bank_position;
  if (new_patch > GR55_PATCH_MAX) new_patch = new_patch - GR55_PATCH_MAX - 1 + GR55_PATCH_MIN;
  GR55_patch_load(sw, new_patch); // Calculate the patch number for this switch
}

void GR55_patch_load(uint8_t sw, uint16_t number) {
  SP[sw].Colour = GR55_PATCH_COLOUR;
  SP[sw].PP_number = number;
  SP[sw].Address = 0x20000001 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55
}

void GR55_patch_select(uint16_t new_patch) {

  //if (my_switch <= bank_size) {
  //if (GR55_bank_selection_active == false) GR55_bank_number = (GR55_patch_number / bank_size); //Reset the bank to current patch
  //uint16_t new_patch = (GR55_bank_number) * bank_size + (my_switch - 1);
  //uint16_t new_patch = SP[my_switch - 1].PP_number; //why does this not work?
  if (new_patch == GR55_patch_number) GR55_select_switch();
  else GR55_SendProgramChange(new_patch);
  GR55_bank_selection_active = false;
  //}
}

void GR55_bank_updown(bool updown, uint8_t bank_size) {
  uint8_t bank_number = (GR55_patch_number / bank_size);

  if (GR55_bank_selection_active == false) {
    GR55_bank_selection_active = true;
    GR55_bank_number = bank_number; //Reset the bank to current patch
    GR55_bank_size = bank_size;
  }
  // Perform bank up:
  if (updown == UP) {
    if (GR55_bank_number >= ((GR55_PATCH_MAX / bank_size) - 1)) GR55_bank_number = (GR55_PATCH_MIN / bank_size); // Check if we've reached the top
    else GR55_bank_number++; //Otherwise move bank up
  }
  // Perform bank down:
  if (updown == DOWN) {
    if (GR55_bank_number <= (GR55_PATCH_MIN / bank_size)) GR55_bank_number = (GR55_PATCH_MAX / bank_size) - 1; // Check if we've reached the bottom
    else GR55_bank_number--; //Otherwise move bank down
  }

  if (GR55_bank_number == bank_number) GR55_bank_selection_active = false; //Check whether were back to the original bank

  load_current_page(true); //Re-read the patchnames for this bank
}

void GR55_page_check() { // Checks if the current patch is on the page and will reload the page if not
  bool onpage = false;
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    if ((SP[s].Type == GR55_RELSEL) && (SP[s].PP_number == GR55_patch_number)) {
      onpage = true;
      GR55_patch_name = SP[s].Label; // Set patchname correctly
    }
  }
  if (!onpage) load_current_page(true);
}

// ** US-20 simulation
// Selecting and muting the GR55 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the GR55 is selected

void GR55_request_guitar_switch_states() {
  delay(10); // Extra delay, otherwise first parameter is not read after patch change
  request_GR55(GR55_SYNTH1_SW, 1);
  request_GR55(GR55_SYNTH2_SW, 1);
  request_GR55(GR55_COSM_GUITAR_SW, 1);
  request_GR55(GR55_NORMAL_PU_SW, 1);
  GR55_request_onoff = true;
}

void GR55_check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (GR55_request_onoff == true) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    if (address == GR55_SYNTH1_SW) {
      GR55_synth1_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_SYNTH2_SW) {
      GR55_synth2_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_COSM_GUITAR_SW) {
      GR55_COSM_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_NORMAL_PU_SW) {
      GR55_nrml_pu_onoff = sxdata[11];  // Store the value
      GR55_request_onoff = false;
    }
  }
}

void GR55_select_switch() {
  Current_device = GR55;
  if (GR55_on) {
    GR55_always_on_toggle();
  }
  else {
    GR55_unmute();
    //GP10_mute();
    //VG99_mute();
  }
}

void GR55_always_on_toggle() {
  if (US20_emulation_active) {
    GR55_always_on = !GR55_always_on; // Toggle GR55_always_on
    if (GR55_always_on) {
      GR55_unmute();
      show_status_message("GR55 always ON");
    }
    else {
      //GR55_mute();
      show_status_message("GR55 can be muted");
    }
  }
}


void GR55_unmute() {
  GR55_on = true;
  //GR55_select_LED = GR55_PATCH_COLOUR; //Switch the LED on
  write_GR55(GR55_SYNTH1_SW, GR55_synth1_onoff); // Switch synth 1 off
  write_GR55(GR55_SYNTH2_SW, GR55_synth2_onoff); // Switch synth 1 off
  write_GR55(GR55_COSM_GUITAR_SW, GR55_COSM_onoff); // Switch COSM guitar on
  write_GR55(GR55_NORMAL_PU_SW, GR55_nrml_pu_onoff); // Switch normal pu on
}

void GR55_mute() {
  if ((US20_emulation_active) && (!GR55_always_on)) {
    GR55_mute_now();
  }
}

void GR55_mute_now() { // Needed a second version, because the GR55 must always mute when engaging global tuner.
  GR55_on = false;
  //  GR55_select_LED = GR55_OFF_COLOUR; //Switch the LED off
  write_GR55(GR55_SYNTH1_SW, 0x01); // Switch synth 1 off
  write_GR55(GR55_SYNTH2_SW, 0x01); // Switch synth 1 off
  write_GR55(GR55_COSM_GUITAR_SW, 0x01); // Switch COSM guitar off
  write_GR55(GR55_NORMAL_PU_SW, 0x01); // Switch normal pu off
}

// ********************************* Section 5: GR55 parameter and assign control ********************************************

// Procedures for the GR55_PARAMETER and GR55_ASSIGN commands

// Procedures for the GR55_PARAMETER:
// 1. Load in SP array - in load_current_page(true)
// 2. Request parameter state - in Request_current_switch()
// 3. Read parameter state - GR55_read_parameter() below
// 4. Press switch - GR55_parameter_press() below - also calls GR55_check_update_label()
// 5. Release switch - GR55_parameter_release() below - also calls GR55_check_update_label()

struct GR55_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target; // Target of the assign as given in the assignments of the GR55 / GR55
  uint32_t Address; // The address of the parameter
  char Name[17]; // The name for the label
  uint8_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  bool Reversed; // The GR-55 has SYNTH 1 SW, SYNTH 2 SW, COSM GUITAR SW and NORMAL PICKUP reversed. For these parameters the on and off values will be read and written in reverse
};

//typedef struct stomper Stomper;
#define GR55_MFX_COLOUR 255 // Just a colour number to pick the colour from the GR55_MFX_colours table
#define GR55_MFX_TYPE_COLOUR 254 //Another number for the MFX type
#define GR55_MOD_COLOUR 253 // Just a colour number to pick the colour from the GR55_MOD_colours table
#define GR55_MOD_TYPE_COLOUR 252 //Another number for the MOD type

// Make sure you edit the GR55_NUMBER_OF_STOMPS when adding new parameters
#define GR55_NUMBER_OF_PARAMETERS 22

const PROGMEM GR55_parameter_struct GR55_parameters[GR55_NUMBER_OF_PARAMETERS] = {
  {0x12B, 0x18000304, "MFX", 1, GR55_MFX_COLOUR, false},
  {0x12C, 0x18000305, "MFX TYPE", 101, GR55_MFX_TYPE_COLOUR, false},
  {0x0E6, 0x18000715, "MOD", 63, GR55_MOD_COLOUR, false},
  {0x0E7, 0x18000716, "MOD TYPE", 163, GR55_MOD_TYPE_COLOUR, false},
  {0x000, 0x18002003, "SYNTH1 SW", 0, FX_GTR_COLOUR, true},
  {0x003, 0x18002005, "PCM1 TONE OCT", 0, FX_GTR_COLOUR, false},
  {0x03B, 0x18002103, "SYNTH2 SW", 0, FX_GTR_COLOUR, true},
  {0x03E, 0x18002105, "PCM2 TONE OCT", 0, FX_GTR_COLOUR, false},
  {0x076, 0x1800100A, "COSM GT SW", 0, FX_GTR_COLOUR, true},
  {0x081, 0x1800101D, "12STRING SW", 0, FX_PITCH_COLOUR, false},
  {0x0D6, 0x18000700, "AMP", 21, FX_AMP_COLOUR, false},
  {0x0D7, 0x18000702, "AMP GAIN", 0, FX_AMP_COLOUR, false},
  {0x0D9, 0x18000704, "AMP GAIN SW", 0, FX_AMP_COLOUR, false},
  {0x0DA, 0x18000705, "AMP SOLO SW", 0, FX_AMP_COLOUR, false},
  {0x0E0, 0x1800070B, "AMP BRIGHT", 0, FX_AMP_COLOUR, false},
  {0x128, 0x1800075A, "NS SWITCH", 0, FX_FILTER_COLOUR, false},
  {0x1EC, 0x18000605, "DELAY SW", 0, FX_DELAY_COLOUR, false},
  {0x1F4, 0x1800060C, "REVERB SW", 0, FX_REVERB_COLOUR, false},
  {0x1FC, 0x18000600, "CHORUS SW", 0, FX_MODULATE_COLOUR,  false},
  {0x204, 0x18000611, "EQ SWITCH", 0, FX_FILTER_COLOUR, false},
  {0x213, 0x18000234, "ALT.TUNING", 0, FX_PITCH_COLOUR, false},
  {0x216, 0x18000230, "PATCH LVL", 0, FX_FILTER_COLOUR, false},
};

#define GR55_SIZE_OF_SUBLISTS 76
const PROGMEM char GR55_sublists[GR55_SIZE_OF_SUBLISTS][9] = {
  // Sublist 1 - 20: MFX FX types
  "EQ", "S FILTR", "PHASER", "STEP PHR", "RING MOD", "TREMOLO", "AUTO PAN", "SLICER", "K ROTARY", "HEXA-CHS",
  "SPACE-D", "FLANGER", "STEP FLR", "AMP SIM", "COMPRESS", "LIMITER", "3TAP DLY", "TIME DLY", "LOFI CPS", "PITCH SH",

  // Sublist 21 - 62: Amp types
  "BOSS CLN", "JC-120", "JAZZ CBO", "FULL RNG", "CLN TWIN", "PRO CRCH", "TWEED", "DELUX CR", "BOSS CRH", "BLUES",
  "WILD CRH", "STACK CR", "VO DRIVE", "VO LEAD", "VO CLEAN", "MATCH DR", "FAT MTCH", "MATCH LD", "BG LEAD", "BG DRIVE",
  "BG RHYTH", "MS'59 I", "MS'59 II", "MS HIGN", "MS SCOOP", "R-FIER V", "R-FIER M", "R-FIER C", "T-AMP LD", "T-AMP CR",
  "T-AMP CL", "BOSS DRV", "SLDN", "LEAD STK", "HEAVY LD", "BOSS MTL", "5150 DRV", "METAL LD", "EDGE LD", "BASS CLN",
  "BASS CRH", "BASS HIG",

  // Sublist 63 - 76: MOD FX types
  "OD/DS", "WAH", "COMP", "LIMITER", "OCTAVE", " PHASER", "FLANGER", "TREMOLO", "ROTARY", "UNI-V",
  "PAN", "DELAY", "CHORUS", "EQ"
};

const PROGMEM uint8_t GR55_MFX_colours[20] = {
  FX_FILTER_COLOUR, // Colour for"EQ",
  FX_FILTER_COLOUR, // Colour for"S FILTR",
  FX_MODULATE_COLOUR, // Colour for"PHASER",
  FX_MODULATE_COLOUR, // Colour for"STEP PHR",
  FX_MODULATE_COLOUR, // Colour for"RING MOD",
  FX_MODULATE_COLOUR, // Colour for"TREMOLO",
  FX_MODULATE_COLOUR, // Colour for"AUTO PAN",
  FX_MODULATE_COLOUR, // Colour for"SLICER",
  FX_MODULATE_COLOUR, // Colour for"K ROTARY",
  FX_MODULATE_COLOUR, // Colour for"HEXA-CHS",
  FX_MODULATE_COLOUR, // Colour for"SPACE-D",
  FX_MODULATE_COLOUR, // Colour for"FLANGER",
  FX_MODULATE_COLOUR, // Colour for"STEP FLR",
  FX_AMP_COLOUR, // Colour for"AMP SIM",
  FX_FILTER_COLOUR, // Colour for"COMPRESS",
  FX_FILTER_COLOUR, // Colour for"LIMITER",
  FX_DELAY_COLOUR, // Colour for"3TAP DLY",
  FX_DELAY_COLOUR, // Colour for"TIME DLY",
  FX_FILTER_COLOUR, // Colour for"LOFI CPS",
  FX_PITCH_COLOUR // Colour for"PITCH SH"
};

const PROGMEM uint8_t GR55_MOD_colours[14] = {
  FX_DIST_COLOUR, // Colour for"OD/DS",
  FX_FILTER_COLOUR, // Colour for"WAH",
  FX_FILTER_COLOUR, // Colour for"COMP",
  FX_FILTER_COLOUR, // Colour for"LIMITER",
  FX_PITCH_COLOUR, // Colour for"OCTAVE",
  FX_MODULATE_COLOUR, // Colour for"PHASER",
  FX_MODULATE_COLOUR, // Colour for"FLANGER",
  FX_MODULATE_COLOUR, // Colour for"TREMOLO",
  FX_MODULATE_COLOUR, // Colour for"ROTARY",
  FX_MODULATE_COLOUR, // Colour for"UNI-V",
  FX_MODULATE_COLOUR, // Colour for"PAN",
  FX_DELAY_COLOUR, // Colour for"DELAY",
  FX_MODULATE_COLOUR, // Colour for"CHORUS",
  FX_FILTER_COLOUR // Colour for"EQ"
};

// Toggle GR55 stompbox parameter
void GR55_parameter_press(uint8_t Sw, uint8_t Cmd, uint8_t number) {

  // Send sysex MIDI command to GR-55
  uint8_t value = 0;
  if (!GR55_parameters[number].Reversed) { // If not reversed - normal operation
    if (SP[Sw].State == 1) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value1;
    if (SP[Sw].State == 2) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value2;
  }
  else { // Reverse value1 and value2
    if (SP[Sw].State == 1) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value2;
    if (SP[Sw].State == 2) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value1;
  }
  if (SP[Sw].State == 3) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value3;
  if (SP[Sw].State == 4) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value4;
  if (SP[Sw].State == 5) value = Page[Current_page].Switch[Sw].Cmd[Cmd].Value5;
  write_GR55(GR55_parameters[number].Address, value);

  // Show message
  GR55_check_update_label(Sw, value);
  show_status_message(SP[Sw].Label);

  load_current_page(false); // To update the other switch states, we re-load the current page
}

void GR55_parameter_release(uint8_t Sw, uint8_t Cmd, uint8_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    write_GR55(GR55_parameters[number].Address, Page[Current_page].Switch[Sw].Cmd[Cmd].Value1);

    load_current_page(false); // To update the other switch states, we re-load the current page
  }
}

void GR55_read_parameter(uint8_t byte1, uint8_t byte2) { //Read the current GR55 parameter
  SP[Current_switch].Target_byte1 = byte1;
  SP[Current_switch].Target_byte2 = byte2;

  uint8_t index = SP[Current_switch].PP_number; // Read the parameter number (index to GR55-parameter array)

  // Set the status
  SP[Current_switch].State = 0;
  if (SP[Current_switch].Type == GR55_PARAMETER) {
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value5) SP[Current_switch].State = 5;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value4) SP[Current_switch].State = 4;
    if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value3) SP[Current_switch].State = 3;
    if (!GR55_parameters[index].Reversed) { // If not reversed - read state the normal way around
      if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value2) SP[Current_switch].State = 2;
      if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value1) SP[Current_switch].State = 1;
    }
    else { // If reversed - reverse value 1 and 2
      if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value1) SP[Current_switch].State = 2;
      if (byte1 == Page[Current_page].Switch[Current_switch].Cmd[0].Value2) SP[Current_switch].State = 1;
    }
  }
  else { // It must be a GR55_ASSIGN
    if (!GR55_parameters[index].Reversed) { // If not reversed - read state the normal way around
      if (byte1 == SP[Current_switch].Assign_min) SP[Current_switch].State = 2;
      if (byte1 == SP[Current_switch].Assign_max) SP[Current_switch].State = 1;
    }
    else { // If reversed - reverse assign_min and assign_max
      if (byte1 == SP[Current_switch].Assign_max) SP[Current_switch].State = 2;
      if (byte1 == SP[Current_switch].Assign_min) SP[Current_switch].State = 1;
    }
  }

  // Set the colour
  uint8_t my_colour = GR55_parameters[index].Colour;

  //Check for special colours:
  switch (my_colour) {
    case GR55_MFX_COLOUR:
      SP[Current_switch].Colour = GR55_MFX_colours[byte2]; //MFX type read in byte2
      break;
    case GR55_MFX_TYPE_COLOUR:
      SP[Current_switch].Colour = GR55_MFX_colours[byte1]; //MFX type read in byte1
      break;
    case GR55_MOD_COLOUR:
      SP[Current_switch].Colour = GR55_MOD_colours[byte2]; //MOD type read in byte2
      break;
    case GR55_MOD_TYPE_COLOUR:
      SP[Current_switch].Colour = GR55_MOD_colours[byte1]; //MOD type read in byte1
      break;
    default:
      SP[Current_switch].Colour =  my_colour;
      break;
  }

  // Set the display message
  String msg = GR55_parameters[index].Name;
  if ((GR55_parameters[index].Sublist > 0) && (GR55_parameters[index].Sublist < 100)) { // Check if a sublist exists
    String type_name = GR55_sublists[GR55_parameters[index].Sublist + byte2 - 1];
    msg = msg + " (" + type_name + ")";
  }
  if (GR55_parameters[index].Sublist > 100) { // Check if state needs to be read
    String type_name = GR55_sublists[GR55_parameters[index].Sublist + byte1 - 101];
    msg = msg + ":" + type_name;
  }
  //Copy it to the display name:
  set_label(Current_switch, msg);
  //update_lcd = Current_switch + 1;
}

void GR55_check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint8_t index = SP[Sw].PP_number; // Read the parameter number (index to GR55-parameter array)
  if (index != NOT_FOUND) {
    if (GR55_parameters[index].Sublist > 100) { // Check if state needs to be read
      clear_label(Sw);
      // Set the display message
      String msg = GR55_parameters[index].Name;
      String type_name = GR55_sublists[GR55_parameters[index].Sublist + value - 101];
      msg = msg + ":" + type_name;

      //Copy it to the display name:
      set_label(Sw, msg);
      //update_lcd = Current_switch + 1;
    }
  }
}

// **** GR55 assigns
// GR55 has eight general assigns, which we can control with a cc-MIDI message.

// Procedures for GR55_ASSIGN:
// 1. Load in SP array - GR55_assign_load() below
// 2. Request - GR55_request_current_assign()
// 3. Read assign - GR55_read_current_assign() - also requests parameter state
// 4. Read parameter state - GR55_read_parameter() above
// 5. Press switch - GR55_assign_press() below
// 6. Release switch - GR55_assign_release() below

#define GR55_NUMBER_OF_ASSIGNS 8
const PROGMEM uint32_t GR55_assign_address[GR55_NUMBER_OF_ASSIGNS] = { 0x1800010C, 0x1800011F, 0x18000132, 0x18000145, 0x18000158, 0x1800016B, 0x1800017E, 0x18000211 };

void GR55_assign_press(uint8_t Sw) { // Switch set to GR55_ASSIGN is pressed
  // Send cc MIDI command to GR-55
  uint8_t cc_number = SP[Sw].Trigger;
  Send_CC(cc_number, 127, GR55_MIDI_channel, GR55_MIDI_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    GR55_check_update_label(Sw, value);
  }
  show_status_message(SP[Sw].Label);

  if (SP[Sw].Assign_on) load_current_page(false); // To update the other switch states, we re-load the current page
}

void GR55_assign_release(uint8_t Sw) { // Switch set to GR55_ASSIGN is released
  // Send cc MIDI command to GR-55
  uint8_t cc_number = SP[Sw].Trigger;
  Send_CC(cc_number, 0, GR55_MIDI_channel, GR55_MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) SP[Sw].State = 2; // Switch state off
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) load_current_page(false); // To update the other switch states, we re-load the current page
  }
}

void GR55_assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to GR55_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void GR55_request_current_assign() {
  uint8_t index = SP[Current_switch].Assign_number - 1;  //index should be between 0 and 7
  SP[Current_switch].Address = GR55_assign_address[index];
  if (index < GR55_NUMBER_OF_ASSIGNS) {
    DEBUGMSG("Request assign" + String(index + 1));
    GR55_read_assign_target = false;
    request_GR55(SP[Current_switch].Address, 12);  //Request 12 bytes for the GR55 assign
  }
  else Request_next_switch(); // Wrong assign number given in Config - skip it
}

void GR55_read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool found;
  String msg;
  uint8_t assign_switch = sxdata[11];
  uint8_t assign_target = (sxdata[12] << 8) + (sxdata[13] << 4) + sxdata[14];
  uint8_t assign_target_min = ((sxdata[15] - 0x04) << 8) + (sxdata[16] << 4) + sxdata[17];
  uint8_t assign_target_max = ((sxdata[18] - 0x04) << 8) + (sxdata[19] << 4) + sxdata[20];
  uint8_t assign_source = sxdata[21];
  uint8_t assign_latch = sxdata[22];

  uint8_t my_trigger = SP[Current_switch].Trigger;
  if ((my_trigger >= 1) && (my_trigger <= 31)) my_trigger = my_trigger + 8; // Trigger is cc01 - cc31
  if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 24; // Trigger is cc64 - cc95

  bool assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source

  DEBUGMSG("GR-55 Assign_switch: 0x" + String(assign_switch, HEX));
  DEBUGMSG("GR-55 Assign_target 0x:" + String(assign_target, HEX));
  DEBUGMSG("GR-55 Assign_min: 0x" + String(assign_target_min, HEX));
  DEBUGMSG("GR-55 Assign_max: 0x" + String(assign_target_max, HEX));
  DEBUGMSG("GR-55 Assign_source: 0x" + String(assign_source, HEX));
  DEBUGMSG("GR-55 Assign_latch: 0x" + String(assign_latch, HEX));
  DEBUGMSG("GR-55 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  if (assign_on) {
    SP[Current_switch].Assign_on = true; // Switch the pedal off
    SP[Current_switch].Latch = assign_latch;

    // Allow for GR-55 assign min and max swap. Is neccesary, because the GR-55 will not save a parameter in the on-state, unless you swap the assign min and max values
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

    // Request the target
    found = GR55_target_lookup(assign_target); // Lookup the address of the target in the GR55_Parameters array
    DEBUGMSG("Request target of assign" + String(SP[Current_switch].Assign_number) + ":" + String(SP[Current_switch].Address, HEX));
    if (found) {
      GR55_read_assign_target = true;
      request_GR55((SP[Current_switch].Address), 2);
    }
    else {
      SP[Current_switch].PP_number = NOT_FOUND;
      SP[Current_switch].Colour = GR55_STOMP_COLOUR;
      // Set the Label
      msg = "CC#" + String(SP[Current_switch].Trigger) + " (ASGN" + String(SP[Current_switch].Assign_number) + ")";
      set_label(Current_switch, msg);
      Request_next_switch();
    }
  }
  else { // Assign is off
    SP[Current_switch].Assign_on = false; // Switch the pedal off
    SP[Current_switch].State = 0; // Switch the stompbox off
    SP[Current_switch].Latch = MOMENTARY; // Make it momentary
    SP[Current_switch].Colour = GR55_STOMP_COLOUR; // Set the on colour to default
    // Set the Label
    msg = "CC#" + String(SP[Current_switch].Trigger);
    set_label(Current_switch, msg);
    Request_next_switch();
  }
}

bool GR55_target_lookup(uint16_t target) {

  // Lookup in GR55_parameter array
  bool found = false;
  for (uint8_t i = 0; i < GR55_NUMBER_OF_PARAMETERS; i++) {
    if (target == GR55_parameters[i].Target) { //Check is we've found the right target
      SP[Current_switch].PP_number = i; // Save the index number
      SP[Current_switch].Address = GR55_parameters[i].Address;
      found = true;
    }
  }
  return found;
}

// GR-55 ptch list
#define GR55_NUMBER_OF_FACTORY_PATCHES 360
const PROGMEM char GR55_preset_patch_names[GR55_NUMBER_OF_FACTORY_PATCHES][17] = {
  // LEAD
  "Metal Synth Lead", "Rock Lead Organ", "GR-300 Ctl:+1oct", "Nice Tenor", "Flute Solo", "Jazz Guitar Vibe", "Legato Solo", "SlowAttack Solo",
  "Synth Brass Lead", "Drive Blues Harp", "Tp Section", "Mellow Cello", "Strange Whistle", "Emotional Lead", "Wave Synth Solo", "Dual Sync Lead",
  "Funky Syn Lead", "SqrPipe For You", "Concert Grand", "Mute Trumpet/Exp", "Epf + 335 Unison", "P90 & Organ Bell", "Feedback Guitar", "CTL=DLY/EXP=WAH",
  "More Blacklord", "Pdl Bend Guitar", "Poly Distortion", "NaturalResoLead", "Organ Syn Lead", "Crims-O-Tron", "Dist Sync Lead", "5th Layer",
  "Screamin Lead", "Portamento Lead", "Dist Sine Solo", "Dist Square Lead", "Buzz Lead", "Metal Saw Lead", "BrassyLead", "Long Echo Lead",
  "RockyOrgan", "Mild Saw Lead", "Simple Square", "+1oct Mild Lead", "Unison Lead", "Lead Beast", "Dream Bell", "Female Chorus",
  "70s Unison", "Comfortable Solo", "Wah Feedback", "Gtr+Organ Unison", "Vibraphone", "Dark Trumpet", "High Note Tp", "Fat Brass Sec",
  "Solo Fr.Horn", "SGT Fr Horn", "Solo Trombone", "Super Low Brass", "Clarinet>EXP Vib", "Oboe", "Soprano Sax", "Alto Sax",
  "Moody Sax", "Guitar+SaxUnison", "Flute+Gtr Unison", "Pan Flute", "Piccolo", "Flutey GT", "Heaven Ocarina", "LofiFlute&Glockn",
  "Recorder", "Chromatic Harmo", "Filter Harp", "Gt + Harmonica", "Heavy Harmonica", "Lead Violin", "Dist Violin", "Drive+Vln+Cello",
  "Double Cello", "Glass Cello", "Overdrive+Cello", "Smooth Lead+Vln", "Brass + Drive", "Organ,Pf & OD Gt", "Shamisen", "for Normal PU L1",
  "for Normal PU L2", "for Normal PU L3", "Heavy PdlBend", "Hard St/Syn FX", "StackOfSoloSynth", "Captain Nylon", "HarpNylon&String", "Sync Key Vox Gt",
  "Liquid Baritone", "String Quartet", "Sax over Organ", "FullBeardBoogie", "MahoganyTones", "EuropeanFeedback", "Funkenstein Bass", "Fuzz Bass&Syn Bs",
  "Weather Forecast", "FlyingJuno Brass", "Drop D Trance", "Soft Syn Lead", "Soft Res Lead", "Octa Sync Mix", "Filtered PolySyn", "Fat Power Note",
  "Anthem Approved", "MILD CLEAN", "Mild Octive", "Bright+1octive", "+1 OctModulation", "Lipstack Dly", "Heavy Gt", "Saturated Dreams",

  // RHYTHM
  "12st AG & Ch Org", "DoubleFlatHeavy", "SoftBrightPad+L4", "Rich Strings", "Poly Sitar", "HeavyBrassRock", "Syn Str.Pdl Reso", "TB-303 Bass",
  "AG+Bell Pad", "Double Low Piano", "E.Piano", "Xylophone Plus", "30 String Guitar", "ST + Tweed", "LP + Stack", "AcGt12st+Strings",
  "Jazz Guitar", "TL&Rotary Organ", "Ballade Wurly", "RnB Section", "Nylon Gt+Strings", "Symphonic Rock!", "GR Brass+Strings", "RockInCathedral",
  "DADGAD Phaser", "Asian DADGAD", "TL+StFlanger Pad", "Heavy Gt W/Sweep", "Fat Drive Mix", "Bright Gtr + Pad", "Electric 12str", "AC->12stAC(CTL)",
  "Nylon String Gtr", "Pedal Wah", "Stolling Rones", "Flat Tuned Drive", "BlueGrass 12-St", "Bell Clean", "AG & Epf", "HnkyTonk Piano",
  "Phaser E.Pf", "Piano + Anlg Pad", "Dyno Epf w/Pad", "ST+FM Epf+Voice", "Drive Wurly", "80s Piano", "Analog Clav S&H", "E.Piano/AcPiano",
  "Pipe Organ", "Cheap Organ", "3xOrganPower", "Simple Clavi", "R12st+Clavi+Xylo", "Harpsichord CTL", "Celesta", "Accordion",
  "Bell&Mallet+(Bs)", "TE+FM Bell Pad", "Marimba", "SteelDrums/Ethno", "Voice Pad SL", "AG+Voice", "Rotary G & Pad", "Gt & Vo Unison",
  "Vox+Pf+Crystal", "Crunch & Voice", "80s Stack Piano", "Like 60s", "Reed Organ(+LP)", "Full Section", "Real & Syn Brass", "Edge Brass",
  "Orchestra", "Pizzicato Gt", "Flange Strings", "Phase Strings", "SynthBrass", "Blade Running", "Seychelles Tour", "EmotionalBallad",
  "Analog Voice Pad", "-2 Tubular & LP", "Bridge of Sy's", "Faded Cherry", "Acid Bass", "Acoustic Bass", "Heavy P-Funk BS", "for Normal PU R1",
  "for Normal PU R2", "for Normal PU R3", "DreaminResonator", "NashvilleRoads", "MoodyBaritoneGTR", "Rotary Poly Key", "Syn Brs&Ana Bell", "ClnCho EXP>Bell",
  "OpenE Repeater", "ES335 Bright", "Dynamic TL!", "Reggae Ricky", "Heavy EXPsw Up 5", "Tight Tele Tack", "Tele Tastic", "Bright ST R+C",
  "Power Ac.Guitar", "Mild Nylon Gt", "Sitar", "Mandlin&AG+Acord", "Kalimba Pad", "AsianOpenG-Slide", "HarpsiOrch+12stG", "Open G Dulcimers",
  "Rotary Wurly Pls", "80s Analog Mix", "Crunch LP&St Pad", "DADGAD Crunch", "Gt->Rock Bass", "Asian Edge", "MostBeautifulGTR", "Stack Of Blues",

  // Other
  "Ultimate Pulse", "Heavy Hit&Groove", "Jazz Trio", "Seq*Tempo Dly+EG", "DarkSideOfTheSun", "Koto Dreams", "Voice Hit", "Heavens Bells",
  "Sine Air Bend", "Question+Answer", "Metamorphosis", "HighlanderGTR", "Sitar Fantasy", "GR-300 Triplet", "Noize Mix Drive", "Scat & Guitar",
  "SE Pad & LP+MS", "DancingAcoustic", "Heavy Pulse", "New Waves", "FourthOfFifth", "E Sitar& Dly Toy", "Trio Concerto", "Paradise Lost",
  "Trademark Riff", "Touchy 5th", "Scuba-Diving", "Big Syn Drum", "Sequence Clean", "Acoustic Heaven", "SparkleBellGTR", "Metal Timpani",
  "Cheezy Movie", "Stalker Violin", "OverblownClnGTR", "MotionBuilder", "Pulsing Bell+EG", "Flying Tremolo", "Trance Organ", "Sequence Trio",
  "Extreme FX", "Rhythmic Pulse", "Scared Score", "EasternFluteGT", "Odd Guitar", "DissonantBeauty", "PluckdBaritoned", "GroovePusher",
  "JazzEP/BassSplit", "Metal Scat", "Quantum Physics", "Enigmatic Rick", "Euro Beat Slicer", "Fuzz Heaven", "Arabian Nights", "Morpheus",
  "Unison+5thPower", "BassFluteSaxTrio", "Exorbitanz", "Armageddon", "Grinder", "EmoCarillion", "Unbelievable", "FAB 4 Together",
  "Esoteric Vibe", "Deja Vu Bass", "GK Paradise", "Is Dis Fat?", "Gladiator", "SlowGearSynth", "Oxygen Lead", "SteelPan + Agogo",
  "Ghostly", "Sneaking Up", "Big Ben", "AggroClav", "Cinematic Art", "Strictly E", "Beat Provider", "Shanai+Rhythm",
  "BackToDaCrib", "Hyper TE Beat", "House Fire", "Trance Groove", "Rainstorm", "Scary Scream", "Comedian", "for Normal PU O1",
  "for Normal PU O2", "for Normal PU O3", "Fantasy E.Guitar", "SpaceAltar", "ElectroG&Passing", "Sweep & Mod", "Tremolo Morphin", "Fairy Jazz GT",
  "Fine Wine(DropD)", "DeepWater(OpenE)", "Bubble in Heaven", "Lo B Rush Hour", "Trance Mission", "Ultraslow Groove", "80`s Kraftgroove", "Trancy CTL=BPM",
  "Trancy Riff BPM", "Slicer Change", "Drop-D Slices", "Bell&SynBrass Gt", "GR-Wonderland", "FallDown(ExpPdl)", "GtrBell (+ExpSw)", "ReverseGt+St Pad",
  "SingleNoteOrch.", "Shadow Crunch Gt", "Dbl Crystal Bell", "Analog Seq & Dly", "Hold Bass>Wah LD", "Tap Dance Guitar", "CompuRhythm", "BritishRaceTrack"
};

void GR55_read_preset_name(uint8_t number, uint16_t patch) {
  String lbl = GR55_preset_patch_names[patch - 297]; //Read the label from the array
  set_label(number, lbl);
  if (patch == GR55_patch_number) GR55_patch_name = lbl; //Keeps the main display name updated
}

