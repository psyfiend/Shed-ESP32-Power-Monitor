#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

const int LIGHTS_MENU_ITEM_COUNT = 4;

// --- UI State Enums ---
// By placing these here, both main.cpp and display_manager.cpp can see them.
enum DisplayMode {
  POWER_MODE_ALL,
  POWER_MODE_CH1,
  POWER_MODE_CH2,
  POWER_MODE_CH3,
  LIGHTS_MODE
};

enum LightsSubMode { 
  LIVE_STATUS, 
  LIGHTS_MENU, 
  EDIT_MOTION_TIMER, 
  EDIT_MANUAL_TIMER 
};
enum PowerSubMode { 
  LIVE_POWER, 
  POWER_SUBSCREEN 
};


// --- Data Structure for Display Updates ---
// This struct bundles all the data needed to draw any screen.
struct DisplayData {
  // Power Data
  float busVoltage[3];
  float current[3];
  float power[3];
  
  // Light Status Data
  bool lightIsOn;
  bool lightManualOverride;

  // NEW: Add timer data for the progress bar
  unsigned long timerRemainingSeconds;
  unsigned long motionTimerDuration;
  unsigned long manualTimerDuration;
  unsigned long lightOnTime;

  // Menu/UI State Data
  int lightsMenuSelection;
  unsigned long tempMotionTimerDuration;
  unsigned long tempManualTimerDuration;
};


// --- Public Functions ---

// Call this in setup() to initialize the screen
void setup_display();

// This is the single, high-level function that main.cpp will call to handle all drawing.
void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data);


#endif // DISPLAY_MANAGER_H

