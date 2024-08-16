#include <Arduino.h>


#define DEBUG 1

#define LEDPIN 27

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



uint8_t current_state = 0;
uint8_t rgb_values[3];
int     pixelInterval = 50; 
uint16_t  pixelCurrent = 0;
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;

int animation_counter = 0;

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

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
 
  // if(pixelInterval != wait)

  //   pixelInterval = wait;                   //  Update delay time
  // strip.setPixelColor(pixelCurrent, color); //  Set pixel's color (in RAM)
  // strip.show();                             //  Update strip to match
  // pixelCurrent++;                           //  Advance current pixel
  // if(pixelCurrent >= NUMPIXELS)           //  Loop the pattern from the first LED
  //   pixelCurrent = 0;
}
void theaterChase(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i + pixelQueue, color); //  Set pixel's color (in RAM)
  }
  strip.show();                             //  Update strip to match
  for(int i=0; i <strip.numPixels(); i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Set pixel's color (in RAM)
  }
  pixelQueue++;                             //  Advance current pixel
  if(pixelQueue >= 3)
    pixelQueue = 0;                         //  Loop the pattern from the first LED
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(uint8_t wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
  // if(pixelInterval != wait)
  //   pixelInterval = wait;                   
  // for(uint16_t i=0; i < strip.numPixels(); i++) {
  //   strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
  // }
  // strip.show();                             //  Update strip to match
  // pixelCycle++;                             //  Advance current cycle
  // if(pixelCycle >= 256)
  //   pixelCycle = 0;                         //  Loop the cycle back to the begining
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time  
  for(int i=0; i < strip.numPixels(); i+3) {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255)); //  Update delay time  
  }
  strip.show();
  for(int i=0; i < NUMPIXELS; i+3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Update delay time  
  }      
  pixelQueue++;                           //  Advance current queue  
  pixelCycle++;                           //  Advance current cycle
  if(pixelQueue >= 3)
    pixelQueue = 0;                       //  Loop
  if(pixelCycle >= 256)
    pixelCycle = 0;                       //  Loop
}



void animation_selact(){

      SerialBT.println("haiiiiiiiii");

  switch(animation_counter) {
    case 1:
      SerialBT.println("RED_Clorwipe");
      colorWipe(strip.Color(255,   0,   0), 20);    // Red
      break;
    case 2:
    SerialBT.println("Green_Clorwipe");
      colorWipe(strip.Color(  0, 255,   0), 20);    // Green
      break;
    case 3:
    SerialBT.println(current_state);
      colorWipe(strip.Color(  0,   0, 255), 20);    // Blue
      break;
    case 4:
    SerialBT.println(current_state);
      rainbow(10);
      break;
    case 5:
    SerialBT.println(current_state);
      theaterChase(strip.Color(  0, 255,   0), 20); // Green
      break;
    case 6:
    SerialBT.println(current_state);
      theaterChase(strip.Color(255,   0, 255), 20); // Cyan
      break;
    case 7:
    SerialBT.println(current_state);
      theaterChase(strip.Color(255,   0,   0), 20); // Red
      break;
    case 8:
    SerialBT.println(current_state);
      theaterChaseRainbow(20);
      break;
  }

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
   else if (command.indexOf("m") == 0) {animation_counter ++ ;if(animation_counter > 9){animation_counter = 0;}SerialBT.println(animation_counter); animation_selact();}
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
  EEPROM.begin(4096);
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