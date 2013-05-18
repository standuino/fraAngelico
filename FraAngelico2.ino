
//Komentar
#include <EEPROM.h>
#include <MIDI.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

byte auduinoZvuk[15][5]; 
byte auduinoEnvelope[15][5];
byte auduinoADSR[15][4];

boolean isSound = false;

//boolean useLFO = true;
//boolean useADSR = true;
long adsrIterator = 0;

uint16_t syncPhaseAcc;
uint16_t syncPhaseInc;
uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;

//INITIAL ADSR CODE TODO:
uint8_t amplitude = 255;

// Map Analogue channels
#define SYNC_CONTROL         (4)
#define GRAIN_FREQ_CONTROL   (0)
#define GRAIN_DECAY_CONTROL  (2)
#define GRAIN2_FREQ_CONTROL  (3)
#define GRAIN2_DECAY_CONTROL (1)


// Changing these will also requires rewriting audioOn()

#if defined(__AVR_ATmega8__)
//
// On old ATmega8 boards.
//    Output is on pin 11
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_PIN       11
#define PWM_VALUE     OCR2
#define PWM_INTERRUPT TIMER2_OVF_vect
#elif defined(__AVR_ATmega1280__)
//
// On the Arduino Mega
//    Output is on pin 3
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       7
#define PWM_PIN       3
#define PWM_VALUE     OCR3C
#define PWM_INTERRUPT TIMER3_OVF_vect
#else
//
// For modern ATmega168 and ATmega328 boards
//    Output is on pin 3
//
#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_INTERRUPT TIMER2_OVF_vect
#endif

// Smooth logarithmic mapping
//
uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};
uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

// Stepped chromatic mapping
//
uint16_t midiTable[] = {
  17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
  77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
  244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
  732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
  4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
  10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
  22121,23436,24830,26306
};
uint16_t mapMidi(uint16_t input) {
  return (midiTable[(1023-input) >> 3]);
}

// Stepped Pentatonic mapping
//
/*
uint16_t pentatonicTable[54] = {
 0,19,22,26,29,32,38,43,51,58,65,77,86,103,115,129,154,173,206,231,259,308,346,
 411,461,518,616,691,822,923,1036,1232,1383,1644,1845,2071,2463,2765,3288,
 3691,4143,4927,5530,6577,7382,8286,9854,11060,13153,14764,16572,19708,22121,26306
 };
 
 uint16_t mapPentatonic(uint16_t input) {
 uint8_t value = (1023-input) / (1024/53);
 return (pentatonicTable[value]);
 }
 */

// midi control variables
// ---------------------------
int midi_note = 0;
int get_note = 0;
int key_pressed = 0;

void audioOn() {
  
#if defined(__AVR_ATmega8__)
  // ATmega8 has different registers
  TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
  TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
  TCCR3A = _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(CS30);
  TIMSK3 = _BV(TOIE3);
#else
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
#endif
}

//variables to handle EEPROM reset functionality
boolean inReset = false;
unsigned long timeAtReset = 0;

long startTime,stopTime;

void HandleNoteOn(byte channel, byte note, byte velocity) {   
  //Velocity 0 acts like note Off
  if (velocity == 0) {
    SetMIDINoteOff(note);
  } else {
    SetMIDIMode(true);
    SetMIDINote(note);
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) { 
  SetMIDINoteOff(note);
}

void HandleProgramChange(byte channel, byte program) { 
  SetMIDIProgram(program);
}

void setup() {
  
  
  
  InitializeUIValues();

  pinMode(PWM_PIN,OUTPUT);
   pinMode(13,OUTPUT);
  
  //Get input channel from EEProm and if some button is pressed change it to the new one
  //Then save back to EEPROM memory
  unsigned char inputChannel = GetMIDIChannel();
  UpdateUIInputs();
  inputChannel = GetSinglePressedButtonIndex(inputChannel);
  SetMIDIChannel(inputChannel);
  
  int diodIndex = 0;
  switch(inputChannel) {
    case 0:
      diodIndex = 3;
      break;
    case 1:
      diodIndex = 4;
      break;
    case 2: 
      diodIndex = 5;
      break;
    case 3:
      diodIndex = 6;
      break;
  }  
  
  FlashDiodBlocking(diodIndex, 3, 100);
  
  MIDI.begin(inputChannel + 1);
  MIDI.turnThruOn();
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleProgramChange(HandleProgramChange);

 //Serial.begin(9600);
  
  LoadPreset(0);
  
  audioOn();
 
}

void loop(){
  MIDI.read();
  UpdateUIInputs();
  UpdateApplicationLayer();
  RenderSound();
  ReflectApplicationLayer();
  ReflectUI();
}

void handleResetEEPROMTask() {
  //Handle reset action
  /*if (!inReset) {
    //Reset combination on the UI is on. Turn on the reset timer
    if (butSwitchState[1] && butSwitchState[2]) {
      timeAtReset = millis();
      inReset = true;  
      flashLED(300);
    }  
  } else {
    //Reset combination is still on
    if (butSwitchState[1] && butSwitchState[2]) {
      //Reset combination is turned on more that 10s -> Reset
      if ((millis() - timeAtReset) > 10000) {
        resetPresets(); 
        timeAtReset = 0;
        inReset = false;
        loadPreset(preset);
        flashLED(300);
      }  
    //Reset combination is not ON Turn of reset timer 
    } else {
      timeAtReset = 0;
      inReset = false;
    }
  }*/  
}

SIGNAL(PWM_INTERRUPT)
{
  uint8_t value;
  uint16_t output;
  
  syncPhaseAcc += syncPhaseInc;
  
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
    if (isSound && amplitude > 0) {
      LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
    }
  }
  
  
  

  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, clipping if necessary
  output >>= 9;
  
  if (amplitude < 1 || !isSound) {
    LED_PORT &= ~(1 << LED_BIT);
  }
  
  output *= amplitude;
  output >>= 9;
  
  if (output > 255) output = 255;

  
  // Output to PWM (this is faster than using analogWrite)  
  PWM_VALUE = output;
}

void resetEEMRPOMPresets() {
    for (int i = 0; i < 1024; i++) {
      EEPROM.write(i, 0);  
    }
}
