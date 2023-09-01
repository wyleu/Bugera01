#include <Arduino.h>
#include <USBComposite.h>

USBHID  HID;
HIDKeyboard Keyboard(HID); // create a profile

USBMIDI midi;

//  MIDI CC
// 102 – 119	Undefined

#define ledPin PC13 //13

void setup() {
  USBComposite.clear();
  USBComposite.setProductId(0x0067);
  USBComposite.setVendorId(0x1eaa);
  USBComposite.setManufacturerString("ZynthianIO");
  USBComposite.setProductString("Test Rig");
  HID.registerComponent();
  midi.registerComponent();
  HID.setReportDescriptor(HID_KEYBOARD);
  USBComposite.begin();

  pinMode(ledPin, OUTPUT);

  pinMode(PB12, INPUT_PULLUP);
  pinMode(PB13, INPUT_PULLUP);
  pinMode(PB14, INPUT_PULLUP);
  pinMode(PB15, INPUT_PULLUP);
  pinMode(PA9, INPUT_PULLUP);
  pinMode(PA8, INPUT_PULLUP);
  
  while (!USBComposite);

}

boolean redstate = false;
boolean greenstate = false;
boolean yellowstate = false;
boolean bluestate = false;
boolean blackstate = false;
boolean whitestate = false;

unsigned long redwait = 0;
unsigned long greenwait = 0;
unsigned long yellowwait = 0;
unsigned long bluewait = 0;
unsigned long blackwait = 0;
unsigned long whitewait = 0;

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

boolean check(int colour, boolean state, int value, unsigned long *wait){

    if ((millis() - *wait) > debounceDelay) {

      if (digitalRead(colour) == LOW and state == false) {
        state = true;
        midi.sendControlChange(15, value, 127);
        digitalWrite(ledPin, LOW);
        *wait = millis();
      }

      if (digitalRead(colour) == HIGH and state == true) {
        state = false;
        midi.sendControlChange(15, value, 0);
        digitalWrite(ledPin, HIGH);
        *wait = millis();
      }
    }

    return state;
}


void loop() {

  redstate = check(PA9, redstate, 102, &redwait);
  greenstate = check(PA8, greenstate, 103, &greenwait);
  yellowstate = check(PB15, yellowstate, 104, &yellowwait);
  bluestate = check(PB14, bluestate, 105, &bluewait);
  blackstate = check(PB13, blackstate, 106, &blackwait);
  whitestate = check(PB12, whitestate, 107, &whitewait);

  // digitalWrite(ledPin, HIGH);
  // delay(1000);
  // digitalWrite(ledPin, LOW);
  // delay(1000);
}
