/*********
   Simple Laser Projector
    Materials:
        Base: https://www.aliexpress.com/item/30K-laser-Galvo-Galvanometer-Based-Optical-Scanner-including-Show-Card/32531362627.html
            Includes 2 Galvanometer Optical Scanners
            Powersupply
            Logic board to do the heavy lifting
            The original control board and sd card are replaced with a DOIT ESP32 for more control.
        Laser: https://www.aliexpress.com/item/10pcs-lot-650nm-6mm-5V-5mW-Laser-Dot-Diode-Module-Copper-Head-Red/32583074085.html
            Simple, cheap, yet pretty visible at night or in shadow.
        (New) Control Board: https://www.aliexpress.com/item/Free-shipping-ESP32-Development-Board-WiFi-Bluetooth-Ultra-Low-Power-Consumption-Dual-Core-ESP-32-ESP/32803195605.html
            Cheap Wifi + Bluetooth with TWO cpu cores (insanity!)
        Proper DACs(Digital to Analog converters): https://www.aliexpress.com/item/MCP4725-I2C-DAC-Breakout-module-development-board/32311696869.html
            While waiting for china shipping I'm using two R/2R DAC Ladders for 8 bit precision of laser movement.
            Operation Amplifiers used for the R/2R DACs: https://www.aliexpress.com/item/Free-Shipping-10-pcs-DIP-IC-8pins-UA741CN-UA741CP-UA741-OP-Amp-LM741-741-TI-Original/2040771771.html 

    Plans (Roughly sorted from easiest to hardest):
        - Make web interface to control laser.
        - Make mobile app to control laser using bluetooth and cell phone accelerometer?
        - Draw text using laser
        - Recreate functionality of the original control board (namely reading ILDA files)
            - https://www.ilda.com/resources/StandardsDocs/ILDA_ISP99_rev002.pdf
        - Draw text
        - Scaling of the display
        - Fix angle hardware limitation (currently I'm only using a single quadrant[positive, positive] of the max rotation of the galvanometers)
        - Colors!
        - Noise reduction
        - Resolution options (skip ever x pixel, trade precision between x and y axis)
        - Machine learning + Computer Vision to outline objects with laser and provide additional information 
        - Perspective shift
            - Alter the perspective that the laser is draw on a surface to make it look like the laser was located somewhere else
            - Think of the perspective tool from Gimp or Photoshop to skew and warp images
            - Combine with computer vision to recognize the skew of a surface and automatically change the image being draw so it looks straight on the projected surface
        - External Monitor
            - Wayland driver support (lol!)

    Author: Michael Pedersen
*********/

#define SERIAL_SPEED 115200
#define DAC_PIN_COUNT 8

int xPins[] = {32, 33, 25, 26, 27, 14, 12, 13};
int xPinCount = DAC_PIN_COUNT;
int yPins[] = {23, 22, 21, 19, 18, 5, 4, 2};
int yPinCount = DAC_PIN_COUNT;

int randomNumber = -255;

void int_to_bin_digit(unsigned int in, int count, int* out)
{
  /* assert: count <= sizeof(int)*CHAR_BIT */
  unsigned int mask = 1U << (count - 1);
  int i;
  for (i = 0; i < count; i++) {
    out[i] = (in & mask) ? 1 : 0;
    in <<= 1;
  }
}

void setX(int x) {
  setPosition(x, xPins);
}

void setY(int y) {
  setPosition(y, yPins);
}

void setPosition(int value, int pins[])
{
  int pinCount = DAC_PIN_COUNT;//sizeof( pins ) / sizeof( pins[0] );

  int digit[8];
  int_to_bin_digit(value, 8, digit);
//   Serial.printf("  %u %u %u %u %u %u %u %u\n", digit[7], digit[6], digit[5], digit[4], digit[3], digit[2], digit[1], digit[0]);

  for (byte p = 0; p < pinCount ; p++) {
    digitalWrite(pins[p], digit[p]);
  }
}

void setup(void) {
  //Setup Randomness
  Serial.println("Setting up Randomness");
  randomSeed(analogRead(0));

  //Setup Serial
  Serial.print("Setting serial speed to ");
  Serial.println(SERIAL_SPEED);
  Serial.begin(SERIAL_SPEED);

  //Setup R/2R DAC for X coordinate
  Serial.println("Setting X Pins");
  for (int i = 0; i < xPinCount; i++) {
    Serial.print("Setting output pin #");
    Serial.println(xPins[i]);
    pinMode(xPins[i], OUTPUT);
  }

  //Setup R/2R DAC for Y Coordinate
  Serial.println("Setting Y Pins");
  for (int i = 0; i < yPinCount; i++) {
    Serial.print("Setting output pin #");
    Serial.println(yPins[i]);
    pinMode(yPins[i], OUTPUT);
  }

  //Set Initial point to 0
  setX(0);
  setY(0);

  Serial.println("Setup Done");
}

void loop(void) {
  //For testing axises (Horizontal or Vertical = bad, Diagonal = Perfect)
  int randomNumber = random(0, 255);

  //Send the X/Y Coordinate
  Serial.printf("Point [%u,%u]", randomNumber, randomNumber);
  setX(randomNumber);
  setY(randomNumber);

  delay(50);
}
