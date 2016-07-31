// Please read VController_v2.ino for information about the license and authors

// ******************************** Page loading and parameter reading ********************************
// Loading of a page from configuration page to active memory (SP array) and reading of the variables

#define SYSEX_WATCHDOG_LENGTH 500 // watchdog length for sysex messages (in msec).
unsigned long SysexWatchdog = 0; // This watchdog will check if a device responds. If it expires it will request the same parameter again.
boolean Sysex_watchdog_running = false;
#define SYSEX_NUMBER_OF_READ_ATTEMPS 3 // Number of times the watchdog will be restarted before moving to the next parameter
uint8_t read_attempt = 1;

void setup_page()
{
  if (VController_on) SCO_switch_VController_on();
  PAGE_load_current(true);
}

void main_page() {
  if (update_page != OFF) {
    if (update_page == PAR_ONLY) {
      DEBUGMSG("** Reload page parameters only");
      PAGE_load_current(false); //Re-read the page - but just the parameters
    }
    if (update_page == FULL) {
      DEBUGMSG("** Reload page full");
      PAGE_load_current(true); //Re-read the page completely
    }
    update_page = OFF;
  }
}

// Load a page from memory into the SP array
void PAGE_load_current(bool read_all) { // Read_all: true will reread all parameters, false will just read the parameters
  update_page = OFF; //Switch LCDs are updated here as well
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) { // Load regular switches
    uint8_t Type = Page[Current_page].Switch[s].Cmd[0].Type;
    uint8_t Data1 = Page[Current_page].Switch[s].Cmd[0].Data1;
    uint8_t Data2 = Page[Current_page].Switch[s].Cmd[0].Data2;
    PAGE_load_switch(s, Type, Data1, Data2, read_all);
  }
  /*for (uint8_t s = 0; s < NUMBER_OF_EXTERNAL_SWITCHES; s++) { // Load external switches
    uint8_t Type = Ext_switch[s].Cmd[0].Type;
    uint8_t Data1 = Ext_switch[s].Cmd[0].Data1;
    uint8_t Data2 = Ext_switch[s].Cmd[0].Data2;
    PAGE_load_switch(s + NUMBER_OF_SWITCHES, Type, Data1, Data2, read_all);  
  }*/
  PAGE_request_first_switch(); //Now start reading in the parameters from the devices
}

void PAGE_load_switch(uint8_t number, uint8_t Type, uint8_t Data1, uint8_t Data2, bool read_all) {
  SP[number].Type = Type;

  switch (SP[number].Type) {
    case GP10_PATCH:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      GP10_patch_load(number, Data1 - 1);
      break;
    case GP10_RELSEL:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      GP10_relsel_load(number, Data1 - 1, Data2);
      break;
    case GP10_BANK_UP:
    case GP10_BANK_DOWN:
      SP[number].Colour = GP10_PATCH_COLOUR;
      break;
    case GP10_PARAMETER:
      SP[number].Read = true; // Parameters always need re-reading
      SP[number].PP_number = Data1; //Store parameter number
      SP[number].Address = GP10_parameters[Data1].Address;
      SP[number].Latch = Data2;
      break;
    case GP10_ASSIGN:
      SP[number].Read = true; // Parameters always need re-reading
      GP10_assign_load(number, Data1, Data2);
      break;
    case GR55_PATCH:
      if ((Data1 >= 1) && (Data2 >= 1) && (Data2 <= 3)) {
        SP[number].Read = read_all; // Patchnames only need reading when read_all is true
        GR55_patch_load(number, ((Data1 - 1) * 3) + (Data2 - 1));
      }
      break;
    case GR55_RELSEL:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      GR55_relsel_load(number, Data1 - 1, Data2);
      break;
    case GR55_BANK_UP:
    case GR55_BANK_DOWN:
      SP[number].Colour = GR55_PATCH_COLOUR;
      break;
    case GR55_PARAMETER:
      SP[number].Read = true; // Parameters always need re-reading
      SP[number].PP_number = Data1; //Store parameter number
      SP[number].Address = GR55_parameters[Data1].Address;
      SP[number].Latch = Data2;
      break;
    case GR55_ASSIGN:
      SP[number].Read = true; // Parameters always need re-reading
      GR55_assign_load(number, Data1, Data2);
      break;
    case VG99_PATCH:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      VG99_patch_load(number, (Data2 * 100) + Data1 - 1);
      break;
    case VG99_RELSEL:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      VG99_relsel_load(number, Data1 - 1, Data2);
      break;
    case VG99_BANK_UP:
    case VG99_BANK_DOWN:
      SP[number].Colour = VG99_PATCH_COLOUR;
      break;
    case VG99_PARAMETER:
      SP[number].Read = true; // Parameters always need re-reading
      SP[number].PP_number = Data1; //Store parameter number
      SP[number].Address = 0x60000000 + VG99_parameters[Data1 / 30][Data1 % 30].Address;
      SP[number].Latch = Data2;
      break;
    case VG99_ASSIGN:
      SP[number].Read = true; // Parameters always need re-reading
      VG99_assign_load(number, Data1, Data2);
      break;
    case ZG3_PATCH:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      ZG3_patch_load(number, (Data2 * 100) + Data1 - 1);
      break;
    case ZG3_RELSEL:
      SP[number].Read = read_all; // Patchnames only need reading when read_all is true
      ZG3_relsel_load(number, Data1 - 1, Data2);
      break;
    case ZG3_BANK_UP:
    case ZG3_BANK_DOWN:
      SP[number].Colour = ZG3_PATCH_COLOUR;
      break;
    case ZG3_FX_TOGGLE:
      SP[number].Read = true; // Parameters always need re-reading
      SP[number].PP_number = Data1; //Store parameter number
      SP[number].Latch = true;
      break;
    case SELECT_PAGE:
      SP[number].PP_number = Data1; //Store page number
      SP[number].Read = false;
      String msg = Page[Data1].Title + Blank_line;
      LCD_set_label(number, msg);
      break;
  }
  if (SP[number].Read) LCD_clear_label(number); //Clear the label of this switch if it needs reading
}

