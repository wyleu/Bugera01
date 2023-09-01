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

boolean check(int colour, boolean state, int value){
    if (digitalRead(colour) == LOW and state == false) {
      state = true;
      midi.sendControlChange(15, value, 127);
      digitalWrite(ledPin, LOW);
    }

    if (digitalRead(colour) == HIGH and state == true) {
      state = false;
      midi.sendControlChange(15, value, 0);
      digitalWrite(ledPin, HIGH);
    }

    return state;
}


void loop() {

  redstate = check(PA9, redstate, 102);
  greenstate = check(PA8, greenstate, 103);
  yellowstate = check(PB15, yellowstate, 104);
  bluestate = check(PB14, bluestate, 105);
  blackstate = check(PB13, blackstate, 106);
  whitestate = check(PB12, whitestate, 107);

  // digitalWrite(ledPin, HIGH);
  // delay(1000);
  // digitalWrite(ledPin, LOW);
  // delay(1000);
}


