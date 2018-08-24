# VController version 2
Second version of the dedicated MIDI controller for Boss GP-10  / Roland GR-55 / Roland VG-99 and Zoom G3
The latest version is the VController version 3: https://github.com/sixeight7/VController_v3

# Features
* Patch selection - view of patch names for all devices.
* Read and show correct effect state for all devices. Different colours for different types of effects (eg. modulation is blue, amps are red, delays are green, etc)
* GP10 and GR55: change fixed parameters and cc assigns on each patch. Display tells you which parameter is changed and buttons have a colour that shows you the type of FX you select.
* VG99: pedal simulates an FC300 for CTL-1 to CTL-8. Parameter names and colours displayed for the most common parameters. CC assigns and fixed parameters also possible.
* For fixed parameters three/four and five state switches can be programmed
* Zoom G3: support of patch names / control of the 6 effects (on/off) - FX name is displayed, tap tempo.
* Global Tap Tempo: all devices pick up the tempo from the V-Controller. There is the option to keep this tempo on all patches on all devices.
* US-20 simulation: smart muting of GP10, GR55 or VG99 by switching off the COSM guitar/synth/normal PU on the devices that are not active.
* Autobass mode: sends a CC message with the number of the lowest string that is being played (CC #15)

# Hardware
Teensy 3.2, 16 switches and 16 neopixel LEDs, 13 1602 LCD display, some additional parts and an enclosure.
See my project blog for details and schematic.

# Software
Developed on Teensy 3.2 
Make sure you have the https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/LiquidCrystal_V1.2.1.zip library installed. It is the only non-standard library used in this sketch.

Start with VController_v2.ino in the VController_v2 folder to browse this code. It has the main setup and loop from where all the other files are called.

I tried to seperate the different tasks of the VController in the different files. Complete seperation is not possible, but it helps a great deal in finding stuff back.

Short desciption of each file in the VController_v2 folder:

* VController_v2.ino: The main file, that will call all the other ones in the right order.
* Commands.ino: shows the commands that can be used in the configuration of the VController
* Config.ino: configuration of what each switch does (on a page)
* Devices.ino: contains the basic settings and variables for the devices.
* EEPROM.ino: deals with putting some settings in memory.
* LCD.ino: sets up hardware for the LCD and decides what to put on the screen.
* LEDs.ino: sets up the neopixel LEDs and decides when to light up which one.
* MIDI.ino: sets up the MIdI hardware and provides the basic functions
* MIDI_FC300.ino: deals with specific midi functions for the FC 300.
* MIDI_GP10.ino: deals with specific midi functions for the Boss GP-10.
* MIDI_GR55.ino: deals with specific midi functions for the Roland GR-55.
* MIDI_VG99.ino: deals with specific midi functions for the Roland VG-99.
* MIDI_ZG3.ino: deals with specific midi functions for the Zoom G3
* Page.ino: here a page of switches is read from the configuration and the correct MIDI function is called from here.
* Switch_check.ino: checks whether a switch is pressed, released or pressed for a longer time.
* Switch_control.ino: basically deals with patch changes and provides the framework for the stompbox modes.
* z_log.ino: Log of latest changes.

More details are up in my project blog.

# To do
* Adding Direct select
* Adding parameter range option - to allow programming devices from the VController
* Add song and set mode.
