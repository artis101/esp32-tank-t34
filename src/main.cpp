#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

// WebServer server(80);

// Motor control pins and PWM channels
const int motorLeftPin1 = 13;  // INT1 on the L298N for left motor
const int motorLeftPin2 = 15;  // INT2 on the L298N for left motor
const int motorRightPin1 = 16; // INT3 on the L298N for right motor
const int motorRightPin2 = 17; // INT4 on the L298N for right motor

// PWM parameters
const int freq = 5000;          // Frequency in Hz
const int pwmChannelLeft1 = 0;  // PWM channel for motorLeftPin1
const int pwmChannelLeft2 = 1;  // PWM channel for motorLeftPin2
const int pwmChannelRight1 = 2; // PWM channel for motorRightPin1
const int pwmChannelRight2 = 3; // PWM channel for motorRightPin2
const int resolution = 8; // Resolution in bits (1-16), 8-bit gives 0-255 range

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("tank-t34-esp32");
  // ArduinoOTA.setPassword("tankpassword");

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
        } else { // U_SPIFFS
          type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
      });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize motor control pins as outputs
  pinMode(motorLeftPin1, OUTPUT);
  pinMode(motorLeftPin2, OUTPUT);
  pinMode(motorRightPin1, OUTPUT);
  pinMode(motorRightPin2, OUTPUT);

  // Setup PWM channels
  ledcSetup(pwmChannelLeft1, freq, resolution);
  ledcSetup(pwmChannelLeft2, freq, resolution);
  ledcSetup(pwmChannelRight1, freq, resolution);
  ledcSetup(pwmChannelRight2, freq, resolution);

  // Attach PWM channels to the pins
  ledcAttachPin(motorLeftPin1, pwmChannelLeft1);
  ledcAttachPin(motorLeftPin2, pwmChannelLeft2);
  ledcAttachPin(motorRightPin1, pwmChannelRight1);
  ledcAttachPin(motorRightPin2, pwmChannelRight2);

  // Stop all motors
  ledcWrite(pwmChannelLeft1, 0);
  ledcWrite(pwmChannelLeft2, 0);
  ledcWrite(pwmChannelRight1, 0);
  ledcWrite(pwmChannelRight2, 0);
}

// void testMotors() {
//   // Left motor forward
//   ledcWrite(pwmChannelLeft1, 255); // Speed control
//   ledcWrite(pwmChannelLeft2, 0);

//   // Right motor forward
//   ledcWrite(pwmChannelRight1, 0); // Speed control
//   ledcWrite(pwmChannelRight2, 255);

//   delay(2000); // Run motors forward for 2 seconds

//   // Stop all motors
//   ledcWrite(pwmChannelLeft1, 0);
//   ledcWrite(pwmChannelLeft2, 0);
//   ledcWrite(pwmChannelRight1, 0);
//   ledcWrite(pwmChannelRight2, 0);

//   delay(1000); // Stop motors for 1 second

//   // Left motor backward
//   ledcWrite(pwmChannelLeft1, 0); // Speed control
//   ledcWrite(pwmChannelLeft2, 180);

//   // Right motor forward
//   ledcWrite(pwmChannelRight1, 0); // Speed control
//   ledcWrite(pwmChannelRight2, 180);

//   delay(1000); // Run motors forward for 2 seconds

//   // Stop all motors
//   ledcWrite(pwmChannelLeft1, 0);
//   ledcWrite(pwmChannelLeft2, 0);
//   ledcWrite(pwmChannelRight1, 0);
//   ledcWrite(pwmChannelRight2, 0);

//   delay(1000); // Stop motors for 1 second

//   // Left motor forward
//   ledcWrite(pwmChannelLeft1, 180); // Speed control
//   ledcWrite(pwmChannelLeft2, 0);

//   // Right motor backward
//   ledcWrite(pwmChannelRight1, 180); // Speed control
//   ledcWrite(pwmChannelRight2, 0);

//   delay(1000); // Run motors forward for 2 seconds

//   // Stop all motors
//   ledcWrite(pwmChannelLeft1, 0);
//   ledcWrite(pwmChannelLeft2, 0);
//   ledcWrite(pwmChannelRight1, 0);
//   ledcWrite(pwmChannelRight2, 0);

//   delay(1000); // Stop motors for 1 second

//   // Left motor backward
//   ledcWrite(pwmChannelLeft1, 0); // Speed control
//   ledcWrite(pwmChannelLeft2, 200);

//   // Right motor backward
//   ledcWrite(pwmChannelRight1, 200); // Speed control
//   ledcWrite(pwmChannelRight2, 0);

//   delay(2000); // Run motors backward for 2 seconds

//   // Stop all motors
//   ledcWrite(pwmChannelLeft1, 0);
//   ledcWrite(pwmChannelLeft2, 0);
//   ledcWrite(pwmChannelRight1, 0);
//   ledcWrite(pwmChannelRight2, 0);

//   delay(1000); // Stop motors for 1 second
// }

void loop() { ArduinoOTA.handle(); }
