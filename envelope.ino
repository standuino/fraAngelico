enum LFOShape { UP_DOWN = 0, DOWN, UP, EVENUP_ODDDOWN, RANDOM_SIGN /* + / -*/, RANDOM_CHANGE_DIERECTION };

int lfoAmt = 0;

uint16_t grainPhaseInc_NoLFO;
uint8_t grainDecay_NoLFO;
uint16_t grain2PhaseInc_NoLFO;
uint8_t grain2Decay_NoLFO;
uint16_t syncPhaseInc_NoLFO;

int lfoStep;
int lfoStepTime;
int lfoStepAmt;
int lfoNowAmt;
LFOShape lfoShape;
byte lfoSteps;
byte lfoType;
int lfoRate;

unsigned char adsrState = 0;
int startADSRTime = 0;


void SetNoSound() {
  grainPhaseInc_NoLFO = 0;
  grainDecay_NoLFO = 0;
  grain2PhaseInc_NoLFO = 0;
  grain2Decay_NoLFO = 0;
  syncPhaseInc_NoLFO= 0;
}

void LoadSound(unsigned char index) {
  
  // No sound is selected mute the sound
  if (index == 0) {
    if (UseADSR()) {
      ReleaseADSR();
    } else {
      SetNoSound();
      isSound = false;
    }
  } else {
    // Sound number 1 is stored in the index 0 because sound number 0 is MUTE
    unsigned char soundIndex = index - 1; 
    LoadCustomSound(auduinoZvuk[soundIndex][0], 
                    auduinoZvuk[soundIndex][1], 
                    auduinoZvuk[soundIndex][2], 
                    auduinoZvuk[soundIndex][3], 
                    map(auduinoZvuk[soundIndex][4], 0, 255, 0, 128));
    
  }
}

void LoadCustomSound(unsigned char grainPhaseInc, unsigned char grainDecay, unsigned char grain2PhaseInc, unsigned char grain2Decay, unsigned char syncPhaseInc) {
    /*Serial.print((int)grainPhaseInc);Serial.print("\t");
    Serial.print((int)grainDecay);Serial.print("\t");
    Serial.print((int)grain2PhaseInc);Serial.print("\t");
    Serial.print((int)grain2Decay);Serial.print("\t");
    Serial.print((int)syncPhaseInc);Serial.print("\t");
    Serial.println("-----------------------------------");*/  
  grainPhaseInc_NoLFO = (mapPhaseInc(grainPhaseInc << 1) / 2);
    grainDecay_NoLFO = ((grainDecay << 1 ) / 8);
    grain2PhaseInc_NoLFO = (mapPhaseInc(grain2PhaseInc << 1) / 2);
    grain2Decay_NoLFO = ((grain2Decay << 1) / 4);
    /*uint16_t newSyncPhaseInc = midiTable[syncPhaseInc];
    if (pitchBand != 127) {
      char 
    }*/
    syncPhaseInc_NoLFO= midiTable[syncPhaseInc];
    isSound = true; 
}

void LoadLFO(unsigned char index) {
  if (index == 0) {
    /*lfoAmt = 0;
    lfoType = 0;
    lfoSteps = 0;
    lfoShape = (LFOShape)0;
    lfoRate = 0; 
    ResetLFO();*/
  } else {
    // Sound number 1 is stored in the index 0 because sound number 0 is MUTE
    unsigned char soundIndex = index - 1; 
    //Get LFO calues for current sound
    LoadCustomLFO(auduinoEnvelope[soundIndex][0], auduinoEnvelope[soundIndex][1], auduinoEnvelope[soundIndex][2], auduinoEnvelope[soundIndex][3], auduinoEnvelope[soundIndex][4]);
  }
}

void LoadCustomLFO(unsigned char amt, unsigned char dest, unsigned char smooth, unsigned char shape, unsigned char rate) {
   
    lfoAmt = pow(amt, 2);  
    lfoType = dest;
    lfoSteps = pow(2, smooth);
    lfoShape = (LFOShape)(shape);
    lfoRate = rate * 8;
    ResetLFO();
}


