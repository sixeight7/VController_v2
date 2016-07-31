// Please read VController_v2.ino for information about the license and authors

// Configuration of the VController

#define NUMBER_OF_PAGES 11
#define NUMBER_OF_SWITCHES 16
#define NUMBER_OF_EXTERNAL_SWITCHES 8
#define NUMBER_OF_COMMANDS 3 // The number of commands one button can execute

struct Cmd_struct {
  uint8_t Type;
  uint8_t Data1;
  uint8_t Data2;
  uint8_t Value1;
  uint8_t Value2;
  uint8_t Value3;
  uint8_t Value4;
  uint8_t Value5;
};

struct Switch_struct {
  //char Label[17]; // Overrule label for switch
  Cmd_struct Cmd[NUMBER_OF_COMMANDS];
};

struct Page_struct {
  char Title[17];
  Switch_struct Switch[NUMBER_OF_SWITCHES];
};

uint8_t Current_page = 0;
uint8_t Previous_page = 0;

// Configuration of the pedal in PROGMEM
// All RELSEL and ASSIGN commands only work in first position!
#define PAGE_UNIT_OFF 0
#define PAGE_MODE_SELECT 1
#define PAGE_GP10_RELSEL 2
#define PAGE_GP10_PARAMETER 3
#define PAGE_GR55_RELSEL 4
#define PAGE_GR55_PARAMETER 5
#define PAGE_VG99_RELSEL 6
#define PAGE_VG99_PARAMETER 7
#define PAGE_COMBO1 8
#define PAGE_COMBO2 9
#define PAGE_ZG3_RELSEL 10

