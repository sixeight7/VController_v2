// Please read VController_v2.ino for information about the license and authors

// Functions for LED control for which I use 12 5mm Neopixel RGB LEDs

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// ***************************** Hardware settings *****************************
// Which pin on the Arduino is connected to the NeoPixels LEDs?
// On a Trinket or Gemma we suggest changing this to 1
#define NEOPIXELLEDPIN            17

// HARDWARE settings
#define NUMLEDS      16 // Number of neopixel LEDs connected
uint8_t LED_order[16] = { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11, 15, 14, 13, 12}; //Order in which the LEDs are connected. First LED = 0

struct colour {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// ***************************** Settings you may want to change *****************************

// Which colours are used for which mode?
// pixels.Color takes GRB (Green - Red - Blue) values, from 0,0,0 up to 255,255,255
// The neopixels are very bright. I find setting them to 10 is just fine for an indicator light.

#define GP10_PATCH_COLOUR 4 //Yellow
#define GR55_PATCH_COLOUR 3 //Blue
#define VG99_PATCH_COLOUR 2 //Red
#define ZG3_PATCH_COLOUR 1 //Green
#define GP10_STOMP_COLOUR 4 //Yellow
#define GR55_STOMP_COLOUR 3 //Blue
#define VG99_STOMP_COLOUR 2 //Red
#define ZG3_STOMP_COLOUR 1 //Green
#define GLOBAL_STOMP_COLOUR 2 //White
#define BPM_COLOUR_ON 2 // Red flasing LED
#define BPM_COLOUR_OFF 0

// Defining FX colours:
#define FX_GTR_COLOUR 6 //White for guitar settings
#define FX_DELAY_COLOUR 1 //Green for delays
#define FX_MODULATE_COLOUR 3 //Blue for modulation FX
#define FX_FILTER_COLOUR 8 //Purple for filter FX
#define FX_PITCH_COLOUR 5 //Turquoise for pitch FX
#define FX_AMP_COLOUR 2 //Red for amp FX and amp solo
#define FX_DIST_COLOUR 9 //Pink for distortion FX
#define FX_REVERB_COLOUR 7 //Yellow for reverb FX

//Lets make some colours (R,G,B)
// Adding 100 to a colour number makes the LED flash!
// To make new colours, set the mode in B_Settings to MODE_COLOUR_MAKER (mode 17)

#define NUMBER_OF_COLOURS 21
colour colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 10, 0} ,  // Colour 1 is Green
  {10, 0, 0} ,  //  Colour 2 is Red
  {0, 0, 10} ,  // Colour 3 is Blue
  {10, 5, 0} ,  // Colour 4 is Orange
  {0, 8, 5} ,  // Colour 5 is Turquoise
  {8, 10, 8} ,  // Colour 6 is White
  {8, 8, 0} ,   // Colour 7 is Yellow
  {4, 0, 8} ,   // Colour 8 is Purple
  {10, 0, 5} ,   // Colour 9 is Pink
  {0, 0, 0} ,   // Colour 10 is LED OFF (dimmed)
  {0, 1, 0} ,   // Colour 11 is Green dimmed
  {1, 0, 0} ,   // Colour 12 is Red dimmed
  {0, 0, 1} ,   // Colour 13 is Blue dimmed
  {1, 1, 0} ,   // Colour 14 is Orange dimmed
  {0, 1, 1} ,   // Colour 15 is Turquoise dimmed
  {1, 1, 1} ,   // Colour 16 is White dimmed
  {1, 1, 0} ,   // Colour 17 is Yellow dimmed
  {1, 0, 2} ,   // Colour 18 is Purple dimmed
  {1, 0, 1} ,   // Colour 19 is Pink dimmed
  {0, 0, 0}   // Colour 20 is available
};

#define LEDFLASH_TIMER_LENGTH 500 // Sets the speed with which the LEDs flash (500 means 500 ms on, 500 msec off)
unsigned long LEDflashTimer = 0;
boolean LED_flashing_state_on = true;

#define STARTUP_TIMER_LENGTH 100 // NeoPixel LED switchoff timer set to 100 ms
unsigned long startupTimer = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(NUMLEDS, NEOPIXELLEDPIN, NEO_GRB + NEO_KHZ800);

boolean update_LEDS = true;
uint8_t global_tap_tempo_LED;

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
}

void main_LED_control()
{
  if (update_LEDS == true) {
    update_LEDS = false;
    LED_update();
  }

  // Check here if LEDs need to flash and make them do it
  LED_flash();
}

void LED_update() {
  //Check the switch_states on the current page
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {

    //Copy the LED state from the switch state
    switch (SP[s].Type) {
      case GP10_PATCH:
        if (GP10_patch_number == SP[s].PP_number) {
          if (GP10_on) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10); // Show off colour
        }
        else LED_show_colour(s, 0);
        break;
      case GP10_RELSEL:
        if (GP10_bank_selection_active) LED_show_colour(s, SP[s].Colour + 100); //Flash the GP10 PATCH LEDs
        else {
          if (GP10_patch_number == SP[s].PP_number) {
            if (GP10_on) {
              LED_show_colour(s, SP[s].Colour);
            }
            else LED_show_colour(s, SP[s].Colour + 10); // Show off colour
          }
          else {
            LED_show_colour(s, 0);
          }
        }
        break;
      case GP10_BANK_UP:
      case GP10_BANK_DOWN:
        if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
        else LED_show_colour(s, GP10_PATCH_COLOUR + 10);
        break;
      case GP10_PARAMETER:
      case GP10_ASSIGN:
        if ((SP[s].Latch == MOMENTARY) || (SP[s].Latch == TOGGLE)) {
          //DEBUGMSG("State pedal " + String(s) + ": " + String(SP[s].State));
          if (SP[s].State == 1) LED_show_colour(s, SP[s].Colour);  // LED on
          if (SP[s].State == 2) LED_show_colour(s, SP[s].Colour + 10); // LED dimmed
          if (SP[s].State == 0) LED_show_colour(s, 0); // LED off
        }
        else { // For the TRI/FOUR/FIVESTATE only light up when pressed.
          if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10);
        }
        break;
      case GR55_PATCH:
        if (GR55_patch_number == SP[s].PP_number) {
          if (GR55_on) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10);
        }
        else LED_show_colour(s, 0);
        break;
      case GR55_RELSEL:
        if (GR55_bank_selection_active) LED_show_colour(s, SP[s].Colour + 100); //Flash the GR55 PATCH LEDs
        else {
          if (GR55_patch_number == SP[s].PP_number) {
            if (GR55_on) LED_show_colour(s, SP[s].Colour);
            else LED_show_colour(s, SP[s].Colour + 10);
          }
          else LED_show_colour(s, 0);
        }
        break;
      case GR55_BANK_UP:
      case GR55_BANK_DOWN:
        if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
        else LED_show_colour(s, GR55_PATCH_COLOUR + 10);
        break;
      case GR55_PARAMETER:
      case GR55_ASSIGN:
        if ((SP[s].Latch == MOMENTARY) || (SP[s].Latch == TOGGLE)) {
          if (SP[s].State == 1) LED_show_colour(s, SP[s].Colour);  // LED on
          if (SP[s].State == 2) LED_show_colour(s, SP[s].Colour + 10); // LED dimmed
          if (SP[s].State == 0) LED_show_colour(s, 0); // LED off
        }
        else { // For the TRI/FOUR/FIVESTATE only light up when pressed.
          if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10);
        }
        break;
      case VG99_PATCH:
        if (VG99_patch_number == SP[s].PP_number) {
          if (VG99_on) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10);
        }
        else LED_show_colour(s, 0);
        break;
      case VG99_RELSEL:
        if (VG99_bank_selection_active) LED_show_colour(s, SP[s].Colour + 100); //Flash the VG99 PATCH LEDs
        else {
          if (VG99_patch_number == SP[s].PP_number) {
            if (VG99_on) LED_show_colour(s, SP[s].Colour);
            else LED_show_colour(s, SP[s].Colour + 10);
          }
          else LED_show_colour(s, 0);
        }
        break;
      case VG99_BANK_UP:
      case VG99_BANK_DOWN:
        if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
        else LED_show_colour(s, VG99_PATCH_COLOUR + 10);
        break;
      case VG99_PARAMETER:
      case VG99_ASSIGN:
        if ((SP[s].Latch == MOMENTARY) || (SP[s].Latch == TOGGLE)) {
          if (SP[s].State == 1) LED_show_colour(s, SP[s].Colour);  // LED on
          if (SP[s].State == 2) LED_show_colour(s, SP[s].Colour + 10); // LED dimmed
          if (SP[s].State == 0) LED_show_colour(s, 0); // LED off
        }
        else { // For the TRI/FOUR/FIVESTATE only light up when pressed.
          if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10);
        }
        break;
      case ZG3_PATCH:
        if (ZG3_patch_number == SP[s].PP_number) {
          if (ZG3_on) LED_show_colour(s, SP[s].Colour);
          else LED_show_colour(s, SP[s].Colour + 10); // Show off colour
        }
        else LED_show_colour(s, 0);
        break;
      case ZG3_RELSEL:
        if (ZG3_bank_selection_active) LED_show_colour(s, SP[s].Colour + 100); //Flash the ZG3 PATCH LEDs
        else {
          if (ZG3_patch_number == SP[s].PP_number) {
            if (ZG3_on) LED_show_colour(s, SP[s].Colour);
            else LED_show_colour(s, SP[s].Colour + 10); // Show off colour
          }
          else LED_show_colour(s, 0);
        }
        break;
      case ZG3_BANK_UP:
      case ZG3_BANK_DOWN:
        if (SP[s].Pressed) LED_show_colour(s, SP[s].Colour);
        else LED_show_colour(s, ZG3_PATCH_COLOUR + 10);
        break;
      case ZG3_FX_TOGGLE:
        if (SP[s].State == 2) LED_show_colour(s, SP[s].Colour);  // LED on
        if (SP[s].State == 1) LED_show_colour(s, SP[s].Colour + 10); // LED dimmed
        if (SP[s].State == 0) LED_show_colour(s, 0); // LED off
        break;
      case COMBI_BANK_UP:
      case COMBI_BANK_DOWN:
        if (SP[s].Pressed) {
          if (Current_device == GP10) LED_show_colour(s, GP10_PATCH_COLOUR);
          if (Current_device == GR55) LED_show_colour(s, GR55_PATCH_COLOUR);
          if (Current_device == VG99) LED_show_colour(s, VG99_PATCH_COLOUR);
        }
        else {
          if (Current_device == GP10) LED_show_colour(s, GP10_PATCH_COLOUR + 10);
          if (Current_device == GR55) LED_show_colour(s, GR55_PATCH_COLOUR + 10);
          if (Current_device == VG99) LED_show_colour(s, VG99_PATCH_COLOUR + 10);
        }
        break;
      case TAP_TEMPO:
        LED_show_colour(s, global_tap_tempo_LED);
        break;
      case SELECT_PAGE:
        if ((SP[s].Pressed) || (SP[s].PP_number == Previous_page)) LED_show_colour(s, GLOBAL_STOMP_COLOUR);
        else LED_show_colour(s, 0);
        break;
      case STANDBYE:
        LED_show_colour(s, 11);
        break;
      default:
        LED_show_colour(s, 0); // Show nothing with undefined LED
    }
  }
  LEDs.show();
  //DEBUGMSG("LEDs updated");
}

void LED_show_colour(uint8_t LED_number, uint8_t colour_number) { // Sets the specified LED to the specified colour
  uint8_t number_fixed = colour_number % 100; // In case of flashing LEDS this will take off the extra 100.
  if (number_fixed < NUMBER_OF_COLOURS) {
    if (colour_number < 100) { // Check if it is not a flashing LED
      // Turn the LED on
      if (PHYSICAL_LEDS) LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[number_fixed].green, colours[number_fixed].red, colours[number_fixed].blue));
      if (VIRTUAL_LEDS) Set_virtual_LED_colour(LED_number, number_fixed); // Update the virtual LEDs on the LCD as well
    }
    else { // Update flashing LED
      if (LED_flashing_state_on == true) {
        // Turn the LED on
        if (PHYSICAL_LEDS) LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[number_fixed].green, colours[number_fixed].red, colours[number_fixed].blue));
        if (VIRTUAL_LEDS) Set_virtual_LED_colour(LED_number, number_fixed); // Update the virtual LEDs on the LCD as well
      }
      else {
        // Turn the LED off
        if (PHYSICAL_LEDS) LEDs.setPixelColor(LED_order[LED_number], 0, 0, 0);
        if (VIRTUAL_LEDS) Set_virtual_LED_colour(LED_number, 0); // Update the virtual LEDs on the LCD as well
      }
    }
  }
  else {
    // Invalid colour: show message and give error
    LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[10].green, colours[10].red, colours[10].blue));
    DEBUGMSG("Invalid colour (number " + String(number_fixed) + ") on LED " + String(LED_number));
  }
}

void LED_turn_all_off() {
  for (uint8_t count = 0; count < NUMLEDS; count++) {
    LEDs.setPixelColor(count, LEDs.Color(0, 0, 0));
  }
  // LEDs.show();
}

void LED_flash() {
  // Check if timer needs to be set
  if (LEDflashTimer == 0) {
    LEDflashTimer = millis();
  }

  // Check if timer runs out
  if (millis() - LEDflashTimer > LEDFLASH_TIMER_LENGTH) {
    LEDflashTimer = millis(); // Reset the timer
    LED_flashing_state_on = !LED_flashing_state_on;
    update_LEDS = true; // Get the LEDs to update
  }
}

