#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PCF8574.h"

#define range 15        //range to detected
#define read_light_pin A0
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
PCF8574 pcf8574(0x20);

unsigned long period = 5000; // wait time
unsigned long last_time = 0; // time stamp
int light;

class task{
public:
uint8_t red_pcf;
uint8_t green_pcf;
uint8_t blue_pcf;

enum states
{
  idle,
  detected,
  reset,
  check
};

states state;
 void RGBpcf_begin(uint8_t pin1,uint8_t pin2,uint8_t pin3);
 void RGB_pcf(byte R, byte G, byte B);
 void work(int dis1_value,int dis2_value,int light_val);
};
void task::RGBpcf_begin(uint8_t pin1,uint8_t pin2,uint8_t pin3){
  pcf8574.pinMode(pin1,OUTPUT);
  pcf8574.pinMode(pin2,OUTPUT);
  pcf8574.pinMode(pin3,OUTPUT);
   
  red_pcf = pin1;
  green_pcf = pin2;
  blue_pcf = pin3;

  state = idle;
}
void task::RGB_pcf(byte R, byte G, byte B)
{
  // Common Anode
  pcf8574.digitalWrite(red_pcf, R);
  pcf8574.digitalWrite(green_pcf, G);
  pcf8574.digitalWrite(blue_pcf, B);
}
void task::work(int dis1_value,int dis2_value,int light_val){
  switch (state)
  {
  case idle:
    if (light_val <= 25)
    {
      RGB_pcf(0, 0, 0);
      if (dis1_value <= range || dis2_value <= range)
      {
        state = detected;
      }
    }
    else
    {
      if (millis() - last_time > period)
      {
        RGB_pcf(1, 1, 1);
        last_time = millis(); // time stamp
      }
      if (dis1_value <= range || dis2_value <= range)
      {
        state = detected;
      }
    }
    break;
  case detected:
    RGB_pcf(1, 1, 0);
    state = check;
    break;
  case check:
    if (dis1_value <= range || dis2_value <= range)
    {
      state = detected;
      display.print("Parked not properly");
    }
    else
    {
      RGB_pcf(0, 1, 1);
      state = reset;
    }
    break;
  case reset:
    last_time = millis();
    state = idle;
    display.clearDisplay();
    break;
  }
}

class Sensors
{
public:
  // defines variables
  long duration;
  int distance;
  int trigPin;
  int echoPin;

  void begin(int echo, int trigger);
  int GetDistance(void);
};
void Sensors::begin(int echo, int trigger)
{
  trigPin = trigger;
  echoPin = echo;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}
int Sensors::GetDistance(void)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;

  return distance;
}

Sensors ultra_sonic_1;
Sensors ultra_sonic_2;
Sensors ultra_sonic_3;
Sensors ultra_sonic_4;
Sensors upper_US_1;
Sensors upper_US_2;

task park1;
task park2;


void setup()
{
  Serial.begin(9600); // Starts the serial communication
  ultra_sonic_1.begin(10, 9);
  ultra_sonic_2.begin(12, 11);
  ultra_sonic_3.begin(6, 7);
  ultra_sonic_4.begin(4, 5);

  upper_US_1.begin(2,3);
  upper_US_2.begin(8,13);

  
  pcf8574.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  // Set RGB pin
  park1.RGBpcf_begin(P0,P1,P2);
  park2.RGBpcf_begin(P3,P4,P5);

  display.display();
  delay(200); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(WHITE);
}
void loop()
{
  display.setTextSize(2);
  display.setCursor(0, 0);
  int LIGHT = map(analogRead(read_light_pin),0, 1023, 100, 0);
  //!parking lot 1
  int dis1 = ultra_sonic_1.GetDistance();
  int dis2 = ultra_sonic_2.GetDistance();
  int detect_park1 = upper_US_1.GetDistance();
  park1.work(dis1,dis2,LIGHT);

  //!parking lot 2
  int dis3 = ultra_sonic_3.GetDistance();
  int dis4 = ultra_sonic_4.GetDistance();
  int detect_park2 = upper_US_2.GetDistance();
  park2.work(dis3,dis4,LIGHT);
  
  //Serial.print("distance up 1 = " + String(detect_park1) + " / distance up 2 = " + String(detect_park1));
  Serial.print("light = " + String(LIGHT));
  delay(40);
  Serial.println("");
  display.display();
}
