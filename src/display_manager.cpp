#include <U8g2lib.h>
#include "display_manager.h"
#include "utils.h" // For format_large_number

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


// --- Display Object ---
// Using U8g2 for the SH1106 128x128 OLED
U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- Forward Declarations for local drawing functions ---
void draw_power_overview(const DisplayData& data);
void draw_power_channel_detail(int channel, const DisplayData& data);
void draw_lights_live_status(const DisplayData& data);
void draw_lights_menu(const DisplayData& data);
void draw_lights_edit_timer(bool isMotionTimer, const DisplayData& data);


void setup_display() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(10, 32, "System Boot...");
  u8g2.sendBuffer();
  delay(1000);
}

void update_display(DisplayMode mode, LightsSubMode lightsSub, PowerSubMode powerSub, const DisplayData& data) {
  u8g2.clearBuffer();

  switch (mode) {
    case POWER_MODE_ALL:
      draw_power_overview(data);
      break;
    case POWER_MODE_CH1:
      if (powerSub == LIVE_POWER) draw_power_channel_detail(1, data);
      // else draw other subscreens
      break;
    case POWER_MODE_CH2:
       if (powerSub == LIVE_POWER) draw_power_channel_detail(2, data);
      // else draw other subscreens
      break;
    case POWER_MODE_CH3:
       if (powerSub == LIVE_POWER) draw_power_channel_detail(3, data);
      // else draw other subscreens
      break;
    case LIGHTS_MODE:
      switch (lightsSub) {
        case LIVE_STATUS:
          draw_lights_live_status(data);
          break;
        case LIGHTS_MENU:
          draw_lights_menu(data);
          break;
        case EDIT_MOTION_TIMER:
          draw_lights_edit_timer(true, data);
          break;
        case EDIT_MANUAL_TIMER:
          draw_lights_edit_timer(false, data);
          break;
      }
      break;
  }

  u8g2.sendBuffer();
}


void draw_lights_live_status(const DisplayData& data) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 12, "Shed Light Control");
    u8g2.drawHLine(0, 16, 128);

    u8g2.setFont(u8g2_font_profont22_mr);
    if (data.lightIsOn) {
        u8g2.drawStr(30, 50, "ON");
    } else {
        u8g2.drawStr(25, 50, "OFF");
    }

    // --- NEW: Draw the Timer Progress Bar ---
    if (data.lightIsOn) {
        unsigned long totalDurationSec = data.lightManualOverride ? 
                                         (data.manualTimerDuration / 1000) : 
                                         (data.motionTimerDuration / 1000);
        
        if (totalDurationSec > 0) {
            // Calculate progress bar width (max 104 pixels)
            int progressWidth = map(data.timerRemainingSeconds, 0, totalDurationSec, 0, 104);
            if (progressWidth < 0) progressWidth = 0;
            if (progressWidth > 104) progressWidth = 104;
            
            // Draw the frame for the progress bar
            u8g2.drawFrame(10, 64, 108, 10);
            // Draw the filled bar
            u8g2.drawBox(12, 66, progressWidth, 6);
        }

        // Display remaining time
        u8g2.setFont(u8g2_font_ncenB08_tr);
        char timeBuf[20];
        sprintf(timeBuf, "%lu s remaining", data.timerRemainingSeconds);
        u8g2.drawStr(10, 88, timeBuf);
        // Display "Time on"
        unsigned long onDurationSec = (millis() - data.lightOnTime) / 1000;
        sprintf(timeBuf, "On for: %lu s", onDurationSec);
        u8g2.drawStr(10, 102, timeBuf);
    }
}

void draw_power_overview(const DisplayData& data) {
    char buf[20];

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 12, "Power Overview");
    u8g2.drawHLine(0, 16, 128);

    u8g2.setFont(u8g2_font_7x13B_tr);
    // Channel 1 - Solar
    u8g2.drawStr(0, 32, "Solar:");
    sprintf(buf, "%.2f V", data.busVoltage[0]);
    u8g2.drawStr(50, 32, buf);
    sprintf(buf, "%.0f W", data.power[0] / 1000); // Convert mW to W
    u8g2.drawStr(0, 48, format_large_number(data.current[0]));
    u8g2.drawStr(50, 48, buf);


    // Channel 2 - Battery
    u8g2.drawStr(0, 72, "Battery:");
    sprintf(buf, "%.2f V", data.busVoltage[1]);
    u8g2.drawStr(50, 72, buf);
    sprintf(buf, "%.0f W", data.power[1] / 1000);
    u8g2.drawStr(0, 88, format_large_number(data.current[1]));
    u8g2.drawStr(50, 88, buf);
    
    // Channel 3 - Load
    u8g2.drawStr(0, 112, "Load:");
    sprintf(buf, "%.2f V", data.busVoltage[2]);
    u8g2.drawStr(50, 112, buf);
    sprintf(buf, "%.0f W", data.power[2] / 1000);
    u8g2.drawStr(0, 128, format_large_number(data.current[2]));
    u8g2.drawStr(50, 128, buf);
}

void draw_power_channel_detail(int channel, const DisplayData& data) {
    char buf[20];
    const char* title = "";
    if (channel == 1) title = "Solar Panel";
    if (channel == 2) title = "Battery";
    if (channel == 3) title = "Load";

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 12, title);
    u8g2.drawHLine(0, 16, 128);

    u8g2.setFont(u8g2_font_profont22_mr);
    sprintf(buf, "%.2f V", data.busVoltage[channel-1]);
    u8g2.drawStr(10, 50, buf);

    u8g2.setFont(u8g2_font_ncenB10_tr);
    sprintf(buf, "Current: %s", format_large_number(data.current[channel-1]));
    u8g2.drawStr(10, 80, buf);

    sprintf(buf, "Power:   %.1f W", data.power[channel-1] / 1000.0);
    u8g2.drawStr(10, 100, buf);
}

void draw_lights_menu(const DisplayData& data) {
    const char* menuItems[] = {"Toggle Light", "Motion Timer", "Manual Timer", "Back"};
    
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 12, "Lights Menu");
    u8g2.drawHLine(0, 16, 128);

    u8g2.setFont(u8g2_font_7x13B_tr);
    for (int i = 0; i < LIGHTS_MENU_ITEM_COUNT; i++) {
        if (i == data.lightsMenuSelection) {
            u8g2.drawBox(0, 24 + i * 16, 128, 16);
            u8g2.setDrawColor(0); // Black text on white background
            u8g2.drawStr(4, 38 + i * 16, menuItems[i]);
            u8g2.setDrawColor(1); // Reset to white text
        } else {
            u8g2.drawStr(4, 38 + i * 16, menuItems[i]);
        }
    }
}

void draw_lights_edit_timer(bool isMotionTimer, const DisplayData& data) {
    char buf[30];
    unsigned long durationToEdit = isMotionTimer ? data.tempMotionTimerDuration : data.tempManualTimerDuration;
    const char* title = isMotionTimer ? "Edit Motion Timer" : "Edit Manual Timer";

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 12, title);
    u8g2.drawHLine(0, 16, 128);

    u8g2.setFont(u8g2_font_profont22_mr);
    unsigned long minutes = durationToEdit / 60000;
    unsigned long seconds = (durationToEdit % 60000) / 1000;
    sprintf(buf, "%02lu:%02lu", minutes, seconds);
    u8g2.drawStr(20, 70, buf);

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 100, "Turn to adjust");
    u8g2.drawStr(10, 115, "Press to save");
}

