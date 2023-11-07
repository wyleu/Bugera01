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
#define LED_PIN PC13 //13
#define DEBUG_FLASH false
#define LED_ON_PAUSE 200          //  Length of on LED Display on LEADING edge trigger.
#define DEFAULT_MIDI_CHANNEL 12  //  Default MIDI channel for startup  (Channel 13) 

#define MODE_ON_PRESS_OFF  0      //  Initially OFF, LED OFF. Sends MIDI ON, LED ON on press, Sends MIDI OFF, LED OFF on release
#define MODE_OFF_PRESS_ON  1      //  Initially ON, LED ON. Sends MIDI OFF, LED OFF on press, Sends MIDI ON, LED ON on release 
#define MODE_ON_PRESS  2          //  Initially OFF, LED OFF. Sends MIDI ON, LED ON on press. No action on release
#define MODE_OFF_PRESS  3         //  Initially ON, LED ON, Send MIDI OFF, LED OFF on Press. No action on release
#define TOGGLE_ON  4              //  Initially OFF, LED OFF, Send MIDI ON, LED ON on Press Toggle Send MIDI ON, LED ON and Send MIDI OFF, LED OFF
#define TOGGLE_OFF  5             //  Initially ON, LED ON, Send MIDI ON, LED ON on Press Toggle Send MIDI OFF, LED OFF and Send MIDI ON, LED ON

int no_of_leds = 60;


void setup() {
  USBComposite.clear();
  USBComposite.setProductId(0x0067);
  USBComposite.setVendorId(0x1eaa);
  USBComposite.setManufacturerString("ZynthianIO");
  USBComposite.setProductString("Wyleu Bugera Pedal");
  HID.registerComponent();
  MIDI.registerComponent();
  HID.setReportDescriptor(HID_KEYBOARD);
  USBComposite.begin();

  pinMode(LED_PIN, OUTPUT);

  pinMode(PA0, INPUT_ANALOG);     // PSU Voltage
  pinMode(PA1, INPUT_ANALOG);     // PSU Voltage

  pinMode(PA8, INPUT_PULLUP);      // Bugera Switch 2 Green
  pinMode(PA9, INPUT_PULLUP);      // Bugera Switch 1 Red  

  pinMode(PB12, INPUT_PULLUP);     // Bugera Switch 6 White/Grey
  pinMode(PB13, INPUT_PULLUP);     // Bugera Switch 5 Purple/Black
  pinMode(PB14, INPUT_PULLUP);     // Bugera Switch 4 Blue
  pinMode(PB15, INPUT_PULLUP);     // Bugera Switch 3 Yellow


  
  while (!USBComposite);
  ws2812_init(1, no_of_leds);
}

 struct Button {
    char colour[20];          // Wire Colour Name
    char alt_colour[20];      // Button Colour Name
    int pin;                  // pin connected to Button
    uint32_t mode;            // Button Mode
    int channel;              // Midi channel  for button
    int midi;                 // Midi value sent on press
    boolean state;            // Button current state
    uint32_t wait;            // Button time pressed
    uint32_t led1;            // LED pos for first led
    uint32_t led2;            // LED pos for second led
    uint32_t led3;            // LED pos for third led
    uint32_t led4;            // LED pos for fourth led
    uint32_t r;               // LED Red value
    uint32_t g;               // LED Green value
    uint32_t b;               // LED Blue value
};


struct Button red_button = {"Red", "Red", PA9, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 102, false, 0, 0, 1, 2, 3, 255, 0, 0};
struct Button green_button = {"Green", "Green", PA8, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 103, false, 0, 9, 10, 11, 12, 0, 255, 0};
struct Button yellow_button = {"Yellow"," Yellow", PB15, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 104, false, 0, 19, 20, 21, 22,  255, 100, 0};
struct Button blue_button = {"Blue", "Blue", PB14, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 105, false, 0, 29, 30, 31, 32, 0, 0, 255};
struct Button black_button = {"Black", "Purple", PB13, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 106, false, 0, 39, 40, 41, 42, 255, 0, 255};
struct Button white_button = {"White", "Grey", PB12, MODE_ON_PRESS_OFF, DEFAULT_MIDI_CHANNEL, 107, false, 0, 49, 50, 51, 52, 255, 255, 255};

struct Voltage {
    char colour[20];
    char alt_colour[20];
    int pin;                  // pin connected to Voltage                      
    int midi;                 // Midi value sent on press
    int value;                // Current Value
};

struct Voltage psu_voltage = {"PSU_Voltage", "PSU_Voltage", PA0 , 108, 0};
struct Voltage switch_voltage = {"SW_Voltage", "SW_Voltage", PA1 , 109, 0};

uint32_t now; 

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers


boolean led_on(Button &button ){
  //  Turn on the LED 
      ws2812_set(button.led1, button.r, button.g, button.b);
      ws2812_set(button.led2, button.r, button.g, button.b);
      ws2812_set(button.led3, button.r, button.g, button.b);
      ws2812_set(button.led4, button.r, button.g, button.b);
      ws2812_refresh();
}

boolean led_off(Button &button ){
  //  Turn off the LED 
      ws2812_set(button.led1, 0, 0, 0);
      ws2812_set(button.led2, 0, 0, 0);
      ws2812_set(button.led3, 0, 0, 0);
      ws2812_set(button.led4, 0, 0, 0);
      ws2812_refresh();
}

boolean button_check(Button &button){
    if ((now - button.wait) > debounceDelay) {
        if (digitalRead(button.pin) == LOW and button.state == false) {

          button.state = true;
          MIDI.sendControlChange(12, button.midi, 127);

          //led_on(button);

          ws2812_set(button.led1, button.r, button.g, button.b);
          ws2812_set(button.led2, button.r, button.g, button.b);
          ws2812_set(button.led3, button.r, button.g, button.b);
          ws2812_set(button.led4, button.r, button.g, button.b);
          ws2812_refresh();

          digitalWrite(LED_PIN, LOW);
          button.wait = now;
        }
        if (digitalRead(button.pin) == HIGH and button.state == true) {
          
          button.state = false;
          MIDI.sendControlChange(12, button.midi, 0);

          //led_off(button);

          ws2812_set(button.led1, 0, 0, 0);
          ws2812_set(button.led2, 0, 0, 0);
          ws2812_set(button.led3, 0, 0, 0);
          ws2812_set(button.led4, 0, 0, 0);
          ws2812_refresh();

          digitalWrite(LED_PIN, HIGH);
          button.wait = now;
        }
    }

    return button.state;

}

int voltage_check(Voltage &voltage){

    int val = analogRead(voltage.pin);
    val = map(val, 0, 4096, 0, 127);
    MIDI.sendControlChange(12, voltage.midi, val);

    return val;
}

void loop() {

  now = millis();

  button_check(red_button);
  button_check(green_button);
  button_check(yellow_button);
  button_check(blue_button);
  button_check(black_button);
  button_check(white_button);

  voltage_check(psu_voltage);
  voltage_check(switch_voltage);

  if(DEBUG_FLASH == true){
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    //MIDI.sendControlChange(13, 110, 68);
  }
}