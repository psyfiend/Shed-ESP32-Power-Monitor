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
#define SENSOR_COLOR TFT_ORANGE // New color for sensor screen
#define TEXT_COLOR TFT_WHITE
#define SHADOW_COLOR TFT_BLACK
#define SUBTLE_TEXT_COLOR 0x632C // Medium Grey
#define SENSOR_THERM_COLOR 0xF800 // Red for thermometer
#define SENSOR_CLOUD_COLOR 0x3498 // Blue for cloud

// --- UI Sizing ---
#define CONTENT_Y_START 0
#define CONTENT_Y_END 239   // Bottom 40px are for the footer
#define FOOTER_Y_START 240
#define FOOTER_HEIGHT 40

// --- Forward Declarations for private drawing functions ---
// Main Screens
void draw_power_overview_screen(const DisplayData& data);
void draw_power_channel_screen(int channel, const DisplayData& data); 
void draw_sensors_screen(const DisplayData& data); // NEW

// Full-screen "Popup" Menus
void draw_lights_menu_screen(const DisplayData& data);
void draw_lights_edit_timer_screen(const DisplayData& data); // Simplified

// Global Footer
void draw_global_footer_bar(const DisplayData& data); // NEW

// New MDI-style Icons
void draw_footer_light_icon(TFT_eSprite* spr, int x, int y, bool on);
void draw_footer_occupancy_icon(TFT_eSprite* spr, int x, int y, bool detected);
void draw_temperature_icon(TFT_eSprite* spr, int x, int y);
void draw_humidity_icon(TFT_eSprite* spr, int x, int y);
void draw_lux_icon(TFT_eSprite* spr, int x, int y);

// Re-used Icons
void draw_sun_icon(TFT_eSprite* spr, int x, int y);
void draw_battery_icon(TFT_eSprite* spr, int x, int y, float voltage);
void draw_load_icon(TFT_eSprite* spr, int x, int y);


// --- Public Functions ---

void setup_display() {
  tft.init();
  tft.setRotation(0);
  
  // Draw the boot message directly to the screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 120);
  tft.println("System Boot...");

  analogWrite(SPI_BLK_PIN, 255);
}

// UPDATED: Signature changed, LightsSubMode is removed
void update_display(DisplayMode mode, PowerSubMode powerSub, const DisplayData& data) {
  
  // The logic is now split:
  // 1. Draw the main content (which takes 0-239px)
  // 2. Draw the global footer (which takes 240-280px)
  // OR draw a full-screen menu
  
  switch (mode) {
    case POWER_MODE_ALL:
      draw_power_overview_screen(data);
      draw_global_footer_bar(data); // Draw footer
      break;
    case POWER_MODE_CH1:
      draw_power_channel_screen(1, data);
      draw_global_footer_bar(data); // Draw footer
      break;
    case POWER_MODE_CH2:
      draw_power_channel_screen(2, data);
      draw_global_footer_bar(data); // Draw footer
      break;
    case POWER_MODE_CH3:
      draw_power_channel_screen(3, data);
      draw_global_footer_bar(data); // Draw footer
      break;
    case SENSORS_MODE: // NEW
      draw_sensors_screen(data);
      draw_global_footer_bar(data); // Draw footer
      break;
    
    // --- Menu screens are full-screen and do NOT draw the footer ---
    case LIGHTS_MENU:
      draw_lights_menu_screen(data);
      break;
    case EDIT_MOTION_TIMER:
    case EDIT_MANUAL_TIMER:
      draw_lights_edit_timer_screen(data); // Simplified
      break;
      
    default:
      // Placeholder for other screens
      tft.fillScreen(BG_COLOR); // Clear screen for default case
      tft.setCursor(10, 20);
      tft.setTextColor(TFT_WHITE, BG_COLOR);
      tft.setTextSize(2);
      tft.println("Screen not implemented");
      break;
  }
}

// --- Screen Drawing Functions ---

