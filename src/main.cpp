#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUDP.h>

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

WiFiUDP udp;
const int udpPort = 4210; // UDP port to listen on

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setupOTA() {
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
}

void setupUDP() {
  udp.begin(udpPort);
  Serial.print("UDP Server started on port ");
  Serial.println(udpPort);
}

void setup() {
  Serial.begin(115200);

  setupWiFi();
  setupOTA();
  setupUDP();

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

void stop() {
  ledcWrite(pwmChannelLeft1, 0);
  ledcWrite(pwmChannelLeft2, 0);
  ledcWrite(pwmChannelRight1, 0);
  ledcWrite(pwmChannelRight2, 0);
}

void leftMotor(int speed) {
  if (speed > 0) {
    ledcWrite(pwmChannelLeft1, speed);
    ledcWrite(pwmChannelLeft2, 0);
    ledcWrite(pwmChannelRight1, 0);
    ledcWrite(pwmChannelRight2, speed);
  } else {
    ledcWrite(pwmChannelLeft1, 0);
    ledcWrite(pwmChannelLeft2, speed * -1);
    ledcWrite(pwmChannelRight1, speed * -1);
    ledcWrite(pwmChannelRight2, 0);
  }
}

void rightMotor(int speed) {
  if (speed > 0) {
    ledcWrite(pwmChannelLeft1, 0);
    ledcWrite(pwmChannelLeft2, speed);
    ledcWrite(pwmChannelRight1, speed);
    ledcWrite(pwmChannelRight2, 0);
  } else {
    ledcWrite(pwmChannelLeft1, speed * -1);
    ledcWrite(pwmChannelLeft2, 0);
    ledcWrite(pwmChannelRight1, 0);
    ledcWrite(pwmChannelRight2, speed * -1);
  }
}

void handleUDP() {
  int packetSize = udp.parsePacket();

  if (packetSize) {
    // buffer to hold incoming packet,
    char packetBuffer[255];
    udp.read(packetBuffer, 255);
    String data = String(packetBuffer);
    Serial.print("Received packet: ");
    Serial.println(data);

    // Parse data to extract motor control values, e.g., "L150,R200" and
    // "L-100,R-100"
    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) {
      return;
    }

    int leftSpeed = data.substring(1, commaIndex).toInt();
    int rightSpeed = data.substring(commaIndex + 2).toInt();

    // Control motors based on extracted values
    switch (data[0]) {
    case 'S':
      stop();
      break;
    case 'L':
      leftMotor(leftSpeed);
      break;
    case 'R':
      rightMotor(rightSpeed);
      break;
    default:
      stop();
      break;
    }
  }
}

void loop() {
  ArduinoOTA.handle();
  handleUDP();
}
