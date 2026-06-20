
// To set a new nodeid edit the next line
#define NODE_ADDRESS  5,1,1,1,0x8E,0x01  // must be unique from an address space owned by you or DIY

// To Force Reset EEPROM to Factory Defaults set this value to 1, else 0.
// Need to do this at least once.
#define RESET_TO_FACTORY_DEFAULTS 1

//************** End of USER DEFINTIONS *****************************

#define ESP32_BOARD // Needed for sketch to work. We only offer this for the ESP32 Devkt 1

/* Debugging -- uncomment to activate debugging statements:
     dP(x) prints x, dPH(x) prints x in hex,
     dPS(string,x) prints string and x
*/
//#define DEBUG Serial

/*
 Allow direct to JMRI via USB, without CAN controller, comment out for CAN
*/

//#define USEGCSERIAL
//#include "GCSerial.h"
//#define NOCAN

#define CAN_RX_PIN (gpio_num_t) 15
#define CAN_TX_PIN (gpio_num_t) 2

#define NUM_CHANNEL 16        // number of pins
uint8_t pin[NUM_CHANNEL] = {4,16,17,18,19,21,22,23,13,12,14,27,26,25,33,32 };  // 16 channel
uint8_t currentEvent[NUM_CHANNEL];
#define NUM_ACTION 40         // number of evets/actions
#define NUM_EVENT NUM_ACTION  // number of action in total

uint8_t state[NUM_CHANNEL]; 
long timer[NUM_CHANNEL];

#define EEPROMSIZE 4096
#define EEPROMbegin { EEPROM.begin(EEPROMSIZE); dP("\nEEPROM begin "); dP(EEPROMSIZE)
#define EEPROMcommit { EEPROM.commit(); dP("EEPROM COMMIT"); }

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "16 Output"        // The model of the board
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

