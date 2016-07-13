// ******************************** MIDI common functions ********************************
// Setup of usbMIDI and Serial midi in and out
// Specific messages are listed under MIDI_GP10, MIDI_GR55 and MIDI_VG99

// Change buffersize of usbMIDI:
// Edit /Applications/Arduino.app/Contents/Resources/Java/hardware/teensy/avr/cores/teensy3/usb_midi.h in Teksteditor and change USB_MIDI_SYSEX_MAX 127

#include <MIDI.h>

#define CHECK4DEVICES_TIMER_LENGTH 1000 // Check every three seconds which Roland devices are connected
unsigned long Check4DevicesTimer = 0;
bool no_device_check = false; // This check should not occur right after a patch change.

uint8_t Current_MIDI_port; // The MIDI port that is being read.
bool bank_selection_active = false;
uint8_t Current_switch = 255; // The parameter that is being read (pointer in the SP array)

// Setup MIDI ports. The USB port is set from the Arduino menu.
struct MySettings : public midi::DefaultSettings
 {
    //static const bool UseRunningStatus = false; // Messes with my old equipment!
    //static const bool Use1ByteParsing = false; // More focus on reading messages - will this help the equipment from stopping with receiving data?
    static const unsigned SysExMaxSize = 127; // Change sysex buffersize - Zoom G3 sends packets up to 120 bytes
 };
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDI1, MySettings); // Enables serial1 port for MIDI communication with custom settings
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI2, MySettings); // Enables serial2 port for MIDI communication with custom settings
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial3, MIDI3, MySettings); // Enables serial3 port for MIDI communication with custom settings
/*
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1); //Enables serial1 port for MIDI communication!
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI2); //Enables serial2 port for MIDI communication!
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI3); //Enables serial3 port for MIDI communication!
*/

void setup_MIDI_common()
{
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  usbMIDI.setHandleProgramChange(OnProgramChange);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleSysEx(OnSysEx);

  //pinMode(0, INPUT_PULLUP); //Add the internal pullup resistor to pin 0 (Rx)
  delay(100);
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOff();
  MIDI1.setHandleNoteOff(OnNoteOff);
  MIDI1.setHandleNoteOn(OnNoteOn) ;
  MIDI1.setHandleProgramChange(OnProgramChange);
  MIDI1.setHandleControlChange(OnControlChange);
  MIDI1.setHandleSystemExclusive(OnSerialSysEx);
  //MIDI1.setHandleActiveSensing(OnActiveSenseMIDI1);

  delay(100);
  MIDI2.begin(MIDI_CHANNEL_OMNI);
  MIDI2.turnThruOff();
  MIDI2.setHandleNoteOff(OnNoteOff);
  MIDI2.setHandleNoteOn(OnNoteOn) ;
  MIDI2.setHandleProgramChange(OnProgramChange);
  MIDI2.setHandleControlChange(OnControlChange);
  MIDI2.setHandleSystemExclusive(OnSerialSysEx);
  //MIDI2.setHandleActiveSensing(OnActiveSenseMIDI2);

  delay(100);
  MIDI3.begin(MIDI_CHANNEL_OMNI);
  MIDI3.turnThruOff();
  MIDI3.setHandleNoteOff(OnNoteOff);
  MIDI3.setHandleNoteOn(OnNoteOn) ;
  MIDI3.setHandleProgramChange(OnProgramChange);
  MIDI3.setHandleControlChange(OnControlChange);
  MIDI3.setHandleSystemExclusive(OnSerialSysEx);
  //MIDI3.setHandleActiveSensing(OnActiveSenseMIDI3);

}

void main_MIDI_common()
{
  Current_MIDI_port = USBMIDI_PORT;
  usbMIDI.read();
  Current_MIDI_port = MIDI1_PORT;
  MIDI1.read();
  Current_MIDI_port = MIDI2_PORT;
  MIDI2.read();
  Current_MIDI_port = MIDI3_PORT;
  MIDI3.read();

  check_for_roland_devices();  // Check actively if any roland devices are out there
  Check_sysex_watchdog(); // check if the watchdog has not expired
}

// ******************************** MIDI In section ********************************************

