/* ESP32 16-Output Node 
   16 channels, ie pins
   N Action eventids
   each:
   -- Can pins RX pin 15 TX pin 2
   -- 16 channel version using pins4,16,17,18,19,21,22,23,13,12,14,27,26,25,33,32
   -- Actions from: Low, High, Flash, Double Strobe, Random, Fire

  EFFECTS:
  None
  Low: pulls the pin low (0V), the parameters make no change
  High: pushes the pin high (3.3V), the parameters make no change
  Flash: alternates between low and high
    Parameter 1: sets how long the pin is high (100-25.5 sec)
    Parameter 2: sets how long the pin is low (100-25.5 sec)
  Double strobe: puts the pin high, then low, then high, and then low.
    Parameter 1: how long the first low is held (100-25.5 sec)
    Parameter 2: how long the second low is held (100-25.5 sec)
  Random: alternates the pin between high and low
    Parameter 1: a random period the pin in held high, between 20ms and 20min
    Parameter 2: a random period the pin in held low, between 20ms and 20min
    Note the random period is between the half and the whole of the chosen period.
    Eg: if the chosen period is 30 sec, then the random periods will be 
        between 25 sec and 30 sec.  
    This should work well for house lights for example.  
  Fire: this uses two consecutive pins. 
    It uses analogWrite to set the brightness of two LEDs attached to the pins.
    You may have to choose higher values to see an effect, as values below a 
    threshold may not light the LED.  
    Parameter 1: sets the randomness of the first pin of the pair 
    Parameter 2: sets the randomness of the second pin of the pair 

*/

/*   
   David Harris 2019, adapted from
   Bob Jacobsen 2010, 2012
   based on examples by Alex Shepherd and David Harris
   Updated 2024.09 DPH
   Updated 2026.06 John Holmes

*/
#include "Config.h"         // Contains configuration, see "Config.h"
#include "ACAN_ESP32Can.h"  // uses ACan class, comment out if using GCSerial
#include "HouseKeeping.h"   // Used for development purposes only do not touch
#include "mdebugging.h"     // debugging
#include "processor.h"      // auto-selects the processor type, and CAN lib, EEPROM lib etc.
#include "OpenLcbCore.h"
#include "OpenLCBHeader.h"  // System house-keeping.



extern "C" {        // the following are defined as external
#define N(x) xN(x)  // allows the insertion of value (x)
#define xN(x) #x    // .. into the CDI string. \
                    // ===== CDI ===== \
                    //   Configuration Description Information in xml, **must match MemStruct below** \
                    //   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf \
                    //   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields.
  const char configDefInfo[] PROGMEM =
    // vvvvv Enter User definitions below CDIheader line vvvvv
    //       It must match the Memstruct struct{} below
    CDIheader R"(
      <name>Application Configuration</name>
      <group replication="4">
        <name>Action Groups</name>
        <hints><visibility hideable='yes' hidden='yes' ></visibility></hints>
        <description>Choose an Action group depending on your requirements. You have 40 Actions that can be spread across the 16 pins.</description>
        <repname>Actions 1-10</repname>
        <repname>Actions 11-20</repname>
        <repname>Actions 21-30</repname>
        <repname>Actions 31-40</repname>

    <group replication="10">
        <name>Actions</name>
        <string size='24'><name>Description</name></string>
        <repname>Action </repname>
        <description>Define what a pin can do when a consumed events is recieved. On initial install we have the first 8 actions pre configured, for pin 4. You may wish to take note of these setting for future reference.</description>
        <eventid><name>Event for action. Copy your produced event from another node and paste here to trigger this action.</name></eventid>
r
        <int size="1">
            <name>Choose the pin from dropdown list for the Action that is required . </name>
            <map>
                <relation><property>0</property><value>None</value></relation>
                <relation><property>1</property><value>4</value></relation>
                <relation><property>2</property><value>16</value></relation>
                <relation><property>3</property><value>17</value></relation>
                <relation><property>4</property><value>18</value></relation>
                <relation><property>5</property><value>19</value></relation>
                <relation><property>6</property><value>21</value></relation>
                <relation><property>7</property><value>22</value></relation>
                <relation><property>8</property><value>23</value></relation>
                <relation><property>9</property><value>13</value></relation>
                <relation><property>10</property><value>12</value></relation>
                <relation><property>11</property><value>14</value></relation>
                <relation><property>12</property><value>27</value></relation>
                <relation><property>13</property><value>26</value></relation>
                <relation><property>14</property><value>25</value></relation>
                <relation><property>15</property><value>33</value></relation>
                <relation><property>16</property><value>32</value></relation>
            </map>
        </int>

        <int size="1">
            <name>Choose the action you require for your chosen pin this choose the algorithm to be used for when you set the values for Parameter 1 and 2 in the next step</name>
            <map>
                <relation><property>0</property><value>None</value></relation>
                <relation><property>1</property><value>Low</value></relation>
                <relation><property>2</property><value>High</value></relation>
                <relation><property>3</property><value>Flash</value></relation>
                <relation><property>4</property><value>Double Strobe</value></relation>
                <relation><property>5</property><value>Random</value></relation>
                <relation><property>6</property><value>Fire (uses two pins)</value></relation>
            </map>
        </int>

        <int size="1">
            <name>Parameter 1 Is the On Duration</name>
            <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
        </int>

        <int size="1">
            <name>Parameter 2 is the Off Duration</name>
            <hints><slider tickSpacing='85' immediate='yes' showValue='yes'> </slider></hints>
        </int>
    </group>
</group>
       )" CDIfooter;
       )" CDIfooter;
  // ^^^^^ Enter User definitions above CDIfooter line ^^^^^
}

