/**
 * This examples shows how to use the MultiPurposeButton class to detect 
 * different kinds of push button events.
 * 
 * @boards  AVR, AVR USB, Nano Every, Nano 33 IoT, Nano 33 BLE, Pi Pico, Due, Teensy 3.x, ESP8266, ESP32
 * 
 * Connections
 * -----------
 * 
 * - 2: Momentary push button (other pin to ground)
 * 
 * The internal pull-up resistor will be enabled.
 * 
 * Behavior
 * --------
 * 
 * - Short presses, long presses, multiple presses, etc. are printed to the 
 *   Serial monitor.
 * 
 * Written by PieterP, 2022-05-07  
 * https://github.com/tttapa/Arduino-Helpers
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <Arduino_Helpers.h> // https://github.com/tttapa/Arduino-Helpers

#include <AH/Hardware/MultiPurposeButton.hpp>
#include <Control_Surface.h> // Include the Control Surface library

// Select the serial port to use.
auto &serial = Serial;
// Instantiate a Serial MIDI interface at the default MIDI baud rate.
SerialMIDI_Interface<decltype(serial)> midi {serial, 115200};

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int current_state = 0;

// Instantiate a NoteButton object
NoteButton b1 {
  8,                       // Push button on pin 5
  {MIDI_Notes::C(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
NoteButton b2 {
  7,                       // Push button on pin 5
  {MIDI_Notes::Db(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
NoteButton b3 {
  4,                       // Push button on pin 5
  {MIDI_Notes::D(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
NoteButton b4 {
  2,                       // Push button on pin 5
  {MIDI_Notes::Eb(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};

// Instantiate the LED that will light up when middle C is playing
NoteLEDPWM l1 {
  9,                 // Pin of built-in LED
  {MIDI_Notes::C(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
// Instantiate the LED that will light up when middle C is playing
NoteLEDPWM l2 {
  6,                 // Pin of built-in LED
  {MIDI_Notes::Db(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
// Instantiate the LED that will light up when middle C is playing
NoteLEDPWM l3 {
  5,                 // Pin of built-in LED
  {MIDI_Notes::D(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};
// Instantiate the LED that will light up when middle C is playing
NoteLEDPWM l4 {
  3,                 // Pin of built-in LED
  {MIDI_Notes::Eb(0), CHANNEL_15}, // Note C4 on MIDI channel 1
};

// expression pedal
CCPotentiometer expression {
  A1,                                   // Analog pin connected to potentiometer
  {MIDI_CC::Channel_Volume, CHANNEL_1}, // Channel volume of channel 1
};

void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);  // initialize the lcd

  Control_Surface.begin(); // Initialize Control Surface


  lcd.begin(16, 2);  // initialize the lcd

  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("Hello LCD");
}

static int old_exp = 0;

void loop() {
  Control_Surface.loop(); // Update the Control Surface

  int exp_pedal = expression.getValue();
  
  if (exp_pedal != old_exp)
  {
	  lcd.home();
	  lcd.clear();
	  lcd.print(exp_pedal);
	  old_exp = exp_pedal;
  }
}
