/*
  RGBLedColors

  Demonstrates the RGB LED on the MXChip AZ3166 DevKit.
  Press Button A to cycle through preset colors.
  Press Button B to turn the LED off.
*/

#include "Arduino.h"
#include "OledDisplay.h"
#include "RGB_LED.h"

RGB_LED rgbLed;

// Color table: name, R, G, B
struct ColorEntry {
  const char* name;
  int r, g, b;
};

static const ColorEntry colors[] = {
  {"Red",     255, 0,   0  },
  {"Green",   0,   255, 0  },
  {"Blue",    0,   0,   255},
  {"Yellow",  255, 255, 0  },
  {"Cyan",    0,   255, 255},
  {"Magenta", 255, 0,   255},
  {"White",   255, 255, 255},
  {"Orange",  255, 165, 0  },
};
static const int NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

int colorIndex = 0;
int lastButtonAState = HIGH;
int lastButtonBState = HIGH;

void showColor()
{
  const ColorEntry& c = colors[colorIndex];
  rgbLed.setColor(c.r, c.g, c.b);
  Screen.clean();
  Screen.print(0, "RGB LED Colors");
  Screen.print(1, c.name, true);
  Screen.print(3, "A:Next  B:Off", true);
  Serial.printf("LED: %s (R=%d G=%d B=%d)\r\n", c.name, c.r, c.g, c.b);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("AZ3166 RGB LED Colors Example");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);
  lastButtonAState = digitalRead(USER_BUTTON_A);
  lastButtonBState = digitalRead(USER_BUTTON_B);
  showColor();
}

void loop()
{
  int buttonAState = digitalRead(USER_BUTTON_A);
  int buttonBState = digitalRead(USER_BUTTON_B);

  if (buttonAState == LOW && lastButtonAState == HIGH) {
    colorIndex = (colorIndex + 1) % NUM_COLORS;
    showColor();
  }

  if (buttonBState == LOW && lastButtonBState == HIGH) {
    rgbLed.turnOff();
    Screen.clean();
    Screen.print(0, "RGB LED Colors");
    Screen.print(1, "LED Off", true);
    Screen.print(3, "A:Next  B:Off", true);
    Serial.println("LED: Off");
  }

  lastButtonAState = buttonAState;
  lastButtonBState = buttonBState;
  delay(50);
}
