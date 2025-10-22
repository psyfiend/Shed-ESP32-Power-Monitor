#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "display_manager.h"
#include "config.h"
#include "utils.h" // For format_large_number

// --- Display Object ---
TFT_eSPI tft = TFT_eSPI();

// --- UI Color Palette ---
#define BG_COLOR 0x2124 // Dark Grey
#define CARD_COLOR 0x31A6 // Lighter Grey
#define SOLAR_COLOR TFT_YELLOW
#define BATTERY_COLOR TFT_GREEN
#define LOAD_COLOR TFT_CYAN
#define TEXT_COLOR TFT_WHITE
#define SHADOW_COLOR TFT_BLACK
#define SUBTLE_TEXT_COLOR 0x632C // Medium Grey

// --- Forward Declarations for private drawing functions ---
void draw_power_overview_screen(const DisplayData& data);
void draw_power_channel_screen(int channel, const DisplayData& data); 
void draw_lights_live_status_screen(const DisplayData& data);
void draw_lights_menu_screen(const DisplayData& data);
void draw_lights_edit_timer_screen(LightsSubMode lightsSub, const DisplayData& data);

// --- Lights menu icons
void draw_motion_icon(int x, int y);
void draw_manual_icon(int x, int y);
void draw_up_down_chevrons(int x, int y);
void draw_light_bulb_icon(int x, int y, bool on);
void draw_occupancy_icon(int x, int y, bool detected);

// --- Power main icons
void draw_sun_icon(int x, int y);
void draw_battery_icon(int x, int y, float voltage);
void draw_load_icon(int x, int y);


// --- Public Functions ---

void setup_display() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 120);
  tft.println("System Boot...");

  analogWrite(SPI_BLK_PIN, 255);
}

void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data) {
  // NOTE: This will flicker. We are focusing on layout first.
  tft.fillScreen(BG_COLOR); // Clear screen with background color

  switch (mode) {
    case POWER_MODE_ALL:
      draw_power_overview_screen(data);
      break;
    case POWER_MODE_CH1:
      draw_power_channel_screen(1, data);
      break;
    case POWER_MODE_CH2:
      draw_power_channel_screen(2, data);
      break;
    case POWER_MODE_CH3:
      draw_power_channel_screen(3, data);
      break;
    // Logic to call the light control screens --->
    case LIGHTS_MODE:
      switch (lightsSub) {
        case LIVE_STATUS:
          draw_lights_live_status_screen(data);
          break;
        case LIGHTS_MENU:
          draw_lights_menu_screen(data);
          break;
        case EDIT_MOTION_TIMER:
        case EDIT_MANUAL_TIMER:
          draw_lights_edit_timer_screen(lightsSub, data);
          break;
      }
      break;
    default:
      // Placeholder for other screens
      tft.setCursor(10, 20);
      tft.setTextColor(TFT_WHITE, BG_COLOR);
      tft.setTextSize(2);
      tft.println("Screen not implemented");
      break;
  }
}

// --- Screen Drawing Functions ---

void draw_power_overview_screen(const DisplayData& data) {
  char val_buf[20]; // Buffer for formatting text values

  // --- 1. Solar Card ---
  tft.fillRoundRect(5, 5, 230, 85, 10, CARD_COLOR);
  draw_sun_icon(20, 25);
  // Power (Large Font)
  tft.setTextDatum(TR_DATUM); // Top-Right alignment
  tft.setTextColor(SOLAR_COLOR, CARD_COLOR);
  tft.setTextSize(4);
  sprintf(val_buf, "%.1fW", data.power[0] / 1000.0);
  tft.drawString(val_buf, 220, 20);
  // Voltage & Current (Small Font)
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.2fV", data.busVoltage[0]);
  tft.drawString(val_buf, 220, 55);
  sprintf(val_buf, "%s", format_large_number(data.current[0]));
  tft.drawString(val_buf, 150, 55);

  // --- 2. Battery Card ---
  tft.fillRoundRect(5, 95, 230, 85, 10, CARD_COLOR);
  draw_battery_icon(20, 115, data.busVoltage[1]);
  // Power (Large Font)
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(BATTERY_COLOR, CARD_COLOR);
  tft.setTextSize(4);
  sprintf(val_buf, "%+.1fW", data.power[1] / 1000.0); // Show + for charge, - for discharge
  tft.drawString(val_buf, 220, 110);
  // Voltage & Current (Small Font)
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.2fV", data.busVoltage[1]);
  tft.drawString(val_buf, 220, 145);
  sprintf(val_buf, "%s", format_large_number(data.current[1]));
  tft.drawString(val_buf, 150, 145);

  // --- 3. Load Card ---
  tft.fillRoundRect(5, 185, 230, 85, 10, CARD_COLOR);
  draw_load_icon(25, 205);
  // Power (Large Font)
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(LOAD_COLOR, CARD_COLOR);
  tft.setTextSize(4);
  sprintf(val_buf, "%.1fW", data.power[2] / 1000.0);
  tft.drawString(val_buf, 220, 200);
  // Voltage & Current (Small Font)
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.2fV", data.busVoltage[2]);
  tft.drawString(val_buf, 220, 235);
  sprintf(val_buf, "%s", format_large_number(data.current[2]));
  tft.drawString(val_buf, 150, 235);
}

