// Please read VController_v2.ino for information about the license and authors

// Here we find the settings for all the devices
#define NUMBER_OF_DEVICES 4
#define GP10 0
#define GR55 1
#define VG99 2
#define ZG3 3

// Common settings
bool VController_on = true; // Does the VController start up switched on or off
bool SEND_GLOBAL_TEMPO_AFTER_PATCH_CHANGE = true; // If true, the tempo of all patches will remain the same. Set it by using the tap tempo of the V-Controller
bool US20_emulation_active = true; // Switch software emulation of US20 on and off
bool PHYSICAL_LEDS = true; // Does the VController have Physical LEDs
bool VIRTUAL_LEDS = false; // Do you want to switch on Virtual LEDs - state indicators on displays?

// External input settings:
// You can connect either a dual switch or an expression pedal to an external port
#define PEDAL true
#define SWITCH false
bool Ext12type = PEDAL;
bool Ext34type = PEDAL;
bool Ext56type = PEDAL;
bool Ext78type = PEDAL;

uint8_t Expr_value = 0;

// Boss GP-10 settings:
#define GP10_MIDI_CHANNEL 1 // Was unable to change patch when GP-10 channel was not 1. Seems to be a bug in the GP-10
// GP-10 patch extend (when using bank up/down) - set values between 0 and 99.
#define GP10_PATCH_MIN 0
#define GP10_PATCH_MAX 98

// Boss GP-10 variables:
uint8_t GP10_MIDI_channel = GP10_MIDI_CHANNEL;
uint8_t GP10_device_id;
uint8_t GP10_patch_number = 0;
uint8_t GP10_bank_number = 0;
uint8_t GP10_bank_size = 10;
uint8_t GP10_not_detected = 0;
#define GP10_MAX_NOT_DETECTED 2 // The number of times the GP-10 does not have to respond before disconnection 
boolean GP10_connected = false;
boolean GP10_bank_selection_active = false;
bool GP10_on = false;
bool GP10_always_on = true;
uint8_t GP10_COSM_onoff = 0;
uint8_t GP10_nrml_pu_onoff = 0;
//uint8_t GP10_select_LED;
uint8_t GP10_assign_read = false; // Assigns are read in three steps on the GP10 1) Read first 64 bytes, 2) Read last 64 bytes, 3) Read targets
uint8_t GP10_assign_mem[0x80]; // Memory space for the data from the assigns

// Roland GR-55 settings:
#define GR55_MIDI_CHANNEL 8
// GR-55 bank extend (when using bank up/down) - set values between 0 and 99 for just the user patches and 0 and 219 for user and preset banks (guitar mode)
// and 0 and 135 for user and presets in bass mode. When we are in bass mode, there is a check whether we exceeded 135.
// To keep the banks equal, make sure the numbers can be divided by three, because we always have three banks on display
//#define GR55_BANK_MIN 0
//uint8_t GR55_BANK_MAX = 219;
#define GR55_PATCH_MIN 0
#define GR55_PATCH_MAX 657

// Roland GR-55 variables:
uint8_t GR55_MIDI_channel = GR55_MIDI_CHANNEL;
uint8_t GR55_device_id;
uint16_t GR55_patch_number = 0;
uint16_t GR55_previous_patch_number = 0;
uint16_t GR55_bank_number = 1;
uint8_t GR55_bank_size = 3;
uint8_t GR55_CC01 = 0;    // the MIDI CC #01 sent by the GR-55
uint8_t GR55_not_detected = 0;
#define GR55_MAX_NOT_DETECTED 2 // The number of times the GP-10 does not have to respond before disconnection
boolean GR55_connected = false;
boolean GR55_bank_selection_active = false;
uint8_t GR55_preset_banks = 40; // Default number of preset banks is 40. When we are in bass mode, there are only 12.
bool GR55_on = false;
bool GR55_always_on = true;
uint8_t GR55_synth1_onoff = 0;
uint8_t GR55_synth2_onoff = 0;
uint8_t GR55_COSM_onoff = 0;
uint8_t GR55_nrml_pu_onoff = 0;
//uint8_t GR55_select_LED;

// Roland VG-99 settings:
#define VG99_MIDI_CHANNEL 9
// VG-99 bank extend (when using bank up/down) - set values between 0 and 19 for just the user patches and 0 and 39 for user and preset banks
#define VG99_PATCH_MIN 0
#define VG99_PATCH_MAX 399

// Roland VG-99 variables:
uint8_t VG99_MIDI_channel = VG99_MIDI_CHANNEL;
uint8_t VG99_device_id;
uint8_t FC300_device_id;
uint16_t VG99_patch_number = 0;
uint16_t VG99_previous_patch_number = 0;
uint8_t VG99_bank_number = 0;
uint8_t VG99_bank_size = 10;
uint8_t VG99_CC01 = 0;    // the MIDI CC #01 sent by the GR-55
uint8_t VG99_not_detected = 0;
#define VG99_MAX_NOT_DETECTED 2 // The number of times the VG-99 does not have to respond before disconnection
boolean VG99_connected = false;
boolean VG99_bank_selection_active = false;
bool VG99_on = false;
bool VG99_always_on = true;
uint8_t VG99_COSM_A_onoff = 0;
uint8_t VG99_COSM_B_onoff = 0;
//uint8_t VG99_select_LED;

// Zoom G3 settings:
#define ZG3_MIDI_CHANNEL 1
// Zoom G3 patch extend (when using bank up/down) - set values between 0 and 99.
#define ZG3_PATCH_MIN 0
#define ZG3_PATCH_MAX 99

// Zoom G3 variables:
uint8_t ZG3_MIDI_channel = ZG3_MIDI_CHANNEL;
uint8_t ZG3_not_detected = 0;
#define ZG3_MAX_NOT_DETECTED 2 // The number of times the Zoom G3 does not have to respond before disconnection
boolean ZG3_connected = false;
uint8_t ZG3_device_id;
uint8_t ZG3_patch_number = 0;
uint8_t ZG3_bank_number = 0;
uint8_t ZG3_bank_size = 10;
boolean ZG3_bank_selection_active = false;
bool ZG3_on = false;

uint16_t bpm=120;

#define OFF 0
#define FULL 1
#define PAR_ONLY 2
uint8_t update_page = OFF;

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed

