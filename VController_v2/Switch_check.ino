// Setup of input ports of switches.
// Check for switch pressed and output the result in the switch_pressed, switch_released or switch_long_pressed variable.

#include <Bounce.h>

// ***************************** Settings you may want to change *****************************
#define SWITCH_UPDATE_TIMER_LENGTH 10 // Update switches every 10 ms
#define LONG_PRESS_TIMER_LENGTH 1000 // Timer used for detecting long-pressing switch. Time is in milliseconds
#define EXTRA_LONG_PRESS_TIMER_LENGTH 3000
#define EXPR_PEDAL_TIMER_LENGTH 50 // Timer to limit the data from the expression pedals
unsigned long Switch_update_timer = 0;
unsigned long Long_press_timer = 0;
unsigned long Extra_long_press_timer = 0;
unsigned long Expr_pedal_timer = 0; 

// ***************************** Hardware settings *****************************
// Define pin numbers
//Pin 0 and 1 reserved for MIDI

//Switches are four rows of three switches, connected like a keypad.
// Assign the pins for the switch row and columns below
// (I found I connected most the rows and columns backwards, but you can assign it any way you like.)
#define SWITCH_ROW1 5 // Row 1 (pin 5) is connected to switch 1,2,3 and 4
#define SWITCH_ROW2 4 // Row 2 (pin 4) is connected to switch 5,6,7 and 8
#define SWITCH_ROW3 3 // Row 3 (pin 3) is connected to switch 9,10,11 and 12
#define SWITCH_ROW4 2 // Row 4 (pin 2) is connected to switch 13,14,15 and 16
#define SWITCH_COL1 13 // Column 1 (pin 13) is connected to the other side of switch 1,5,9 and 13
#define SWITCH_COL2 12 // Column 2 (pin 12) is connected to the other side of switch 2,6,10 and 14
#define SWITCH_COL3 11 // Column 3 (pin 11) is connected to the other side of switch 3,7,11 and 15
#define SWITCH_COL4 6  // Column 4 (pin 6) is connected to the other side of switch 4,8,12 and 16

#define SWITCH_EXT1 20  // Tip external input jack 1/2
#define SWITCH_EXT2 21  // Ring external input jack 1/2
#define EXPR_PED12 6 //Pin D20 is also A6
#define SWITCH_EXT3 22  // Tip external input jack 3/4
#define SWITCH_EXT4 23  // Ring external input jack 3/4
#define EXPR_PED34 8 //Pin D22 is also A8
#define SWITCH_EXT5 15  // Tip external input jack 5/6
#define SWITCH_EXT6 16  // Ring external input jack 5/6
#define EXPR_PED56 1 //Pin D15 is also A1
#define SWITCH_EXT7 27  // Tip external input jack 7/8
#define SWITCH_EXT8 28  // Ring external input jack 7/8
#define EXPR_PED78 16 //Pin D27 is also A16

//Pin 17 reserved for Neopixel LEDs
//Pin 18 and 19 reserved for I2C bus (LCD)

// ************************ Settings you probably won't have to change ************************

uint8_t switch_pressed = 0; //Variable set when switch is pressed
uint8_t switch_released = 0; //Variable set when switch is released
uint8_t switch_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)
uint8_t switch_extra_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)

uint16_t expr_ped12_val = 0; // Value for storing value expr. pedal 1/2
uint16_t expr_ped34_val = 0;
uint16_t expr_ped56_val = 0;
uint16_t expr_ped78_val = 0;

uint8_t switch_long_pressed_memory = 0;
uint8_t active_column = 1; // Which switch column is being read?

boolean switch_ext1_polarity_reversed = false; // Pedal accepts both normally open and normally closed external switches
boolean switch_ext2_polarity_reversed = false;
boolean switch_ext3_polarity_reversed = false;
boolean switch_ext4_polarity_reversed = false;
boolean switch_ext5_polarity_reversed = false;
boolean switch_ext6_polarity_reversed = false;
boolean switch_ext7_polarity_reversed = false;
boolean switch_ext8_polarity_reversed = false;

