#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <VL53L0X.h>

const char* ssid = "SSID"; // Your WiFi SSID
const char* password = "PASSWORD"; // Your WiFi password
const char* host = "smtp.gmail.com"; // SMTP server address
const int smtpPort = 465; // SMTP port
const char* emailSender = "YOUR_EMAIL"; // Your email address
const char* emailSenderPassword = "YOUR_EMAIL_PASSWORD"; // Your email password
const char* recipientEmail = "RECIPIENT_EMAIL"; // Recipient's email address
const char* subjectSoilMoisture = "Low Soil Moisture Alert";
const char* subjectIntrusion = "Intrusion Detected";

const int soilMoisturePin = 5; // Soil moisture sensor pin
const int relayPin = 6; // Relay pin
const int lidarInterruptPin = D3; // Interrupt pin for Lidar sensor

VL53L0X lidar;
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  pinMode(soilMoisturePin, INPUT);
  pinMode(relayPin, OUTPUT);

  // Initialize Lidar sensor
  Wire.begin();
  lidar.init();
  lidar.setTimeout(500);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected");
}

void loop() {
  int soilMoisture = analogRead(soilMoisturePin);

  // Check soil moisture level
  if (soilMoisture < 500) { // Adjust threshold according to your sensor's calibration
    // Turn on relay (water pump)
    digitalWrite(relayPin, HIGH);

    // Send email notification
    sendEmail(subjectSoilMoisture, "Soil moisture level is low. Water pump turned on.");
  } else {
    // Turn off relay (water pump)
    digitalWrite(relayPin, LOW);
  }

  // Check for intrusion using Lidar sensor
  if (digitalRead(lidarInterruptPin)) {
    // Send email notification
    sendEmail(subjectIntrusion, "Intrusion detected in the farm.");
    // Wait for some time to avoid multiple notifications
    delay(5000);
  }

  delay(10000); // Adjust delay time according to your requirements
}

void sendEmail(const char* subject, const char* body) {
  if (client.connect(host, smtpPort)) {
    Serial.println("Connected to server");
    client.println("EHLO example.com");
    if (client.waitUntilConnected()) {
      Serial.println("Sending HELO");
      client.println("HELO example.com");
      if (client.waitUntilConnected()) {
        Serial.println("Sending AUTH LOGIN");
        client.println("AUTH LOGIN");
        if (client.waitUntilConnected()) {
          Serial.println("Sending username");
          client.println(base64::encode(emailSender));
          if (client.waitUntilConnected()) {
            Serial.println("Sending password");
            client.println(base64::encode(emailSenderPassword));
            if (client.waitUntilConnected()) {
              Serial.println("Sending MAIL FROM");
              client.println("MAIL FROM:<" + String(emailSender) + '>');
              if (client.waitUntilConnected()) {
                Serial.println("Sending RCPT TO");
                client.println("RCPT TO:<" + String(recipientEmail) + '>');
                if (client.waitUntilConnected()) {
                  Serial.println("Sending DATA");
                  client.println("DATA");
                  if (client.waitUntilConnected()) {
                    Serial.println("Sending email");
                    client.println("From: <" + String(emailSender) + '>');
                    client.println("To: <" + String(recipientEmail) + '>');
                    client.println("Subject: " + String(subject));
                    client.println(body);
                    client.println(".");
                    if (client.waitUntilConnected()) {
                      Serial.println("Email sent");
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } else {
    Serial.println("Failed to connect to server");
  }
  client.stop();
}
