#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long period = 5000; // wait time
unsigned long last_time = 0; // time stamp

enum states
{
  idle,
  detected,
  reset,
  check
};
states state;
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

void RGB(byte R, byte G, byte B)
{
  // Common Anode
  digitalWrite(A1, R);
  digitalWrite(A2, G);
  digitalWrite(A3, B);
}

void setup()
{
  Serial.begin(9600); // Starts the serial communication
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  ultra_sonic_1.begin(10, 9);
  ultra_sonic_2.begin(12, 13);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(200); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(WHITE);
  state = idle;
  RGB(1, 1, 1);
}
void loop()
{
  display.setTextSize(2);
  display.setCursor(0, 0);
  int dis1 = ultra_sonic_1.GetDistance();
  int dis2 = ultra_sonic_2.GetDistance();
  char park_lot_1[] = {dis1, dis2};
  int light = map(analogRead(A0), 0, 1024, 100, 0);
  Serial.print("distance 1 = " + String(dis1) + " / distance 2 = " + String(dis2));
  switch (state)
  {
  case idle:
    if (light <= 25)
    {
      RGB(0, 0, 0);
      if (dis1 <= 15 || dis2 <= 15)
      {
        state = detected;
      }
    }
    else
    {
      if (millis() - last_time > period)
      {
        RGB(1, 1, 1);
        last_time = millis(); // time stamp
      }
      if (dis1 <= 15 || dis2 <= 15)
      {
        state = detected;
      }
    }
    break;
  case detected:
    RGB(1, 1, 0);
    state = check;
    break;
  case check:
    if (dis1 <= 15 || dis2 <= 15)
    {
      state = detected;
      display.print("Parked not properly");
    }
    else
    {
      RGB(0, 1, 1);
      state = reset;
    }
    break;
  case reset:
    last_time = millis();
    state = idle;
    display.clearDisplay();
    break;
  }
  delay(40);
  Serial.println("");
  display.display();
}
