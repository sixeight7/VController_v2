// Here we find the settings for all the devices
#define GP10 1
#define GR55 2
#define VG99 3
#define ZG3 4

// Common settings
bool VController_on = false;
bool SEND_GLOBAL_TEMPO_AFTER_PATCH_CHANGE = true; // If true, the tempo of all patches will remain the same. Set it by using the tap tempo of the V-Controller
bool US20_emulation_active = true; // Switch software emulation of US20 on and off

// External input settings:
// You can connect either a dual switch or an expression pedal to an external port
#define PEDAL true
#define SWITCH false
bool Ext12type = SWITCH;
bool Ext34type = SWITCH;
bool Ext56type = SWITCH;
bool Ext78type = SWITCH;

// Boss GP-10 settings:
#define GP10_MIDI_CHANNEL 1
// GP-10 patch extend (when using bank up/down) - set values between 0 and 99.
#define GP10_PATCH_MIN 0
#define GP10_PATCH_MAX 98

// Boss GP-10 variables:
uint8_t GP10_MIDI_channel = GP10_MIDI_CHANNEL;
uint8_t GP10_device_id;
uint8_t GP10_patch_number = 0;
uint8_t GP10_bank_number = 0;
uint8_t GP10_bank_size = 10;
boolean GP10_detected = false;
boolean GP10_bank_selection_active = false;
bool GP10_on = false;
bool GP10_always_on = true;
uint8_t GP10_COSM_onoff = 0;
uint8_t GP10_nrml_pu_onoff = 0;
//uint8_t GP10_select_LED;
uint8_t GP10_assign_read = false; // Assigns are read in three steps on the GP10 1) Read first 64 bytes, 2) Read last 64 bytes, 3) Read targets
uint8_t GP10_assign_mem[0x80]; // Memory space for the data from the assigns

// Roland GR-55 settings:
#define GR55_MIDI_CHANNEL 7
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
boolean GR55_detected = false;
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
#define VG99_MIDI_CHANNEL 8
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
boolean VG99_detected = false;
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
boolean ZG3_detected = false;
uint8_t ZG3_device_id;
uint8_t ZG3_patch_number = 0;
uint8_t ZG3_bank_number = 0;
uint8_t ZG3_bank_size = 10;
boolean ZG3_bank_selection_active = false;
bool ZG3_on = false;

uint16_t bpm=120;

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed
