/*
  SD card connection

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
   // Arduino-pico core
   ** MISO - Pin 21 - GPIO 16
   ** MOSI - Pin 25 - GPIO 19
   ** CS   - Pin 22 - GPIO 17
   ** SCK  - Pin 24 - GPIO 18
*/

// only AVR and ARM CPU
// #include <MemoryFree.h>

#include "globals.h"

// =========================================================================================
// Guido Lehwalder's Code-Revision-Number
// =========================================================================================
#define GL_REV "GL20250304.0"
//#define GL_REV  "GL20241103.0"

#include <SPI.h>
#include <SdFat.h>        // One SD library to rule them all - Greinman SdFat from Library Manager

// =========================================================================================
// Board definitions go into the "hardware" folder, if you use a board different than the
// Arduino DUE, choose/change a file from there and reference that file here
// =========================================================================================

// Raspberry Pi Pico - normal (LED = GPIO25)
#include "hardware/pico/pico_sd_spi.h"

// Raspberry Pi Pico W(iFi)   (LED = GPIO32)
// #include "hardware/pico/pico_w_sd_spi.h"

// =========================================================================================
// Startup Messages Text (please write it yourself)
// =========================================================================================
#define PICO_OR_W "Pico"
#define PICOCORE_VER "v4.5.1"
#define SDFAT_VER "v2.3.0"
#define PICODVI_VER "v1.3.0"
#define TINYUSB_VER "v3.4.4"
// =========================================================================================
// Delays for LED blinking
// =========================================================================================
#define sDELAY 200
#define DELAY  400

#include "abstraction_arduino.h"

// =========================================================================================
// Serial port speed
// =========================================================================================
#define SERIALSPD 115200

// =========================================================================================
// PUN: device configuration
// =========================================================================================
#ifdef USE_PUN
File32 pun_dev;
int pun_open = FALSE;
#endif

// =========================================================================================
// LST: device configuration
// =========================================================================================
#ifdef USE_LST
File32 lst_dev;
int lst_open = FALSE;
#endif

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

