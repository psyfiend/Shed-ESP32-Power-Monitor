#include "utils.h"
#include "config.h" // Needed for the timer duration constants

// --- Buffer for formatted string ---
static char formatBuffer[20]; // <---- ADDED

// --- Helper function to format large numbers with units ---
const char* format_large_number(float value) { // <---- ADDED
  if (abs(value) >= 1000) {
    sprintf(formatBuffer, "%.2f A", value / 1000.0);
  } else {
    sprintf(formatBuffer, "%.0f mA", value);
  }
  return formatBuffer;
}

String formatDuration(unsigned long milliseconds) {
    unsigned long totalSeconds = milliseconds / 1000;
    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours = (totalSeconds / 3600);
    String formattedString = "";
    if (hours < 10) formattedString += "0";
    formattedString += String(hours) + ":";
    if (minutes < 10) formattedString += "0";
    formattedString += String(minutes) + ":";
    if (seconds < 10) formattedString += "0";
    formattedString += String(seconds);
    return formattedString;
}

unsigned long get_current_timer_duration(bool isManualOverride) {
  // This function is now the single source of truth for timer logic.
  return isManualOverride ? MANUAL_TIMER_DURATION : MOTION_TIMER_DURATION;
}