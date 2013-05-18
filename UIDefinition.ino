                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    #include "HWDefines.h"
/*-----------------------------------------------------------------------*/
/*-------------------------- APPLICATION LAYER --------------------------*/
/*-----------------------------------------------------------------------*/

enum AuduinoMode { PLAY_MODE, PLAY_ADSR_MODE, PLAY_LFO_MODE, EDIT_MODE, EDIT_LFO_MODE, EDIT_ADSR_MODE, SWITCH_PATTERN_MODE};
enum SelectedPattern {PATTERN_1 = 0, PATTERN_2, PATTERN_3, PATTERN_4};

AuduinoMode currentMode = PLAY_MODE;
unsigned char currentPattern = 0;
unsigned char currentSound = 0;
unsigned char currentPreset = 0;
unsigned char customSoundValues[NUMBER_OF_KNOBS];
//unsigned char customSoundValues[NUMBER_OF_KNOBS];

boolean forceLoadLFO = false;
boolean forceLoadADSR = false;

unsigned char releasedNoteBuffer = 0;
unsigned char releasedNoteCounter = 0;

unsigned char programSwitchCount = 0;
unsigned char programSwitchValue = 1;

boolean midiMode = false;
unsigned char midiProgram = 1;
unsigned char midiBufferIndex = 0;
unsigned char midiBuffer[5];

void SetLFOKnobValues() {

  knobMaxValues[KNOB_LFO_RATE_INDEX] = 100;
  knobMaxValues[KNOB_LFO_DEST_INDEX] = 6;
  knobMaxValues[KNOB_LFO_SMOOTH_INDEX] = 8;
  knobMaxValues[KNOB_LFO_SHAPE_INDEX] = 6;
  knobMaxValues[KNOB_LFO_AMOUT_INDEX] = 255; 
}

void SetSynthKnobValues() {
  knobMaxValues[0] = knobMaxValues[1] = knobMaxValues[2] = knobMaxValues[3] = knobMaxValues[4] = 255;
}

void SetADSRKnobValues() {
  knobMaxValues[0] = knobMaxValues[1] = knobMaxValues[2] = knobMaxValues[3] = knobMaxValues[4] = 255;
}

void SetEditLFOMode() {
  
}

void ResetInstrumentButtons() {
  SetHashState(&buttonStates, BUTTON_INSTRUMENT_1_INDEX, FALSE);
  SetHashState(&buttonStates, BUTTON_INSTRUMENT_2_INDEX, FALSE);
  SetHashState(&buttonStates, BUTTON_INSTRUMENT_3_INDEX, FALSE);
  SetHashState(&buttonStates, BUTTON_INSTRUMENT_4_INDEX, FALSE);
}

void SetInstrumentButtonsToSwitches() {
  SetButtonMode(BUTTON_INSTRUMENT_1_INDEX, SWITCH);
  SetButtonMode(BUTTON_INSTRUMENT_2_INDEX, SWITCH);
  SetButtonMode(BUTTON_INSTRUMENT_3_INDEX, SWITCH);
  SetButtonMode(BUTTON_INSTRUMENT_4_INDEX, SWITCH);
}

void SetInstrumentButtonsToButtons() {
  SetButtonMode(BUTTON_INSTRUMENT_1_INDEX, BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_2_INDEX, BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_3_INDEX, BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_4_INDEX, BUTTON);
}

void SetInstrumentButtonsToTwoStatesButtons() {
  SetButtonMode(BUTTON_INSTRUMENT_1_INDEX, TWO_STATES_RELEASE_BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_2_INDEX, TWO_STATES_RELEASE_BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_3_INDEX, TWO_STATES_RELEASE_BUTTON);
  SetButtonMode(BUTTON_INSTRUMENT_4_INDEX, TWO_STATES_RELEASE_BUTTON);
}

void SetEditModeUI() {
  SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE); 
  SetHashState(&buttonStates, BUTTON_ADSR_INDEX, FALSE);
  SetSynthKnobValues();  
}

void SetEditADSRModeUI() {
  SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE); 
  SetHashState(&buttonStates, BUTTON_ADSR_INDEX, FALSE);
  SetADSRKnobValues();  
}

boolean UseADSR() {
  switch (currentMode) {
    case PLAY_MODE:
    case PLAY_ADSR_MODE:
    case PLAY_LFO_MODE:
    case EDIT_ADSR_MODE:
      return true;
      break;
  }
  return false;
}

