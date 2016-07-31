// Please read VController_v2.ino for information about the license and authors

/* Log
10-11-2015 Start programming v2. Developed structure for VController
19-11-2015 Finished GP10 patch selection mode
20-12-2015 Patch change mode for GR55 and VG99 as well.
22-12-2015 US20 emulation done
24-12-2015 Wrote PATCH modes and fixed Bank up/down problems 
24-12-2015 Problem with occasional reboot in VG99 patch change. Previous version did not seem to have this problem...
26-12-2015 Started writing assign mode for GP-10. Does contains bugs.
27-12-2015 Assign mode finished. Bug was a string written out of bounds..
29-12-2015 Programmed TRI/FOUR/FIVESTATE options in GP10
30-12-2015 Parameter and assign_mode for GR55 written
31-12-2015 Allow for multiple commands / added tap tempo and bass mode from version 1
01-01-2016 Parameter and assign mode for VG-99
02-01-2016 Added FC300 CTL1-8 for VG-99
17-01-2016 Worked on individual display messages
2?-03-2016 Updated firmware to work with the new hardware
30-03-2016 Simplified updating individual displays
02-04-2016 Added EEPROM.ino with global read/store and on/standbye mode for the VController
13-06-2016 Started adding page for Zoom G3
11-07-2016 Changed LCD_set_patch_number_and_name() so the main display can deal with the Zoom G3 and more devices as well
12-07-2016 Added virtual LEDs (LED state in display)
16-07-2016 Changed GP10/GR55/VG99/ZG3_detected into connect/offline system.
26-07-2016 Updated inconsistent display update system
27-07-2016 Consistent naming scheme for all functions and voids
28-07-2016 Added support for expression pedals - they send fixed cc messages now
*/