// ===== MemStruct =====
//   Memory structure of EEPROM, **must match CDI above**
//     -- nodeVar has system-info, and includes the node name and description fields
typedef struct {
  EVENT_SPACE_HEADER eventSpaceHeader;  // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
  char nodeName[20];                    // optional node-name, used by ACDI
  char nodeDesc[24];                    // optional node-description, used by ACDI
                                        // vvvvv Enter User definitions below vvvvv

  struct {
    char desc[24];
    EventID eid;
    uint8_t pini;
    char action;
    uint8_t durn;
    uint8_t rate;
  } action[NUM_ACTION];

  // ^^^^^ Enter User definitions above ^^^^^
} MemStruct;  // EEPROM memory structure, must match the CDI above

extern "C" {

  /*
  ===== eventid Table =====
    Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
      -- each eventid needs to be identified as a consumer, a producer or both.
      -- PEID = Producer-EID, CEID = Consumer, and PCEID = Producer/Consumer
      -- note matching references to MemStruct.
   This line defines a useful macro to make filling the table easier
  */

#define aEID(i) CEID(action[i].eid), \
                CEID(action[i + 1].eid), \
                CEID(action[i + 2].eid), \
                CEID(action[i + 3].eid), \
                CEID(action[i + 4].eid)

  const EIDTab eidtab[NUM_EVENT] PROGMEM = {
    aEID(0),
    aEID(5),
    aEID(10),
    aEID(15),
    aEID(20),
    aEID(25),
    aEID(30),
    aEID(35),
  };

  // SNIP Short node description for use by the Simple Node Information Protocol
  // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
  extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" OlcbCommonVersion;  // last zero in double-quote
                                                                                                                     //extern const char SNII_const_data[] PROGMEM = "\001RailStars\000Io 8-Out 32-InOut 16-Servo\0001.0\0002.0" ; // last zero in double-quote
                                                                                                                     ////extern const char SNII_const_data[] PROGMEM = "\001OpenLCB\0008Ouput\0001.0\0002.0\000"; // last zero in double-quote

};  // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {
  // 0xD7,0x58,0x00,0,0,0};
  pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent | pTeach | !pStream | !pReservation,  // 1st byte
  pACDI | pSNIP | pCDI | !pRemote | !pDisplay | !pTraction | !pFunction | !pDCC,               // 2nd byte
  0, 0, 0, 0                                                                                   // remaining 4 bytes
};

// ===== Process Consumer-eventIDs =====
// USER defined