const PROGMEM Page_struct Page[NUMBER_OF_PAGES] = {
  // ******************************* PAGE 00 *************************************************
  { // Page settings:
    "                ", // Page title: Unit is off
    { // Switch settings:
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {STANDBYE}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 01 *************************************************
  { // Page settings:
    "MODE SELECT    ", // Page title: Unit is off
    { // Switch settings:
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {SELECT_PAGE, PAGE_COMBO2}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {SELECT_PAGE, PAGE_GP10_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {SELECT_PAGE, PAGE_GR55_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {SELECT_PAGE, PAGE_VG99_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {SELECT_PAGE, PAGE_COMBO1}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {SELECT_PAGE, PAGE_GP10_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {SELECT_PAGE, PAGE_GR55_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {SELECT_PAGE, PAGE_VG99_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {SELECT_PAGE, PAGE_ZG3_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {STANDBYE}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 02 *************************************************
  { // Page settings:
    "GP-10", // Page title
    { // Switch settings:
      {{ {GP10_RELSEL, 1, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 01 **
      {{ {GP10_RELSEL, 2, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 02 **
      {{ {GP10_RELSEL, 3, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 03 **
      {{ {GP10_RELSEL, 4, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 04 **
      {{ {GP10_RELSEL, 5, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 05 **
      {{ {GP10_RELSEL, 6, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 06 **
      {{ {GP10_RELSEL, 7, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 07 **
      {{ {GP10_RELSEL, 8, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 08 **
      {{ {GP10_RELSEL, 9, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 09 **
      {{ {GP10_RELSEL, 10, 10}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 10 **
      {{ {GP10_PARAMETER, 16, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {GP10_BANK_DOWN, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {GP10_BANK_UP, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_GP10_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 03 *************************************************
  { // Page settings:
    "PAR GP-10", // Page title
    { // Switch settings:
      {{ {GP10_ASSIGN, 1, 21}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {GP10_ASSIGN, 2, 22}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {GP10_ASSIGN, 3, 23}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {GP10_ASSIGN, 4, 24}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {GP10_PARAMETER, 0, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {GP10_PARAMETER, 1, FIVESTATE, 0, 4, 8, 9, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {GP10_PARAMETER, 3, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {GP10_PARAMETER, 4, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {GP10_PARAMETER, 16, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_GP10_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 04 *************************************************
  { // Page settings:
    "GR-55", // Page title
    { // Switch settings:
      {{ {GR55_RELSEL, 1, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 01 **
      {{ {GR55_RELSEL, 2, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 02 **
      {{ {GR55_RELSEL, 3, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 03 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {GR55_RELSEL, 4, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 05 **
      {{ {GR55_RELSEL, 5, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 06 **
      {{ {GR55_RELSEL, 6, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 07 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {GR55_RELSEL, 7, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 09 **
      {{ {GR55_RELSEL, 8, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 10 **
      {{ {GR55_RELSEL, 9, 9}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {GR55_BANK_DOWN, 9}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {GR55_BANK_UP, 9}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_GR55_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 05 *************************************************
  { // Page settings:
    "PAR GR-55", // Page title
    { // Switch settings:
      {{ {GR55_ASSIGN, 6, 26}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {GR55_ASSIGN, 7, 27}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {GR55_ASSIGN, 8, 28}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {GR55_PARAMETER, 0, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {GR55_PARAMETER, 1, FIVESTATE, 1, 3, 5, 6, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {GR55_PARAMETER, 9, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {GR55_PARAMETER, 2, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {GR55_PARAMETER, 3, FIVESTATE, 5, 1, 2, 3, 4}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {GR55_PARAMETER, 8, TOGGLE, 1, 0}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_GR55_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 06 *************************************************
  { // Page settings:
    "VG-99", // Page title
    { // Switch settings:
      {{ {VG99_RELSEL, 1, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 01 **
      {{ {VG99_RELSEL, 2, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 02 **
      {{ {VG99_RELSEL, 3, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 03 **
      {{ {VG99_RELSEL, 4, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 04 **
      {{ {VG99_RELSEL, 5, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 05 **
      {{ {VG99_RELSEL, 6, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 06 **
      {{ {VG99_RELSEL, 7, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 07 **
      {{ {VG99_RELSEL, 8, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 08 **
      {{ {VG99_RELSEL, 9, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 09 **
      {{ {VG99_RELSEL, 10, 10}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 10 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {VG99_BANK_DOWN, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {VG99_BANK_UP, 10}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_VG99_PARAMETER}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },
  // ******************************* PAGE 07 *************************************************
  { // Page settings:
    "PAR VG-99", // Page title
    { // Switch settings:
      {{ {FC300_CTL1}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {FC300_CTL2}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {FC300_CTL3}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {FC300_CTL4}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {FC300_CTL5}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {FC300_CTL6}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {FC300_CTL7}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {FC300_CTL8}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {SELECT_PAGE, PAGE_VG99_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
    }
  },// ******************************* PAGE 08 *************************************************
  { // Page settings:
    "GP+GR", // Page title
    { // Switch settings:
      {{ {GP10_RELSEL, 1, 5}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 01 **
      {{ {GP10_RELSEL, 2, 5}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 02 **
      {{ {GP10_RELSEL, 3, 5}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 03 **
      {{ {GP10_RELSEL, 4, 5}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 04 **
      {{ {GR55_RELSEL, 1, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 05 **
      {{ {GR55_RELSEL, 2, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 06 **
      {{ {GR55_RELSEL, 3, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 07 **
      {{ {GP10_RELSEL, 5, 5}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 08 **
      {{ {GR55_RELSEL, 4, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 09 **
      {{ {GR55_RELSEL, 5, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 10 **
      {{ {GR55_RELSEL, 6, 6}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {GP10_BANK_DOWN, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {GP10_BANK_UP, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_COMBO2}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },// ******************************* PAGE 09 *************************************************
  { // Page settings:
    "GPVGGR", // Page title
    { // Switch settings:
      {{ {GP10_RELSEL, 1, 4}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 01 **
      {{ {GP10_RELSEL, 2, 4}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 02 **
      {{ {GP10_RELSEL, 3, 4}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 03 **
      {{ {GP10_RELSEL, 4, 4}, {GR55_MUTE}, {VG99_MUTE} }}, // ** Switch 04 **
      {{ {VG99_RELSEL, 1, 4}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 05 **
      {{ {VG99_RELSEL, 2, 4}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 06 **
      {{ {VG99_RELSEL, 3, 4}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 07 **
      {{ {VG99_RELSEL, 4, 4}, {GP10_MUTE}, {GR55_MUTE} }}, // ** Switch 08 **
      {{ {GR55_RELSEL, 1, 3}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 09 **
      {{ {GR55_RELSEL, 2, 3}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 10 **
      {{ {GR55_RELSEL, 3, 3}, {GP10_MUTE}, {VG99_MUTE} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {COMBI_BANK_DOWN, 4}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {COMBI_BANK_UP, 4}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, PAGE_COMBO1}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  },// ******************************* PAGE 10 *************************************************
  { // Page settings:
    "ZM G3", // Page title: Zoom G3
    { // Switch settings:
      {{ {ZG3_RELSEL, 1, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 01 **
      {{ {ZG3_RELSEL, 2, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 02 **
      {{ {ZG3_RELSEL, 3, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 03 **
      {{ {ZG3_RELSEL, 4, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 04 **
      {{ {ZG3_FX_TOGGLE, 1}, {NOTHING}, {NOTHING} }}, // ** Switch 05 **
      {{ {ZG3_FX_TOGGLE, 2}, {NOTHING}, {NOTHING} }}, // ** Switch 06 **
      {{ {ZG3_FX_TOGGLE, 3}, {NOTHING}, {NOTHING} }}, // ** Switch 07 **
      {{ {ZG3_RELSEL, 5, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 08 **
      {{ {ZG3_FX_TOGGLE, 4}, {NOTHING}, {NOTHING} }}, // ** Switch 09 **
      {{ {ZG3_FX_TOGGLE, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 10 **
      {{ {ZG3_FX_TOGGLE, 6}, {NOTHING}, {NOTHING} }}, // ** Switch 11 **
      {{ {TAP_TEMPO}, {NOTHING}, {NOTHING} }}, // ** Switch 12 **
      {{ {ZG3_BANK_DOWN, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 13 **
      {{ {ZG3_BANK_UP, 5}, {NOTHING}, {NOTHING} }}, // ** Switch 14 **
      {{ {SELECT_NEXT_DEVICE}, {NOTHING}, {NOTHING} }}, // ** Switch 15 **
      {{ {SELECT_PAGE, GP10_RELSEL}, {NOTHING}, {NOTHING} }}, // ** Switch 16 **
    }
  }
};

//External expression pedals - what data do they send?
uint8_t Exp_CC_number[4] = {7, 30, 31, 32}; // The cc numbers that are being sent.
uint8_t Exp_CC_channel[4] = {1, 1, 1, 1}; // The MIDI channel of the cc messages
uint8_t Exp_CC_port[4] = {ALL_PORTS, ALL_PORTS, ALL_PORTS, ALL_PORTS}; // the port of the cc messages

/*const PROGMEM Switch_struct Ext_switch[NUMBER_OF_EXTERNAL_SWITCHES] = {
  {{ {GP10_PARAMETER, 1, RANGE, 0, 10}, {NOTHING}, {NOTHING} }}, // External switch 1 and Expression pedal 1
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 2
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 3 and Expression pedal 2
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 4
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 5 and Expression pedal 3
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 6
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 7 and Expression pedal 4
  {{ {NOTHING}, {NOTHING}, {NOTHING} }}, // External switch 8
};*/

boolean switch_state[NUMBER_OF_PAGES][NUMBER_OF_SWITCHES];

#define SP_LABEL_SIZE 16
// Switch parameter memory
struct SP_struct {
  uint8_t Type;
  bool Read;            // Switches do not always need reading - after load this variable will indicate if it is neccesary or not.
  uint8_t State;        // State of the switch: on (1) or off(0) or three extra states for TRI/FOUR/FIVESTATE (2-4)
  uint8_t LED_state;    // State of the LED: on (1), off (0), dimmed (2) or blink (3)
  bool Pressed;         // True when switch is pressed, false when released.
  uint8_t Latch;            // MOMENTARY (0), LATCH (1) TRI/FOUR/FIVESTATE (2-4)
  char Title[SP_LABEL_SIZE + 1];        //Title shows on LCD line 1
  char Label[SP_LABEL_SIZE + 1];        //Label shows on LCD line 2
  uint8_t Colour;        //LED settings
  uint8_t Device;        //Which device will be read?
  uint8_t Trigger;       // The trigger of the pedal
  uint16_t PP_number;    //When it is a patch, here we store the patch number, for an parameter/assign it is the pointer to the Parameter table
  uint32_t Address;      // The address that needs to be read
  uint8_t Assign_number; // The number of the assign
  bool Assign_on;        // Is the assign on?
  //uint16_t Assign_target;// Assign: target
  uint16_t Assign_min;   // Assign: min-value (switch is off)
  uint16_t Assign_max;   // Assign: max_value (switch is on)
  uint8_t Target_byte1;  // Once the assign target is known, the state of the target is read into two bytes
  uint8_t Target_byte2;  // This byte often contains the type of the assign - which we exploit in the part of parameter feedback
  uint8_t Target_byte3;  // For the Zoom G5, i needed some extra locations for target data (1 for each FX slot)
  uint8_t Target_byte4;
  uint8_t Target_byte5;
  uint8_t Target_byte6;
};

// Reserve the memory for the switches on the page and the external switches
SP_struct SP[NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES];  // SP = Switch Parameters

