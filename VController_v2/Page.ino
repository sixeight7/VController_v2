// ******************************** Page loading and parameter reading ********************************
// Loading of a page from configuration page to active memory (SP array) and reading of the variables

#define SYSEX_WATCHDOG_LENGTH 500 // watchdog length for sysex messages (in msec).
unsigned long SysexWatchdog = 0; // This watchdog will check if a device responds. If it expires it will request the same parameter again.
boolean Sysex_watchdog_running = false;
#define SYSEX_NUMBER_OF_READ_ATTEMPS 3 // Number of times the watchdog will be restarted before moving to the next parameter
uint8_t read_attempt = 1;

void setup_page()
{
  //reset_all_switch_states();
  load_current_page(true);
}

// Load a page from memory into the SP array
void load_current_page(bool read_all) { // Read_all: true will reread all parameters, false will just read the parameters
  update_switch_lcds = OFF; //Switch LCDs are updated here as well
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    SP[s].Type = Page[Current_page].Switch[s].Cmd[0].Type;

    uint8_t Data1 = Page[Current_page].Switch[s].Cmd[0].Data1;
    uint8_t Data2 = Page[Current_page].Switch[s].Cmd[0].Data2;
    /*uint8_t Value1 = Page[Current_page].Switch[s].Cmd[0].Value1;
    uint8_t Value2 = Page[Current_page].Switch[s].Cmd[0].Value2;
    uint8_t Value3 = Page[Current_page].Switch[s].Cmd[0].Value3;
    uint8_t Value4 = Page[Current_page].Switch[s].Cmd[0].Value4;
    uint8_t Value5 = Page[Current_page].Switch[s].Cmd[0].Value5;*/

    switch (SP[s].Type) {
      case GP10_PATCH:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        GP10_patch_load(s, Data1 - 1);
        break;
      case GP10_RELSEL:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        GP10_relsel_load(s, Data1 - 1, Data2);
        break;
      case GP10_BANK_UP:
      case GP10_BANK_DOWN:
        SP[s].Colour = GP10_PATCH_COLOUR;
        break;
      case GP10_PARAMETER:
        SP[s].Read = true; // Parameters always need re-reading
        SP[s].PP_number = Data1; //Store parameter number
        SP[s].Address = GP10_parameters[Data1].Address;
        SP[s].Latch = Data2;
        break;
      case GP10_ASSIGN:
        SP[s].Read = true; // Parameters always need re-reading
        GP10_assign_load(s, Data1, Data2);
        break;
      case GR55_PATCH:
        if ((Data1 >= 1) && (Data2 >= 1) && (Data2 <= 3)) {
          SP[s].Read = read_all; // Patchnames only need reading when read_all is true
          GR55_patch_load(s, ((Data1 - 1) * 3) + (Data2 - 1));
        }
        break;
      case GR55_RELSEL:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        GR55_relsel_load(s, Data1 - 1, Data2);
        break;
      case GR55_BANK_UP:
      case GR55_BANK_DOWN:
        SP[s].Colour = GR55_PATCH_COLOUR;
        break;
      case GR55_PARAMETER:
        SP[s].Read = true; // Parameters always need re-reading
        SP[s].PP_number = Data1; //Store parameter number
        SP[s].Address = GR55_parameters[Data1].Address;
        SP[s].Latch = Data2;
        break;
      case GR55_ASSIGN:
        SP[s].Read = true; // Parameters always need re-reading
        GR55_assign_load(s, Data1, Data2);
        break;
      case VG99_PATCH:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        VG99_patch_load(s, (Data2 * 100) + Data1 - 1);
        break;
      case VG99_RELSEL:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        VG99_relsel_load(s, Data1 - 1, Data2);
        break;
      case VG99_BANK_UP:
      case VG99_BANK_DOWN:
        SP[s].Colour = VG99_PATCH_COLOUR;
        break;
      case VG99_PARAMETER:
        SP[s].Read = true; // Parameters always need re-reading
        SP[s].PP_number = Data1; //Store parameter number
        SP[s].Address = 0x60000000 + VG99_parameters[Data1 / 30][Data1 % 30].Address;
        SP[s].Latch = Data2;
        break;
      case VG99_ASSIGN:
        SP[s].Read = true; // Parameters always need re-reading
        VG99_assign_load(s, Data1, Data2);
        break;
      case ZG3_PATCH:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        ZG3_patch_load(s, (Data2 * 100) + Data1 - 1);
        break;
      case ZG3_RELSEL:
        SP[s].Read = read_all; // Patchnames only need reading when read_all is true
        ZG3_relsel_load(s, Data1 - 1, Data2);
        break;
      case ZG3_BANK_UP:
      case ZG3_BANK_DOWN:
        SP[s].Colour = ZG3_PATCH_COLOUR;
        break;
      case ZG3_FX_TOGGLE:
        SP[s].Read = true; // Parameters always need re-reading
        SP[s].PP_number = Data1; //Store parameter number
        SP[s].Latch = true;
        break;
    }
    if (SP[s].Read) clear_label(s); //Clear the label of this switch if it needs reading
  }

  Request_first_switch(); //Now start reading in the parameters from the devices
}

