// Please read VController_v2.ino for information about the license and authors

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
bool status_message_showing = false;
bool update_main_lcd = false; // True if main display needs updating

String Current_patch_number_string = "";
String Current_patch_name = "                "; // Patchname displayed in the main display
uint16_t Current_patch_number = 0;              // Patchnumber displayed in the main display
uint8_t Current_device = 0;                     // The device from which the patchname comes

String Display_number_string[NUMBER_OF_DISPLAYS]; // Placeholder for patchnumbers on display
String Blank_line = "                ";

void setup_LCD_control()
{
  // Initialize main LCD
  Main_lcd.begin (16, 2); // Main LCD is 16x2
  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(VController_on); // Backlight state is the same as initial on or off state...

  // Show startup screen
  if (VController_on) LCD_show_startup_message();

  // Initialize individual LCDs
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    lcd[i].setBacklight(VController_on); // Backlight state is the same as initial on or off state...
    LCD_init_virtual_LED(i); // Initialize the virtual LEDs
  }
}

void main_LCD_control()
{
  if (VController_on) {
    if  ((update_main_lcd == true) && (status_message_showing == false)) {
      update_main_lcd = false;

      Main_lcd.home();
      String msg = Page[Current_page].Title;
      msg.trim();
      Main_lcd.print(Blank_line.substring(0, 16 - msg.length()) + msg);
      LCD_set_patch_number_and_name();
      Main_lcd.setCursor (0, 0);
      Main_lcd.print(Current_patch_number_string);
      Main_lcd.setCursor (0, 1);       // go to start of 2nd line
      Main_lcd.print(Current_patch_name); // Show the current patchname
    }
    
    if ((status_message_showing) && (millis() - messageTimer >= MESSAGE_TIMER_LENGTH)) {
      status_message_showing = false;
      update_main_lcd = true; // Now update the main display, so the status message will be overwritten
    }
  }
}

void LCD_update(uint8_t number) {

  if (number < NUMBER_OF_DISPLAYS) { // Check if there is a display for this switch

    //Determine what to display from the Switch type
    DEBUGMSG("Update display no:" + String(number));

    switch (SP[number].Type) {
      case GP10_PATCH:
      case GP10_RELSEL:
        // What to show on the individual display
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
        LCD_centre_print_label(number);
        break;
      case GP10_PARAMETER:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ GP10 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case GP10_ASSIGN:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print("[GP10 ASGN" + String(SP[number].Assign_number) + "]");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case GR55_PATCH:
      case GR55_RELSEL:
        // What to show on the individual display
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
        LCD_centre_print_label(number);
        break;
      case GR55_PARAMETER:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ GR55 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case GR55_ASSIGN:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print("[GR55 ASGN" + String(SP[number].Assign_number) + "]" + char(0) + char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case VG99_PATCH:
      case VG99_RELSEL:
        // What to show on the individual display
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
        LCD_centre_print_label(number);
        break;
      case VG99_PARAMETER:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0)); // Virtual LEDs
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ VG99 ] ");
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case VG99_ASSIGN:
        // What to show on the individual display
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
        LCD_centre_print_label(number);
        break;
      case ZG3_PATCH:
      case ZG3_RELSEL:
        // What to show on the individual display
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
        LCD_centre_print_label(number);
        break;
      case ZG3_FX_TOGGLE:
        // What to show on the individual display
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(" [ FX" + String(SP[number].PP_number) + " ]  " );
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        lcd[number].print(char(0));
        //lcd[number].print(SP[number].Label); // Show the current patchname
        LCD_centre_print_label(number);
        break;
      case SELECT_PAGE:
        lcd[number].setCursor (0, 0);
        lcd[number].print(char(0));
        lcd[number].print(" SELECT PAGE  ");
        lcd[number].print(char(0));
        LCD_centre_print_label(number);
        DEBUGMSG("Number: " + String(number) + " Label: " + SP[number].Label);
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
}

void LCD_show_status_message(String message)
{
  if (VController_on) {
    Main_lcd.home();
    Main_lcd.setCursor (0, 1);       // go to start of 2nd line
    Main_lcd.print(message + Blank_line.substring(0, 16 - message.length())); //Print message plus remaining spaces
    messageTimer = millis();
    status_message_showing = true;
  }
}