void draw_power_overview_screen(const DisplayData& data) {
  char val_buf[20]; 
  
  // --- FIX: Clear gaps instead of whole screen to prevent flicker ---
  tft.fillRect(0, 0, 240, 5, BG_COLOR); // Top gap
  tft.fillRect(0, 80, 240, 5, BG_COLOR); // Gap 1 (Card 1 Y=5, H=75 -> 80)
  tft.fillRect(0, 160, 240, 5, BG_COLOR); // Gap 2 (Card 2 Y=85, H=75 -> 160)
  tft.fillRect(0, 5, 5, 235, BG_COLOR); // Left gap
  tft.fillRect(235, 5, 5, 235, BG_COLOR); // Right gap
  
  // Create a reusable sprite for the cards. 
  // NEW SIZING: 75px tall to fit 3 + gaps in 240px
  TFT_eSprite card_spr = TFT_eSprite(&tft);
  int card_height = 75;
  int card_width = 230;
  int card_y_gap = 5;
  int card_y = 5; // Start Y-pos

  // --- 1. Solar Card ---
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR); 
  draw_sun_icon(&card_spr, 15, 15); // Adjusted Y
  // Power (Large Font)
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SOLAR_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.1fW", data.power[0] / 1000.0);
  card_spr.drawString(val_buf, 215, 10); // Adjusted Y
  // Voltage & Current (Small Font)
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.2fV", data.busVoltage[0]);
  card_spr.drawString(val_buf, 215, 45); // Adjusted Y
  sprintf(val_buf, "%s", format_large_number(data.current[0]));
  card_spr.drawString(val_buf, 145, 45); // Adjusted Y
  // Push the finished sprite to the screen
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite(); 

  // --- 2. Battery Card ---
  card_y += card_height + card_y_gap; // New Y-pos (85)
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR);
  draw_battery_icon(&card_spr, 15, 15, data.busVoltage[1]); // Adjusted Y
  
  // Voltage (Large Font)
  card_spr.setTextDatum(TR_DATUM);
  card_spr.setTextColor(BATTERY_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.2fV", data.busVoltage[1]);
  card_spr.drawString(val_buf, 215, 10); // Adjusted Y
  // Power & Current (Small Font)
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%+.1fW", data.power[1] / 1000.0); 
  card_spr.drawString(val_buf, 215, 45); // Adjusted Y
  sprintf(val_buf, "%s", format_large_number(data.current[1]));
  card_spr.drawString(val_buf, 145, 45); // Adjusted Y
  // Push the finished sprite to the screen
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite();

  // --- 3. Load Card ---
  card_y += card_height + card_y_gap; // New Y-pos (165)
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR);
  draw_load_icon(&card_spr, 20, 15); // Adjusted Y
  
  // Current (Large Font)
  card_spr.setTextDatum(TR_DATUM);
  card_spr.setTextColor(LOAD_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%s", format_large_number(data.current[2]));
  card_spr.drawString(val_buf, 215, 10); // Adjusted Y
  // Power & Voltage (Small Font)
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.1fW", data.power[2] / 1000.0);
  card_spr.drawString(val_buf, 215, 45); // Adjusted Y
  sprintf(val_buf, "%.2fV", data.busVoltage[2]);
  card_spr.drawString(val_buf, 145, 45); // Adjusted Y
  // Push the finished sprite to the screen
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite();
}