// Sysex for detecting Roland devices
#define Anybody_out_there {0xF0, 0x7E, 0x10, 0x06, 0x01, 0xF7}  //Detects GP-10 and GR-55

void OnNoteOn(byte channel, byte note, byte velocity)
{
  bass_mode_note_on(channel, note, velocity);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  bass_mode_note_off(channel, note, velocity);
}

void OnProgramChange(byte channel, byte program)
{
  DEBUGMSG("PC #" + String(program) + " received on channel " + String(channel)); // Show on serial debug screen
  check_PC_in_GP10(channel, program);
  check_PC_in_GR55(channel, program);
  check_PC_in_VG99(channel, program);
  check_PC_in_ZG3(channel, program);
}

void OnControlChange(byte channel, byte control, byte value)
{
  DEBUGMSG("CC #" + String(control) + " Value:" + String(value) + " received on channel " + String(channel)); // Show on serial debug screen

  // Check the source by checking the channel
  if (channel == GP10_MIDI_channel) { // GP10 outputs a control change message

  }

  if (channel == GR55_MIDI_channel) { // GR55 outputs a control change message
    if (control == 0) {
      GR55_CC01 = value;
    }
  }

  if (channel == VG99_MIDI_channel) { // GR55 outputs a control change message
    if (control == 0) {
      VG99_CC01 = value;
    }
  }

}

void OnSysEx(const unsigned char* sxdata, short unsigned int sxlength, bool sx_comp)
{
  //MIDI1.sendSysEx(sxlength, sxdata); // MIDI through usb to serial
  //MIDI2.sendSysEx(sxlength, sxdata); // MIDI through usb to serial
  debug_sysex(sxdata, sxlength, "in-USB   ");

  if ((sxdata[1] == 0x41) && sx_comp) { //Check if it is a message from a Roland device
    check_SYSEX_in_GP10(sxdata, sxlength);
    check_SYSEX_in_GR55(sxdata, sxlength);
    check_SYSEX_in_VG99(sxdata, sxlength);
    //check_SYSEX_in_VG99fc(sxdata, sxlength);

  }

  if (sxdata[1] == 0x52) { //Check if it is a message from a ZOOM device
    check_SYSEX_in_ZG3(sxdata, sxlength);
  }

  if (sxdata[1] == 0x7E) { //Check if it is a Universal Non-Real Time message
    check_SYSEX_in_universal(sxdata, sxlength);
  }
}

void OnSerialSysEx(byte *sxdata, unsigned sxlength)
{
  //usbMIDI.sendSysEx(sxlength, sxdata); // MIDI through serial to usb
  debug_sysex(sxdata, sxlength, "in-serial" + String(Current_MIDI_port));

  if (sxdata[1] == 0x41) { //Check if it is a message from a Roland device
    check_SYSEX_in_GP10(sxdata, sxlength);
    check_SYSEX_in_GR55(sxdata, sxlength);
    check_SYSEX_in_VG99(sxdata, sxlength);
    check_SYSEX_in_VG99fc(sxdata, sxlength);
  }

  if (sxdata[1] == 0x52) { //Check if it is a message from a ZOOM device
    check_SYSEX_in_ZG3(sxdata, sxlength);
  }

  if (sxdata[1] == 0x7E) { //Check if it is a Universal Non-Real Time message
    check_SYSEX_in_universal(sxdata, sxlength);
  }
}


// General MIDI commands:
// Send Program Change message
void Send_PC(uint8_t Program, uint8_t Channel, uint8_t Port) {
  if (Port & USBMIDI_PORT) usbMIDI.sendProgramChange(Program, Channel);
  if (Port & MIDI1_PORT) MIDI1.sendProgramChange(Program, Channel);
  if (Port & MIDI2_PORT) MIDI2.sendProgramChange(Program, Channel);
  if (Port & MIDI3_PORT) MIDI3.sendProgramChange(Program, Channel);
}