void draw_power_channel_screen(int channel, const DisplayData& data) {
  char val_buf[20];
  const char* channel_name = "";
  uint16_t primary_color = TEXT_COLOR;
  
  // Set title and color based on channel
  switch (channel) {
    case 1:
      channel_name = "SOLAR"; 
      primary_color = SOLAR_COLOR;
      break;
    case 2:
      channel_name = "BATTERY";
      primary_color = BATTERY_COLOR;
      break;
    case 3:
      channel_name = "LOAD";
      primary_color = LOAD_COLOR;
      break;
  }
  
  // --- Draw Header ---
  tft.setTextDatum(TC_DATUM); // Top-Center alignment
  tft.setTextColor(primary_color, BG_COLOR);
  tft.setTextSize(3); 
  tft.drawString(channel_name, 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);

  // --- Draw Primary Data Points ---
  tft.setTextDatum(TL_DATUM); // Top-Left alignment
  tft.setTextColor(TEXT_COLOR, BG_COLOR);
  tft.setTextSize(2);
  
  // Voltage
  tft.drawString("Voltage:", 20, 60);
  tft.setTextDatum(TR_DATUM); // Top-Right
  tft.setTextSize(3);
  sprintf(val_buf, "%.2f V", data.busVoltage[channel - 1]);
  tft.drawString(val_buf, 220, 60);

  // Current
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.drawString("Current:", 20, 100);
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(3);
  sprintf(val_buf, "%s", format_large_number(data.current[channel - 1]));
  tft.drawString(val_buf, 220, 100);

  // Power
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.drawString("Power:", 20, 140);
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(3);
  if (channel == 2) { // Special case for battery to show +/-
    sprintf(val_buf, "%+.2f W", data.power[channel - 1] / 1000.0);
  } else {
    sprintf(val_buf, "%.2f W", data.power[channel - 1] / 1000.0);
  }
  tft.drawString(val_buf, 220, 140);

  // --- Placeholder for future graph ---
  tft.drawRoundRect(10, 180, 220, 90, 5, CARD_COLOR);
  tft.setTextDatum(MC_DATUM); 
  tft.setTextSize(1);
  tft.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  tft.drawString("[ Future Graph Area ]", 120, 225);
}

// <--- Lights live status screen --->
void draw_lights_live_status_screen(const DisplayData& data) {
  char buf[30];

  // --- Header ---
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(LOAD_COLOR, BG_COLOR);
  tft.setTextSize(3);
  tft.drawString("LIGHTS", 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);

  // --- Icons ---
  draw_occupancy_icon(40, 90, data.occupancyDetected);
  draw_light_bulb_icon(100, 90, data.lightIsOn);

  // --- Main Status Text & Icons ---
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(5);
  if (data.lightIsOn) {
    if (data.lightManualOverride) {
      tft.setTextColor(TEXT_COLOR, BG_COLOR);
      tft.drawString("MANUAL", 120, 60);
      draw_manual_icon(160, 90);
    } else {
      tft.setTextColor(TEXT_COLOR, BG_COLOR);
      tft.drawString("MOTION", 120, 60);
      draw_motion_icon(160, 90);
    }
  } else {
    tft.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
    tft.drawString("OFF", 120, 60);
  }

  // --- Timer Progress Bar & Info ---
  if (data.lightIsOn) {
    unsigned long totalDurationSec = data.lightManualOverride ? 
                                     (data.manualTimerDuration / 1000) : 
                                     (data.motionTimerDuration / 1000);
    
    if (totalDurationSec > 0) {
      int progressWidth = map(data.timerRemainingSeconds, 0, totalDurationSec, 0, 218);
      if (progressWidth < 1) progressWidth = 0;
      if (progressWidth > 218) progressWidth = 218;
      
      tft.drawRoundRect(10, 180, 220, 20, 5, CARD_COLOR);
      if (progressWidth > 0) {
        tft.fillRoundRect(11, 181, progressWidth, 18, 4, LOAD_COLOR);
      }
    }

    // Display remaining time
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TEXT_COLOR, BG_COLOR);
    tft.setTextSize(2);
    sprintf(buf, "%lu s remaining", data.timerRemainingSeconds);
    tft.drawString(buf, 120, 215);
  }

  // --- Footer instructions ---
  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  tft.drawString("Press button to open menu", 120, 275);
}