boolean UseLFO() {
  switch (currentMode) {
    case PLAY_MODE:
    case PLAY_ADSR_MODE:
    case PLAY_LFO_MODE:
    case EDIT_LFO_MODE:
    case EDIT_ADSR_MODE:
      return true;
      break;
  }
  return false;
}

void UpdateApplicationLayer() {
  //Define transitions between states
  switch (currentMode) {
    case PLAY_MODE:
      if (GetHashState(buttonStateChanges, BUTTON_EDIT_MODE_INDEX) == TRUE) {
        currentMode = EDIT_MODE; 
        ResetInstrumentButtons();
        SetInstrumentButtonsToSwitches();
        //SetEditModeUI();
      } else if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) {
        currentMode = PLAY_ADSR_MODE;
      } else if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
        currentMode = PLAY_LFO_MODE;
        SetLFOKnobValues();
      } else {
        ProceedPlayMode();
      }
      break;
    case PLAY_ADSR_MODE:
     if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) { 
       currentMode = PLAY_MODE;
     } else if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
       currentMode = SWITCH_PATTERN_MODE;
       ResetInstrumentButtons();
       SetInstrumentButtonsToTwoStatesButtons();
     } else {
       ProceedPlayMode();
     }
     break; 
    case PLAY_LFO_MODE:
     if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) { 
       currentMode = SWITCH_PATTERN_MODE;
       ResetInstrumentButtons();
       SetInstrumentButtonsToTwoStatesButtons();
       SetSynthKnobValues();
     } else if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
       currentMode = PLAY_MODE;
       SetSynthKnobValues();
     } else {
       ProceedPlayMode();
     }
     break;
    case EDIT_MODE:
      if (GetHashState(buttonStateChanges, BUTTON_EDIT_MODE_INDEX) == TRUE) {
        StorePreset(currentPreset);
        currentMode = PLAY_MODE;
        ResetInstrumentButtons();
        SetInstrumentButtonsToButtons();
      } else if(GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) {
        currentMode = EDIT_ADSR_MODE;
        SetButtonMode(BUTTON_LFO_INDEX, BUTTON);
        FreezeKnobs();
      } else if(GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
        currentMode = EDIT_LFO_MODE;
        FreezeKnobs();
        SetLFOKnobValues();
      } else {
        ProceedEditMode();
      }
      break;
    case SWITCH_PATTERN_MODE:
      if ((GetHashState(buttonStateChanges, BUTTON_EDIT_MODE_INDEX) == TRUE) && (GetHashState(buttonStates, BUTTON_EDIT_MODE_INDEX) == TRUE)) {
        ExportPresets();
      }
      else if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) {
        currentMode = PLAY_LFO_MODE;
        SetLFOKnobValues();
      } 
      else if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
        currentMode = PLAY_ADSR_MODE;
      } else {
        if (ProceedSwitchPatternMode()) {
          currentMode = PLAY_MODE;
          ResetInstrumentButtons();
          SetInstrumentButtonsToButtons();
        }
      }
      break;
    case EDIT_ADSR_MODE:
      if (GetHashState(buttonStateChanges, BUTTON_EDIT_MODE_INDEX) == TRUE) {
        currentMode = PLAY_MODE;
        ResetInstrumentButtons();
        SetInstrumentButtonsToButtons();
        StorePreset(currentPreset);
        //SwitchToPlayMode();
        //SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
        //useLFO = true;
        SetButtonMode(BUTTON_LFO_INDEX, SWITCH);
        //SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
      } else if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) {
        currentMode = EDIT_MODE;  
        SetButtonMode(BUTTON_LFO_INDEX, SWITCH);
        //SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
        FreezeKnobs();
      } else {
        ProceedEditADSRMode();
      }     
      break;
    case EDIT_LFO_MODE:
      if (GetHashState(buttonStateChanges, BUTTON_EDIT_MODE_INDEX) == TRUE) {
        currentMode = PLAY_MODE;
        ResetInstrumentButtons();
        SetInstrumentButtonsToButtons();
        SetSynthKnobValues();
        StorePreset(currentPreset);
        //SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
        //useLFO = true;
        //SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
      } else if (GetHashState(buttonStateChanges, BUTTON_ADSR_INDEX) == TRUE) {
        currentMode = EDIT_ADSR_MODE;
        SetSynthKnobValues();  
        SetHashState(&buttonStates, BUTTON_LFO_INDEX, FALSE);
        SetButtonMode(BUTTON_LFO_INDEX, BUTTON);
        FreezeKnobs();
      } else if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
        currentMode = EDIT_MODE;  
        SetSynthKnobValues();
        FreezeKnobs();
      }else {
        ProceedEditMode();
      }     
      break;
  }
  //UpdateLFOApplicationLayer();
}  