Bounce switch1 = Bounce(SWITCH_ROW1, 50); // Every switch needs its own bouncer even though they share the same pins
Bounce switch2 = Bounce(SWITCH_ROW1, 50); // State of bouncer is LOW when switch is pressed and high when switch is not pressed
Bounce switch3 = Bounce(SWITCH_ROW1, 50); // Fallingedge means switch is pressed. Risingedge means switch is released.
Bounce switch4 = Bounce(SWITCH_ROW1, 50);
Bounce switch5 = Bounce(SWITCH_ROW2, 50);
Bounce switch6 = Bounce(SWITCH_ROW2, 50);
Bounce switch7 = Bounce(SWITCH_ROW2, 50);
Bounce switch8 = Bounce(SWITCH_ROW2, 50);
Bounce switch9 = Bounce(SWITCH_ROW3, 50);
Bounce switch10 = Bounce(SWITCH_ROW3, 50);
Bounce switch11 = Bounce(SWITCH_ROW3, 50);
Bounce switch12 = Bounce(SWITCH_ROW3, 50);
Bounce switch13 = Bounce(SWITCH_ROW4, 50);
Bounce switch14 = Bounce(SWITCH_ROW4, 50);
Bounce switch15 = Bounce(SWITCH_ROW4, 50);
Bounce switch16 = Bounce(SWITCH_ROW4, 50);

Bounce switch_ext1 = Bounce(SWITCH_EXT1, 50);
Bounce switch_ext2 = Bounce(SWITCH_EXT2, 50);
Bounce switch_ext3 = Bounce(SWITCH_EXT3, 50);
Bounce switch_ext4 = Bounce(SWITCH_EXT4, 50);
Bounce switch_ext5 = Bounce(SWITCH_EXT5, 50);
Bounce switch_ext6 = Bounce(SWITCH_EXT6, 50);
Bounce switch_ext7 = Bounce(SWITCH_EXT7, 50);
Bounce switch_ext8 = Bounce(SWITCH_EXT8, 50);

boolean check_released = false; // On the first run release should not be checked, becasue bounce handler states are LOW by default.

void setup_switch_check()
{
  //Enable internal pullup resistors for switch row pins
  pinMode(SWITCH_ROW1, INPUT_PULLUP);
  pinMode(SWITCH_ROW2, INPUT_PULLUP);
  pinMode(SWITCH_ROW3, INPUT_PULLUP);
  pinMode(SWITCH_ROW4, INPUT_PULLUP);

  pinMode(SWITCH_COL1, OUTPUT);
  digitalWrite(SWITCH_COL1, LOW);   //Enable the first column
  pinMode(SWITCH_COL2, INPUT);      //Here's the trick - keep the other output ports input, and there will be no shorting the outputs!!!
  pinMode(SWITCH_COL3, INPUT);
  pinMode(SWITCH_COL4, INPUT);

  // Setup of external switch 1/2
  if (Ext12type == SWITCH) {
    pinMode(SWITCH_EXT1, INPUT_PULLUP); //Also enable input pullup resostors for S1 and S2
    pinMode(SWITCH_EXT2, INPUT_PULLUP);
    delay(10); //Short delay to allow the ports to settle
    // Check polarity of external switches
    // Pedal accepts both normally open and normally closed external switches
    if (digitalRead(SWITCH_EXT1) == LOW) switch_ext1_polarity_reversed = true;
    if (digitalRead(SWITCH_EXT2) == LOW) switch_ext2_polarity_reversed = true;
  }
  else { // Expression pedal connected
    pinMode(SWITCH_EXT2, OUTPUT); // We want to put a plus voltage on the ring of the connector
    digitalWrite(SWITCH_EXT2, HIGH);
  }

  // Setup of external switch 3/4
  if (Ext34type == SWITCH) {
    pinMode(SWITCH_EXT3, INPUT_PULLUP); //Also enable input pullup resostors for S1 and S2
    pinMode(SWITCH_EXT4, INPUT_PULLUP);
    delay(10); //Short delay to allow the ports to settle
    // Check polarity of external switches
    // Pedal accepts both normally open and normally closed external switches
    if (digitalRead(SWITCH_EXT3) == LOW) switch_ext3_polarity_reversed = true;
    if (digitalRead(SWITCH_EXT4) == LOW) switch_ext4_polarity_reversed = true;
  }
  else { // Expression pedal connected
    pinMode(SWITCH_EXT4, OUTPUT); // We want to put a plus voltage on the ring of the connector
    digitalWrite(SWITCH_EXT4, HIGH);
  }

  // Setup of external switch 5/6
  if (Ext56type == SWITCH) {
    pinMode(SWITCH_EXT5, INPUT_PULLUP); //Also enable input pullup resostors for S1 and S2
    pinMode(SWITCH_EXT6, INPUT_PULLUP);
    delay(10); //Short delay to allow the ports to settle
    // Check polarity of external switches
    // Pedal accepts both normally open and normally closed external switches
    if (digitalRead(SWITCH_EXT5) == LOW) switch_ext5_polarity_reversed = true;
    if (digitalRead(SWITCH_EXT6) == LOW) switch_ext6_polarity_reversed = true;
  }
  else { // Expression pedal connected
    pinMode(SWITCH_EXT6, OUTPUT); // We want to put a plus voltage on the ring of the connector
    digitalWrite(SWITCH_EXT6, HIGH);
  }

  // Setup of external switch 7/8
  if (Ext78type == SWITCH) {
    pinMode(SWITCH_EXT7, INPUT_PULLUP); //Also enable input pullup resostors for S1 and S2
    pinMode(SWITCH_EXT8, INPUT_PULLUP);
    delay(10); //Short delay to allow the ports to settle
    // Check polarity of external switches
    // Pedal accepts both normally open and normally closed external switches
    if (digitalRead(SWITCH_EXT7) == LOW) switch_ext7_polarity_reversed = true;
    if (digitalRead(SWITCH_EXT8) == LOW) switch_ext8_polarity_reversed = true;
  }
  else { // Expression pedal connected
    pinMode(SWITCH_EXT8, OUTPUT); // We want to put a plus voltage on the ring of the connector
    digitalWrite(SWITCH_EXT8, HIGH);
  }

  Expr_pedal_timer = millis(); // Set timer for the expression pedals
  Switch_update_timer = millis(); // Set timer for the switch update
}