void draw_power_channel_screen(int channel, const DisplayData& data) {
  char val_buf[20];
  const char* channel_name = "";
  uint16_t primary_color = TEXT_COLOR;
  
  switch (channel) {
    case 1: channel_name = "SOLAR"; primary_color = SOLAR_COLOR; break;
    case 2: channel_name = "BATTERY"; primary_color = BATTERY_COLOR; break;
    case 3: channel_name = "LOAD"; primary_color = LOAD_COLOR; break;
  }
  
  // --- Draw Header (Directly) ---
  tft.fillRect(0, 0, 240, 36, BG_COLOR); // Clear header area
  tft.setTextDatum(TC_DATUM); 
  tft.setTextColor(primary_color, BG_COLOR);
  tft.setTextSize(3); 
  tft.drawString(channel_name, 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);
  
  // --- Clear gap below header ---
  tft.fillRect(0, 36, 240, 4, BG_COLOR); // 4px gap

  // --- Sprite 1: Data (V, A, W) ---
  // Covers Y=40 to Y=139 (100px tall)
  TFT_eSprite data_spr = TFT_eSprite(&tft);
  data_spr.createSprite(240, 100);
  data_spr.fillSprite(BG_COLOR); 

  data_spr.setTextDatum(TL_DATUM); 
  data_spr.setTextColor(TEXT_COLOR, BG_COLOR);
  data_spr.setTextSize(2);
  
  // Voltage (Local Y = 0)
  data_spr.drawString("Voltage:", 20, 0);
  data_spr.setTextDatum(TR_DATUM); 
  data_spr.setTextSize(3);
  sprintf(val_buf, "%.2f V", data.busVoltage[channel - 1]);
  data_spr.drawString(val_buf, 220, 0);

  // Current (Local Y = 35)
  data_spr.setTextDatum(TL_DATUM);
  data_spr.setTextSize(2);
  data_spr.drawString("Current:", 20, 35);
  data_spr.setTextDatum(TR_DATUM);
  data_spr.setTextSize(3);
  sprintf(val_buf, "%s", format_large_number(data.current[channel - 1]));
  data_spr.drawString(val_buf, 220, 35);

  // Power (Local Y = 70)
  data_spr.setTextDatum(TL_DATUM);
  data_spr.setTextSize(2);
  data_spr.drawString("Power:", 20, 70);
  data_spr.setTextDatum(TR_DATUM);
  data_spr.setTextSize(3);
  if (channel == 2) { 
    sprintf(val_buf, "%+.2f W", data.power[channel - 1] / 1000.0);
  } else {
    sprintf(val_buf, "%.2f W", data.power[channel - 1] / 1000.0);
  }
  data_spr.drawString(val_buf, 220, 70);

  data_spr.pushSprite(0, 40);
  data_spr.deleteSprite(); 

  // --- Sprite 2: Graph Area ---
  // Covers Y=140 to Y=239 (100px tall)
  TFT_eSprite graph_spr = TFT_eSprite(&tft);
  graph_spr.createSprite(240, 100);
  graph_spr.fillSprite(BG_COLOR);

  graph_spr.drawRoundRect(10, 5, 220, 90, 5, CARD_COLOR);
  graph_spr.setTextDatum(MC_DATUM); 
  graph_spr.setTextSize(1);
  graph_spr.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  graph_spr.drawString("[ Future Graph Area ]", 120, 50); 
  
  graph_spr.pushSprite(0, 140);
  graph_spr.deleteSprite(); 
}

// --- NEW SENSOR SCREEN ---
void draw_sensors_screen(const DisplayData& data) {
  char val_buf[20];
  
  // --- Draw Header (Directly) ---
  tft.fillRect(0, 0, 240, 36, BG_COLOR); // Clear header area
  tft.setTextDatum(TC_DATUM); 
  tft.setTextColor(SENSOR_COLOR, BG_COLOR);
  tft.setTextSize(3); 
  tft.drawString("SENSORS", 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);
  
  // --- FIX: Clear gaps instead of whole screen ---
  tft.fillRect(0, 36, 240, 9, BG_COLOR); // Gap 1 (45-36=9)
  tft.fillRect(0, 105, 240, 5, BG_COLOR); // Gap 2 (110-105=5)
  tft.fillRect(0, 170, 240, 5, BG_COLOR); // Gap 3 (175-170=5)
  tft.fillRect(0, 235, 240, 5, BG_COLOR); // Bottom gap (240-235=5)
  tft.fillRect(0, 45, 5, 195, BG_COLOR); // Left gap
  tft.fillRect(235, 45, 5, 195, BG_COLOR); // Right gap


  // Create a reusable sprite for the sensor cards
  TFT_eSprite card_spr = TFT_eSprite(&tft);
  int card_height = 60;
  int card_width = 230;
  int card_y_gap = 5; // FIX: Was 10, too large
  int card_y = 45; // Start Y-pos

  // --- 1. Temperature Card ---
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR); 
  draw_temperature_icon(&card_spr, 15, 10); // (x, y)
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.1f C", data.temperature); // Celsius
  card_spr.drawString(val_buf, 215, 15);
  
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite();

  // --- 2. Humidity Card ---
  card_y += card_height + card_y_gap; // New Y-pos (110)
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR); 
  draw_humidity_icon(&card_spr, 15, 10);
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.0f %%", data.humidity); // Percent
  card_spr.drawString(val_buf, 215, 15);
  
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite();

  // --- 3. Lux Card ---
  card_y += card_height + card_y_gap; // New Y-pos (175)
  card_spr.createSprite(card_width, card_height);
  card_spr.fillRoundRect(0, 0, card_width, card_height, 10, CARD_COLOR); 
  draw_lux_icon(&card_spr, 15, 10);
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.0f lx", data.lux); // Lux
  card_spr.drawString(val_buf, 215, 15);
  
  card_spr.pushSprite(5, card_y);
  card_spr.deleteSprite();
}