unsigned char GetSinglePressedButtonIndex(unsigned char defaultValue) {
  if (GetHashState(buttonStates, BUTTON_INSTRUMENT_4_INDEX) == TRUE) {
    return 3;   
  } else if (GetHashState(buttonStates, BUTTON_INSTRUMENT_3_INDEX) == TRUE) {
    return 2;   
  } else if (GetHashState(buttonStates, BUTTON_INSTRUMENT_2_INDEX) == TRUE) {
    return 1;   
  } else if (GetHashState(buttonStates, BUTTON_INSTRUMENT_1_INDEX) == TRUE) {
    return 0;   
  }    
  return defaultValue;
}

void ReflectApplicationLayer() {
  
  //Turn off all diaods
  diodStates = 0;
  switch (currentMode) {
    case PLAY_MODE:
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, FALSE);
      break;
    case PLAY_LFO_MODE:
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, FALSE);
      break;
    case PLAY_ADSR_MODE:
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, TRUE);
      break;
    case EDIT_MODE:
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, FALSE);
      break;
    case EDIT_LFO_MODE:
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, FALSE);
      break;
    case EDIT_ADSR_MODE: {
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, FALSE);
      SetHashState(&diodStates, DIOD_ADSR_INDEX, TRUE);
    }
      break;
    case SWITCH_PATTERN_MODE:
      SetHashState(&diodStates, DIOD_ADSR_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_LFO_INDEX, TRUE);
      SetHashState(&diodStates, DIOD_EDIT_MODE_INDEX, FALSE);
      break;
  }
  SetHashState(&diodStates, DIOD_INSTRUMENT_1_INDEX, GetHashState(buttonStates, BUTTON_INSTRUMENT_1_INDEX));
  SetHashState(&diodStates, DIOD_INSTRUMENT_2_INDEX, GetHashState(buttonStates, BUTTON_INSTRUMENT_2_INDEX));
  SetHashState(&diodStates, DIOD_INSTRUMENT_3_INDEX, GetHashState(buttonStates, BUTTON_INSTRUMENT_3_INDEX));
  SetHashState(&diodStates, DIOD_INSTRUMENT_4_INDEX, GetHashState(buttonStates, BUTTON_INSTRUMENT_4_INDEX));
}     

unsigned char GetNewSound(unsigned char defaultSound) {
   unsigned char newSound = GetHashState(buttonStates, BUTTON_INSTRUMENT_1_INDEX) |
                           (GetHashState(buttonStates, BUTTON_INSTRUMENT_2_INDEX) << 1) |
                           (GetHashState(buttonStates, BUTTON_INSTRUMENT_3_INDEX) << 2) |
                           (GetHashState(buttonStates, BUTTON_INSTRUMENT_4_INDEX) << 3);
                           
   if (newSound == releasedNoteBuffer) {
     if (releasedNoteCounter < RELEASE_BUTTON_BUFFER) {
       releasedNoteCounter++;
     }
     if (releasedNoteCounter == RELEASE_BUTTON_BUFFER) {
       return newSound;
     }
   } else {
     releasedNoteCounter = 0;
     if (releasedNoteBuffer != newSound) {
      releasedNoteBuffer = newSound;
      return 0;
     } else {
      releasedNoteBuffer = newSound;
     }
   }
   return defaultSound;
}

void ChangeSound(unsigned char index) {
  
  // No sound is selected mute the sound
  if (index != 0) {
    
    // Sound number 1 is stored in the index 0 because sound number 0 is MUTE
    unsigned char soundIndex = index - 1; 
    for (int i = 0; i < NUMBER_OF_KNOBS; i++) {
      auduinoZvuk[soundIndex][i] = GetKnobValue(GetKnobIndexForSynthValueIndex(i), auduinoZvuk[soundIndex][i]);
    }
  }
}

boolean ChangeLFO(unsigned char index) {
  
  boolean lfoChanged = false;
  // No sound is selected mute the sound
  if (index != 0) {
    // Sound number 1 is stored in the index 0 because sound number 0 is MUTE
    unsigned char soundIndex = index - 1; 
    for (int i = 0; i < NUMBER_OF_KNOBS; i++) {
      if (GetHashState(knobChanges, GetKnobIndexForLFOValueIndex(i)) == TRUE) {
        //Serial.println((int)GetKnobValue(GetKnobIndexForLFOValueIndex(i), auduinoEnvelope[soundIndex][i]));
        auduinoEnvelope[soundIndex][i] = GetKnobValue(GetKnobIndexForLFOValueIndex(i), auduinoEnvelope[soundIndex][i]);
        lfoChanged = true;
      }
    }
  }
  return lfoChanged;
}