void LCD_clear_label(uint8_t no) {
  for (uint8_t i = 0; i < 16; i++) {
    SP[no].Label[i] = ' ';
  }
}

void LCD_set_title(uint8_t no, String &msg) {
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[no].Title[i] = msg[i];
  }
}

void LCD_set_label(uint8_t no, String &msg) {
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[no].Label[i] = msg[i];
  }
}

void LCD_set_current_patch_name(uint8_t no) { //Copies the patchname from the SP array to the main display
  for (uint8_t i = 0; i < SP_LABEL_SIZE; i++) {
    Current_patch_name[i] = SP[no].Label[i];
  }
}


#define PATCH_NUMBER_SEPERATOR "+"

void LCD_set_patch_number_and_name() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  Current_patch_number_string = "";
  String patch_names[NUMBER_OF_DEVICES]; //Array of strings for the patchnames

  if (GP10_on) {
    GP10_display_patch_number_string(); // Adds GP-10 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = GP10_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (GR55_on) {
    GR55_display_patch_number_string(); // Adds GR-55 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = GR55_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (VG99_on) {
    VG99_display_patch_number_string(); // Adds VG-99 name to patch number string
    Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
    patch_names[number_of_active_devices] = VG99_patch_name; //Add patchname to string
    number_of_active_devices++;
  }

  if (ZG3_on) {
    ZG3_display_patch_number_string(); // Adds ZG3 name to patch number string
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
}

void LCD_centre_print_title(uint8_t s) {
  // Find out the number of spaces at the end of the label
  uint8_t endpoint = SP_LABEL_SIZE - 1; // Go to last character
  while ((endpoint > 0) && (SP[s].Title[endpoint] == ' ')) endpoint--; //Find last character that is not a space

  // Find the correct position on second line and print label
  lcd[s].setCursor (0, 0); // Go to start of 2nd line
  lcd[s].print("                "); // Clear the title
  lcd[s].setCursor ((SP_LABEL_SIZE - endpoint - 1) / 2, 1);  // Set the cursor to the start of the label
  lcd[s].print(SP[s].Title); // Print it here.
}

void LCD_centre_print_label(uint8_t s) {
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
// A virtual LED is user defined character 0 on the display. The virtaul LED is changed by changing the user defined character.
// To have one or more virtual LEDS in a display message, user defined character 0 has to be included on the position where the LEDs have to appear
// The state of these virtual LEDs is set in the LEDs section.

uint8_t Display_LED[NUMBER_OF_SWITCHES]; // For showing state of the LEDs on the display. Can have state on (1), off (0) and dimmed (2)
// Custom characters for virtual LEDs
byte vLED_on[8] = { // The character for LED on
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_off[8] = { // The character for LED off
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_dimmed[8] = { // The character for LED dimmed
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};

void LCD_init_virtual_LED(uint8_t number) { // Initialize virtual LED
  Display_LED[number] = 0;
  if (number < NUMBER_OF_DISPLAYS) lcd[number].createChar(0, vLED_off);
}

void LCD_set_virtual_LED(uint8_t number, uint8_t state) { // Will set the state of a virtual LED
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

void Set_virtual_LED_colour(uint8_t number, uint8_t colour) { // Called from LEDs.ino-show_colour()
  if ((colour == 0) | (colour > 9)) LCD_set_virtual_LED(number, 0); //Virtual LED off
  else LCD_set_virtual_LED(number, 1); //Virtual LED on

  /*if (colour == 0) LCD_set_virtual_LED(number, 0); //Virtual LED off
  else if (colour < 10) LCD_set_virtual_LED(number, 1); //Virtual LED on
  else if (colour > 10) LCD_set_virtual_LED(number, 2); //Dimmed
  else LCD_set_virtual_LED(number, 0); //Virtual LED off for colour 10 as well*/
}

void LCD_show_startup_message() {
  Main_lcd.home(); // go home
  Main_lcd.print("V-controller v2");  // Show startup message
  LCD_show_status_message("  by SixEight");  //Please give me the credits :-)
}
