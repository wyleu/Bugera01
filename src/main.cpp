#include <Arduino.h>
#include <USBComposite.h>
#include "ws2812.h"


// https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/master/STM32F1/libraries/USBComposite/USBMIDI.h#L159


USBMIDI usb_midi;
USBCompositeSerial usb_serial;

//  MIDI CC
// 102 – 119	Undefined

#define NUM_LEDS 20
#define DATA_PIN PA3
#define LED_PIN PC13 //13

#define DEBUG_FLASH false
#define JABBER false                  // MIDI Jabber for fault finding
#define SERIAL_OUT false              // Enable USB Serial out 
#define LED_TEST true                 // Cycle the LED's on startupi

#define VOLTS_PSU_MEASURE  false      // Measure PSU Voltage
#define SWITCH_PSU_MEASURE false      // Measure PSU Voltage
#define CV1_VOLTS_MEASURE  true       // Measure CV1 Voltage
#define CV2_VOLTS_MEASURE  true       // Measure CV2 Voltage

#define CONTROL_VOLTAGE_1   PA0
#define CONTROL_VOLTAGE_2   PA1
#define VOLTAGE_PSU         PA2 
#define VOLTAGE_SWITCH      PA3

#define LED_ON_PAUSE 200             //  Length of on LED Display on LEADING edge trigger.
#define DEFAULT_MIDI_CHANNEL 12      //  Default MIDI channel for startup  (Channel 13) 

#define MODE_ON_PRESS_OFF  0         //  Initially OFF, LED OFF. Sends MIDI ON, LED ON on press, Sends MIDI OFF, LED OFF on release
#define MODE_OFF_PRESS_ON  1         //  Initially ON, LED ON. Sends MIDI OFF, LED OFF on press, Sends MIDI ON, LED ON on release 
#define MODE_ON_PRESS  2             //  Initially OFF, LED OFF. Sends MIDI ON, LED ON on press. No action on release
#define MODE_OFF_PRESS  3            //  Initially ON, LED ON, Send MIDI OFF, LED OFF on Press. No action on release
#define TOGGLE_ON  4                 //  Initially OFF, LED OFF, Send MIDI ON, LED ON on Press Toggle Send MIDI ON, LED ON and Send MIDI OFF, LED OFF
#define TOGGLE_OFF  5                //  Initially ON, LED ON, Send MIDI ON, LED ON on Press Toggle Send MIDI OFF, LED OFF and Send MIDI ON, LED ON

int no_of_leds = 60;


#define FOOT_RED PA9
#define FOOT_GREEN PA8
#define FOOT_YELLOW PB15
#define FOOT_BLUE PB14
#define FOOT_PURPLE PB13
#define FOOT_WHITE PB12

#define PWM_LED_RED PB8              // 680R
#define PWM_LED_GREEN PB7            // 390R
#define PWM_LED_BLUE PB6             // 270R


#define ENCODER_A PB3
#define ENCODER_B PB4
#define ENCODER_SW PB5


