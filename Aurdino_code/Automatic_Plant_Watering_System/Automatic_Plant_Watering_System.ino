#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensor Pins
const int moisturePin = A0;
const int ldrPin = A1;
const int trigPin = 10;
const int echoPin = 11;

// Output Pins
const int motorRelayPin = 7;    // IN1 of relay (Motor)
const int ledRelayPin = 6;      // IN2 of relay (12V LED strip)
const int buzzerPin = 8;

// Thresholds
const int moistureThreshold = 600; // adjust as needed
const int lightThreshold = 300;
const int tankEmptyThreshold = 13; // cm
const int maxTankHeight = 15;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(motorRelayPin, OUTPUT);
  pinMode(ledRelayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(motorRelayPin, HIGH);  // motor OFF
  digitalWrite(ledRelayPin, HIGH);    // LED OFF
  digitalWrite(buzzerPin, LOW);       // buzzer OFF

  Serial.println("🌱 Automated Plant Watering System Initialized 🌱");
}

long readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if (duration == 0) {
    Serial.println("⚠️ Invalid ultrasonic reading!");
    return -1;
  }

  return duration * 0.034 / 2;
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ Failed to read from DHT sensor!");
    temp = 0;
    hum = 0;
  }

  int soilValue = analogRead(moisturePin);
  int lightValue = analogRead(ldrPin);
  long waterDistance = readDistanceCM();

  int levelCM = (waterDistance >= 0) ? maxTankHeight - waterDistance : 0;
  int waterLevelPercent = constrain((levelCM * 100) / maxTankHeight, 0, 100);

  // Display sensor readings
  Serial.println("------------ Sensor Data ------------");
  Serial.print("Temperature: "); Serial.print(temp); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
  Serial.print("Soil Moisture: "); Serial.println(soilValue);
  Serial.print("Light Level: "); Serial.println(lightValue);
  Serial.print("Water Level: "); Serial.print(waterLevelPercent); Serial.println(" %");

  // Light logic (info only)
  if (lightValue < lightThreshold) {
    Serial.println("It's dark, LED ON.");
  }

  // Watering Logic
  if (soilValue > moistureThreshold) { // Soil is dry
    if (waterDistance >= 0 && waterDistance < tankEmptyThreshold) { // Water available
      Serial.println("💧 Soil is dry. Pumping water...");

      digitalWrite(motorRelayPin, LOW);   // Motor ON
      digitalWrite(ledRelayPin, LOW);     // LED ON
      delay(10000);                       // Water for 10 seconds

      digitalWrite(motorRelayPin, HIGH);  // Motor OFF
      digitalWrite(ledRelayPin, HIGH);    // LED OFF
    } else {
      Serial.println("⚠️ Water tank is empty!");
      digitalWrite(motorRelayPin, HIGH);   // Motor OFF
      digitalWrite(ledRelayPin, HIGH);     // LED OFF
      digitalWrite(buzzerPin, HIGH);       // Buzzer ON
      delay(5000);                         // Alert for 5 seconds
      digitalWrite(buzzerPin, LOW);        // Buzzer OFF
    }
  } else {
    Serial.println("✅ Soil is wet. No watering needed.");
    digitalWrite(motorRelayPin, HIGH);    // Motor OFF
    digitalWrite(ledRelayPin, HIGH);      // LED OFF
    digitalWrite(buzzerPin, LOW);         // Buzzer OFF
  }

  Serial.println("-------------------------------------\n");
  delay(5000); // 5 seconds between checks
}