#define PV(x) \
  { \
    dP(" " #x "="); \
    dP(x); \
  }
#define PVL(x) \
  { \
    dP("\n" #x "="); \
    dP(x); \
  }
void pceCallback(uint16_t index) {
  dP("\npceCallback(");
  dP(index);
  PV(NODECONFIG.read(EEADDR(action[index].pini)));
  PV(NODECONFIG.read(EEADDR(action[index].action)));
  PV(NODECONFIG.read(EEADDR(action[index].durn)));
  PV(NODECONFIG.read(EEADDR(action[index].rate)));
  uint8_t i = NODECONFIG.read(EEADDR(action[index].pini)) - 1;  // correct to 0-index
  currentEvent[i] = index;
  timer[i] = millis() + NODECONFIG.read(EEADDR(action[index].durn));
  state[i] = 0;
  PV(i);
  PV(currentEvent[i]);
  PV(timer[i]);
  PV(state[i]);
}

// userInitAll() -- include any initialization after Factory reset "Mfg clear" or "User clear"
//  -- clear or pre-define text variables.
// USER defined
bool initialized = false;
void userInitAll() {
  NODECONFIG.put(EEADDR(nodeName), ESTRING("ESP32"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("16 Output"));
  for (int i = 0; i < NUM_ACTION; i++) NODECONFIG.write(EEADDR(action[i].pini), 0);
  for (int i = 0; i < NUM_CHANNEL; i++) {
    pinMode(pin[i], OUTPUT);
    state[i] = 0;
    timer[i] = 0;
    currentEvent[i] = 255;
  }
  initialized = true;
}

// userSoftReset() - include any initialization after a soft reset, ie after configuration changes.
// USER defined
void userSoftReset() {
  //dP("\n In userSoftReset()"); Serial.flush();
  REBOOT;  // defined in processor.h for each mpu
}

// userHardReset() - include any initialization after a hard reset, ie on boot-up.
// USER defined
void userHardReset() {
  //dP("\n In userHardReset()"); Serial.flush();
  REBOOT;  // defined in processor.h for each mpu
}

/*
 ===== Callback from a Configuration write =====
 Use this to detect changes in the node's configuration
 This may be useful to take immediate action on a change.
 param address - address in space of change
 param length  - length of change
 NB: if address=0 and length==0xffff, then user indicated UPDATE_COMPLETE

 USER defined
*/

void userConfigWritten(uint32_t address, uint16_t length, uint16_t func) {
}

NodeID nodeid(NODE_ADDRESS);  // this node's nodeid, do not move
#include "OpenLCBMid.h"       // System house-keeping

enum Action { aLOW = 1,
              aHIGH,
              aFLASH,
              aDSTROBE,
              aRANDOM,
              aFIRE };
void doActions() {
  static uint8_t i = 255;
  uint8_t pi, actn, rate, durn;
  //static long last = 0;
  //if( (now-last) <(50/NUM_CHANNEL) ) return;
  //last = now;
  if (!initialized) return;
  if (++i >= NUM_CHANNEL) i = 0;
  uint8_t ei = currentEvent[i];
  if (ei == 255) return;
  long now = millis();
  //PVL(i); PV(ei);
  pi = NODECONFIG.read(EEADDR(action[ei].pini)) - 1;  // pin number
  actn = NODECONFIG.read(EEADDR(action[ei].action));
  durn = NODECONFIG.read(EEADDR(action[ei].durn));
  rate = NODECONFIG.read(EEADDR(action[ei].rate));
  //PVL(pi); PV(pin[pi]); PV(actn); PV(durn); PV(rate); PV(state[i]);
  switch (actn) {
    case aLOW:
      if (state[i]) return;
      dP("\naLOW");
      digitalWrite(pin[pi], LOW);
      state[i] = 1;
      break;
    case aHIGH:
      if (state[i]) return;
      dP("\naHIGH");
      digitalWrite(pin[pi], HIGH);
      state[i] = 1;
      break;
    case aFLASH:
      if (timer[i] && (now - timer[i]) > 0) {
        dP("\naFLASH");
        switch (state[i]) {
          case 0:
            digitalWrite(pin[pi], HIGH);
            timer[i] = now + durn * 100;
            state[i] = 1;
            break;
          case 1:
            digitalWrite(pin[pi], LOW);
            timer[i] = now + rate * 100;
            state[i] = 0;
            break;
        }
      }
      break;
    case aDSTROBE:
      if (timer[i] && (now - timer[i]) > 0) {
        dP("\naDSTROBE");
        PV(state[i]);
        switch (state[i]) {
          case 0:
            digitalWrite(pin[pi], HIGH);
            timer[i] = now + 100;
            PV(timer[i]);
            state[i] = 1;
            break;
          case 1:
            digitalWrite(pin[pi], LOW);
            timer[i] = now + durn * 100;
            PV(timer[i]);
            state[i] = 2;
            break;
          case 2:
            digitalWrite(pin[pi], HIGH);
            timer[i] = now + 100;
            PV(timer[i]);
            state[i] = 3;
            break;
          case 3:
            digitalWrite(pin[pi], LOW);
            timer[i] = now + rate * 100;
            PV(timer[i]);
            state[i] = 0;
            break;
        }
      }
      break;
    case aRANDOM:
      if (timer[i] && (now - timer[i]) > 0) {
        dP("\naRANDOM");
        PV(state[i]);
        switch (state[i]) {
          case 0:
            digitalWrite(pin[pi], HIGH);
            timer[i] = now + random(durn * durn * 5, durn * durn * 20);
            PV(timer[i]);
            state[i] = 1;
            break;
          case 1:
            digitalWrite(pin[pi], LOW);
            timer[i] = now + random(rate * rate * 5, rate * rate * 20);
            PV(timer[i]);
            state[i] = 0;
            break;
        }
      }
      break;
    case aFIRE:
      if (state[i] != 0) return;
      if (durn == 0) {
        analogWrite(pin[pi], 0);
        analogWrite(pin[pi + 1], 0);
        state[i] = 1;
      }
      if (timer[i] && (now - timer[i]) > 0) {
        dP("\naFIRE");
        dP(pin[pi]);
        if (pi + 1 > NUM_CHANNEL) break;
        analogWrite(pin[pi], random(durn));
        analogWrite(pin[pi + 1], random(rate));
        timer[i] = now + 15;
      }
      break;
  }
}

// ==== Setup does initial configuration =============================
void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(500);
  Serial.print("\n Testing 16 Output");
#endif

  NodeID nodeid(NODE_ADDRESS);  // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);
  // set output pins to channel
  for (int c = 0; c < NUM_CHANNEL; c++) {
    pinMode(pin[c], OUTPUT);
  }

#if 1
  // test values
  NODECONFIG.put( EEADDR( action[0].desc ), ESTRING ("Off") );
  NODECONFIG.write(EEADDR(action[0].pini), 1);
  NODECONFIG.write(EEADDR(action[0].action), aLOW);

  NODECONFIG.put( EEADDR( action[1].desc ), ESTRING ("On") );
  NODECONFIG.write(EEADDR(action[1].pini), 1);
  NODECONFIG.write(EEADDR(action[1].action), aHIGH);

  NODECONFIG.put( EEADDR( action[2].desc ), ESTRING ("Flash") );
  NODECONFIG.write(EEADDR(action[2].pini), 1);
  NODECONFIG.write(EEADDR(action[2].action), aFLASH);
  NODECONFIG.write(EEADDR(action[2].durn), 10);
  NODECONFIG.write(EEADDR(action[2].rate), 10);

  NODECONFIG.put( EEADDR( action[3].desc ), ESTRING ("Strobe") );
  NODECONFIG.write(EEADDR(action[3].pini), 1);
  NODECONFIG.write(EEADDR(action[3].action), aDSTROBE);
  NODECONFIG.write(EEADDR(action[3].durn), 2);
  NODECONFIG.write(EEADDR(action[3].rate), 10);

  NODECONFIG.put( EEADDR( action[4].desc ), ESTRING ("Random 1") );
  NODECONFIG.write(EEADDR(action[4].pini), 1);
  NODECONFIG.write(EEADDR(action[4].action), aRANDOM);
  NODECONFIG.write(EEADDR(action[4].durn), 20);
  NODECONFIG.write(EEADDR(action[4].rate), 20);

  NODECONFIG.put( EEADDR( action[5].desc ), ESTRING ("Random 2") );
  NODECONFIG.write(EEADDR(action[5].pini), 1);
  NODECONFIG.write(EEADDR(action[5].action), aRANDOM);
  NODECONFIG.write(EEADDR(action[5].durn), 5);
  NODECONFIG.write(EEADDR(action[5].rate), 3);

  NODECONFIG.put( EEADDR( action[6].desc ), ESTRING ("Fire on") );
  NODECONFIG.write(EEADDR(action[6].pini), 1);
  NODECONFIG.write(EEADDR(action[6].action), aFIRE);
  NODECONFIG.write(EEADDR(action[6].durn), 135);
  NODECONFIG.write(EEADDR(action[6].rate), 160);

  NODECONFIG.put( EEADDR( action[7].desc ), ESTRING ("Fire off") );
  NODECONFIG.write(EEADDR(action[7].pini), 1);
  NODECONFIG.write(EEADDR(action[7].action), aFIRE);
  NODECONFIG.write(EEADDR(action[7].durn), 0);
  NODECONFIG.write(EEADDR(action[7].rate), 0);

  for (uint8_t index = 0; index < 3; index++) {
    uint8_t pini = NODECONFIG.read(EEADDR(action[index].pini));
    uint8_t action = NODECONFIG.read(EEADDR(action[index].action));
    uint8_t durn = NODECONFIG.read(EEADDR(action[index].durn));
    uint8_t rate = NODECONFIG.read(EEADDR(action[index].rate));
    PVL(index);
    PV(pini);
    PV(action);
    PV(durn);
    PV(rate);
  }
  for (int i = 0; i < NUM_CHANNEL; i++) {
    PVL(i);
    PV(currentEvent[i]);
    PV(timer[i]);
    PV(state[i]);
  }
#endif
}

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
void loop() {

  bool activity = Olcb_process();  // System processing happens here, with callbacks for app action.

  doActions();
}