// <--- Lights menu screen --->
void draw_lights_menu_screen(const DisplayData& data) {
  const char* menuItems[] = {"Toggle Light", "Motion Timer", "Manual Timer", "Back"};

  // --- Header ---
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(LOAD_COLOR, BG_COLOR);
  tft.setTextSize(3);
  tft.drawString("LIGHTS MENU", 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);

  // --- Draw Menu Items ---
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  for (int i = 0; i < LIGHTS_MENU_ITEM_COUNT; i++) {
    int yPos = 60 + i * 40;
    if (i == data.lightsMenuSelection) {
      tft.fillRoundRect(20, yPos - 10, 200, 35, 5, LOAD_COLOR);
      tft.setTextColor(SHADOW_COLOR, LOAD_COLOR);
      tft.drawString(menuItems[i], 30, yPos);
    } else {
      tft.setTextColor(TEXT_COLOR, BG_COLOR);
      tft.drawString(menuItems[i], 30, yPos);
    }
  }
}

// <--- Screen for editing timers --->
void draw_lights_edit_timer_screen(LightsSubMode lightsSub, const DisplayData& data) {
  char buf[30];
  unsigned long durationToEdit;
  const char* title;

  if (lightsSub == EDIT_MOTION_TIMER) {
      durationToEdit = data.tempMotionTimerDuration;
      title = "Edit Motion Timer";
  } else {
      durationToEdit = data.tempManualTimerDuration;
      title = "Edit Manual Timer";
  }

  // --- Header ---
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(LOAD_COLOR, BG_COLOR);
  tft.setTextSize(3);
  tft.drawString(title, 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);

  // --- Display Time Value ---
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TEXT_COLOR, BG_COLOR);
  tft.setTextSize(5);
  unsigned long minutes = durationToEdit / 60000;
  unsigned long seconds = (durationToEdit % 60000) / 1000;
  sprintf(buf, "%02lu:%02lu", minutes, seconds);
  tft.drawString(buf, 120, 130);

  // --- Footer instructions ---
  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  tft.drawString("Turn to adjust, Press to save", 120, 275);
}


// --- Icon Drawing Functions ---
// These functions draw icons procedurally, saving memory vs. storing bitmaps.

void draw_sun_icon(int x, int y) {
  tft.fillCircle(x + 20, y + 20, 16, SOLAR_COLOR);
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = (x + 20) + 18 * cos(angle);
    int y1 = (y + 20) + 18 * sin(angle);
    int x2 = (x + 20) + 24 * cos(angle);
    int y2 = (y + 20) + 24 * sin(angle);
    tft.drawLine(x1, y1, x2, y2, SOLAR_COLOR);
  }
}

void draw_battery_icon(int x, int y, float voltage) {
  // Draw battery body
  tft.fillRoundRect(x, y + 8, 60, 35, 5, TEXT_COLOR);
  tft.fillRoundRect(x + 2, y + 10, 56, 31, 3, CARD_COLOR);

  // Draw battery terminals
  tft.fillRect(x + 10, y, 10, 8, TEXT_COLOR); // Left terminal
  tft.fillRect(x + 40, y, 10, 8, TEXT_COLOR); // Right terminal

  // Calculate charge level (simple mapping from voltage)
  int fill_width = map(voltage * 100, 1100, 1350, 0, 52); // 11.0V to 13.5V range
  if (fill_width < 0) fill_width = 0;
  if (fill_width > 52) fill_width = 52;
  
  tft.fillRoundRect(x + 4, y + 12, fill_width, 27, 2, BATTERY_COLOR);
}