// *************************************** MIDI parameter request ***************************************
// This will read patch names and parameter names that are on the current page (SP array)

void PAGE_request_first_switch() {
  Current_switch = 0; //After the name is read, the assigns can be read
  //LCD_update(1); //update first LCD before moving on...
  read_attempt = 1;
  DEBUGMSG("Start reading switch parameters");
  Request_current_switch();
}

void PAGE_request_next_switch() {
  LCD_update(Current_switch); //update LCD before moving on...
  Current_switch++;
  read_attempt = 1;
  Request_current_switch();
}

void Request_current_switch() { //Will request the next assign - the assigns are read one by one, otherwise the data will not arrive!
  if (Current_switch < NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES) {
    if (SP[Current_switch].Read) {
      DEBUGMSG("Request parameter " + String(Current_switch));
      //LCD_update(Current_switch + 1); //update LCD before moving on...

      switch (SP[Current_switch].Type) {
        case GP10_PATCH:
        case GP10_RELSEL:
          if (GP10_connected) {
            request_GP10(SP[Current_switch].Address, 12);  //Request the 12 bytes of the GP10 Patch name
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case GP10_PARAMETER:
          if (GP10_connected) {
            request_GP10(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case GP10_ASSIGN:
          if (GP10_connected) {
            if (!GP10_assign_read) { //Check if the assign area needs to be read first
              GP10_request_complete_assign_area();
            }
            else {
              GP10_assign_request(); //otherwise just request the individual assign target
            }
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case GR55_PATCH:
        case GR55_RELSEL:
          if ((GR55_connected) && (SP[Current_switch].PP_number < 297)) { // This is a user patch - we read these from memory
            request_GR55(SP[Current_switch].Address, 16);  //Request the 16 bytes of the GR55 Patch name
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else if ((GR55_connected) && (SP[Current_switch].PP_number < (297 + GR55_NUMBER_OF_FACTORY_PATCHES))) { // Fixed patch - read from GR55_preset_patch_names array
            GR55_read_preset_name(Current_switch, SP[Current_switch].PP_number);
            PAGE_request_next_switch();
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case GR55_PARAMETER:
          if (GR55_connected) {
            request_GR55(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case GR55_ASSIGN:
          if (GR55_connected) {
            GR55_request_current_assign();
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case VG99_PATCH:
        case VG99_RELSEL:
          if (VG99_connected) {
            request_VG99(SP[Current_switch].Address, 16);  //Request the 16 bytes of the GR55 Patch name
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case VG99_PARAMETER:
          if (VG99_connected) {
            request_VG99(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case VG99_ASSIGN:
          if (VG99_connected) {
            VG99_request_current_assign();
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case ZG3_PATCH:
        case ZG3_RELSEL:
          if (ZG3_connected) {
            ZG3_request_patch(SP[Current_switch].PP_number);  //Request the 12 bytes of the GP10 Patch name
            PAGE_start_sysex_watchdog(); // Start the watchdog
          }
          else {
            PAGE_request_next_switch();
          }
          break;
        case ZG3_FX_TOGGLE:
          if (ZG3_connected) ZG3_FX_set_type_and_state(Current_switch);
          PAGE_request_next_switch();
          break;
        default:
          PAGE_request_next_switch();
      }
    }
    else PAGE_request_next_switch();
  }
  else {
    PAGE_stop_sysex_watchdog(); // Stop the watchdog
    no_device_check = false;
  }
}

// Sysex watchdog will request the data for the current switch again in case the device did not respond in time.
void PAGE_start_sysex_watchdog() {
  SysexWatchdog = millis() + SYSEX_WATCHDOG_LENGTH;
  Sysex_watchdog_running = true;
  DEBUGMSG("Sysex watchdog started");
}

void PAGE_stop_sysex_watchdog() {
  Sysex_watchdog_running = false;
  DEBUGMSG("Sysex watchdog stopped");
}

void PAGE_check_sysex_watchdog() {
  if ((millis() > SysexWatchdog) && (Sysex_watchdog_running)) {
    DEBUGMSG("Sysex watchdog expired");
    read_attempt++;
    if (read_attempt > SYSEX_NUMBER_OF_READ_ATTEMPS) PAGE_request_next_switch();
    else Request_current_switch(); // Try reading the current parameter again
  }
}