void FreezeKnobs () {
  for (int i = 0; i < NUMBER_OF_KNOBS; i++) {
    SetHashState(&knobFreezed, i, TRUE);
  }
}

void UnFreezeKnobs () {
  for (int i = 0; i < NUMBER_OF_KNOBS; i++) {
    SetHashState(&knobFreezed, i, FALSE);
  }
}

void ProceedEditMode() {
  unsigned char newSound = GetNewSound(currentSound);
  if (currentSound != newSound) {
    currentSound = newSound;
    FreezeKnobs();
    LoadSound(currentSound);
    LoadLFO(currentSound);
  }
  if (!UseLFO()) {
    ChangeSound(currentSound);
    LoadSound(currentSound);
  } else {
    if (ChangeLFO(currentSound)) {
      LoadLFO(currentSound);
    }
  }
}

boolean ProceedSwitchPatternButton(unsigned char buttonIndex , unsigned char patternIndex) {
  if (GetHashState(buttonStateChanges, buttonIndex) == TRUE) {
    if (GetHashState(buttonStates, buttonIndex) == TRUE) {
      currentPreset=patternIndex + 4;
      LoadPreset(patternIndex + 4);
    } else {
      currentPreset=patternIndex;
      LoadPreset(patternIndex);
    }
    return true;
  }
  return false;
}

boolean ProceedSwitchPatternMode() {
  if (ProceedSwitchPatternButton(BUTTON_INSTRUMENT_1_INDEX, 0)) {
    return true;
  }
  if (ProceedSwitchPatternButton(BUTTON_INSTRUMENT_2_INDEX, 1)) {
    return true;
  }
  if (ProceedSwitchPatternButton(BUTTON_INSTRUMENT_3_INDEX, 2)) {
    return true;
  }
  if (ProceedSwitchPatternButton(BUTTON_INSTRUMENT_4_INDEX, 3)) {
    return true;
  }
  return false;
}