// --- DELETED: draw_lights_live_status_screen() ---
// This is now replaced by the global footer bar


// <--- Lights menu screen (Full Screen) --->
void draw_lights_menu_screen(const DisplayData& data) {
  // This is a full-screen menu, so it draws over everything
  // and does *not* call the footer.
  tft.fillScreen(BG_COLOR); // Clear whole screen

  const char* menuItems[] = {"Toggle Light", "Motion Timer", "Manual Timer", "Back"};

  // --- Header ---
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(LOAD_COLOR, BG_COLOR);
  tft.setTextSize(3);
  tft.drawString("LIGHTS MENU", 120, 5);
  tft.drawFastHLine(10, 35, 220, CARD_COLOR);

  // --- FIX: Split 240x240 sprite into two smaller ones ---
  
  // --- Sprite 1: Menu Items 0 & 1 ---
  TFT_eSprite item_spr_1 = TFT_eSprite(&tft);
  item_spr_1.createSprite(240, 120); // Covers Y=40 to Y=159
  item_spr_1.fillSprite(BG_COLOR); 

  item_spr_1.setTextDatum(TL_DATUM);
  item_spr_1.setTextSize(2);
  for (int i = 0; i < 2; i++) {
    // Local Y = (60 + i * 40) - 40 = 20 + i * 40
    int yPos = 20 + i * 40; 
    if (i == data.lightsMenuSelection) {
      item_spr_1.fillRoundRect(20, yPos - 10, 200, 35, 5, LOAD_COLOR);
      item_spr_1.setTextColor(SHADOW_COLOR, LOAD_COLOR);
      item_spr_1.drawString(menuItems[i], 30, yPos);
    } else {
      item_spr_1.setTextColor(TEXT_COLOR, BG_COLOR);
      item_spr_1.drawString(menuItems[i], 30, yPos);
    }
  }
  item_spr_1.pushSprite(0, 40);
  item_spr_1.deleteSprite(); 

  // --- Sprite 2: Menu Items 2 & 3 ---
  TFT_eSprite item_spr_2 = TFT_eSprite(&tft);
  item_spr_2.createSprite(240, 120); // Covers Y=160 to Y=279
  item_spr_2.fillSprite(BG_COLOR); 

  item_spr_2.setTextDatum(TL_DATUM);
  item_spr_2.setTextSize(2);
  for (int i = 2; i < 4; i++) {
    // Local Y = (60 + i * 40) - 160 = 20 + (i-2) * 40
    int yPos = 20 + (i-2) * 40; 
    if (i == data.lightsMenuSelection) {
      item_spr_2.fillRoundRect(20, yPos - 10, 200, 35, 5, LOAD_COLOR);
      item_spr_2.setTextColor(SHADOW_COLOR, LOAD_COLOR);
      item_spr_2.drawString(menuItems[i], 30, yPos);
    } else {
      item_spr_2.setTextColor(TEXT_COLOR, BG_COLOR);
      item_spr_2.drawString(menuItems[i], 30, yPos);
    }
  }
  item_spr_2.pushSprite(0, 160);
  item_spr_2.deleteSprite(); 
}

