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
#include <uClock.h>

#include <Arduino_Helpers.h> // https://github.com/tttapa/Arduino-Helpers

#include <AH/Hardware/MultiPurposeButton.hpp>
#include <Control_Surface.h> // Include the Control Surface library


#define DEFAULT_TEMPO 60

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line displayz
static int g_tick = 0;

BEGIN_CS_NAMESPACE

#define STATE_OFF   0
#define STATE_ON    1
#define STATE_BLINK 2
#define STATE_PULSE 3

// My Note Led class
/// Generic base class for classes that listen for MIDI Note, Control Change and
/// Key Pressure events on a single address and turn on an LED with a
/// brightness/duty cycle proportional to the MIDI value.
///
/// @tparam Type
///         The type of MIDI messages to listen for:
///         - MIDIMessageType::NOTE_ON
///         - MIDIMessageType::CONTROL_CHANGE
///         - MIDIMessageType::KEY_PRESSURE
template <MIDIMessageType Type>
class MyNoteCCKPLEDPWM
    : public MatchingMIDIInputElement<Type, TwoByteMIDIMatcher> {
  public:
    using Matcher = TwoByteMIDIMatcher;
    using Parent = MatchingMIDIInputElement<Type, Matcher>;

    /// @param  ledPin
    ///         The PWM pin with the LED connected.
    /// @param  address
    ///         The address to listen to.
    MyNoteCCKPLEDPWM(pin_t ledPin, MIDIAddress address)
        : Parent(address), ledPin(ledPin) {}

  private:
    void handleUpdate(typename Matcher::Result match) override {
        auto value = AH::increaseBitDepth<8, 7, uint8_t, uint8_t>(match.value);
		switch (value/2) 
		{
			case 3:
				state = STATE_OFF;
				break;
			case 1:
				g_tick = 0; // Start is always on the beat (is it ?)
				state = STATE_BLINK;
				break;
			case 4:
				state = STATE_PULSE;
				break;
			case 5:
				state = STATE_ON;
				break;
			default:
				break;
				// ignore
		}
    }

  public:
    /// Set the pinmode of the LED to OUTPUT.
    void begin() override {
        AH::ExtIO::pinMode(ledPin, OUTPUT);
        AH::ExtIO::digitalWrite(ledPin, LOW);
    }

	void clockUpdate(int tick) {
		// tick == 0 means we are on the beat.
		if (state)
		{
			if (state == STATE_ON)
			{
				AH::ExtIO::digitalWrite(ledPin, HIGH);
			} else if (state == STATE_BLINK) {
				AH::ExtIO::digitalWrite(ledPin, (tick<12)?HIGH:LOW);
			} else {
				if (tick == 0)
					duty = 100;

				duty += incr;
				if (duty >= 100)
				{
					incr = -incr;
					duty = 100;
				}
				if (duty <= 4) {
					duty = 4;
					incr = -incr;
				}
				AH::ExtIO::analogWrite(ledPin, duty);
			}
		}
		else
		{
			AH::ExtIO::digitalWrite(ledPin, LOW);
		}
	}

    /// Turn off the LED.
    void reset() override { AH::ExtIO::digitalWrite(ledPin, LOW); }

  private:
    pin_t ledPin;
	int duty = 0;
	int incr = 96/12;
	int state = 0;
};

/// Class that listens for MIDI Note events on a single address and turns
/// on an LED with a brightness/duty cycle proportional to the velocity.
/// @ingroup    midi-input-elements-leds
using MyNoteLEDPWM = MyNoteCCKPLEDPWM<MIDIMessageType::NOTE_ON>;

END_CS_NAMESPACE


#define BUTTON_CHANNEL CHANNEL_15
#define BUTTON_NOTE_0 MIDI_Notes::C(0)
#define BUTTON_NOTE_1 MIDI_Notes::Db(0)
#define BUTTON_NOTE_2 MIDI_Notes::D(0)
#define BUTTON_NOTE_3 MIDI_Notes::Eb(0)

#define BUTTON_NOTE(i) BUTTON_NOTE_##i

NoteButton buttons[] {
	{8, {BUTTON_NOTE(0), BUTTON_CHANNEL}},
	{7, {BUTTON_NOTE(1), BUTTON_CHANNEL}},
	{4, {BUTTON_NOTE(2), BUTTON_CHANNEL}},
	{2, {BUTTON_NOTE(3), BUTTON_CHANNEL}}
};

MyNoteLEDPWM leds[] { 
	{11, {BUTTON_NOTE(0), BUTTON_CHANNEL}},
	{6 , {BUTTON_NOTE(1), BUTTON_CHANNEL}},
	{5 , {BUTTON_NOTE(2), BUTTON_CHANNEL}},
	{3 , {BUTTON_NOTE(3), BUTTON_CHANNEL}}
};

void midiClock(uint32_t tick)
{
	g_tick ++;
	g_tick %= 24;
	for( int i=0; i < 4; i++) {
		leds[i].clockUpdate(g_tick);
	}
}

bool realTimeMessageCallback(RealTimeMessage rt) {
	uClock.setMode(1);
	uClock.clockMe();
	return true; // Return true to indicate that handling is done,
				 // and Control_Surface shouldn't handle it anymore.
				 // If you want Control_Surface to handle it as well,
				 // return false.
}

// expression pedal
CCPotentiometer expression {
	A1, // Analog pin connected to potentiometer
	{MIDI_CC::Channel_Volume, CHANNEL_1}, // Channel volume of channel 1
};

USBSerialMIDI_Interface midi {115200};

void setup() {
  Control_Surface.begin(); // Initialize Control Surface

  // Capture MIDI realtime events
  Control_Surface.setMIDIInputCallbacks(nullptr, nullptr, nullptr,
                                        realTimeMessageCallback); 

  // Start internal clock
  uClock.init();
  uClock.setClock96PPQNOutput(midiClock);
  uClock.setTempo(DEFAULT_TEMPO);
  uClock.start();

  // Start the screen
  lcd.begin(16, 2);  // initialize the lcd
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();

  lcd.print("Tempo  Expr.");
}

static int old_exp = 0;
static int old_tempo = 60;
static int loops = 0;
static bool refresh = true;

void loop() {
  Control_Surface.loop(); // Update the Control Surface

  int exp_pedal = expression.getValue();
  int tempo = uClock.getTempo();

  if ((tempo != old_tempo) || (exp_pedal != old_exp))
	  refresh = true;
  
	if (++loops % 1000 == 0 && refresh)
	{
		lcd.setCursor(0,1);
		lcd.print("   ");
		lcd.setCursor(0,1);
		lcd.print(tempo);
		lcd.setCursor(9,1);
		lcd.print("   ");
		lcd.setCursor(9,1);
		lcd.print(exp_pedal);
		old_tempo = tempo;
		old_exp = exp_pedal;
		refresh = false;
	}
}