void ResetLFO() {
  //Reset LFO values so it strats to play from the beginning
  lfoStepTime = lfoRate / lfoSteps;
  lfoStep = 0;
  startTime = millis();
  //Serial.println("Reseted");
}

int currentRandomDirChangeMode = 1;
boolean ignoreOverflow = false;

int GetAmountForStep(byte _currentShape, int _currentStep, int _lfoAmount, int _stepCount) {
  switch (_currentShape) {
    //Up and down
    case UP_DOWN:
    ignoreOverflow = false;
      if (_currentStep < (_stepCount / 2)) {
        return _currentStep * (_lfoAmount / (_stepCount / 2));
      } else {
        return (_stepCount - _currentStep) * (_lfoAmount / (_stepCount / 2));
      }
      break;
    //Up
    case UP:
      ignoreOverflow = false;
      return (_lfoAmount / _stepCount) * _currentStep;
      break;
    //Down
    case DOWN:
      ignoreOverflow = false;
      return (_lfoAmount / _stepCount) * (_stepCount - _currentStep);
      break;
    //Odd up Even down
    case EVENUP_ODDDOWN:
      ignoreOverflow = false;
      if ((_currentStep % 2) == 0) {
        //Even values
        return (_lfoAmount / _stepCount) * _currentStep;
      } else {
        //Odd Values
        return -((_lfoAmount / _stepCount) * _currentStep);
      } 
      break;
    //direction is random in each step
    case RANDOM_SIGN: 
      ignoreOverflow = false;    
      return ((_lfoAmount / _stepCount) * _currentStep) * (pow(-1, random(2 , 3)));
      break;
      
    //get change of direction from random function
    case RANDOM_CHANGE_DIERECTION:     
      ignoreOverflow = true; 
      if (random(0, 100) == 5) {
        currentRandomDirChangeMode *= -1;
      }
      return ((_lfoAmount / _stepCount) * _currentStep) * currentRandomDirChangeMode;
      break;
  }
}

/*int renderOldLFO() {
  if (lfoShape == 7){  
    lfoUp = true;
  } else {
    lfoUp = false;
  }
  //int oldStepTime = lfoRate / lfoSteps;
  if (lfoUp) {
    if (lfoShape == 9) {
      lfoStep =- lfoStep;
      if (lfoStep >=0 ) {
        lfoStep++; 
      } else {
        lfoStep--; 
      }
    } else {
      lfoStep++;
      startTime = millis();
    }
  } else {
    lfoStep--;
    startTime = millis();
  }
  switch( lfoShape + 1) {
        case 6:
          if(lfoStep==-lfoSteps){
            lfoUp=true;
          } 
          if(lfoStep==lfoSteps){
            lfoUp=false;
          }
          break;
        case 7:
          if(lfoStep==lfoSteps){
            lfoUp=true;
            lfoStep=-lfoSteps;
          } 

          break;

        case 8:
        lfoUp=false;
          if(lfoStep==-lfoSteps){
            lfoUp=false;
            lfoStep=lfoSteps;
          } 
          break;

        case 9:
          if(lfoStep==lfoSteps || lfoStep==-lfoSteps){
            lfoUp=true;
            lfoStep=-lfoSteps;
          } 
          break;

        case 10:
          lfoUp=random(0,2);
          if(lfoStep==lfoSteps || lfoStep==-lfoSteps){
            lfoStep=0;  
          }

          break;
        } 
    return lfoAmt * lfoStep * 4 / lfoSteps;
}*/

void RenderLFO() {
 
  //When the amount for the current LDFo is zero do not count it
  if (lfoAmt == 0) {
    lfoNowAmt = 0; 
    return;  
  }  
  
  //Switch to another step when needed and Change the current amount for the current step
  if ((millis() - startTime) >= lfoStepTime) {
    lfoStep++;
    if (lfoStep >= lfoSteps) {
      lfoStep = 0;
    }  
    startTime = millis();
  }
  
  lfoNowAmt = GetAmountForStep(lfoShape, lfoStep, lfoAmt, lfoSteps) * 4;
  
  switch (lfoType) {
    case 0:
      lfoNowAmt = mapPhaseInc(lfoNowAmt) / 2 ;
      break;
    case 1:
      lfoNowAmt = lfoNowAmt / 8;
      break;
    case 2:
      lfoNowAmt = mapPhaseInc(lfoNowAmt) / 2 ;
      break;
    case 3:
      lfoNowAmt = lfoNowAmt / 4 ;
      break;
    case 4:
      lfoNowAmt = midiTable[lfoNowAmt] ;
      break;
    case 5:
      lfoNowAmt >>= 9;
      break;
    default:
      lfoNowAmt = midiTable[lfoNowAmt];
      break;
  }
}

