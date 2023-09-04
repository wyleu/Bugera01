#include <Arduino.h>
#include <USBComposite.h>
#include "ws2812.h"


USBHID  HID;
HIDKeyboard Keyboard(HID); // create a profile

USBMIDI MIDI;

//  MIDI CC
// 102 – 119	Undefined

#define NUM_LEDS 20
#define DATA_PIN PA3
#define ledPin PC13 //13

int no_of_leds = 60;


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
  ws2812_init(1, no_of_leds);
}

 struct Button {
    char colour[20];          // Wire Colour Name
    char alt_colour[20];      // Button Colour Name
    int pin;                  // pin connected to Button
    int value;                // Midi value sent on press
    boolean state;            // Button current state
    uint32_t wait;            // Button time pressed
    uint32_t led1;            // LED pos for first led
    uint32_t led2;            // LED pos for second led
    uint32_t r;               // LED Red value
    uint32_t g;               // LED Green value
    uint32_t b;               // LED Blue value
};

struct Button red_button = {"Red", "Red", PA9, 102, false, 0, 1, 2, 255, 0, 0};
struct Button green_button = {"Green", "Green", PA8,  103, false, 0, 11, 12, 0, 255, 0};
struct Button yellow_button = {"Yellow"," Yellow", PB15,  104, false, 0, 21, 22, 255, 100, 0};
struct Button blue_button = {"Blue", "Blue", PB14, 105, false, 0, 31, 32, 0, 0, 255};
struct Button black_button = {"Black", "Purple", PB13, 106, false, 0, 41, 42, 255, 0, 255};
struct Button white_button = {"White", "Grey", PB12, 107, false, 0, 51, 52, 255, 255, 255};

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
        MIDI.sendControlChange(12, value, 127);
        digitalWrite(ledPin, LOW);
        *wait = now;
      }

      if (digitalRead(colour) == HIGH and state == true) {
        state = false;
        MIDI.sendControlChange(12, value, 0);
        digitalWrite(ledPin, HIGH);
        *wait = now;
      }
    }

    return state;
}

boolean button_check(Button &button){
    if ((now - button.wait) > debounceDelay) {
        if (digitalRead(button.pin) == LOW and button.state == false) {

          button.state = true;
          MIDI.sendControlChange(12, button.value, 127);

          ws2812_set(button.led1, button.r, button.g, button.b);
          ws2812_set(button.led2, button.r, button.g, button.b);
          ws2812_refresh();

          digitalWrite(ledPin, LOW);
          button.wait = now;
        }
        if (digitalRead(button.pin) == HIGH and button.state == true) {
          
          button.state = false;
          MIDI.sendControlChange(12, button.value, 0);

          ws2812_set(button.led1, 0, 0, 0);
          ws2812_set(button.led2, 0, 0, 0);
          ws2812_refresh();

          digitalWrite(ledPin, HIGH);
          button.wait = now;
        }
    }

    return button.state;

}

void loop() {

  now = millis();

  button_check(red_button);
  button_check(green_button);
  button_check(yellow_button);
  button_check(blue_button);
  button_check(black_button);
  button_check(white_button);

  // digitalWrite(ledPin, HIGH);
  // delay(1000);
  // digitalWrite(ledPin, LOW);
  // delay(1000);
}