#include <Arduino.h>
#include <USBComposite.h>

USBHID  HID;
HIDKeyboard Keyboard(HID); // create a profile

USBMIDI MIDI;

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
  MIDI.registerComponent();
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

uint32_t redwait = 0;
uint32_t greenwait = 0;
uint32_t yellowwait = 0;
uint32_t bluewait = 0;
uint32_t blackwait = 0;
uint32_t whitewait = 0;

uint32_t now; 

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

boolean check(int colour, boolean state, int value, uint32_t *wait){

    if ((now - *wait) > debounceDelay) {

      if (digitalRead(colour) == LOW and state == false) {
        state = true;
        MIDI.sendControlChange(15, value, 127);
        digitalWrite(ledPin, LOW);
        *wait = now;
      }

      if (digitalRead(colour) == HIGH and state == true) {
        state = false;
        MIDI.sendControlChange(15, value, 0);
        digitalWrite(ledPin, HIGH);
        *wait = now;
      }
    }

    return state;
}


void loop() {

  now = millis();

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
