# LCC_16_Output_Node
ESP32 Devkit 1 A basic 16 output node offering a number of actions per pin

**Disclaimer and Limitation of Liability**

This sketch (software) has been developed specifically for the **M5Stack Atom Lite** and the **M5Stack MiniCan** CAN transceiver module. It has only been tested on the author’s personal model railway layout.

**The sketch is provided “AS IS” and “AS AVAILABLE”**, without any warranties or guarantees of any kind. The author explicitly disclaims all warranties, whether express, implied, or statutory, including but not limited to any warranties of merchantability, fitness for a particular purpose, accuracy, reliability, or non-infringement.

The author accepts **no responsibility or liability** for:
- Any malfunction, failure, or unexpected behaviour of the sketch
- Damage to hardware, loss of data, or disruption to your layout
- Incompatibility caused by updates to third-party libraries, Arduino core, JMRI, or other software
- Any direct, indirect, incidental, consequential, or punitive damages arising from the use or inability to use this sketch

This code is offered strictly for **educational and hobbyist purposes** to help railway modellers learn how to use the OpenLCB Single Thread Library. It is not intended for commercial use, safety-critical applications, or any situation where failure could cause damage or injury.

By downloading, using, or modifying this sketch, you acknowledge that you assume **all risk** and full responsibility for any outcomes resulting from its use.

The author reserves the right to modify or remove this sketch at any time without notice.


# ESP32 16-Output OpenLCB Node

A powerful 16-channel output node for OpenLCB/LCC networks, built for ESP32. Supports 40 configurable actions across 16 GPIO pins with rich effects like flashing, strobing, random timing, and fire simulation.

## Features

- **16 Output Channels**: Uses ESP32 pins: 4, 16, 17, 18, 19, 21, 22, 23, 13, 12, 14, 27, 26, 25, 33, 32
- **40 Configurable Actions**: Each action tied to an OpenLCB event
- **Multiple Effects**:
  - **Low**: Steady LOW (0V)
  - **High**: Steady HIGH (3.3V)
  - **Flash**: Alternating HIGH/LOW with configurable on/off times
  - **Double Strobe**: HIGH-LOW-HIGH-LOW pattern
  - **Random**: Realistic random timing for lights (great for house lighting)
  - **Fire**: Dual-pin flickering simulation using PWM (`analogWrite`)
- **OpenLCB Compliant**: Full event-driven control via JMRI or other LCC tools
- **Persistent Configuration**: Stored in EEPROM via CDI
- **Debugging Support**: Optional serial debugging

## Hardware Requirements

- **ESP32 Dev Board**
- **CAN Bus Interface** (MCP2515 or native TWAI)
- **Power**: 5V for ESP32, 3.3V logic outputs (use drivers for high power loads)
- **Loads**: LEDs, relays, etc. Fire effect works best with PWM-capable pins.

## Pin Mapping

| Index | GPIO |
|-------|------|
| 1     | 4    |
| 2     | 16   |
| 3     | 17   |
| 4     | 18   |
| 5     | 19   |
| 6     | 21   |
| 7     | 22   |
| 8     | 23   |
| 9     | 13   |
| 10    | 12   |
| 11    | 14   |
| 12    | 27   |
| 13    | 26   |
| 14    | 25   |
| 15    | 33   |
| 16    | 32   |

## Action Types & Parameters

Each action (40 total) configures:
- **Event ID**
- **Pin #** (0 = disabled)
- **Action**
- **Parameter 1** (`durn`)
- **Parameter 2** (`rate`)

**Effect Details** (see top of sketch for full description):

- **Low / High**: Static output
- **Flash**: Alternates with Param1 = ON time, Param2 = OFF time (×100 ms)
- **Double Strobe**: Quick double pulse pattern
- **Random**: Realistic randomized HIGH/LOW periods (great for house lights)
- **Fire**: PWM flickering on two consecutive pins

## Setup & Usage

1. Install required OpenLCB libraries.
2. Configure `Config.h` (Node ID, CAN settings).
3. Upload the sketch.
4. Use **JMRI** to discover the node and edit the CDI:
   - Assign events
   - Choose pins and actions
   - Set timing parameters
5. Trigger events from your layout logic.
6. The node comes with 8 pre-defind examples you can use for pin. For fire it uses the next pin in the array so if pin 4 it uses pin 5 as well for a better effect.

## Development

- Based on work by **David Harris**, **Bob Jacobsen**, and others.
- Updated by **John Holmes** (2025).
- Debugging: Uncomment `#define DEBUG Serial`.

## License

Open source (follows original OpenLCB example licensing).

---
