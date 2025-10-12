#include "config.h"
#include <Arduino.h>

// --- Rotary Encoder State Variables ---
// Using the stable counter-based ISR logic
volatile int encoderCounter = 0;
volatile int lastKnownPosition = HIGH;

// --- Button State Variables ---
volatile bool buttonPressedFlag = false; // Now a volatile flag set by ISR
volatile unsigned long lastDebounceTime = 0; // For ISR debouncing
const int DEBOUNCE_DELAY = 250; // Debounce delay in ms

// --- Interrupt Service Routine (ISR) ---
// This is the stable ISR you provided. It increments/decrements a persistent counter.
void IRAM_ATTR handleEncoderRotation() {
  int newClockState = digitalRead(ENCODER_CLK_PIN);
  if (newClockState != lastKnownPosition) {
    // A transition has occurred
    if (newClockState == LOW) {
      // We act on the falling edge
      if (digitalRead(ENCODER_DT_PIN) == LOW) {
        encoderCounter = encoderCounter - 1; // Direction 1
      } else {
        encoderCounter = encoderCounter + 1; // Direction 2
      }
    }
    lastKnownPosition = newClockState;
  }
}

// --- Button Press ISR ---
void IRAM_ATTR handleButtonPress() { // New ISR for the button
  // Simple debounce logic inside the ISR
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    buttonPressedFlag = true;
    lastDebounceTime = millis();
  }
}

void setup_encoder() {
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  
  // Initialize last known position for the ISR
  lastKnownPosition = digitalRead(ENCODER_CLK_PIN);

  // Attach the interrupt to trigger on any state change
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), handleEncoderRotation, CHANGE);

  // Attach a new interrupt for the button press on the FALLING edge (HIGH to LOW)
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW_PIN), handleButtonPress, FALLING);
}

int get_encoder_value() {
  int value;
  noInterrupts(); // Disable interrupts to safely read multi-byte variable
  value = encoderCounter;
  interrupts(); // Re-enable interrupts
  return value;
}

bool button_was_clicked() {
    bool result = false;
    noInterrupts(); // Protect access to volatile flag
    if (buttonPressedFlag) {
        result = true;
        buttonPressedFlag = false; // Reset the flag
    }
    interrupts();
    return result;
}

