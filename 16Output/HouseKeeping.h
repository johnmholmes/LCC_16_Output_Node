/*
Do not delete as maybe required in the future.

*/

#define OLCB_NO_BLUE_GOLD  // don't delete or comment out

#ifndef OLCB_NO_BLUE_GOLD
  // ===== Blue/Gold =====
  // Blue-gold refers to two standard buttons offering a rudimentary control system for an node.
  //   Features: teaching/learning of eventids; node identification; node reset.

  // Board choices, each has differing i/o choices
    #include "boardChoices.h"

  // This section uses the ButtonLed lib to muliplex an input and output onto a single pin.
  // It includes sampling every 32 ms, and blink patterns.

  // Patterns
  // Each pattern is 32 bits, each bit is used sequencely to blink the LED on and off, at 64 ms per bit.
    #define ShortBlinkOn   0x00010001L
    #define ShortBlinkOff  0xFFFEFFFEL
    uint32_t patterns[NUM_EVENT] = { // two per channel, one per event
      ShortBlinkOn
    };
  //ButtonLed pA(14, LOW);
  // An array of buttons/leds.
    ButtonLed* buttons[NUM_EVENT] = {
       // One for each event; each channel is a pair
       &pA
    };
#endif // OLCB_NO_BLUE_GOLD   // this app uses ButtonLed lib for its i/o.