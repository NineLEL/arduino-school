#include <DHT.h>
// #include <LiquidCrystal_I2C.h>

// กำหนดขาเซนเซอร์และอุปกรณ์
const int soilMoisturePin = A5; // ขาความชื้นดิน
const int ledPin = LED_BUILTIN; // ไฟ LED
const int waterPumpPin = 7; // ขาปั๊มน้ำ
const int fertilizerPin = 8; // ขาปั๊มปุ๋ย

// DHT configuration
#define DHTTYPE DHT11 // ระบุประเภทเซนเซอร์เป็น DHT11
#define DHTPIN 2     // พินที่ต่อกับ DHT
DHT dht(DHTPIN, DHTTYPE);

// LCD configuration
// LiquidCrystal_I2C lcd(0x27, 16, 2);

// เกณฑ์และการตั้งค่า
unsigned long lastWaterTime = 0;
unsigned long lastFertilizeTime = 0;
const int moistureThreshold = 512; // 50% moisture (in ADC) - เกณฑ์ความชื้นดินที่ต้องการ
const int temperatureThreshold = 35; // 35°C - เกณฑ์อุณหภูมิที่ต้องการ
const unsigned long waterInterval = 10800000; // 3 ชั่วโมง (10800000 ms) - เวลาที่รดน้ำ
const unsigned long fertilizeInterval = 432000000; // 5 วัน (432000000 ms) - เวลาที่ใช้ในการให้ปุ๋ย
const unsigned long pumpDuration = 10000; // ปั๊มน้ำทำงาน 10000 วินาที - ระยะเวลาที่ปั๊มน้ำจะทำงาน

unsigned long lastFertilized = 0; // เก็บเวลาการให้ปุ๋ยล่าสุด
unsigned long pumpStartTime = 0; // เก็บเวลาเริ่มปั๊มน้ำ
bool pumpActive = false; // สถานะปั๊มน้ำ
int wateringCount = 0; // จำนวนครั้งที่รดน้ำไป

// ฟังก์ชันอ่านค่าความชื้นดิน
void readSoilMoisture(float &moisturePercent) {
  int moistureValue = analogRead(soilMoisturePin); // อ่านค่าความชื้นดิน
  moisturePercent = map(moistureValue, 0, 1023, 0, 100); // แปลง ADC เป็นเปอร์เซ็นต์
  // moisturePercent = 49;
}

// ฟังก์ชันอ่านค่า DHT
void readDHT(float &temperature) {
  // อ่านค่าอุณหภมิและความชื้นจาก DHT11
  temperature = dht.readTemperature();
  // temperature = 34;

  // ตรวจสอบความถูกต้องของค่าที่อ่านได้
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor");
    temperature = -1;
  }
}

void controlWaterPump(float moisturePercent, float temperature) {
  static unsigned long pumpStartTime = 0; // Time when the pump starts
  static bool pumpActive = true; // Status of the pump

  // Check if it's time to water (every 3 hours) and the pump is not already active
  if (!pumpActive && millis() - lastWaterTime >= waterInterval) {
    if (moisturePercent < 60 && temperature <= temperatureThreshold) {
      Serial.println("Starting water pump for 5 seconds...");
      digitalWrite(waterPumpPin, LOW); // Turn the pump ON
      pumpStartTime = millis(); // Record the start time
      pumpActive = true; // Mark the pump as active
      wateringCount++; // Increment watering count
    } else {
      // Conditions not met, update the last check time
      lastWaterTime = millis();
    }
  }

  // Stop the pump after 5 seconds
  if (pumpActive && millis() - pumpStartTime >= pumpDuration) {
    Serial.println("Stopping water pump after 5 seconds...");
    digitalWrite(waterPumpPin, HIGH); // Turn the pump OFF
    pumpActive = false; // Mark the pump as inactive
    lastWaterTime = millis(); // Update the last watering time
  }

  // Display pump status
  Serial.print("Pump Active Status: ");
  Serial.println(pumpActive ? "True" : "False");
}


void fertilize(float moisturePercent, float temperature) {
  static unsigned long fertStartTime = 0; // Time when the fertilizer pump starts
  static bool fertActive = true; // Status of the fertilizer pump

  // Check if it's time to fertilize (every 5 days) and the pump is not already active
  if (!fertActive && millis() - lastFertilized >= fertilizeInterval) {
    if (moisturePercent < moistureThreshold && temperature <= temperatureThreshold) {
      Serial.println("Starting fertilizer pump for 5 seconds...");
      digitalWrite(fertilizerPin, LOW); // Turn the fertilizer pump ON
      fertStartTime = millis(); // Record the start time
      fertActive = true; // Mark the pump as active
    } else {
      // Conditions not met, update the last check time
      lastFertilized = millis();
    }
  }

  // Stop the fertilizer pump after 5 seconds
  if (fertActive && millis() - fertStartTime >= pumpDuration) {
    Serial.println("Stopping fertilizer pump after 5 seconds...");
    digitalWrite(fertilizerPin, HIGH); // Turn the fertilizer pump OFF
    fertActive = false; // Mark the pump as inactive
    lastFertilized = millis(); // Update the last fertilizing time
  }

  // Display fertilizer pump status
  Serial.print("Fertilizer Pump Active Status: ");
  Serial.println(fertActive ? "True" : "False");
}


// ฟังก์ชันแสดงข้อมูลบน LCD
// void displayLCD(float temperature, float moisturePercent, int wateringCount) {
//   lcd.clear();
//   lcd.setCursor(0, 0); // แสดงข้อมูลบรรทัดแรก
//   lcd.print("Temp: ");
//   lcd.print(temperature);
//   lcd.print("C");

//   lcd.setCursor(0, 1); // แสดงข้อมูลบรรทัดที่สอง
//   lcd.print("M: ");
//   lcd.print(moisturePercent);
//   lcd.print("% ");
//   lcd.print("N: ");
//   lcd.print(wateringCount);
// }

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(waterPumpPin, OUTPUT);
  pinMode(fertilizerPin, OUTPUT);

  Serial.begin(9600);
  digitalWrite(waterPumpPin, LOW);
  digitalWrite(fertilizerPin, LOW);
  dht.begin(); // เริ่มต้นเซนเซอร์ DHT
}

void loop() {
  float moisturePercent, temperature;
  // int wateringCount;
  readSoilMoisture(moisturePercent);
  readDHT(temperature);

  Serial.println("--------------------");
  Serial.print("Moisture Percent: ");
  Serial.println(moisturePercent);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // การรดน้ำ
  controlWaterPump(moisturePercent, temperature);

  // การให้ปุ๋ย
  fertilize(moisturePercent, temperature);
  delay(4000);
}