// <--- Screen for editing timers (Full Screen) --->
void draw_lights_edit_timer_screen(const DisplayData& data) {
  // This is also a full-screen menu
  tft.fillScreen(BG_COLOR); 

  char buf[30];
  unsigned long durationToEdit;
  const char* title;

  // We have to check the *main* mode to know which timer to edit
  if (data.currentMode == EDIT_MOTION_TIMER) {
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

  // --- Create sprite for the body ---
  TFT_eSprite body_spr = TFT_eSprite(&tft);
  body_spr.createSprite(240, 244); // 280 - 36 = 244
  body_spr.fillSprite(BG_COLOR); // Clear sprite

  // --- Display Time Value (to sprite) ---
  body_spr.setTextDatum(MC_DATUM);
  body_spr.setTextColor(TEXT_COLOR, BG_COLOR);
  body_spr.setTextSize(5);
  unsigned long minutes = durationToEdit / 60000;
  unsigned long seconds = (durationToEdit % 60000) / 1000;
  sprintf(buf, "%02lu:%02lu", minutes, seconds);
  // Local Y = 130 - 36 = 94
  body_spr.drawString(buf, 120, 94);

  // --- Footer instructions (to sprite) ---
  body_spr.setTextDatum(BC_DATUM);
  body_spr.setTextSize(1);
  body_spr.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  // Local Y = 275 - 36 = 239
  body_spr.drawString("Turn to adjust, Press to save", 120, 239);
  
  body_spr.pushSprite(0, 36);
  body_spr.deleteSprite(); 
}

// --- NEW GLOBAL FOOTER ---
void draw_global_footer_bar(const DisplayData& data) {
  // Create a sprite for the footer area
  TFT_eSprite footer_spr = TFT_eSprite(&tft);
  footer_spr.createSprite(240, FOOTER_HEIGHT);
  footer_spr.fillSprite(BG_COLOR); // Clear sprite

  // Draw dividing line
  footer_spr.drawFastHLine(0, 0, 240, CARD_COLOR);

  // Draw Icons (Centered)
  draw_footer_light_icon(&footer_spr, 70, 5, data.lightIsOn);
  draw_footer_occupancy_icon(&footer_spr, 140, 5, data.occupancyDetected);

  // --- Timer Progress Bar (at very bottom) ---
  if (data.lightIsOn && data.timerRemainingSeconds > 0) {
    unsigned long totalDurationSec = data.lightManualOverride ? 
                                     (data.manualTimerDuration / 1000) : 
                                     (data.motionTimerDuration / 1000);
    
    if (totalDurationSec > 0) {
      int progressWidth = map(data.timerRemainingSeconds, 0, totalDurationSec, 0, 240);
      if (progressWidth < 0) progressWidth = 0;
      if (progressWidth > 240) progressWidth = 240;
      
      // Draw at local sprite coordinates
      footer_spr.fillRect(0, FOOTER_HEIGHT - 4, progressWidth, 4, LOAD_COLOR);
    }
  }

  // Push the footer sprite to the screen
  footer_spr.pushSprite(0, FOOTER_Y_START);
  footer_spr.deleteSprite();
}


// --- Icon Drawing Functions ---

// --- NEW MDI-style Icons ---
void draw_footer_light_icon(TFT_eSprite* spr, int x, int y, bool on) {
  uint16_t icon_color = on ? SOLAR_COLOR : SUBTLE_TEXT_COLOR;
  
  // mdi:lightbulb
  // Bulb top
  spr->fillCircle(x + 15, y + 12, 12, icon_color);
  // Bulb base
  spr->fillRoundRect(x + 7, y + 22, 16, 8, 2, icon_color);
}

void draw_footer_occupancy_icon(TFT_eSprite* spr, int x, int y, bool detected) {
  uint16_t icon_color = detected ? SOLAR_COLOR : SUBTLE_TEXT_COLOR;
  
  // mdi:motion-sensor (person part only)
  // Head
  spr->fillCircle(x + 15, y + 7, 7, icon_color);
  // Body
  spr->fillRoundRect(x + 5, y + 15, 20, 15, 3, icon_color);
}

void draw_temperature_icon(TFT_eSprite* spr, int x, int y) {
  // mdi:sun-thermometer
  // Sun part (yellow)
  spr->fillCircle(x + 30, y + 15, 8, SOLAR_COLOR);
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = (x + 30) + 9 * cos(angle);
    int y1 = (y + 15) + 9 * sin(angle);
    int x2 = (x + 30) + 12 * cos(angle);
    int y2 = (y + 15) + 12 * sin(angle);
    spr->drawLine(x1, y1, x2, y2, SOLAR_COLOR);
  }
  
  // Thermometer part (black)
  spr->fillCircle(x + 10, y + 35, 8, SHADOW_COLOR); // Bulb
  spr->fillRoundRect(x + 5, y + 5, 10, 32, 5, SHADOW_COLOR); // Body
  
  // Inner white
  spr->fillCircle(x + 10, y + 35, 6, TEXT_COLOR); 
  spr->fillRoundRect(x + 7, y + 7, 6, 30, 3, TEXT_COLOR);

  // Inner red gauge
  spr->fillCircle(x + 10, y + 35, 4, SENSOR_THERM_COLOR);
  spr->fillRoundRect(x + 8, y + 20, 4, 15, 2, SENSOR_THERM_COLOR);
}