void main_switch_check()
{
  if (millis() - Switch_update_timer > SWITCH_UPDATE_TIMER_LENGTH) {
    update_switches();
    Switch_update_timer = millis(); // Reset timer for the switch update
  }
}

void update_switches()
{
  // Run through the switch columns, One column is being activated on every run of this routine.
  // Check if a switch is pressed of released and set the switch_pressed and switch_released variable accordingly
  switch_pressed = 0;
  switch_released = 0;
  switch_long_pressed = 0;

  switch (active_column) {
    case 1:

      switch13.update();  //Update the switches in the bounce library
      switch9.update();
      switch5.update();
      switch1.update();

      // Check for switches in column 1 being pressed
      if (switch13.fallingEdge()) switch_pressed = 13;
      if ((switch9.fallingEdge()) && (switch13.read() == HIGH)) switch_pressed = 9; // Also check state of the switch above
      if ((switch5.fallingEdge()) && (switch9.read() == HIGH)) switch_pressed = 5; // Also check state of the switch above
      if ((switch1.fallingEdge()) && (switch5.read() == HIGH)) switch_pressed = 1; // Also check state of the switch above

      // Check for switches in column 1 being released
      if ((check_released) && (switch13.risingEdge())) switch_released = 13;
      if ((check_released) && (switch9.risingEdge()) && (switch13.read() == HIGH)) switch_released = 9; // Also check state of the switch above
      if ((check_released) && (switch5.risingEdge()) && (switch9.read() == HIGH)) switch_released = 5; // Also check state of the switch above
      if ((check_released) && (switch1.risingEdge()) && (switch5.read() == HIGH)) switch_released = 1; // Also check state of the switch above

      digitalWrite(SWITCH_COL1, HIGH);
      pinMode(SWITCH_COL1, INPUT);

      pinMode(SWITCH_COL2, OUTPUT); //Switch on the second column
      digitalWrite(SWITCH_COL2, LOW);

      active_column = 2;

      // Also check for EXT1 and EXT2 switches here
      if (Ext12type == SWITCH) {
        switch_ext1.update();
        if (switch_ext1_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext1.fallingEdge()) switch_pressed = 17;
          if ((check_released) && (switch_ext1.risingEdge())) switch_released = 17;
        }
        else {
          if (switch_ext1.risingEdge()) switch_pressed = 17;
          if ((check_released) && (switch_ext1.fallingEdge())) switch_released = 17;
        }

        switch_ext2.update();
        if (switch_ext2_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext2.fallingEdge()) switch_pressed = 18;
          if ((check_released) && (switch_ext2.risingEdge())) switch_released = 18;
        }
        else {
          if (switch_ext2.risingEdge()) switch_pressed = 18;
          if ((check_released) && (switch_ext2.fallingEdge())) switch_released = 18;
        }
      }

      break;

    case 2:

      switch14.update();
      switch10.update();
      switch6.update();
      switch2.update();

      // Check for switches in column 2 being pressed
      if (switch14.fallingEdge()) switch_pressed = 14;
      if ((switch10.fallingEdge()) && (switch14.read() == HIGH)) switch_pressed = 10; // Also check state of the switch above
      if ((switch6.fallingEdge()) && (switch10.read() == HIGH)) switch_pressed = 6; // Also check state of the switch above
      if ((switch2.fallingEdge()) && (switch6.read() == HIGH)) switch_pressed = 2; // Also check state of the switch above

      // Check for switches in column 2 being released
      if ((check_released) && (switch14.risingEdge())) switch_released = 14;
      if ((check_released) && (switch10.risingEdge()) && (switch14.read() == HIGH)) switch_released = 10; // Also check state of the switch above
      if ((check_released) && (switch6.risingEdge()) && (switch10.read() == HIGH)) switch_released = 6; // Also check state of the switch above
      if ((check_released) && (switch2.risingEdge()) && (switch6.read() == HIGH)) switch_released = 2; // Also check state of the switch above

      digitalWrite(SWITCH_COL2, HIGH);
      pinMode(SWITCH_COL2, INPUT);

      pinMode(SWITCH_COL3, OUTPUT);  //Switch on the third column
      digitalWrite(SWITCH_COL3, LOW);

      active_column = 3;

      // Also check for EXT3 and EXT4 switches here
      if (Ext34type == SWITCH) {
        switch_ext3.update();
        if (switch_ext3_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext3.fallingEdge()) switch_pressed = 19;
          if ((check_released) && (switch_ext3.risingEdge())) switch_released = 19;
        }
        else {
          if (switch_ext3.risingEdge()) switch_pressed = 19;
          if ((check_released) && (switch_ext3.fallingEdge())) switch_released = 19;
        }

        switch_ext4.update();
        if (switch_ext4_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext4.fallingEdge()) switch_pressed = 20;
          if ((check_released) && (switch_ext4.risingEdge())) switch_released = 20;
        }
        else {
          if (switch_ext4.risingEdge()) switch_pressed = 20;
          if ((check_released) && (switch_ext4.fallingEdge())) switch_released = 20;
        }
      }

      break;

    case 3:

      switch15.update();
      switch11.update();
      switch7.update();
      switch3.update();

      // Check for switches in column 3 being pressed
      if (switch15.fallingEdge())  switch_pressed = 15;
      if ((switch11.fallingEdge()) && (switch15.read() == HIGH)) switch_pressed = 11; // Also check state of the switch above
      if ((switch7.fallingEdge()) && (switch11.read() == HIGH)) switch_pressed = 7; // Also check state of the switch above
      if ((switch3.fallingEdge()) && (switch7.read() == HIGH)) switch_pressed = 3; // Also check state of the switch above

      // Check for switches in column 3 being released
      if ((check_released) && (switch15.risingEdge())) switch_released = 15;
      if ((check_released) && (switch11.risingEdge()) && (switch15.read() == HIGH)) switch_released = 11; // Also check state of the switch above
      if ((check_released) && (switch7.risingEdge()) && (switch11.read() == HIGH)) switch_released = 7; // Also check state of the switch above
      if ((check_released) && (switch3.risingEdge()) && (switch7.read() == HIGH)) switch_released = 3; // Also check state of the switch above

      check_released = true; //Here all the bounce handler states are high and the release state can be checked now.

      digitalWrite(SWITCH_COL3, HIGH);
      pinMode(SWITCH_COL3, INPUT);

      pinMode(SWITCH_COL4, OUTPUT);   //Switch on the first column
      digitalWrite(SWITCH_COL4, LOW);

      active_column = 4;

      // Also check for EXT5 and EXT6 switches here
      if (Ext56type == SWITCH) {
        switch_ext5.update();
        if (switch_ext5_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext5.fallingEdge()) switch_pressed = 21;
          if ((check_released) && (switch_ext5.risingEdge())) switch_released = 21;
        }
        else {
          if (switch_ext5.risingEdge()) switch_pressed = 21;
          if ((check_released) && (switch_ext5.fallingEdge())) switch_released = 21;
        }

        switch_ext6.update();
        if (switch_ext6_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext6.fallingEdge()) switch_pressed = 22;
          if ((check_released) && (switch_ext6.risingEdge())) switch_released = 22;
        }
        else {
          if (switch_ext6.risingEdge()) switch_pressed = 22;
          if ((check_released) && (switch_ext6.fallingEdge())) switch_released = 22;
        }
      }

      break;

    case 4:

      switch16.update();
      switch12.update();
      switch8.update();
      switch4.update();

      // Check for switches in column 3 being pressed
      if (switch16.fallingEdge())  switch_pressed = 16;
      if ((switch12.fallingEdge()) && (switch16.read() == HIGH)) switch_pressed = 12; // Also check state of the switch above
      if ((switch8.fallingEdge()) && (switch12.read() == HIGH)) switch_pressed = 8; // Also check state of the switch above
      if ((switch4.fallingEdge()) && (switch8.read() == HIGH)) switch_pressed = 4; // Also check state of the switch above

      // Check for switches in column 3 being released
      if ((check_released) && (switch16.risingEdge())) switch_released = 16;
      if ((check_released) && (switch12.risingEdge()) && (switch16.read() == HIGH)) switch_released = 12; // Also check state of the switch above
      if ((check_released) && (switch8.risingEdge()) && (switch12.read() == HIGH)) switch_released = 8; // Also check state of the switch above
      if ((check_released) && (switch4.risingEdge()) && (switch8.read() == HIGH)) switch_released = 4; // Also check state of the switch above

      check_released = true; //Here all the bounce handler states are high and the release state can be checked now.

      digitalWrite(SWITCH_COL4, HIGH);
      pinMode(SWITCH_COL4, INPUT);

      pinMode(SWITCH_COL1, OUTPUT);   //Switch on the first column
      digitalWrite(SWITCH_COL1, LOW);

      active_column = 1;

      // Also check for EXT7 and EXT8 switches here
      if (Ext78type == SWITCH) {
        switch_ext7.update();
        if (switch_ext7_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext7.fallingEdge()) switch_pressed = 23;
          if ((check_released) && (switch_ext7.risingEdge())) switch_released = 23;
        }
        else {
          if (switch_ext7.risingEdge()) switch_pressed = 23;
          if ((check_released) && (switch_ext7.fallingEdge())) switch_released = 23;
        }

        switch_ext8.update();
        if (switch_ext8_polarity_reversed == false) { //Take switch polarity into account
          if (switch_ext8.fallingEdge()) switch_pressed = 24;
          if ((check_released) && (switch_ext8.risingEdge())) switch_released = 24;
        }
        else {
          if (switch_ext8.risingEdge()) switch_pressed = 24;
          if ((check_released) && (switch_ext8.fallingEdge())) switch_released = 24;
        }
      }
      break;
  }

  // Now check for Long pressing a button
  if (switch_pressed > 0) {
    Long_press_timer = millis(); // Set timer on switch pressed
    Extra_long_press_timer = millis(); // Set timer on switch pressed
    switch_long_pressed_memory = switch_pressed; // Remember the button that was pressed
  }

  if (switch_released > 0) {
    Long_press_timer = 0;  //Reset the timer on switch released
    Extra_long_press_timer = 0;
  }

  if ((millis() - Long_press_timer > LONG_PRESS_TIMER_LENGTH) && (Long_press_timer > 0)) {
    switch_long_pressed = switch_long_pressed_memory; //pass on the buttonvalue we remembered before
    Long_press_timer = 0;
  }

  if ((millis() - Extra_long_press_timer > EXTRA_LONG_PRESS_TIMER_LENGTH) && (Extra_long_press_timer > 0)) {
    switch_extra_long_pressed = switch_long_pressed_memory; //pass on the buttonvalue we remembered before
    Extra_long_press_timer = 0;
  }

  // Check expression pedals
  if (millis() - Expr_pedal_timer > EXPR_PEDAL_TIMER_LENGTH) {

    if (Ext34type == PEDAL) {
      expr_ped34_val = analogRead(EXPR_PED34);
      //show_status_message("PEDAL " + String(expr_ped34_val));
    }

    Expr_pedal_timer = millis(); // Reset timer for the expression pedals
  }
}