void Send_CC(uint8_t Controller, uint8_t Value, uint8_t Channel, uint8_t Port) {
  if (Port & USBMIDI_PORT) usbMIDI.sendControlChange(Controller, Value, Channel);
  if (Port & MIDI1_PORT) MIDI1.sendControlChange(Controller, Value, Channel);
  if (Port & MIDI2_PORT) MIDI2.sendControlChange(Controller, Value, Channel);
  if (Port & MIDI3_PORT) MIDI3.sendControlChange(Controller, Value, Channel);
}

void Send_Note_On(uint8_t Note, uint8_t Velocity, uint8_t Channel, uint8_t Port) {
  if (Port & USBMIDI_PORT) usbMIDI.sendNoteOn(Note, Velocity, Channel);
  if (Port & MIDI1_PORT) MIDI1.sendNoteOn(Note, Velocity, Channel);
  if (Port & MIDI2_PORT) MIDI2.sendNoteOn(Note, Velocity, Channel);
  if (Port & MIDI3_PORT) MIDI3.sendNoteOn(Note, Velocity, Channel);
}

void Send_Note_Off(uint8_t Note, uint8_t Velocity, uint8_t Channel, uint8_t Port) {
  if (Port & USBMIDI_PORT) usbMIDI.sendNoteOff(Note, Velocity, Channel);
  if (Port & MIDI1_PORT) MIDI1.sendNoteOff(Note, Velocity, Channel);
  if (Port & MIDI2_PORT) MIDI2.sendNoteOff(Note, Velocity, Channel);
  if (Port & MIDI3_PORT) MIDI3.sendNoteOff(Note, Velocity, Channel);
}

// *************************************** Common functions ***************************************

void check_SYSEX_in_universal(const unsigned char* sxdata, short unsigned int sxlength) // Check for universal SYSEX message - identity reply
{
  // Check if it is an identity reply from Roland
  // There is no check on the second byte (device ID), in case a device has a different device ID
  if ((sxdata[3] == 0x06) && (sxdata[4] == 0x02) && (sxdata[5] == 0x41)) {
    GP10_identity_check(sxdata, sxlength);
    GR55_identity_check(sxdata, sxlength);
    VG99_identity_check(sxdata, sxlength);
    //FC300_identity_check(sxdata, sxlength);
  }
  
  // Check if it is an identity reply from Zoom
  if ((sxdata[3] == 0x06) && (sxdata[4] == 0x02) && (sxdata[5] == 0x52)) {
    ZG3_identity_check(sxdata, sxlength);
  }
}

// check for Roland devices
uint8_t check_device_no = 0;
void check_for_roland_devices()
{
  // Check if timer needs to be set
  if (Check4DevicesTimer == 0) {
    Check4DevicesTimer = millis();
  }

  // Check if timer runs out
  if (millis() - Check4DevicesTimer > CHECK4DEVICES_TIMER_LENGTH) {
    Check4DevicesTimer = millis(); // Reset the timer

    // Send the message to all MIDI ports if no_device_check is not true!!
    if (no_device_check == false) {
      uint8_t sysexbuffer[6] = Anybody_out_there;
      if (check_device_no == 0 ) usbMIDI.sendSysEx(6, sysexbuffer);
      if (check_device_no == 1 ) MIDI1.sendSysEx(5, sysexbuffer);
      if (check_device_no == 2 ) MIDI2.sendSysEx(5, sysexbuffer);
      if (check_device_no == 3 ) MIDI3.sendSysEx(5, sysexbuffer);
      debug_sysex(sysexbuffer, 6, "CKout");
      check_device_no++;
      if (check_device_no > 3) check_device_no = 0;
    }
  }
}

//Debug sysex messages by sending them to the serial monitor
void debug_sysex(const unsigned char* sxdata, short unsigned int sxlength, String my_source) {
  if (debug_active) {
    Serial.print(my_source + ":");
    for (uint8_t i = 0; i < sxlength; i++) {
      if (sxdata[i] < 0x10) Serial.print("0" + String(sxdata[i], HEX) + " ");
      else Serial.print(String(sxdata[i], HEX) + " ");
    }
    Serial.println();
  }
}

// Calculate the Roland checksum
uint8_t calc_checksum(uint16_t sum) {
  uint8_t checksum = 0x80 - (sum % 0x80);
  if (checksum == 0x80) checksum = 0;
  return checksum;
}