uint8_t attackTime = 0;
uint8_t decayTime = 0;
uint8_t sustainLevel = 255;
uint8_t releaseTime = 0;
uint8_t startLevel = 0;

void LoadADSR(unsigned char index) {
  if (index != 0) {
    unsigned char soundIndex = index - 1;
    LoadCustomADSR(auduinoADSR[soundIndex][0], auduinoADSR[soundIndex][1], auduinoADSR[soundIndex][2], auduinoADSR[soundIndex][3]);
  }  
}

void LoadCustomADSR(unsigned char attack, unsigned char decay, unsigned char sustain, unsigned char release) {
    attackTime = attack;
    decayTime = decay;
    sustainLevel = 255 - sustain;
    releaseTime = release; 
}

void ResetADSR() {
  adsrState = 0;
  startADSRTime = millis();
  isSound = true; 
  //Serial.println("RESET");
}

void ReleaseADSR() {
  startLevel = amplitude;
  adsrState = 3;
  startADSRTime = millis();
}

void RenderADSR() {
  if (!UseADSR()) {
    amplitude = 255;
  } else {
    int timeFromStart = millis() - startADSRTime;
    switch (adsrState) {
      case 0:
        if (timeFromStart > ((int)attackTime << 4)) {
          adsrState = 1;
          startADSRTime = millis();    
        } else {
          amplitude = (255.0 / ((float)(attackTime << 4))) * (double)timeFromStart; 
        }
        break;
      case 1:
        if (timeFromStart > ((int)decayTime << 4)) {
          adsrState = 2;     
        } else {
          amplitude = 255 - ((255.0 - (float)sustainLevel) / ((float)(decayTime << 4))) * (double)timeFromStart; 
        }
        break;
      case 2:
        amplitude = sustainLevel;
        break;
      case 3:
        if (timeFromStart > ((int)releaseTime << 4) || (releaseTime == 0)) {
          adsrState = 4;
          SetNoSound();       
        } else {
          amplitude = startLevel - (((float)startLevel) / ((float)(releaseTime << 4))) * (double)timeFromStart; 
        }
        break;   
      case 4:
        amplitude = 0;
        break;
    }
  }
}

void RenderSound() {
  
  RenderLFO();
  RenderADSR();

  int grainPhaseIncLFO = 0;
  int grainDecayLFO = 0;
  int grain2PhaseIncLFO = 0;
  int grain2DecayLFO = 0;
  int syncPhaseIncLFO = 0;
  if (UseLFO() && isSound) {
    switch (lfoType ) {
      case 0:
        syncPhaseIncLFO = lfoNowAmt;
        break;
      case 1:
        grainPhaseIncLFO = lfoNowAmt;
        break;
      case 2:
        grainDecayLFO = lfoNowAmt;
        break;
      case 3:
        grain2PhaseIncLFO = lfoNowAmt;
        break;
      case 4:
        grain2DecayLFO = lfoNowAmt;
        break; 
      case 5:
        amplitude = amplitude + lfoNowAmt;
        break; 
      default:
        syncPhaseIncLFO = lfoNowAmt;
        break;
    }
  }
    grainPhaseInc = grainPhaseInc_NoLFO  + grainPhaseIncLFO;
    grainDecay = grainDecay_NoLFO + grainDecayLFO;
    grain2PhaseInc = grain2PhaseInc_NoLFO + grain2PhaseIncLFO;
    grain2Decay = grain2Decay_NoLFO + grain2DecayLFO;
    //Zde udelat pro posledni nastroj [15] aby se vyska brala z midi not
    syncPhaseInc = syncPhaseInc_NoLFO + syncPhaseIncLFO;
}