void setup(void) {

//  vreg_set_voltage(VREG_VOLTAGE_1_20);
//  delay(10);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

// =========================================================================================
// Serial Port Definition
// =========================================================================================
//   Serial =USB / Serial1 =UART0/COM1 / Serial2 =UART1/COM2

   Serial1.setRX(1); // Pin 2
   Serial1.setTX(0); // Pin 1

// =========================================================================================

  // _clrscr();
  // _puts("Opening serial-port...\r\n");

  // Serial.setFIFOSize(128);
  Serial.begin(SERIALSPD);
  while (!Serial) {  // Wait until serial is connected
    digitalWrite(LED, HIGH^LEDinv);
    delay(sDELAY);
    digitalWrite(LED, LOW^LEDinv);
    delay(DELAY);
  }


#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  
// =========================================================================================  
// Printing the Startup-Messages
// =========================================================================================

  _clrscr();

  // if (bootup_press == 1)
  //   { _puts("Recognized \e[1m#\e[0m key as pressed! :)\r\n\r\n");
  //   }
  
  _puts("CP/M Emulator \e[1mv" VERSION "\e[0m   by   \e[1mMarcelo  Dantas\e[0m\r\n");
    
  #ifdef ABDOS
  _puts("     (A)BDOS.SYS     by   \e[1mPavel    Zampach\e[0m\r\n");
  #endif
  
  _puts("---------------------------------------------------\r\n");  
  _puts("     running    on   Raspberry Pi [\e[1m Pico \e[0m]\r\n");
//  _puts("     running    on   Raspberry Pi [\e[1mPico-W\e[0m]\r\n");
  _puts("     compiled with   RP2040       [\e[1mv4.2.0\e[0m] \r\n");  
//  _puts("               and   ESP8266SdFat [\e[1mv2.2.2\e[0m] \r\n");
  _puts("               and   SDFat        [\e[1mv2.2.3\e[0m] \r\n");  
  
  _puts("                     Revision     [\e[1m");
  _puts(GL_REV);
  _puts("\e[0m]\r\n");
  
  _puts("---------------------------------------------------\r\n");

  _puts("BIOS              at [\e[1m0x");
  _puthex16(BIOSjmppage);
//  _puts(" - ");
  _puts("\e[0m]\r\n");

  #ifdef ABDOS
	_puts("ABDOS.SYS \e[1menabled\e[0m at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #else
	_puts("BDOS            at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #endif

  _puts("CCP               at [\e[1m0x");
  _puthex16(CCPaddr);
        _puts("\e[0m]     [\e[1m");
        _puts(CCPname);
  _puts("\e[0m]\r\n");

   #if BANKS > 1
  _puts("Banked Memory        [\e[1m");
  _puthex8(BANKS);
    _puts("\e[0m]banks\r\n");
  #else
  _puts("Banked Memory        [\e[1m");
  _puthex8(BANKS);
  _puts("\e[0m]bank\r\n");
  #endif

   // Serial.printf("Free Memory          [\e[1m%d bytes\e[0m]\r\n", freeMemory());

  _puts("CPU-Clock            [\e[1m270Mhz\e[0m]\r\n");



// =========================================================================================
// Redefine SPI-Pins - if needed : (SPI.) = SPI0 / (SPI1.) = SPI1
// =========================================================================================
  SPI.setRX(16);   // MISO
  SPI.setCS(17);   // Card Select
  SPI.setSCK(18);  // Clock
  SPI.setTX(19);   // MOSI

// =========================================================================================
// Setup SD card writing settings
// Info at: https://github.com/greiman/SdFat/issues/285#issuecomment-823562829
// =========================================================================================

#undef SDFAT_FILE_TYPE
#define SDFAT_FILE_TYPE 1           // Uncomment for Due, Teensy or RPi Pico
#define ENABLE_DEDICATED_SPI 1      // Dedicated SPI 1=ON 0=OFF
#define SDMHZ_TXT "19"              // for outputing SDMHZ-Text
// normal is 12Mhz because of https://www.pschatzmann.ch/home/2021/03/14/rasperry-pico-with-the-sdfat-library/
#define SDMHZ 19                    // setting Mhz  - SPI-Bus = SPI_FULL_SPEED = 50Mhz
#define SS 5
// select required SPI-Bus : (&SPI) = SPI0 / (&SPI1) = SPI1
#define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SD_SCK_MHZ(SDMHZ), &SPI) // self-selecting the Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_FULL_SPEED, &SPI)
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_QUARTER_SPEED, &SPI)  // for breadboard QUARTER - 2Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_HALF_SPEED, &SPI)     // for breadboard HALF    - 4Mhz
// #define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI, SPI_DIV3_SPEED, &SPI)        // for z.B. DUE        - 16Mhz

// =========================================================================================


  _puts("Init MicroSD-Card    [ \e[1m");
//  old SDINIT
//  if (SD.begin(SDINIT)) {

// NEW SDCONFIG = formerly SDINIT
if (SD.begin(SD_CONFIG)) {
  _puts(SDMHZ_TXT);
  _puts("Mhz\e[0m]\r\n");
  _puts("---------------------------------------------------");

                        
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
#ifdef ABDOS
      _PatchBIOS();
#endif
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
  Status = 0;

#ifdef CCP_INTERNAL
        _ccp();
#else
        if (!_RamLoad((uint8 *)CCPname, CCPaddr, 0)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
     		// Loads an autoexec file if it exists and this is the first boot
		    // The file contents are loaded at ccpAddr+8 up to 126 bytes then the size loaded is stored at ccpAddr+7
		    if (firstBoot) {
			    if (_sys_exists((uint8*)AUTOEXEC)) {
				    uint16 cmd = CCPaddr + 8;
				    uint8 bytesread = (uint8)_RamLoad((uint8*)AUTOEXEC, cmd, 125);
				    uint8 blen = 0;
				    while (blen < bytesread && _RamRead(cmd + blen) > 31)
				    	blen++;
				    _RamWrite(cmd + blen, 0x00);
				    _RamWrite(--cmd, blen);
			    }
			    if (BOOTONLY)
				    firstBoot = FALSE;
		    }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(DSKByte));
        PC = CCPaddr;
        Z80run();
#endif
        if (Status == 1)
#ifdef DEBUG
	#ifdef DEBUGONHALT
    			Debug = 1;
		    	Z80debug();
	#endif
#endif
          break;

#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
}

void loop(void) {
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY);
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY * 4);
}
