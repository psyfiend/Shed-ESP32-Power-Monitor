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
#define SUBTLE_TEXT_COLOR 0x8C51 // Brighter
#define SENSOR_THERM_COLOR 0xF800 // Red for thermometer
#define SENSOR_CLOUD_COLOR 0x3498 // Blue for cloud

// --- UI Sizing ---
#define CONTENT_Y_START 0
#define CONTENT_Y_END 239   // Bottom 40px are for the footer
#define FOOTER_Y_START 240
#define FOOTER_HEIGHT 40

// --- Forward Declarations for private drawing functions ---
// --- UPDATED: No more modeChanged flag needed ---
void draw_power_overview_screen(const DisplayData& data);
void draw_power_channel_screen(int channel, const DisplayData& data); 
void draw_sensors_screen(const DisplayData& data); 

// Full-screen "Popup" Menus
void draw_lights_menu_screen(const DisplayData& data);
void draw_lights_edit_timer_screen(const DisplayData& data); 

// Global Footer
void draw_global_footer_bar(const DisplayData& data); 

// Icons...
void draw_footer_light_icon(TFT_eSprite* spr, int x, int y, bool on);
void draw_footer_occupancy_icon(TFT_eSprite* spr, int x, int y, bool detected);
void draw_temperature_icon(TFT_eSprite* spr, int x, int y);
void draw_humidity_icon(TFT_eSprite* spr, int x, int y);
void draw_lux_icon(TFT_eSprite* spr, int x, int y);
void draw_pressure_icon(TFT_eSprite* spr, int x, int y); 
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

// --- UPDATED: Signature back to original ---
void update_display(DisplayMode mode, PowerSubMode powerSub, const DisplayData& data) {
  
  switch (mode) {
    case POWER_MODE_ALL:
      draw_power_overview_screen(data);
      draw_global_footer_bar(data); // Footer updates every time
      break;
    case POWER_MODE_CH1:
      draw_power_channel_screen(1, data);
      draw_global_footer_bar(data); 
      break;
    case POWER_MODE_CH2:
      draw_power_channel_screen(2, data);
      draw_global_footer_bar(data); 
      break;
    case POWER_MODE_CH3:
      draw_power_channel_screen(3, data);
      draw_global_footer_bar(data); 
      break;
    case SENSORS_MODE: 
      draw_sensors_screen(data);
      draw_global_footer_bar(data); 
      break;
    
    // --- Menu screens are full-screen and do NOT draw the footer ---
    case LIGHTS_MENU:
      draw_lights_menu_screen(data);
      break;
    case EDIT_MOTION_TIMER:
    case EDIT_MANUAL_TIMER:
      draw_lights_edit_timer_screen(data); 
      break;
      
    default:
      tft.fillScreen(BG_COLOR); 
      tft.setCursor(10, 20);
      tft.setTextColor(TFT_WHITE, BG_COLOR);
      tft.setTextSize(2);
      tft.println("Screen not implemented");
      break;
  }
}

// --- Screen Drawing Functions ---

