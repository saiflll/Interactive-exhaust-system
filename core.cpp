#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define GAS_SENSOR A0
#define FAN_PIN 3

#define CLK 4
#define DT 5
#define SW 6

int gasThreshold = 300;
int tempThreshold = 40;

int lastClkState;
bool buttonPressed = false;
int menuIndex = 0;

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  lastClkState = digitalRead(CLK);

  Serial.begin(9600);
  dht.begin();

  EEPROM.get(0, tempThreshold);
  EEPROM.get(4, gasThreshold);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  encoderHandler();

  float temp = dht.readTemperature();
  int gasValue = analogRead(GAS_SENSOR);

  bool gasDanger = gasValue > gasThreshold;
  bool tempHigh = temp > tempThreshold;

  if (menuIndex == 0) {
    if (gasDanger || tempHigh) {
      digitalWrite(FAN_PIN, HIGH);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }

    // Tampilan utama
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: "); display.print(temp); display.println(" C");
    display.print("Gas : "); display.println(gasValue);
    display.print("Fan : ");
    display.println((gasDanger || tempHigh) ? "ON" : "OFF");
    display.print("T.Th: "); display.println(tempThreshold);
    display.print("G.Th: "); display.println(gasThreshold);
    display.display();
  } else if (menuIndex == 1) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Set Temp Threshold:");
    display.print("Current: "); display.print(tempThreshold); display.println(" C");
    display.display();
  } else if (menuIndex == 2) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Set Gas Threshold:");
    display.print("Current: "); display.println(gasThreshold);
    display.display();
  }

  delay(100);
}

void encoderHandler() {
  int clkState = digitalRead(CLK);
  int dtState = digitalRead(DT);

  // Rotasi
  if (clkState != lastClkState) {
    if (dtState != clkState) {
      // Kanan
      if (menuIndex == 1) tempThreshold++;
      if (menuIndex == 2) gasThreshold += 10;
    } else {
      // Kiri
      if (menuIndex == 1) tempThreshold--;
      if (menuIndex == 2) gasThreshold -= 10;
    }
    EEPROM.put(0, tempThreshold);
    EEPROM.put(4, gasThreshold);
  }
  lastClkState = clkState;

  // Tombol tekan
  if (digitalRead(SW) == LOW) {
    if (!buttonPressed) {
      menuIndex = (menuIndex + 1) % 3;  // 0: normal, 1: temp, 2: gas
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }
}
