// VController v2
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
  main_switch_check(); //Check for switches pressed
  main_switch_control(); // take the configured action
  main_LED_control(); //Update the LEDs
  main_LCD_control();
  main_MIDI_common(); //Read MIDI ports
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
- Teensy 3.1 or Teensy LC
- 3 MIDI input/outputs and MIDI over USB
- Extra EEPROM Flash memory: 24LC512

Software features:
- Patch and parameter control for Boss GP-10, Roland GR-55 and Roland VG-99
- Patchnames are read from devices
- Parameter states and assigns are read from devices
- Programmable pages of switches. There can be programmed on the Config page

Software structure:
- Initialization of LCDs, LEDs and MIDI ports
- Configuration of the switches is done in the page[] array on the Config page.
- On startup, detection of devices, page change or patch change the current page is loaded in the SP[] array (void load_page)
- After the page is loaded, the patchnames and parameter states are read (void Request_first_parameter, Request_next_parameter and Request_current_parameter)
- Main loop checks for switches being pressed (Switch_check/Switch Control), midi received and update of LCD and LED states.

*/

