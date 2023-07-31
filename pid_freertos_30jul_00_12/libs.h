////////////////////////////////////////////////// PID /////////////////////////////////////////////////////////////////////////////

#include <PID_v1.h>

////////////////////////////////////////////////// DHT 22 SENSOR //////////////////////////////////////////////////

#include "DHT.h"

////////////////////////////////////////////////// BUS I2C /////////////////////////////////////////////////////////////////////////

#include <Wire.h>

///////////////// Definitions /////////////

#define SDA_PIN 21
#define SCL_PIN 22

////////////////////////////////////////////////// Librerías de la OLED SSD1306 ////////////////////////////////////////////////////
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

///////////////// Definitions /////////////

#define OLED_ADDRESS 0X3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
