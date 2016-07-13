// Functions for LCD control
// Main LCD is a large 16x2 LCD display with a serial i2c module attached
// Futhermore we have 12 16x2 LCD displays for the bottom 12 switches

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// ***************************** Hardware settings *****************************
#define BACKLIGHT_PIN     3
#define EN_PIN  2
#define RW_PIN  1
#define RS_PIN  0
#define D4_PIN  4
#define D5_PIN  5
#define D6_PIN  6
#define D7_PIN  7

// Main display:
LiquidCrystal_I2C	Main_lcd(0x27, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display

// Displays above the switches
#define NUMBER_OF_DISPLAYS 12

//Declare an array of LiquidCrystal objects, each with the same pin settings, just a different i2c address
LiquidCrystal_I2C lcd[NUMBER_OF_DISPLAYS] = {
  LiquidCrystal_I2C (0x21, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x22, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x23, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x24, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x25, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x26, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x39, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x3A, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x3B, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x3D, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x20, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
  LiquidCrystal_I2C (0x3E, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN)
};

#define MESSAGE_TIMER_LENGTH 1500 // time that status messages are shown (in msec)
unsigned long messageTimer = 0;
uint8_t update_lcd = 0; // True if individual display needs updating
bool update_main_lcd = false; // True if main display needs updating
#define OFF 0
#define FULL 1
#define PARAMETERS 2
bool update_switch_lcds = OFF; // True if main display needs updating

String Current_patch_number_string = "";
String Current_patch_name = "                "; // Patchname displayed in the main display
uint16_t Current_patch_number = 0;              // Patchnumber displayed in the main display
uint8_t Current_device = 0;                     // The device from which the patchname comes

String Display_number_string[NUMBER_OF_DISPLAYS]; // Placeholder for patchnumbers on display

// A virtual LED is a user defined character on the display
// The state of these LEDs are set in the LED section.
// It is updated from there.
// Custom characters for virtual LEDs
uint8_t Display_LED[NUMBER_OF_SWITCHES]; // For showing state of the LEDs on the display. Can have state on (1), off (0) and dimmed (2)
byte vLED_on[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000
};
byte vLED_off[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_dimmed[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void setup_LCD_control()
{
  Main_lcd.begin (16, 2); //  <<----- My LCD was 16x2

  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(LOW); //switch off the backlight on the main display

  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    lcd[i].setBacklight(LOW); // Switch off the backlight
    Init_virtual_LED(i); // Initialize the virtual LEDs
  }
}

void main_LCD_control()
{
  // Display mode, unless a status message is being displayed
  if (update_lcd > 0) {
    update_LCDs(update_lcd - 1); // Updates the individual LCDs
    update_lcd = 0;
  }

  if (VController_on) {
    if  ((update_main_lcd == true) && (millis() - messageTimer >= MESSAGE_TIMER_LENGTH)) {
      update_main_lcd = false;

      Main_lcd.home();
      Main_lcd.print(Page[Current_page].Title);
      set_patch_number_and_name();
      Main_lcd.setCursor (0, 0);
      Main_lcd.print(Current_patch_number_string);
      Main_lcd.setCursor (0, 1);       // go to start of 2nd line
      Main_lcd.print(Current_patch_name); // Show the current patchname
    }

    if (update_switch_lcds != OFF) {
      if (update_switch_lcds == FULL) load_current_page(true); //Re-read the page completely
      if (update_switch_lcds == PARAMETERS) load_current_page(false); //Re-read the page - but just the parameters
      update_switch_lcds = OFF;
    }
  }
}

void update_LCDs(uint8_t number) {

  //Determine what to display from the Switch type
  DEBUGMSG("Update display no:" + String(number));

  switch (SP[number].Type) {
    case GP10_PATCH:
    case GP10_RELSEL:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        Display_number_string[number] = ""; //Clear the display_number_string
        GP10_number_format(SP[number].PP_number, Display_number_string[number]);
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("   " + Display_number_string[number] + "    ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }

      // What to show on the main display
      /*if ((Current_device == GP10) && (GP10_patch_number == SP[number].PP_number)) { //Show this patchnumber on the main diplay
        display_GP10_patch_number_string();
        set_current_patch_name(number);
        update_main_lcd = true;
      }*/
      break;
    case GP10_PARAMETER:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ GP10 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case GP10_ASSIGN:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print("[GP10 ASGN" + String(SP[number].Assign_number) + "]");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case GR55_PATCH:
    case GR55_RELSEL:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        Display_number_string[number] = ""; //Clear the display_number_string
        GR55_number_format(SP[number].PP_number, Display_number_string[number]);
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("  " + Display_number_string[number] + "  ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      // What to show on the main display
      /*if ((Current_device == GR55) && (GR55_patch_number == SP[number].PP_number)) { //Show this patchnumber on the main diplay
        display_GR55_patch_number_string();
        set_current_patch_name(number);
        update_main_lcd = true;
      }*/
      break;
    case GR55_PARAMETER:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ GR55 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case GR55_ASSIGN:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("[GR55 ASGN" + String(SP[number].Assign_number) + "]" + char(0) + char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case VG99_PATCH:
    case VG99_RELSEL:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        Display_number_string[number] = ""; //Clear the display_number_string
        VG99_number_format(SP[number].PP_number, Display_number_string[number]);
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("   " + Display_number_string[number] + "   ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }

      // What to show on the main display
      /*if ((Current_device == VG99) && (VG99_patch_number == SP[number].PP_number)) { //Show this patchnumber on the main diplay
        display_VG99_patch_number_string();
        set_current_patch_name(number);
        update_main_lcd = true;
      }*/
      break;
    case VG99_PARAMETER:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ VG99 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case VG99_ASSIGN:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        if (SP[number].Assign_number < 16) { // Normal assign
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].print("[VG99 ASGN" + String(SP[number].Assign_number) + "]");
          lcd[number].print(char(0));
          lcd[number].print(char(0));
        }
        else {
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].print("   [CTL" + String(SP[number].Assign_number - 16) + "]   ");
          lcd[number].print(char(0));
          lcd[number].print(char(0));
        }
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case ZG3_PATCH:
    case ZG3_RELSEL:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        Display_number_string[number] = ""; //Clear the display_number_string
        ZG3_number_format(SP[number].PP_number, Display_number_string[number]);
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("    " + Display_number_string[number] + "    ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }

      // What to show on the main display
      /*if ((Current_device == GP10) && (GP10_patch_number == SP[number].PP_number)) { //Show this patchnumber on the main diplay
        display_GP10_patch_number_string();
        set_current_patch_name(number);
        update_main_lcd = true;
      }*/
      break;
    case ZG3_FX_TOGGLE:
      // What to show on the individual display
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ FX" + String(SP[number].PP_number) + " ]  " );
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        centre_print_label(number);
      }
      break;
    case GP10_BANK_UP:
    case GP10_BANK_DOWN:
    case GR55_BANK_UP:
    case GR55_BANK_DOWN:
    case VG99_BANK_UP:
    case VG99_BANK_DOWN:
    case COMBI_BANK_UP:
    case COMBI_BANK_DOWN:
      //  set_patch_number_string();
      break;
    case TAP_TEMPO:
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(" <TAP TEMPO>  ");
        lcd[number].print(char(0));
        lcd[number].setCursor (0, 1);
        lcd[number].print("    " + String(bpm) + " BPM    ");
      }
      break;
    default:
      if (number < NUMBER_OF_DISPLAYS) {
        lcd[number].setCursor (0, 0);
        lcd[number].print("                ");
        lcd[number].setCursor (0, 1);
        lcd[number].print("                ");
      }
  }
}

void show_status_message(String message)
{
  if (VController_on) {
    Main_lcd.home();
    Main_lcd.setCursor (0, 1);       // go to start of 2nd line
    Main_lcd.print("                "); //Clear the line first
    Main_lcd.setCursor (0, 1);       // go to start of 2nd line
    Main_lcd.print(message);
    messageTimer = millis();
  }
}

void clear_label(uint8_t no) {
  for (uint8_t i = 0; i < 16; i++) {
    SP[no].Label[i] = ' ';
  }
}

void set_title(uint8_t no, String &msg) {
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[no].Title[i] = msg[i];
  }
}

void set_label(uint8_t no, String &msg) {
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[no].Label[i] = msg[i];
  }
}

void set_current_patch_name(uint8_t no) { //Copies the patchname from the SP array to the main display
  for (uint8_t i = 0; i < SP_LABEL_SIZE; i++) {
    Current_patch_name[i] = SP[no].Label[i];
  }
}


#define PATCH_NUMBER_SEPERATOR "+"
void set_patch_number_and_name() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  Current_patch_number_string = "";
  String patch_names[NUMBER_OF_DEVICES]; //Array of strings for the patchnames

  if (GP10_on) {
    display_GP10_patch_number_string(); // Adds GP10 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = GP10_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (GR55_on) {
    display_GR55_patch_number_string(); // Adds GP10 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = GR55_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (VG99_on) {
    display_VG99_patch_number_string(); // Adds GP10 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = VG99_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (ZG3_on) {
    display_ZG3_patch_number_string(); // Adds GP10 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = ZG3_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  // Cut last character of patch number string
  uint8_t l = Current_patch_number_string.length();
  Current_patch_number_string.remove(l - 1);

  if (number_of_active_devices > 2) Current_patch_number_string = Current_patch_number_string + "     "; //Add spaces at the end to clear any remaining leters

  // Show patchname
  switch (number_of_active_devices) {
    case 0:
      Current_patch_name = "                ";
      break;
    case 1: // Only one device active
      Current_patch_name = patch_names[0];
      break;
    case 2: // Show 7 bytes of both names
      Current_patch_name = patch_names[0].substring(0, 7) + "  " + patch_names[1].substring(0, 7);
      break;
    case 3: // Show 4 bytes of both names
      Current_patch_name = patch_names[0].substring(0, 4) + "  " + patch_names[1].substring(0, 4) + "  " + patch_names[2].substring(0, 4);
      break;
    default: // More then 3 devices
      Current_patch_name = String(number_of_active_devices) + " devices on    ";
      break;
  }


  /*uint8_t what_is_on = (VG99_on << 2) + (GR55_on << 1) + GP10_on; // Make number of three bits to quickly determine what is on and what of off
  Current_patch_number_string = "";
  switch (what_is_on) {
    case B001: //Just the GP10 is on
      display_GP10_patch_number_string();
      Current_patch_name = GP10_patch_name;
      break;
    case B010: //Just the GR55 is on
      display_GR55_patch_number_string();
      Current_patch_name = GR55_patch_name;
      break;
    case B100: //Just the VG99 is on
      display_VG99_patch_number_string();
      Current_patch_name = VG99_patch_name;
      break;
    case B011: //GP10 and GR55 are on
      display_GP10_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "+"; //Add a slash in between
      display_GR55_patch_number_string();
      Current_patch_name = GP10_patch_name.substring(0, 7) + "|" + GR55_patch_name.substring(0, 8);
      break;
    case B101: //GP10 and VG99 are on
      display_GP10_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "+"; //Add a slash in between
      display_VG99_patch_number_string();
      Current_patch_name = GP10_patch_name.substring(0, 7) + "|" + VG99_patch_name.substring(0, 8);
      break;
    case B110: //GR55 and VG99 are on
      display_GR55_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "+"; //Add a slash in between
      display_VG99_patch_number_string();
      Current_patch_name = GR55_patch_name.substring(0, 7) + "|" + VG99_patch_name.substring(0, 8);
      break;
    case B111: //GP10, GR55 and VG99 are on
      display_GP10_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "+"; //Add a slash in between
      display_GR55_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "+"; //Add a slash in between
      display_VG99_patch_number_string();
      Current_patch_number_string = Current_patch_number_string + "  "; //Add spaces at the end to clear any remaining leters
      Current_patch_name = GP10_patch_name.substring(0, 4) + "|" + GR55_patch_name.substring(0, 5) + "|" + VG99_patch_name.substring(0, 5);
      break;
  }*/
}

void display_GP10_patch_number_string() {
  // Uses GP10_patch_number as input and returns Current_patch_number_string as output in format "P01"
  if (GP10_bank_selection_active == false) {
    GP10_number_format(GP10_patch_number, Current_patch_number_string);
  }
  else {
    //Current_patch_number_string = "P" + String(GP10_bank_number) + "-";
    String start_number, end_number;
    GP10_number_format(GP10_bank_number * GP10_bank_size, start_number);
    GP10_number_format((GP10_bank_number + 1) * GP10_bank_size - 1, end_number);
    Current_patch_number_string = Current_patch_number_string + start_number + "-" + end_number;
  }
}

void GP10_number_format(uint8_t number, String &Output) {
  Output = Output + "P" + String((number + 1) / 10) + String((number + 1) % 10);
}

void display_GR55_patch_number_string() {
  if (GR55_bank_selection_active == false) {
    GR55_number_format(GR55_patch_number, Current_patch_number_string);
  }
  else {
    GR55_number_format(GR55_bank_number * GR55_bank_size, Current_patch_number_string);
  }
}

void GR55_number_format(uint16_t number, String &Output) {
  // Uses GR55_patch_number as input and returns Current_patch_number_string as output in format "U01-1"
  // First character is L for Lead, R for Rhythm, O for Other or U for User
  // In guitar mode GR55_preset_banks is set to 40, in bass mode it is set to 12, because there a less preset banks in bass mode.

  uint16_t patch_number_corrected = 0; // Need a corrected version of the patch number to deal with the funny numbering system of the GR-55
  uint16_t bank_number_corrected = 0; //Also needed, because with higher banks, we start counting again

  bank_number_corrected = (number / 3); // Calculate the bank number from the patch number

  if (bank_number_corrected < 99) {
    Output = Output + "U";
    patch_number_corrected = number;  //In the User bank all is normal
  }

  else {
    if (bank_number_corrected >= (99 + (2 * GR55_preset_banks))) {   // In the Other bank we have to adjust the bank and patch numbers so we start with O01-1
      Output = Output + "O";
      patch_number_corrected = number - (297 + (6 * GR55_preset_banks));
      bank_number_corrected = bank_number_corrected - (99 + (2 * GR55_preset_banks));
    }

    else {
      if (bank_number_corrected >= (99 + GR55_preset_banks)) {   // In the Rhythm bank we have to adjust the bank and patch numbers so we start with R01-1
        Output = Output + "R";
        patch_number_corrected = number - (297 + (3 * GR55_preset_banks));
        bank_number_corrected = bank_number_corrected - (99 + GR55_preset_banks);
      }

      else    {// In the Lead bank we have to adjust the bank and patch numbers so we start with L01-1
        Output = Output + "L";
        patch_number_corrected = number - 297;
        bank_number_corrected = bank_number_corrected - 99;
      }
    }
  }

  // Then add the bank number
  Output = Output + String(((patch_number_corrected / 3) + 1) / 10) + String(((patch_number_corrected / 3) + 1) % 10);
  // Finally add the patch number
  Output = Output + "-" + String((patch_number_corrected % 3) + 1);
}

void display_VG99_patch_number_string() {
  if (VG99_bank_selection_active == false) {
    VG99_number_format(VG99_patch_number, Current_patch_number_string);
  }
  else {
    //Current_patch_number_string = "P" + String(GP10_bank_number) + "-";
    String start_number1, end_number1;
    VG99_number_format(VG99_bank_number * VG99_bank_size, start_number1);
    VG99_number_format((VG99_bank_number + 1) * VG99_bank_size - 1, end_number1);
    Current_patch_number_string = Current_patch_number_string + start_number1 + "-" + end_number1;
  }

}

void VG99_number_format(uint16_t number, String &Output) {
  // Uses VG99_patch_number as input and returns Current_patch_number_string as output in format "U001"
  // First character is U for User or P for Preset patches
  if (number > 199) Output = Output + "P";
  else Output = Output + "U";

  // Then add the patch number
  Output = Output + String((number + 1) / 100) + String(((number + 1) / 10) % 10) + String((number + 1) % 10);
}

void display_ZG3_patch_number_string() {
  if (ZG3_bank_selection_active == false) {
    ZG3_number_format(ZG3_patch_number, Current_patch_number_string);
  }
  else {
    String start_number1, end_number1;
    ZG3_number_format(ZG3_bank_number * ZG3_bank_size, start_number1);
    ZG3_number_format((ZG3_bank_number + 1) * ZG3_bank_size - 1, end_number1);
    Current_patch_number_string = Current_patch_number_string + start_number1 + "-" + end_number1;
  }

}
void ZG3_number_format(uint8_t number, String &Output) {
  char BankChar = 65 + (number / 10);
  Output = Output + BankChar + String(number % 10);
}

void centre_print_title(uint8_t s) {
  // Find out the number of spaces at the end of the label
  uint8_t endpoint = SP_LABEL_SIZE - 1; // Go to last character
  while ((endpoint > 0) && (SP[s].Title[endpoint] == ' ')) endpoint--; //Find last character that is not a space

  // Find the correct position on second line and print label
  lcd[s].setCursor (0, 0); // Go to start of 2nd line
  lcd[s].print("                "); // Clear the title
  lcd[s].setCursor ((SP_LABEL_SIZE - endpoint - 1) / 2, 1);  // Set the cursor to the start of the label
  lcd[s].print(SP[s].Title); // Print it here.
}

void centre_print_label(uint8_t s) {
  // Find out the number of spaces at the end of the label
  uint8_t endpoint = SP_LABEL_SIZE - 1; // Go to last character
  while ((endpoint > 0) && (SP[s].Label[endpoint] == ' ')) endpoint--; //Find last character that is not a space

  // Find the correct position on second line and print label
  lcd[s].setCursor (0, 1); // Go to start of 2nd line
  lcd[s].print("        "); // Clear the start of the line
  lcd[s].setCursor ((SP_LABEL_SIZE - endpoint - 1) / 2, 1);  // Set the cursor to the start of the label
  lcd[s].print(SP[s].Label); // Print it here.
}

void LCD_backlight_on() { // Will switch all backlights on
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].setBacklight(HIGH);
  }
  Main_lcd.setBacklight(HIGH);
}

