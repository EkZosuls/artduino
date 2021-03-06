#include <SoftwareSerial.h>
#include <avr/interrupt.h>

SoftwareSerial mySerial(2, 3); // RX, TX

const int potInput = A0;
const int adjustedPotRange = 860;
int potValue = 0;
int shortDelay = 0;
int longDelay = 0;
  byte note = 0;
  byte resetMIDI = 4;
  byte ledPin = 13; //MIDI traffic inidicator
volatile int beatSelector = 0; // determines which beat is used - changed by interrupt
volatile long lastPress = 0;
unsigned long shutoffTime = 0;
unsigned long shutoffMinutes = 1;
int lastLeverPosition = 0;


void setup(void) 
{
  
  pinMode(potInput, INPUT);
  
  
  Serial.begin(9600);   
  Serial.println("noses");
  shutoffTime = shutoffMinutes * 1000 * 20;  // 1 minutes in milliseconds


//Setup soft serial for MIDI control
  mySerial.begin(31250);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  int  instrument = 0x01;
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127) 
  //talkMIDI(0xB0, 0, 0x00); //Default bank GM1
  talkMIDI(0xB0, 0, 0x78); //Bank select drums
  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command
  //the instrument number will be in a switch tree once the selector switch is implemented  
  
  // setting up interrupts
  sei();                    // enable interrupts
  EIMSK |= (1 << INT0);     // pin 2
  EICRA |= (1 << ISC01);
}

void loop(void)
{
  potValue = analogRead(potInput);
  //Serial.println(potValue);
  potValue = (potValue * 2) - 900; // Usually in the range 470-900 so adjust to 40 - 900
  // talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command
  switch (beatSelector) {
    case 0:
      noBeats();
      break;
    case 1:
      beat1(potValue);
      break;
    case 2:
      beat2(potValue);
      break;
    case 3:
      moarCowbell(potValue);
      break;
    case 4:
      beat4(potValue);
      break;
    case 5:
      beat5(potValue);
      break;
    case 6:
      beat6(potValue);
      break;
    default:
      noBeats();
  }
  if (abs(lastLeverPosition - potValue) >= 5) {
    lastPress = millis();
  }
  if ((millis() - lastPress) > shutoffTime) {
    beatSelector = 0;
  }
  lastLeverPosition = potValue;
  //Serial.println(potValue);             // debug value
  //Serial.println(lastPress);
  //Serial.println(millis());
}

// functions
void noBeats(void) {
  delay(50);
}

void beat1(int potValue) {
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  longDelay = 40.0 * potValue / adjustedPotRange + 10;
  noteOn(0,36,60);  // Bass Drum 1
  delay(shortDelay);
  noteOn(0,36,60);
  delay(longDelay);
  //noteOff(0,36,60);
  noteOn(0,51,60);  // Ride Cymbal 1
  delay(shortDelay);
  noteOff(0,51,60);
  delay(potValue);
}

void beat2(int potValue) {
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  longDelay = 40.0 * potValue / adjustedPotRange + 10;
  noteOn(0,44,60); // Pedal Hi-hat
  delay(shortDelay);
  noteOn(0,44,60);
  delay(longDelay);
  //noteOff(0,36,60);
  noteOn(0,45,60); // Low Tom
  delay(shortDelay);
  noteOff(0,45,60);
  delay(potValue);
}

void moarCowbell(int potValue) {
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  noteOn(0,56,60); // Cowbell
  delay(shortDelay);
  noteOff(0,56,60);
  delay(potValue);
}

void beat4(int potValue) {
  // A little more swing
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  longDelay = 40.0 * potValue / adjustedPotRange + 10;
  noteOn(0,45,60); // Open Hi-hat
  delay(longDelay);
  noteOff(0,45,60);
  delay(potValue);

  noteOn(0,44,60); // Pedal Hi-hat
  delay(longDelay);
  noteOff(0,45,60);
  delay(potValue / 2);
  noteOn(0,44,60); // Pedal Hi-hat
  delay(shortDelay);
  noteOff(0,45,60);
  delay(potValue);
}

void beat5(int potValue) {
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  longDelay = 40.0 * potValue / adjustedPotRange + 10;
  noteOn(0,69,60); // Cabasa
  delay(potValue);
  noteOff(0,69,60);
  delay(potValue);

  noteOn(0,70,60); // Maracas
  delay(longDelay);
  noteOff(0,70,60);
  delay(potValue / 2);
  noteOn(0,70,60);
  delay(shortDelay);
  noteOff(0,70,60);
  delay(potValue);
}

void beat6(int potValue) {
  // A little bit of everything
  shortDelay = 10.0 * potValue / adjustedPotRange + 10;
  longDelay = 40.0 * potValue / adjustedPotRange + 10;
  noteOn(0,67,60); // High Agogo
  delay(longDelay);
  noteOff(0,67,60);
  delay(potValue);

  noteOn(0,68,60); // Low Agogo
  delay(35);
  noteOff(0,68,60);
  delay(potValue / 2);
  
  noteOn(0,68,60); // Low Agogo
  delay(shortDelay);
  noteOff(0,68,60);
  delay(potValue / 2);
  
  noteOn(0,58,60); // Vibra Slap
  delay(longDelay);
  noteOff(0,58,60);
  delay(potValue);
}

ISR(INT0_vect) {
  if (millis() - lastPress > 200) {
    beatSelector++;
  }
  lastPress = millis();
  //Serial.println(beatSelector);
  if (beatSelector == 7) {       // this num should be 1 greater than the highest beat function num
    beatSelector = 0;            // loop back to zero when no beats are left
  }
}


//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}