void ProceedPlayMode() {
  unsigned char newSound = GetNewSound(currentSound);
  if (midiMode) {
    if (midiBuffer[midiBufferIndex - 1] < 16) {
      newSound = midiBuffer[midiBufferIndex - 1] + 1;
    } else if (midiProgram == 0) {
      newSound = 0;
    } else {
      newSound = midiProgram;
    }
  }
  if (currentSound != newSound) {
    currentSound = newSound;
    LoadSound(currentSound);
    if (currentMode == PLAY_LFO_MODE) {
      forceLoadLFO = true;
      LoadADSR(currentSound);
    } else if (currentMode == PLAY_ADSR_MODE){
      forceLoadADSR = true;
      LoadLFO(currentSound);
    } else {
      FreezeKnobs(); //pedal
      LoadLFO(currentSound);
      LoadADSR(currentSound);
    } 
    //Load custom LFO values
    if (currentSound != 0) {
      for (int i = 0; i < NUMBER_OF_KNOBS; i++) {
        if (midiMode && (i == 4)) {
          customSoundValues[i] = midiBuffer[midiBufferIndex - 1]; 
        } else {
          customSoundValues[i] = auduinoZvuk[currentSound - 1][i];
        } 
      }
    } 
    if (newSound != 0) {
      ResetADSR();
    } 
  }
  if ((currentMode == PLAY_MODE) && (newSound != 0)) {
      for (int i = 0; i < NUMBER_OF_KNOBS;  i++) {
        if (GetHashState(knobChanges, GetKnobIndexForSynthValueIndex(i)) == TRUE) {
          if (midiMode && (i == 4)) {
            customSoundValues[i] = GetKnobValue(GetKnobIndexForSynthValueIndex(i), midiBuffer[midiBufferIndex - 1]);
            if (customSoundValues[i] != midiBuffer[midiBufferIndex - 1]) {
              customSoundValues[i] = map(customSoundValues[i], 0, 255, 0, 100);  
            }
          } else {
            customSoundValues[i] = GetKnobValue(GetKnobIndexForSynthValueIndex(i), auduinoZvuk[newSound - 1][i]);    
          }
        }
      } 
  }
  if (midiMode && (midiBuffer[midiBufferIndex - 1] > 15)) {
    unsigned char soundIndex = midiProgram - 1;
    LoadCustomSound(customSoundValues[0], 
                    customSoundValues[1], 
                    customSoundValues[2], 
                    customSoundValues[3], 
                    midiBuffer[midiBufferIndex - 1]);
  }  
  if (currentMode == PLAY_LFO_MODE) {
    UnFreezeKnobs();
    if (currentSound != 0) { 
      unsigned char newKnobValues[5];
      boolean lfoChanged = false;
      for (int i = 0; i < 5; i++) {
        if (GetHashState(knobChanges, GetKnobIndexForLFOValueIndex(i)) == TRUE) {
          lfoChanged = true;
        }
        newKnobValues[i] = GetKnobValue(GetKnobIndexForLFOValueIndex(i), auduinoEnvelope[currentSound - 1][i]);
      }    
      if (lfoChanged || forceLoadLFO) {
        forceLoadLFO = false;
        LoadCustomLFO(newKnobValues[0], 
                      newKnobValues[1], 
                      newKnobValues[2], 
                      newKnobValues[3], 
                      newKnobValues[4]);
      }
    }
  } if (currentMode == PLAY_ADSR_MODE) {
    
    UnFreezeKnobs();
    if (currentSound != 0) { 
      unsigned char newKnobValues[5];
      boolean adsrChanged = false;
      for (int i = 0; i < 5; i++) {
        if (GetHashState(knobChanges, GetKnobIndexForADSRValueIndex(i)) == TRUE) {
          adsrChanged = true;
        }
        newKnobValues[i] = GetKnobValue(GetKnobIndexForADSRValueIndex(i), auduinoADSR[currentSound - 1][i]);
      }    
      if (adsrChanged || forceLoadADSR) {
        forceLoadADSR = false;
        LoadCustomADSR(newKnobValues[0], 
                       newKnobValues[1], 
                       newKnobValues[2], 
                       newKnobValues[3]);
      }
    }
  } else {
    if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
      LoadLFO(currentSound); 
    }
    if ((newSound != 0) && !midiMode) {
      LoadCustomSound(customSoundValues[0], customSoundValues[1], customSoundValues[2], customSoundValues[3], map(customSoundValues[4], 0, 255, 0, 128));
    }
  }

  //Check for current sound if the sound takes longer than 10 execution of this fuinction it rewrites current midi program
  if ((currentSound != 0) && (programSwitchValue == currentSound)) {
    if (programSwitchCount < 20) {
      programSwitchCount++;
      if (programSwitchCount == 20) {
        midiProgram = programSwitchValue;
      } 
    }
  } else {
    programSwitchValue = currentSound;
    programSwitchCount = 0;
  }
}

void ProceedEditADSRMode() {
  unsigned char newSound = GetNewSound(currentSound);
  if (currentSound != newSound) {
    
    currentSound = newSound;
    FreezeKnobs();
    LoadSound(currentSound);
    LoadLFO(currentSound);
  }
  if (currentSound != 0) {
    //useLFO = true;
    boolean adsrChanged = false;
    for (unsigned char i = 0; i < 4; i++) {
      if (GetHashState(knobChanges, GetKnobIndexForADSRValueIndex(i)) == TRUE) {
          auduinoADSR[currentSound - 1][i] = GetKnobValue(GetKnobIndexForADSRValueIndex(i), auduinoADSR[currentSound - 1][i]);  
          adsrChanged = true;    
      }  
    }
    if (adsrChanged) {
      LoadADSR(currentSound);
    }
    if (GetHashState(buttonStateChanges, BUTTON_LFO_INDEX) == TRUE) {
      if (GetHashState(buttonStates, BUTTON_LFO_INDEX) == TRUE) {
        LoadSound(currentSound);
        ResetADSR();
      } else {
        ReleaseADSR();
      }  
    }
  }
}

void SetMIDIMode(boolean newMode) {
  midiMode = newMode;
}

void SetMIDINote(unsigned char note) {
  midiBuffer[midiBufferIndex] = note;
  if (midiBufferIndex < 5) {
    midiBufferIndex++;
  }
  midiMode = true;
  ResetADSR();
}

void SetMIDIProgram(unsigned char program) {
  midiProgram = program; 
}

void SetMIDINoteOff(unsigned char note) {
  int i = 0;
  boolean noteFound = false;
  for (i; i < 5; i++) {
    if (noteFound) {
      midiBuffer[i - 1] = midiBuffer[i];
    } else if (midiBuffer[i] == note) {
      midiBuffer[i] = 0;
      noteFound = true;
    }
  }
  if (noteFound) {
    midiBufferIndex--;
  }
  if (midiBufferIndex == 0) {
    midiMode = false;
  }
}
