#include <MIDI.h>

#include <elapsedMillis.h>

#include <MIDI.h>
#include "BetterButton.h"
#include "Arduino.h"
#include "Chord.h"
#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
int pos1 = 0;
int pos2 = 1;
int pos3 = 2;
int pos4 = 3;
int pos5 = 4;
int pos6 = 5;
int pos7 = 6;
int pos8 = 7;
int pos9 = 8;
int pos10 = 9;
int pos11 = 10;
int pos12 = 11;

int posArray[] = {pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9, pos10, pos11, pos12};

int octaveMod = 0;
AudioInputAnalog          adc1;           //xy=164,95
AudioAnalyzeNoteFrequency notefreq;
AudioConnection           patchCord1(adc1, notefreq);
elapsedMillis timeElapsed;
double  midiNote;
int midiNoteInt;

int ledStates[] = {LOW, LOW};
int greenLed = 22;
int redLed = 23;
int blueLed = 7;

unsigned long beat1 = 0;
unsigned long beat2 = 0;
int counter = 0;
int timer = 2000;
int switchPin = 33;



int tempo = 1200;

unsigned long beatArray[] = {beat1, beat2};
unsigned long stepTime1 = 0;


unsigned long lastStepTime = 0;
unsigned long lastStepTime2 = 0;
int chordCounter = 0;
int nextPin = 30;
int prevPin = 32;
int buttonPin2 = 31;

BetterButton nextChord(nextPin, 1);
BetterButton btn2(buttonPin2, 2);
BetterButton prevChord(prevPin, 3);

Chord C("Maj", "C", "7");
Chord x("Maj", "F");
Chord y("Maj", "G", "b7");
String temp = "";
//Chord chordList[] = {C, x, y};
Chord chordList[50];
int memory;
boolean done = true;

Chord* chordList2;
int* testArray;
String o;


/////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  userInfo();
  Serial.println("done");

  AudioMemory(30);
  notefreq.begin(.01);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(nextPin, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(switchPin, INPUT);
  for (int i = 0; i < 12; i++) {
    pinMode(posArray[i], INPUT);
  }

  nextChord.pressHandler(goNext);
  btn2.pressHandler(tapTempo);
  prevChord.pressHandler(goPrev);
  nextChord.releaseHandler(offPress);
  btn2.releaseHandler(offPress);
  prevChord.releaseHandler(offPress);

  int counter = 0;

}



void loop() {

//  Serial.println(digitalRead(switchPin));
  checkOctave();
  detectNote();
  nextChord.process();
  prevChord.process();
  btn2.process();


  if (digitalRead(switchPin) == HIGH) {
    btn2.pressHandler(tapTempo);
    playChords(chordCounter);
  }
  else
    btn2.pressHandler(triggerChord);


  lightLeds();

}

void playChords(int j) {
  if (millis() > lastStepTime + tempo) {
    for (int i = 0; i < chordList[j].NUMNOTES; i++) {
       usbMIDI.sendNoteOff(chordList[j].noteArray[i] + octaveMod, 0, 1);
    }

    for (int i = 0; i < chordList[j].NUMNOTES; i++) {
      usbMIDI.sendNoteOn(chordList[j].noteArray[i] + octaveMod, 127, 1);


    }
    lastStepTime = millis();
  }
}


void goNext(int buttonNum) {
  int counter = 0;

  if (chordCounter + 1 > memory-1)
    chordCounter = 0;
  else chordCounter++;

  for (int i = 0; i < chordList[chordCounter].NUMNOTES; i++) {

    temp = String(chordList[chordCounter].noteArray[i]);
    Serial.print(temp);
    temp = "";
    if (counter < chordList[chordCounter].NUMNOTES - 1) {
      Serial.print(",");
      counter++;
    }
  }
  Serial.println();

}


void goPrev(int buttonNum) {
  int counter = 0;

  if (chordCounter - 1 < 0)
    chordCounter = memory-1;
  else chordCounter--;

  for (int i = 0; i < chordList[chordCounter].NUMNOTES; i++) {

    temp = String(chordList[chordCounter].noteArray[i]);
    Serial.print(temp);
    temp = "";
    if (counter < chordList[chordCounter].NUMNOTES - 1) {
      Serial.print(",");
      counter++;
    }
  }
  Serial.println();

}
void tapTempo(int buttonNum) {
  if (counter + 1 > 1)
    counter = 0;
  else
    counter++;

  beatArray[counter] = millis();


  if (beatArray[0] > beatArray[1])
    stepTime1 = beatArray[0] - beatArray[1];
  else
    stepTime1 = beatArray[1] - beatArray[0];


  tempo = stepTime1;


  //  Serial.println(stepTime1);
}