void LCD_backlight_off() { // Will switch all backlights off
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].setBacklight(LOW); //switch on the backlight
  }
  Main_lcd.setBacklight(LOW);
  Main_lcd.clear(); // Clear main display
}

//*** Virtual LEDs

void Init_virtual_LED(uint8_t number) { // Initialize virtual LED
  Display_LED[number] = 0;
  if (number < NUMBER_OF_DISPLAYS) lcd[number].createChar(0, vLED_off);
}

void Set_virtual_LED(uint8_t number, uint8_t state) { // Will set the state of a virtual LED
  if (Display_LED[number] != state) { // Check if state is new
    Display_LED[number] = state; // Update state
    // Update virtual LED
    if (number < NUMBER_OF_DISPLAYS) {
      if (state == 0) lcd[number].createChar(0, vLED_off);
      if (state == 1) lcd[number].createChar(0, vLED_on);
      if (state == 2) lcd[number].createChar(0, vLED_dimmed);
    }
  }
}

void Set_virtual_LED_colour(uint8_t number, uint8_t colour) {
  if ((colour == 0) | (colour > 9)) Set_virtual_LED(number, 0); //Virtual LED off
  else Set_virtual_LED(number, 1); //Virtual LED on
  
  /*if (colour == 0) Set_virtual_LED(number, 0); //Virtual LED off
  else if (colour < 10) Set_virtual_LED(number, 1); //Virtual LED on
  else if (colour > 10) Set_virtual_LED(number, 2); //Dimmed
  else Set_virtual_LED(number, 0); //Virtual LED off for colour 10 as well*/
}
