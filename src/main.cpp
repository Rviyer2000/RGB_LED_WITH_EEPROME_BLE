#include <Arduino.h>


#define DEBUG 1

#define LEDPIN 23

#define RED 255, 0, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255
#define WARM 255, 128, 0
#define PURPLE 255, 0, 255
#define CYAN 0, 255, 255
#define WHITE 255, 255, 255
#define OFF 0, 0, 0

#define EEPROM_ADDR_numpixels 0   // Address for the first integer
#define EEPROM_ADDR_brightness 4   // Address for the second integer (assuming 2 bytes per int)
#define EEPROM_ADDR_r 8   
#define EEPROM_ADDR_g 12   
#define EEPROM_ADDR_b 16  


#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <BluetoothSerial.h>
#include <EEPROM.h>

long NUMPIXELS  = 13;
int number_of_changing_pixels = 2 ;

int16_t brightness = 200;
int brightness_control_level = 20;

int16_t r = 255;
int16_t g = 0;
int16_t b = 0;

String command = "0";
long colorNumber = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
BluetoothSerial SerialBT;

void setStripColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void led_color_set(){
  colorNumber = command.toInt();
    r = (colorNumber / 1000000) % 1000;
    g = (colorNumber / 1000) % 1000;
    b = colorNumber % 1000;
    setStripColor(r, g, b);

    Serial.print("R: ");
    Serial.print(r);
    Serial.print(" G: ");
    Serial.print(g);
    Serial.print(" B: ");
    Serial.println(b);
}

void eeprom_update(){
  EEPROM.put(EEPROM_ADDR_numpixels, NUMPIXELS);
  EEPROM.put(EEPROM_ADDR_brightness, brightness);
  EEPROM.put(EEPROM_ADDR_r , r );
  EEPROM.put(EEPROM_ADDR_g , g );
  EEPROM.put(EEPROM_ADDR_b , b );
  EEPROM.commit();  // Ensure data is written to flash memory
  SerialBT.println("Values saved to EEPROM: " + String( NUMPIXELS) + ", " + String(brightness));

}

void process_command() {
  if (DEBUG) {
    Serial.println("Received command: " + command);
  }

        if (command.indexOf("R") == 0) {setStripColor(255, 0, 0);} // Set full RED color
   else if (command.indexOf("G") == 0) {setStripColor(0, 255, 0);} // Set GREEN color
   else if (command.indexOf("B") == 0) {setStripColor(0, 0, 255);} // Set BLUE color
   else if (command.indexOf("Wa") == 0) {setStripColor(255,128, 0);} // Set Warm color
   else if (command.indexOf("P") == 0) {setStripColor(255,0, 255);} // Set Warm color
   else if (command.indexOf("C") == 0) {setStripColor(0,255, 255);} // Set Warm color
   else if (command.indexOf("WH") == 0) {setStripColor(255,255, 255);} // Set Warm color
   else if (command.indexOf("Z") == 0) {setStripColor(0,0, 0);} // Set Warm color
   else if (command.indexOf("b+") == 0) { brightness += brightness_control_level;if (brightness > 255) {brightness = 255;}strip.setBrightness(brightness);SerialBT.println("brightness = " + String(brightness));strip.show();}
   else if (command.indexOf("b-") == 0) {  brightness -= brightness_control_level;if (brightness < 1) {brightness = 1;}strip.setBrightness(brightness);SerialBT.println("brightness = " + String(brightness));strip.show();} 
   else if (command.indexOf("n+") == 0) { NUMPIXELS += number_of_changing_pixels; strip.updateLength(NUMPIXELS);SerialBT.println("Nuber of pixels = " + String(NUMPIXELS));setStripColor(r, g, b);}
   else if (command.indexOf("n-") == 0) { NUMPIXELS -= number_of_changing_pixels; strip.updateLength(NUMPIXELS);SerialBT.println("Nuber of pixels = " + String(NUMPIXELS));setStripColor(r, g, b);}
   else if (command.indexOf("S") == 0) {eeprom_update();} // Set GREEN color
   else if (command == "OFF") {setStripColor(0, 0, 0);} 
   else {led_color_set();}
}

void command_handler() {
  if (SerialBT.available() > 0) {
    command = SerialBT.readString();
    process_command();
    command = "0"; // Reset command
  }
}

void eeprom_setup(){
  EEPROM.begin(512);
  EEPROM.get(EEPROM_ADDR_numpixels, NUMPIXELS);
  strip.updateLength(NUMPIXELS);
  EEPROM.get(EEPROM_ADDR_brightness, brightness);
  strip.setBrightness(brightness);
  EEPROM.get(EEPROM_ADDR_r, r);
  EEPROM.get(EEPROM_ADDR_g, g);
  EEPROM.get(EEPROM_ADDR_b, b);
  setStripColor(r, g, b);
}


void setup() {
  eeprom_setup();
  Serial.begin(9600);
  SerialBT.begin("IYER");
  strip.begin();
  strip.setBrightness(brightness);
}

void loop() {
  command_handler();
}