void draw_load_icon(int x, int y) {
  // Draw outlet plate
  tft.fillRoundRect(x, y, 40, 40, 5, TEXT_COLOR);
  tft.fillRoundRect(x + 2, y + 2, 36, 36, 4, LOAD_COLOR);
  
  // Draw the "sockets"
  tft.fillRoundRect(x + 8, y + 8, 8, 14, 2, SHADOW_COLOR);  // Left slot
  tft.fillRoundRect(x + 24, y + 8, 8, 14, 2, SHADOW_COLOR); // Right slot
  tft.fillCircle(x + 20, y + 30, 4, SHADOW_COLOR);      // Ground
}

// <--- ADDED: New icon for motion trigger --->
void draw_motion_icon(int x, int y) {
  // Head
  tft.fillCircle(x + 20, y + 10, 8, TEXT_COLOR);
  // Body
  tft.fillRoundRect(x + 10, y + 20, 20, 20, 5, TEXT_COLOR);
}

// <--- ADDED: New icon for manual trigger --->
void draw_manual_icon(int x, int y) {
  // Palm
  tft.fillRoundRect(x, y + 10, 30, 30, 5, TEXT_COLOR);
  // Thumb
  tft.fillRoundRect(x + 25, y + 15, 15, 8, 3, TEXT_COLOR);
}

void draw_up_down_chevrons(int x, int y) {
  // Draw up chevron
  tft.drawLine(x - 15, y - 45, x, y - 55, SUBTLE_TEXT_COLOR);
  tft.drawLine(x + 15, y - 45, x, y - 55, SUBTLE_TEXT_COLOR);
  tft.drawLine(x - 15, y - 40, x, y - 50, SUBTLE_TEXT_COLOR);
  tft.drawLine(x + 15, y - 40, x, y - 50, SUBTLE_TEXT_COLOR);
  // Draw down chevron
  tft.drawLine(x - 15, y + 45, x, y + 55, SUBTLE_TEXT_COLOR);
  tft.drawLine(x + 15, y + 45, x, y + 55, SUBTLE_TEXT_COLOR);
  tft.drawLine(x - 15, y + 40, x, y + 50, SUBTLE_TEXT_COLOR);
  tft.drawLine(x + 15, y + 40, x, y + 50, SUBTLE_TEXT_COLOR);
}

void draw_light_bulb_icon(int x, int y, bool on) {
  uint16_t bulb_color = on ? SOLAR_COLOR : SUBTLE_TEXT_COLOR;
  // Bulb
  tft.fillCircle(x + 20, y + 15, 15, bulb_color);
  // Base
  tft.fillRect(x + 10, y + 30, 20, 8, bulb_color);

  if (on) {
    // Rays of light
    for (int i = 0; i < 8; i++) {
      float angle = i * PI / 4;
      if (angle > PI * 0.2 && angle < PI * 0.8) continue; // Don't draw rays pointing down
      int x1 = (x + 20) + 17 * cos(angle);
      int y1 = (y + 15) + 17 * sin(angle);
      int x2 = (x + 20) + 21 * cos(angle);
      int y2 = (y + 15) + 21 * sin(angle);
      tft.drawLine(x1, y1, x2, y2, bulb_color);
    }
  }
}

void draw_occupancy_icon(int x, int y, bool detected) {
  uint16_t icon_color = detected ? SOLAR_COLOR : SUBTLE_TEXT_COLOR;
  // Head
  tft.fillCircle(x + 20, y + 8, 8, icon_color);
  // Body
  tft.fillRoundRect(x + 10, y + 18, 20, 20, 5, icon_color);
  // Waves
  tft.drawCircleHelper(x + 35, y + 8, 10, 1, icon_color);
  tft.drawCircleHelper(x + 35, y + 8, 15, 1, icon_color);

  if (!detected) {
    // Draw strike-through line
    tft.drawLine(x + 5, y + 35, x + 35, y + 5, icon_color);
    tft.drawLine(x + 6, y + 35, x + 36, y + 5, icon_color);
  }
}