void draw_humidity_icon(TFT_eSprite* spr, int x, int y) {
  // mdi:cloud-percent
  // Cloud
  spr->fillCircle(x + 15, y + 20, 10, SENSOR_CLOUD_COLOR);
  spr->fillCircle(x + 25, y + 15, 12, SENSOR_CLOUD_COLOR);
  spr->fillCircle(x + 35, y + 22, 10, SENSOR_CLOUD_COLOR);
  spr->fillRoundRect(x + 10, y + 20, 30, 12, 5, SENSOR_CLOUD_COLOR);
  
  // Percent
  spr->setTextDatum(MC_DATUM);
  spr->setTextColor(TEXT_COLOR, SENSOR_CLOUD_COLOR);
  spr->setTextSize(2);
  spr->drawString("%", x + 25, y + 20);
}

void draw_lux_icon(TFT_eSprite* spr, int x, int y) {
  // mdi:brightness-7
  spr->fillCircle(x + 20, y + 20, 12, SOLAR_COLOR);
  spr->fillCircle(x + 20, y + 20, 8, CARD_COLOR); // "cutout"
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = (x + 20) + 14 * cos(angle);
    int y1 = (y + 20) + 14 * sin(angle);
    int x2 = (x + 20) + 20 * cos(angle);
    int y2 = (y + 20) + 20 * sin(angle);
    spr->drawLine(x1, y1, x2, y2, SOLAR_COLOR);
  }
}

// --- Re-used Power Icons ---
void draw_sun_icon(TFT_eSprite* spr, int x, int y) {
  spr->fillCircle(x + 20, y + 20, 16, SOLAR_COLOR);
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = (x + 20) + 18 * cos(angle);
    int y1 = (y + 20) + 18 * sin(angle);
    int x2 = (x + 20) + 24 * cos(angle);
    int y2 = (y + 20) + 24 * sin(angle);
    spr->drawLine(x1, y1, x2, y2, SOLAR_COLOR);
  }
}

void draw_battery_icon(TFT_eSprite* spr, int x, int y, float voltage) {
  // Draw battery body
  spr->fillRoundRect(x, y + 8, 60, 35, 5, TEXT_COLOR);
  spr->fillRoundRect(x + 2, y + 10, 56, 31, 3, CARD_COLOR);

  // Draw battery terminals
  spr->fillRect(x + 10, y, 10, 8, TEXT_COLOR); // Left terminal
  spr->fillRect(x + 40, y, 10, 8, TEXT_COLOR); // Right terminal

  // Calculate charge level
  int fill_width = map(voltage * 100, 1100, 1350, 0, 52); // 11.0V to 13.5V
  if (fill_width < 0) fill_width = 0;
  if (fill_width > 52) fill_width = 52;
  
  spr->fillRoundRect(x + 4, y + 12, fill_width, 27, 2, BATTERY_COLOR);
}

void draw_load_icon(TFT_eSprite* spr, int x, int y) {
  // Draw outlet plate
  spr->fillRoundRect(x, y, 40, 40, 5, TEXT_COLOR);
  spr->fillRoundRect(x + 2, y + 2, 36, 36, 4, LOAD_COLOR);
  
  // Draw the "sockets"
  spr->fillRoundRect(x + 8, y + 8, 8, 14, 2, SHADOW_COLOR);  // Left slot
  spr->fillRoundRect(x + 24, y + 8, 8, 14, 2, SHADOW_COLOR); // Right slot
  spr->fillCircle(x + 20, y + 30, 4, SHADOW_COLOR);      // Ground
}

// --- DELETED all old light/occupancy icons ---