void setup() {

  USBComposite.clear();
  USBComposite.setProductId(0x0068);
  USBComposite.setVendorId(0x1eaa);
  USBComposite.setManufacturerString("ZynthianIO");
  USBComposite.setProductString("Wyleu Bugera Pedal");
  usb_midi.registerComponent();
  usb_serial.registerComponent();
  
  USBComposite.begin();


  pinMode(LED_PIN, OUTPUT);

  pinMode(VOLTAGE_PSU, INPUT_ANALOG);         // PSU Voltage
  pinMode(VOLTAGE_SWITCH, INPUT_ANALOG);      // PSU Voltage

  pinMode(ENCODER_A, INPUT_PULLUP);           // Encoder A
  pinMode(ENCODER_B, INPUT_PULLUP);           // Encoder B
  pinMode(ENCODER_SW, INPUT_PULLDOWN);        // Encoder Sw
  pinMode(PWM_LED_BLUE, OUTPUT);              // LED Blue
  pinMode(PWM_LED_GREEN, OUTPUT);             // LED Green
  pinMode(PWM_LED_RED, OUTPUT);               // LED Red


  pinMode(FOOT_GREEN, INPUT_PULLUP);      // Bugera Switch 2 Green
  pinMode(FOOT_RED, INPUT_PULLUP);        // Bugera Switch 1 Red  
  pinMode(FOOT_WHITE, INPUT_PULLUP);      // Bugera Switch 6 White/Grey
  pinMode(FOOT_PURPLE, INPUT_PULLUP);     // Bugera Switch 5 Purple/Black
  pinMode(FOOT_BLUE, INPUT_PULLUP);       // Bugera Switch 4 Blue
  pinMode(FOOT_YELLOW, INPUT_PULLUP);     // Bugera Switch 3 Yellow

  ws2812_init(1, no_of_leds);
}

 struct Button {
    char colour[20];          // Wire Colour Name
    char alt_colour[20];      // Button Colour Name
    int pin;                  // pin connected to Button
    uint32_t mode;            // Button Mode
    int channel;              // Midi channel  for button
    int midi;                 // Midi value sent on press
    bool state;               // Button current state
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

struct Button prev_button;

struct Voltage {
    char colour[20];
    char alt_colour[20];
    int pin;                  // pin connected to Voltage   
    int channel;              // MIDI Channel                   
    int midi;                 // Midi value sent on change
    int value;                // Current Value
    int last_value;           // The previous value to not transmit unchanged values.
    int threshold;            // Range change required to produce a MIDI CC update
};

struct Voltage cv1_voltage = {"CV1_Voltage", "PSU_Voltage", PA0 ,DEFAULT_MIDI_CHANNEL, 108, 0, 0, 50};
struct Voltage cv2_voltage = {"CV2_Voltage", "SW_Voltage", PA1 ,DEFAULT_MIDI_CHANNEL, 109, 0, 0, 50};
struct Voltage psu_voltage = {"PSU_Voltage", "PSU_Voltage", PA2 ,DEFAULT_MIDI_CHANNEL, 110, 0, 0, 50};
struct Voltage switch_voltage = {"SW_Voltage", "SW_Voltage", PA3 ,DEFAULT_MIDI_CHANNEL, 111, 0, 0, 50};

uint32_t now; 

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 10; // the debounce time; increase if the output flickers

int led_test = LED_TEST;



bool led_left_on(Button &button ){
  //  Turn on the Left LED 
      ws2812_set(button.led1, button.r, button.g, button.b);
      ws2812_set(button.led2, button.r, button.g, button.b);
      ws2812_refresh();
      return true;
}

bool led_right_on(Button &button ){
  //  Turn on the Right LED 
      ws2812_set(button.led3, button.r, button.g, button.b);
      ws2812_set(button.led4, button.r, button.g, button.b);
      ws2812_refresh();
      return true;
}

bool led_left_off(Button &button ){
  //  Turn off the Left LED 
      ws2812_set(button.led1, 0, 0, 0);
      ws2812_set(button.led2, 0, 0, 0);
      ws2812_refresh();
      return true;
}

bool led_right_off(Button &button ){
  //  Turn off the Right LED 
      ws2812_set(button.led3, 0, 0, 0);
      ws2812_set(button.led4, 0, 0, 0);
      ws2812_refresh();
      return false;
}

bool leds_on(Button &button ){
    //  Turn on the LED 
    led_left_on(button);
    led_right_on(button);
    analogWrite(PWM_LED_RED, 255 - button.r);        
    analogWrite(PWM_LED_GREEN, 255 - button.g);  
    analogWrite(PWM_LED_BLUE, 255 - button.b);  

    return true;
      
}

bool leds_off(Button &button ){
    //  Turn off the LEDs
    led_left_off(button);
    led_right_off(button);

    analogWrite(PWM_LED_RED, 255);        
    analogWrite(PWM_LED_GREEN, 255);  
    analogWrite(PWM_LED_BLUE, 255); 

    return false;
}

bool button_on(Button &button){
          usb_midi.sendControlChange(button.channel, button.midi, 127);
          led_right_off(prev_button);

          leds_on(button);  
      return true;
}

bool button_off(Button &button){
          usb_midi.sendControlChange(button.channel, button.midi, 0);
          leds_off(button);
          prev_button = button;
          led_right_on(button);
      return false;
}

bool button_check(Button &button){
    if ((now - button.wait) > debounceDelay) {
        if (digitalRead(button.pin) == LOW and button.state == false) {
          // Button Pressed
          button.state = true;
          button_on(button);

          usb_serial.print("Pressed ");
          usb_serial.println(button.alt_colour);            
        }
        if (digitalRead(button.pin) == HIGH and button.state == true) {
          // Button Released
          button.state = false;
          button_off(button);

          usb_serial.print("Released ");
          usb_serial.println(button.alt_colour);       
        }
        button.wait = now;
    }
  return button.state;
}

bool voltage_change(Voltage &voltage){
      int val = map(voltage.value, 0, 4096, 0, 127);
      usb_midi.sendControlChange(voltage.channel, voltage.midi, val);
  return true;
}

int voltage_check(Voltage &voltage){

    voltage.value = analogRead(voltage.pin);
    if ((voltage.value > voltage.last_value + voltage.threshold) or (voltage.value < voltage.last_value - voltage.threshold)){
        voltage_change(voltage);
        voltage.last_value = voltage.value;
    }

    return voltage.value;
}

void loop() {

  now = millis();
  // usb_serial.println("Serial LOUD Noise !");
  // usb_serial.println("Hello World");

  button_check(red_button);
  button_check(green_button);
  button_check(yellow_button);
  button_check(blue_button);
  button_check(black_button);
  button_check(white_button);


  if (VOLTS_PSU_MEASURE == true){
      voltage_check(psu_voltage);
  }
    if (SWITCH_PSU_MEASURE == true){
      voltage_check(switch_voltage);
  }
    if (CV1_VOLTS_MEASURE == true){
      voltage_check(cv1_voltage);
  }
    if (CV2_VOLTS_MEASURE == true){
      voltage_check(cv2_voltage);
  }

  if(DEBUG_FLASH == true){
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

  if(SERIAL_OUT == true){
    usb_serial.println("Serial Noise !");
  }

  if(JABBER == true){
    usb_midi.sendControlChange(13, 110, 68);
  }
  if(led_test == true){
    int i;
    for (i = 0 ; i < 53 ; i++) {
    
      ws2812_set(i, 255, 255, 255);
      ws2812_refresh();
      delay(30);
    }  
    delay(300);
    led_test = false;
    for (i = 0 ; i < 53 ; i++) {
      ws2812_set(i, 0, 0, 0);
      ws2812_refresh();
    }
  }
}