// *************************************** MIDI parameter request ***************************************
// This will read patch names and parameter names that are on the current page (SP array)

void Request_first_switch() {
  Current_switch = 0; //After the name is read, the assigns can be read
  //update_LCDs(1); //update first LCD before moving on...
  read_attempt = 1;
  DEBUGMSG("Start reading switch parameters");
  Request_current_switch();
}

void Request_next_switch() {
  update_LCDs(Current_switch); //update LCD before moving on...
  Current_switch++;
  read_attempt = 1;
  Request_current_switch();
}

void Request_current_switch() { //Will request the next assign - the assigns are read one by one, otherwise the data will not arrive!
  if (Current_switch < NUMBER_OF_SWITCHES) {
    if (SP[Current_switch].Read) {
      DEBUGMSG("Request parameter " + String(Current_switch));
      //update_LCDs(Current_switch + 1); //update LCD before moving on...
      
      switch (SP[Current_switch].Type) {
        case GP10_PATCH:
        case GP10_RELSEL:
          if (GP10_detected) {
            request_GP10(SP[Current_switch].Address, 12);  //Request the 12 bytes of the GP10 Patch name
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case GP10_PARAMETER:
          if (GP10_detected) {
            request_GP10(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case GP10_ASSIGN:
          if (GP10_detected) {
            if (!GP10_assign_read) { //Check if the assign area needs to be read first
              GP10_request_complete_assign_area();
            }
            else {
              GP10_assign_request(); //otherwise just request the individual assign target
            }
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case GR55_PATCH:
        case GR55_RELSEL:
          if ((GR55_detected) && (SP[Current_switch].PP_number < 297)) { // This is a user patch - we read these from memory
            request_GR55(SP[Current_switch].Address, 16);  //Request the 16 bytes of the GR55 Patch name
            Start_sysex_watchdog(); // Start the watchdog
          }
          else if ((GR55_detected) && (SP[Current_switch].PP_number < (297 + GR55_NUMBER_OF_FACTORY_PATCHES))) { // Fixed patch - read from GR55_preset_patch_names array
            GR55_read_preset_name(Current_switch, SP[Current_switch].PP_number);
            Request_next_switch();
          }
          else {
            Request_next_switch();
          }
          break;
        case GR55_PARAMETER:
          if (GR55_detected) {
            request_GR55(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case GR55_ASSIGN:
          if (GR55_detected) {
            GR55_request_current_assign();
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case VG99_PATCH:
        case VG99_RELSEL:
          if (VG99_detected) {
            request_VG99(SP[Current_switch].Address, 16);  //Request the 16 bytes of the GR55 Patch name
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case VG99_PARAMETER:
          if (VG99_detected) {
            request_VG99(SP[Current_switch].Address, 2);  //Request the 2 bytes of the GP10 Parameter setting
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case VG99_ASSIGN:
          if (VG99_detected) {
            VG99_request_current_assign();
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case ZG3_PATCH:
        case ZG3_RELSEL:
          if (ZG3_detected) {
            ZG3_request_patch(SP[Current_switch].PP_number);  //Request the 12 bytes of the GP10 Patch name
            Start_sysex_watchdog(); // Start the watchdog
          }
          else {
            Request_next_switch();
          }
          break;
        case ZG3_FX_TOGGLE:
          if (ZG3_detected) ZG3_FX_set_type_and_state(Current_switch);
          Request_next_switch();
          break;
        default:
          Request_next_switch();
      }
    }
    else Request_next_switch();
  }
  else {
    Stop_sysex_watchdog(); // Stop the watchdog
    no_device_check = false;
  }
}

void Start_sysex_watchdog() {
  SysexWatchdog = millis() + SYSEX_WATCHDOG_LENGTH;
  Sysex_watchdog_running = true;
  DEBUGMSG("Sysex watchdog started");
}

void Stop_sysex_watchdog() {
  Sysex_watchdog_running = false;
  DEBUGMSG("Sysex watchdog stopped");
}

void Check_sysex_watchdog() {
  if ((millis() > SysexWatchdog) && (Sysex_watchdog_running)) {
    DEBUGMSG("Sysex watchdog expired");
    read_attempt++;
    if (read_attempt > SYSEX_NUMBER_OF_READ_ATTEMPS) Request_next_switch();
    else Request_current_switch(); // Try reading the current parameter again
  }
}


