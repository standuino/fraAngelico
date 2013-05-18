#define PRESET_SIZE 225
void ExportPresets() {
  Serial.end(); 
  Serial.begin(9600);
  Serial.print("prog_uchar flashPpreset[900] PROGMEM = {");
  boolean first = true;
  for (int i = 0; i < 4 * PRESET_SIZE; i++) {
    if (!first) {
      Serial.print(",");
    }
    if ((i != 0) && ((i % 75) == 0)) {
      Serial.println((int)(EEPROM.read(i)));
    } else {
      Serial.print((int)(EEPROM.read(i)));
    }
    first = false; 
  }   
  Serial.print("} ");
  Serial.end();
  MIDI.begin(GetMIDIChannel());
}

void StorePreset(unsigned char index) {
  if (index < 4) {
    int offset = index * PRESET_SIZE;
    for (int j = 0; j < 15; j++) {
      for (int k = 0; k < 5; k++) {
        EEPROM.write(offset + ((5 * j) + k), auduinoZvuk[j][k]);
        EEPROM.write(offset + 75 + ((5 * j) + k), auduinoEnvelope[j][k]);
        if (k < 4) {
          EEPROM.write(offset + 150 + ((4 * j) + k), auduinoADSR[j][k]);
        }
      }
    }
  }
}

void LoadPreset(unsigned char index) {
  int offset = (index % 4) * PRESET_SIZE;
  if (index < 4) {
    for (int j = 0; j < 15; j++) {
      for (int k = 0; k < 5; k++) {
        auduinoZvuk[j][k] = EEPROM.read(offset + ((5 * j) + k));
        auduinoEnvelope[j][k] = EEPROM.read(offset + 75 + ((5 * j) + k));
        if (k < 4) {
          auduinoADSR[j][k] = EEPROM.read(offset + 150 + ((4 * j) + k));
        }      
      }
    }
  } else {
    for (int j = 0; j < 15; j++) {
      for (int k = 0; k < 5; k++) {
        auduinoZvuk[j][k] = pgm_read_word_near(flashPpreset + (offset + ((5 * j) + k)));
        auduinoEnvelope[j][k] = pgm_read_word_near(flashPpreset + 75 + (offset + ((5 * j) + k)));
        if (k < 4) {
          auduinoADSR[j][k] = pgm_read_word_near(flashPpreset + 150 + (offset + ((4 * j) + k)));
        } 
      }
    }
  }
}

unsigned char GetMIDIChannel() {
  return EEPROM.read(PRESET_SIZE * 4);
}

void SetMIDIChannel(unsigned char value) {
  EEPROM.write(PRESET_SIZE * 4, value);
}