// --- UPDATED: Using full-width sprites to kill ghosting & flicker ---
void draw_power_overview_screen(const DisplayData& data) {
  char val_buf[20]; 

  TFT_eSprite card_spr = TFT_eSprite(&tft);
  
  // Card dimensions
  int card_height = 75;
  int card_width = 230;
  int card_y_gap = 5;
  int card_y = 5; // Start Y-pos
  int card_x = 5;

  // --- 1. Solar Card Sprite (Full-width band) ---
  // This sprite is 80px tall (5px gap + 75px card)
  card_spr.createSprite(240, 80); 
  card_spr.fillRect(0, 0, 240, 80, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 0, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=0
  
  draw_sun_icon(&card_spr, card_x + 15, 15); 
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SOLAR_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.1fW", data.power[0] / 1000.0);
  card_spr.drawString(val_buf, card_x + 215, 10); 
  
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.2fV", data.busVoltage[0]);
  card_spr.drawString(val_buf, card_x + 215, 45); 
  sprintf(val_buf, "%s", format_large_number(data.current[0]));
  card_spr.drawString(val_buf, card_x + 145, 45); 
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=5
  card_spr.deleteSprite(); 

  // --- 2. Battery Card Sprite (Full-width band) ---
  card_y += card_height + card_y_gap; // New Y-pos (85)
  // This sprite is 80px tall (5px gap + 75px card)
  card_spr.createSprite(240, 80); 
  card_spr.fillRect(0, 0, 240, 80, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 0, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=0

  draw_battery_icon(&card_spr, card_x + 15, 15, data.busVoltage[1]); 
  
  card_spr.setTextDatum(TR_DATUM);
  card_spr.setTextColor(BATTERY_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%.2fV", data.busVoltage[1]);
  card_spr.drawString(val_buf, card_x + 215, 10); 
  
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%+.1fW", data.power[1] / 1000.0); 
  card_spr.drawString(val_buf, card_x + 215, 45); 
  sprintf(val_buf, "%s", format_large_number(data.current[1]));
  card_spr.drawString(val_buf, card_x + 145, 45); 
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=85
  card_spr.deleteSprite();

  // --- 3. Load Card Sprite (Full-width band) ---
  card_y += card_height + card_y_gap; // New Y-pos (165)
  // This sprite is 75px tall (no bottom gap needed)
  card_spr.createSprite(240, 75); 
  card_spr.fillRect(0, 0, 240, 75, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 0, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=0

  draw_load_icon(&card_spr, card_x + 20, 15); 
  
  card_spr.setTextDatum(TR_DATUM);
  card_spr.setTextColor(LOAD_COLOR, CARD_COLOR);
  card_spr.setTextSize(4);
  sprintf(val_buf, "%s", format_large_number(data.current[2]));
  card_spr.drawString(val_buf, card_x + 215, 10); 
  
  card_spr.setTextSize(2);
  card_spr.setTextColor(TEXT_COLOR, CARD_COLOR);
  sprintf(val_buf, "%.1fW", data.power[2] / 1000.0);
  card_spr.drawString(val_buf, card_x + 215, 45); 
  sprintf(val_buf, "%.2fV", data.busVoltage[2]);
  card_spr.drawString(val_buf, card_x + 145, 45); 
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=165
  card_spr.deleteSprite();
}

// --- UPDATED: Using full-width sprites to kill ghosting & flicker ---
void draw_power_channel_screen(int channel, const DisplayData& data) {
  char val_buf[20];
  const char* channel_name = "";
  uint16_t primary_color = TEXT_COLOR;
  
  switch (channel) {
    case 1: channel_name = "SOLAR"; primary_color = SOLAR_COLOR; break;
    case 2: channel_name = "BATTERY"; primary_color = BATTERY_COLOR; break;
    case 3: channel_name = "LOAD"; primary_color = LOAD_COLOR; break;
  }

  // --- Sprite 1: Header (Full-width band) ---
  TFT_eSprite header_spr = TFT_eSprite(&tft);
  header_spr.createSprite(240, 40); // 40px tall for header + line
  header_spr.fillRect(0, 0, 240, 40, BG_COLOR);

  header_spr.setTextDatum(TC_DATUM); 
  header_spr.setTextColor(primary_color, BG_COLOR);
  header_spr.setTextSize(3); 
  header_spr.drawString(channel_name, 120, 5);
  header_spr.drawFastHLine(10, 35, 220, CARD_COLOR);

  header_spr.pushSprite(0, 0);
  header_spr.deleteSprite();
  
  // --- Sprite 2: Data (V, A, W) (Full-width band) ---
  TFT_eSprite data_spr = TFT_eSprite(&tft);
  data_spr.createSprite(240, 100); // 100px tall
  data_spr.fillRect(0, 0, 240, 100, BG_COLOR); 

  data_spr.setTextDatum(TL_DATUM); 
  data_spr.setTextColor(TEXT_COLOR, BG_COLOR);
  data_spr.setTextSize(2);
  
  data_spr.drawString("Voltage:", 20, 0);
  data_spr.setTextDatum(TR_DATUM); 
  data_spr.setTextSize(3);
  sprintf(val_buf, "%.2f V", data.busVoltage[channel - 1]);
  data_spr.drawString(val_buf, 220, 0);

  data_spr.drawString("Current:", 20, 35);
  data_spr.setTextDatum(TR_DATUM);
  data_spr.setTextSize(3);
  sprintf(val_buf, "%s", format_large_number(data.current[channel - 1]));
  data_spr.drawString(val_buf, 220, 35);
  
  data_spr.drawString("Power:", 20, 70);
  data_spr.setTextDatum(TR_DATUM);
  data_spr.setTextSize(3);
  if (channel == 2) { 
    sprintf(val_buf, "%+.2f W", data.power[channel - 1] / 1000.0);
  } else {
    sprintf(val_buf, "%.2f W", data.power[channel - 1] / 1000.0);
  }
  data_spr.drawString(val_buf, 220, 70);

  data_spr.pushSprite(0, 40); // Push at Y=40
  data_spr.deleteSprite(); 

  // --- Sprite 3: Graph Area (Full-width band) ---
  TFT_eSprite graph_spr = TFT_eSprite(&tft);
  graph_spr.createSprite(240, 100); // 100px tall
  graph_spr.fillRect(0, 0, 240, 100, BG_COLOR);

  graph_spr.drawRoundRect(10, 5, 220, 90, 5, CARD_COLOR);
  graph_spr.setTextDatum(MC_DATUM); 
  graph_spr.setTextSize(1);
  graph_spr.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  graph_spr.drawString("[ Future Graph Area ]", 120, 50); 
  
  graph_spr.pushSprite(0, 140); // Push at Y=140
  graph_spr.deleteSprite(); 
}

// --- UPDATED: Using full-width sprites to kill ghosting & flicker ---
void draw_sensors_screen(const DisplayData& data) {
  char val_buf[20];
  
  // --- Sprite 1: Header (Full-width band) ---
  TFT_eSprite header_spr = TFT_eSprite(&tft);
  header_spr.createSprite(240, 40); // 40px tall for header + line
  header_spr.fillRect(0, 0, 240, 40, BG_COLOR); 
  
  header_spr.setTextDatum(TC_DATUM); 
  header_spr.setTextColor(SENSOR_COLOR, BG_COLOR);
  header_spr.setTextSize(3); 
  header_spr.drawString("SENSORS", 120, 5);
  header_spr.drawFastHLine(10, 35, 220, CARD_COLOR);

  header_spr.pushSprite(0, 0);
  header_spr.deleteSprite();

  // Create a reusable sprite for the sensor cards
  TFT_eSprite card_spr = TFT_eSprite(&tft);
  int card_height = 44;
  int card_width = 230;
  int card_y_gap = 5; 
  int card_y = 45; // Start Y-pos
  int card_x = 5;

  // --- 1. Temperature Card (Full-width band) ---
  // This sprite is 49px tall (5px gap + 44px card)
  card_spr.createSprite(240, 49);
  card_spr.fillRect(0, 0, 240, 49, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 5, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=5
  
  draw_temperature_icon(&card_spr, card_x + 15, 5); // Nudged up
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(3);
  sprintf(val_buf, "%.1f F", data.temperature); // Fahrenheit
  card_spr.drawString(val_buf, card_x + 215, 15); // Local Y = 10 + 5
  
  card_spr.pushSprite(0, 40); // Push sprite to screen Y=40
  card_spr.deleteSprite();

  // --- 2. Humidity Card (Full-width band) ---
  card_y = 40 + 49; // New Y-pos (89)
  // This sprite is 49px tall (5px gap + 44px card)
  card_spr.createSprite(240, 49);
  card_spr.fillRect(0, 0, 240, 49, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 5, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=5
 
  draw_humidity_icon(&card_spr, card_x + 15, 7); // Nudged up 
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(3);
  sprintf(val_buf, "%.0f %%", data.humidity); // Percent
  card_spr.drawString(val_buf, card_x + 215, 15); // Local Y = 10 + 5
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=89
  card_spr.deleteSprite();

  // --- 3. Lux Card (Full-width band) ---
  card_y += 49; // New Y-pos (138)
  card_spr.createSprite(240, 49);
  card_spr.fillRect(0, 0, 240, 49, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 5, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=5
  
  draw_lux_icon(&card_spr, card_x + 15, 7); // Nudged up
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(3);
  sprintf(val_buf, "%.0f lx", data.lux); // Lux
  card_spr.drawString(val_buf, card_x + 215, 15); // Local Y = 10 + 5
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=138
  card_spr.deleteSprite();

  // --- 4. Pressure Card (Full-width band) ---
  card_y += 49; // New Y-pos (187)
  // This sprite is 53px tall (5px gap + 44px card + 4px bottom gap)
  card_spr.createSprite(240, 53);
  card_spr.fillRect(0, 0, 240, 53, BG_COLOR); // Clear gap and bg
  card_spr.fillRoundRect(card_x, 5, card_width, card_height, 10, CARD_COLOR); // Draw card at local Y=5
  
  draw_pressure_icon(&card_spr, card_x + 15, 7); // Nudged up
  
  card_spr.setTextDatum(TR_DATUM); 
  card_spr.setTextColor(SENSOR_COLOR, CARD_COLOR);
  card_spr.setTextSize(3);
  sprintf(val_buf, "%.0f hPa", data.barometricPressure); 
  card_spr.drawString(val_buf, card_x + 215, 15); // Local Y = 10 + 5
  
  card_spr.pushSprite(0, card_y); // Push sprite to screen Y=187
  card_spr.deleteSprite();
}


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
  
  // --- Sprite 1: Menu Items 0 & 1 ---
  TFT_eSprite item_spr_1 = TFT_eSprite(&tft);
  item_spr_1.createSprite(240, 120); 
  item_spr_1.fillRect(0, 0, 240, 120, BG_COLOR); 

  item_spr_1.setTextDatum(TL_DATUM);
  item_spr_1.setTextSize(2);
  for (int i = 0; i < 2; i++) {
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
  item_spr_1.pushSprite(0, 40); // No transparency needed
  item_spr_1.deleteSprite(); 

  // --- Sprite 2: Menu Items 2 & 3 ---
  TFT_eSprite item_spr_2 = TFT_eSprite(&tft);
  item_spr_2.createSprite(240, 120); 
  item_spr_2.fillRect(0, 0, 240, 120, BG_COLOR); 

  item_spr_2.setTextDatum(TL_DATUM);
  item_spr_2.setTextSize(2);
  for (int i = 2; i < 4; i++) {
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
  item_spr_2.pushSprite(0, 160); // No transparency needed
  item_spr_2.deleteSprite(); 
}

// <--- Screen for editing timers (Full Screen) --->
void draw_lights_edit_timer_screen(const DisplayData& data) {
  // This is also a full-screen menu
  tft.fillScreen(BG_COLOR); 

  char buf[30];
  unsigned long durationToEdit;
  const char* title;

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
  body_spr.createSprite(240, 244); 
  body_spr.fillRect(0, 0, 240, 244, BG_COLOR); 

  // --- Display Time Value (to sprite) ---
  body_spr.setTextDatum(MC_DATUM);
  body_spr.setTextColor(TEXT_COLOR, BG_COLOR);
  body_spr.setTextSize(5);
  unsigned long minutes = durationToEdit / 60000;
  unsigned long seconds = (durationToEdit % 60000) / 1000;
  sprintf(buf, "%02lu:%02lu", minutes, seconds);
  body_spr.drawString(buf, 120, 94);

  // --- Footer instructions (to sprite) ---
  body_spr.setTextDatum(BC_DATUM);
  body_spr.setTextSize(1);
  body_spr.setTextColor(SUBTLE_TEXT_COLOR, BG_COLOR);
  body_spr.drawString("Turn to adjust, Press to save", 120, 239);
  
  body_spr.pushSprite(0, 36);
  body_spr.deleteSprite(); 
}

// --- NEW GLOBAL FOOTER ---
void draw_global_footer_bar(const DisplayData& data) {
  // Create a sprite for the footer area
  TFT_eSprite footer_spr = TFT_eSprite(&tft);
  footer_spr.createSprite(240, FOOTER_HEIGHT);
  footer_spr.fillRect(0, 0, 240, FOOTER_HEIGHT, BG_COLOR); 

  // Draw dividing line
  footer_spr.drawFastHLine(0, 0, 240, CARD_COLOR);

  // Draw Icons (Centered)
  draw_footer_light_icon(&footer_spr, 70, 5, data.lightIsOn);
  draw_footer_occupancy_icon(&footer_spr, 140, 5, data.occupancyDetected);

  // --- Timer Progress Bar (at very bottom) ---
  footer_spr.fillRect(0, FOOTER_HEIGHT - 4, 240, 4, BG_COLOR); 
  
  if (data.lightIsOn && data.timerRemainingSeconds > 0) {
    unsigned long totalDurationSec = data.lightManualOverride ? 
                                     (data.manualTimerDuration / 1000) : 
                                     (data.motionTimerDuration / 1000);
    
    if (totalDurationSec > 0) {
      int progressWidth = map(data.timerRemainingSeconds, 0, totalDurationSec, 0, 240);
      if (progressWidth < 0) progressWidth = 0;
      if (progressWidth > 240) progressWidth = 240;
      
      footer_spr.fillRect(0, FOOTER_HEIGHT - 4, progressWidth, 4, LOAD_COLOR);
    }
  }

  footer_spr.pushSprite(0, FOOTER_Y_START);
  footer_spr.deleteSprite();
}


// --- Icon Drawing Functions ---
// (No changes from here down)

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

void draw_pressure_icon(TFT_eSprite* spr, int x, int y) {
  // mdi:gauge
  uint16_t body_color = SUBTLE_TEXT_COLOR;
  uint16_t needle_color = SENSOR_COLOR;
  uint16_t center_color = SHADOW_COLOR;
  
  int cx = x + 20;
  int cy = y + 20;

  // Draw the gauge body (semi-circle)
  for (int i = 0; i <= 180; i++) {
    float angle = (i - 180) * PI / 180.0;
    spr->drawPixel(cx + 18 * cos(angle), cy + 18 * sin(angle), body_color);
    spr->drawPixel(cx + 17 * cos(angle), cy + 17 * sin(angle), body_color);
    spr->drawPixel(cx + 16 * cos(angle), cy + 16 * sin(angle), body_color);
  }
  
  // Draw the base "hump"
  spr->fillRoundRect(x + 10, y + 30, 20, 10, 3, body_color);

  // Draw Ticks
  float angle_low = (0 - 160) * PI / 180.0;
  float angle_high = (0 - 20) * PI / 180.0;
  spr->drawLine(cx + 12 * cos(angle_low), cy + 12 * sin(angle_low), 
                cx + 18 * cos(angle_low), cy + 18 * sin(angle_low), body_color);
  spr->drawLine(cx + 12 * cos(angle_high), cy + 12 * sin(angle_high), 
                cx + 18 * cos(angle_high), cy + 18 * sin(angle_high), body_color);
                
  // Draw Needle (pointing 3/4)
  float angle_needle = (0 - 45) * PI / 180.0; // ~2:30 position
  spr->drawLine(cx, cy, 
                cx + 16 * cos(angle_needle), cy + 16 * sin(angle_needle), needle_color);
  spr->drawLine(cx+1, cy, 
                cx + 1 + 16 * cos(angle_needle), cy + 16 * sin(angle_needle), needle_color);
  
  // Center hub
  spr->fillCircle(cx, cy, 3, center_color);
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

