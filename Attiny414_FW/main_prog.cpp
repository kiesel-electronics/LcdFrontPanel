/*
 * main_prog.cpp
 *
 * Created: 08.07.2023 20:58:56
 *  Author: Friedrich
 */ 
#include <Arduino.h>
#include <Wire.h>
#include <tinyNeoPixel_Static.h>
#include <BasicEncoder.h>
#include <debounce.h>

// define pins
#define PIN_ENC_SW      PIN_PA1
#define PIN_ENC_A       PIN_PA2
#define PIN_ENC_B       PIN_PA3
#define PIN_ENC_ADDR    PIN_PA4
#define PIN_CHARGE_PUMP PIN_PA5
#define PIN_WS_LED      PIN_PA6
#define PIN_ENC_INT     PIN_PA7
#define PIN_SCL         PIN_PB0
#define PIN_SDA         PIN_PB1
#define PIN_AUX_OUT_1   PIN_PB2
#define PIN_AUX_OUT_2   PIN_PB3

#define INT_PIN_HIGH_TIME_MS  5

#define STEPS_PER_CLICK 4

unsigned long TaskTimer;
unsigned long LastIntTimer;
bool intPinStatus;

void triggerIntPin(void) {
	LastIntTimer = millis();
	intPinStatus = true;
	digitalWrite(PIN_ENC_INT, 1);
}

// RGB LED
char red = 0;
char green = 0;
char blue = 0;
byte pixels[3];
tinyNeoPixel led = tinyNeoPixel(1, PIN_WS_LED, NEO_RGB, pixels);


// Rotary Encoder
BasicEncoder encoder(PIN_ENC_A, PIN_ENC_B, LOW, 1);
uint8_t button_cnt = 0;
static void buttonHandler(uint8_t btnId, uint8_t btnState) {
	if (btnState == BTN_PRESSED) {
		button_cnt++;
	} else {
		button_cnt++;
	}
	triggerIntPin();
}
static Button myButton(0, buttonHandler);


// I2C functions
void receiveDataWire(int16_t numBytes) {
	char aux_pins;
	if (numBytes >= 3) {
		red = Wire.read();
		green = Wire.read();
		blue = Wire.read();
		led.setPixelColor(0, led.Color(red, green, blue, 0));
		led.show();
		if (numBytes >= 4) {
			aux_pins = Wire.read();
			digitalWrite(PIN_AUX_OUT_1, aux_pins&0x01);
			digitalWrite(PIN_AUX_OUT_2, (aux_pins>>1)&0x01);
		}
	}
}

void transmitDataWire(void) {
	uint32_t enc_cnt = encoder.get_count()/STEPS_PER_CLICK; 
	Wire.write((uint8_t) enc_cnt);
	Wire.write((uint8_t)(enc_cnt >> 8));
	Wire.write((uint8_t) button_cnt);
}

void setup (void) {
	// Init inputs
	pinMode(PIN_ENC_SW, INPUT);
	pinMode(PIN_ENC_A, INPUT);
	pinMode(PIN_ENC_B, INPUT);
	pinMode(PIN_ENC_ADDR, INPUT);

	// Init outputs
	pinMode(PIN_CHARGE_PUMP, OUTPUT);
	pinMode(PIN_WS_LED, OUTPUT);
	pinMode(PIN_ENC_INT, OUTPUT);
	pinMode(PIN_AUX_OUT_1, OUTPUT);
	pinMode(PIN_AUX_OUT_2, OUTPUT);


	// Setup PWM for charge pump for the LCD contrast adjustment
	// The charge pump is necessary to make the contrast adjustment work with 3.3V.
	analogWrite(PIN_CHARGE_PUMP, 128);

	// Setup RGB LED
	led.setPixelColor(0, led.Color(red, green, blue)); 
	led.show();

	// Setup I2c
    Wire.onReceive(receiveDataWire);  // give the Wire library the name of the function
    //                                   that will be called on a master write event
    Wire.onRequest(transmitDataWire); // same as above, but master read event
	if (digitalRead(PIN_ENC_ADDR)) {
		Wire.begin(0x34);                 // join i2c bus with address 0x54
	} else {
		Wire.begin(0x54);                 // join i2c bus with address 0x34
	}

	TaskTimer = millis();
	LastIntTimer = TaskTimer;
	intPinStatus = false;
	digitalWrite(PIN_ENC_INT, 0);
}


void loop (void) {
	encoder.service();
	
	if (encoder.get_change() != 0) {
         triggerIntPin();
	}
	
	myButton.update(digitalRead(PIN_ENC_SW));
	
	TaskTimer = millis();
	if (intPinStatus) {
		if (TaskTimer - LastIntTimer >= INT_PIN_HIGH_TIME_MS) {
			digitalWrite(PIN_ENC_INT, 0);
			intPinStatus = false;
		}
	}
}