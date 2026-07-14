#include "display_manager.h"

#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "config.h"

static Adafruit_SSD1306 s_disp[2] = {
    Adafruit_SSD1306(DISP_WIDTH, DISP_HEIGHT, &Wire, -1),
    Adafruit_SSD1306(DISP_WIDTH, DISP_HEIGHT, &Wire, -1),
};

void DisplayManager::init(uint8_t addr1, uint8_t addr2)
{
  const uint8_t addrs[2] = {addr1, addr2};
  for (uint8_t i = 0; i < 2; i++)
  {
    s_disp[i].begin(SSD1306_SWITCHCAPVCC, addrs[i]);
    s_disp[i].clearDisplay();
    s_disp[i].display();
  }
}

void DisplayManager::update(uint8_t idx, const char* line1, const char* line2)
{
  if (idx > 1)
  {
    return;
  }

  s_disp[idx].clearDisplay();
  s_disp[idx].setTextSize(1);
  s_disp[idx].setTextColor(SSD1306_WHITE);
  s_disp[idx].setCursor(0, 0);
  s_disp[idx].print(line1);
  s_disp[idx].setCursor(0, 16);
  s_disp[idx].print(line2);
  s_disp[idx].display();
}