void offPress(int buttonNum) {}


void detectNote() {
  if (notefreq.available()) {
    digitalWrite(blueLed, HIGH);
    float note = notefreq.read();
    float prob = notefreq.probability();
    note = roundNote(note);
    midiNote = 12 * (log2(note / 440)) + 69;
    //    Serial.printf("Note: %3.2f | Probability: %.2f\n", midiNote, prob);
    midiNote = (round(midiNote));
    midiNoteInt = ((int)(midiNote)) % 12;
//    Serial.println(midiNoteInt);
  }
  else if (millis() > lastStepTime2 + 1000) {
    digitalWrite(blueLed, LOW);
    lastStepTime2 = millis();
  }
}

boolean compareNote(int j) {
  detectNote();
  for (int i = 0; i < 8; i++) {
    if (isInArray(midiNoteInt, chordList[j]))
      return true;

    else
      midiNote = midiNoteInt + (12 * i);
  }
  return false;

}


boolean isInArray(int e, Chord c) {
  for (int i = 0; i < c.NUMNOTES; i++) {
    if (e == c.noteArray[i])
      return true;
  }
  return false;



}

float log2(float x) {
  return (log10(x) / log10(2));
}


void triggerChord(int buttonNum) {

  for (int i = 0; i < chordList[chordCounter].NUMNOTES; i++) {
    usbMIDI.sendNoteOff(chordList[chordCounter].noteArray[i] + octaveMod, 0, 1);
  }

  for (int i = 0; i < chordList[chordCounter].NUMNOTES; i++) {
    usbMIDI.sendNoteOn(chordList[chordCounter].noteArray[i] + octaveMod, 127, 1);


  }

}

float roundNote(float n) {

  float startNote;
  double a = 1.05946;
  //  Serial.println(80 < n && n < 112);

  if (80 < n && n < 112) {
    startNote = 65.406;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
  else if (112 < n && n < 224) {
    startNote = 110;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
  else if (224 < n && n < 336) {
    startNote = 220;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
  else if (336 < n && n < 448) {
    startNote = 329.63;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
  else if (448 < n && n < 560) {
    startNote = 440;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        //        Serial.println(startNote);
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
  else if (560 < n && n < 672) {
    startNote = 554.37;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }
    }
  }

  else if (672 < n && n < 784) {
    startNote = 659.25;
    for (int i = 0; i < 10; i++ ) {
      if (abs(startNote - n) < 2.5) {
        return startNote;
      }
      else {
        startNote = startNote * a;
      }

    }
  }
}

void lightLeds() {


  if (compareNote(chordCounter))
  {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
  }
  else {
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  }

}

void checkOctave() {
  for (int i = 0; i < 12; i++) {
    if (digitalRead(posArray[i]))
      //      Serial.println(
      octaveMod = 12 * i;
  }
}



void userInfo() {
  
  String numOfChords;
  String root;
  String quality;
  String suspensions;
  boolean check = true;
  Serial.println("Please input the number of chords in your progression");
  while (check == true) {
    if (Serial.available() > 0) {
      check = false;
      numOfChords = Serial.readString();
      memory=numOfChords.toInt();
      Serial.print("Num of chords is ");
      Serial.println(numOfChords);
    }
  }

  //
  for (int i = 0; i < memory; i++) {
    check == true;
    Serial.print("Input the root of chord ");
    Serial.println(i + 1);
    check = true;
    while (check == true) {
      if (Serial.available() > 0) {
        root = Serial.readString();
//        Serial.println(quality);
        check = false;
      }
    }
    check = true;
    Serial.print("Input the quality of chord ");
    Serial.println(i + 1);
    while (check == true) {
      if (Serial.available() > 0) {
        quality = Serial.readString();
//        Serial.println(root);
        check = false;
      }
    }
    check = true;
    Serial.print("Input the suspensions of chord ");
    Serial.println(i + 1);
    while (check == true) {
      if (Serial.available() > 0) {
        suspensions = Serial.readString();
//        Serial.println(suspensions);
        check = false;
      }
    }
    chordList[i]=Chord(quality,root,suspensions);
  }


}







