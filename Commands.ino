//The commands specified on this page can be used on the Config page.

//Possible switch types:
//00 - 09 Common functions
#define NOTHING 0
#define SELECT_PAGE 01
#define TOGGLE_PAGE 02
#define COMBI_BANK_UP 03
#define COMBI_BANK_DOWN 04
#define TAP_TEMPO 05

//10 - 19 General MIDI commands
#define MIDI_PC 10   // Command: {MIDI_PC, Program, Channel, Port}
#define MIDI_CC 11   // Command: {MIDI_CC, Controller, MOMENTARY/TOGGLE/TRISTATE, Channel, Port, Value1, Value2, Value3}
#define MIDI_NOTE 12 // Command: {MIDI_NOTE, Note, Velocity, Channel, Port}

//20 - 29: Boss GP10 types
#define GP10_PATCH 20  //Command: {GP10_PATCH, number} // Select fixed patch number.
#define GP10_RELSEL 21 //Command: {GP10_RELSEL, number, bank_size} // Select patch in bank
#define GP10_BANK_UP 22 //Command: {GP10_BANK_UP, bank_size}
#define GP10_BANK_DOWN 23 //Command: {GP10_BANK_DOWN, bank_size}
#define GP10_PARAMETER 24 //Command: {GP10_PARAMETER, number, MOMENTARY/TOGGLE/TRISTATE, VALUE_ON, VALUE_OFF}
#define GP10_ASSIGN 25 //Command: {GP10_ASSIGN, assign_number, cc_number}
#define GP10_MUTE 26 //Command: {GP10_MUTE} - will mute the GP10, unless GP10_always_on is true
String GP10_patch_name = "                "; // Patchname displayed in the main display

//30 - 39: Roland GR55 types
#define GR55_PATCH 30  //Command: {GR55_PATCH, bank, number} // Select fixed patch number. Patch number will be: (bank * 3) + number
#define GR55_RELSEL 31 //Command: {GR55_RELSEL, number, bank_size}
#define GR55_BANK_UP 32 //Command: {GR55_BANK_UP, bank_size}
#define GR55_BANK_DOWN 33 //Command: {GR55_BANK_DOWN, bank_size}
#define GR55_PARAMETER 34 //Command: {GP10_PARAMETER, number, MOMENTARY/TOGGLE/TRISTATE, VALUE_ON, VALUE_OFF}
#define GR55_ASSIGN 35 //Command: {GP10_ASSIGN, assign_number, cc_number}
#define GR55_MUTE 36 //Command: {GR55_MUTE} - will mute the GR55, unless GR55_always_on is true
String GR55_patch_name = "                "; // Patchname displayed in the main display

//40 - 49: Roland VG99 types
#define VG99_PATCH 40  //Command: {VG99_PATCH, number, bank} // Select fixed patch number. Patch number will be: (bank * 100) + number
#define VG99_RELSEL 41 //Command: {VG99_RELSEL, number, bank_size}
#define VG99_BANK_UP 42 //Command: {VG99_BANK_UP, bank_size}
#define VG99_BANK_DOWN 43 //Command: {VG99_BANK_DOWN, bank_size}
#define VG99_PARAMETER 44 //Command: {GP10_PARAMETER, number, MOMENTARY/TOGGLE/TRISTATE, VALUE_ON, VALUE_OFF} // number = part * 30 + parameter_number
#define VG99_ASSIGN 45 //Command: {GP10_ASSIGN, assign_number, cc_number}
#define VG99_MUTE 46 //Command: {VG99_MUTE} - will mute the VG99, unless VG99_always_on is true
String VG99_patch_name = "                "; // Patchname displayed in the main display

//Toggle types
#define MOMENTARY 0
#define TOGGLE 1
#define TRISTATE 2
#define FOURSTATE 3
#define FIVESTATE 4

// Ports you can use - because they follow binary rules, you can also send commands to combinations of ports:
// To send to MIDI port 1, 2 and 3 - specify port as 1 + 2 + 4 = 7!
#define MIDI1_PORT 1
#define MIDI2_PORT 2
#define MIDI3_PORT 4
#define USBMIDI_PORT 8

// VG99 commands
#define FC300_CTL1 VG99_ASSIGN, 17, 1
#define FC300_CTL2 VG99_ASSIGN, 18, 2
#define FC300_CTL3 VG99_ASSIGN, 19, 3
#define FC300_CTL4 VG99_ASSIGN, 20, 4
#define FC300_CTL5 VG99_ASSIGN, 21, 5
#define FC300_CTL6 VG99_ASSIGN, 22, 6
#define FC300_CTL7 VG99_ASSIGN, 23, 7
#define FC300_CTL8 VG99_ASSIGN, 24, 8


