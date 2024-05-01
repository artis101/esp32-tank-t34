#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUDP.h>

// LEDs
const int ledPin = 22; // Built-in LED on ESP32
bool blinkLed = false;
unsigned long lastBlinkTime = 0;

// Status bar LED's
const int statusBarLed1 = 2;
const int statusBarLed2 = 0;
const int statusBarLed3 = 4;

// Turret LED's
const int turretBarrelLed = 34;

// Motor control pins and PWM channels
const int motorLeftPin1 = 13;  // INT1 on the L298N for left motor
const int motorLeftPin2 = 15;  // INT2 on the L298N for left motor
const int motorRightPin1 = 16; // INT3 on the L298N for right motor
const int motorRightPin2 = 17; // INT4 on the L298N for right motor

// Turret motor control pins
const int turretMotorPin1 = 25; // INT1 on the second L298N for turret motor
const int turretMotorPin2 = 26; // INT2 on the second L298N for turret motor

// PWM parameters
const int freq = 5000;           // Frequency in Hz
const int pwmChannelLeft1 = 0;   // PWM channel for motorLeftPin1
const int pwmChannelLeft2 = 1;   // PWM channel for motorLeftPin2
const int pwmChannelRight1 = 2;  // PWM channel for motorRightPin1
const int pwmChannelRight2 = 3;  // PWM channel for motorRightPin2
const int pwmChannelTurret1 = 4; // PWM channel for turretMotorPin1
const int pwmChannelTurret2 = 5; // PWM channel for turretMotorPin2
const int resolution = 8; // Resolution in bits (1-16), 8-bit gives 0-255 range

WiFiUDP udp;
const int udpPort = 4210; // UDP port to listen on

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(statusBarLed1, !digitalRead(statusBarLed1));
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // turn on LED on successful connection
  digitalWrite(ledPin, LOW);
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

  pinMode(ledPin, OUTPUT);

  // Initialize status bar LEDs
  pinMode(statusBarLed1, OUTPUT);
  pinMode(statusBarLed2, OUTPUT);
  pinMode(statusBarLed3, OUTPUT);

  // turn off status bar LEDs initially
  digitalWrite(statusBarLed1, HIGH);
  digitalWrite(statusBarLed2, HIGH);
  digitalWrite(statusBarLed3, HIGH);

  // initialize turret barrel LED
  pinMode(turretBarrelLed, OUTPUT);
  digitalWrite(turretBarrelLed, LOW);

  // Initialize motor control pins as outputs
  pinMode(motorLeftPin1, OUTPUT);
  pinMode(motorLeftPin2, OUTPUT);
  pinMode(motorRightPin1, OUTPUT);
  pinMode(motorRightPin2, OUTPUT);
  pinMode(turretMotorPin1, OUTPUT);
  pinMode(turretMotorPin2, OUTPUT);

  // Setup PWM channels
  ledcSetup(pwmChannelLeft1, freq, resolution);
  ledcSetup(pwmChannelLeft2, freq, resolution);
  ledcSetup(pwmChannelRight1, freq, resolution);
  ledcSetup(pwmChannelRight2, freq, resolution);
  ledcSetup(pwmChannelTurret1, freq, resolution);
  ledcSetup(pwmChannelTurret2, freq, resolution);

  // Attach PWM channels to the pins
  ledcAttachPin(motorLeftPin1, pwmChannelLeft1);
  ledcAttachPin(motorLeftPin2, pwmChannelLeft2);
  ledcAttachPin(motorRightPin1, pwmChannelRight1);
  ledcAttachPin(motorRightPin2, pwmChannelRight2);
  ledcAttachPin(turretMotorPin1, pwmChannelTurret1);
  ledcAttachPin(turretMotorPin2, pwmChannelTurret2);

  // Stop all motors
  ledcWrite(pwmChannelLeft1, 0);
  ledcWrite(pwmChannelLeft2, 0);
  ledcWrite(pwmChannelRight1, 0);
  ledcWrite(pwmChannelRight2, 0);
  ledcWrite(pwmChannelTurret1, 0);
  ledcWrite(pwmChannelTurret2, 0);

  setupWiFi();
  setupOTA();
  setupUDP();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // setup done, turn on status bar LED 2
  digitalWrite(statusBarLed2, LOW);
}

void stop() {
  ledcWrite(pwmChannelLeft1, 0);
  ledcWrite(pwmChannelLeft2, 0);
  ledcWrite(pwmChannelRight1, 0);
  ledcWrite(pwmChannelRight2, 0);
  ledcWrite(pwmChannelTurret1, 0);
  ledcWrite(pwmChannelTurret2, 0);
  blinkLed = false;
}

void leftMotor(int speed) {
  if (speed > 0) {
    ledcWrite(pwmChannelLeft1, speed);
    ledcWrite(pwmChannelLeft2, 0);
  } else {
    ledcWrite(pwmChannelLeft1, 0);
    ledcWrite(pwmChannelLeft2, speed * -1);
  }

  blinkLed = speed != 0;
}

void rightMotor(int speed) {
  if (speed > 0) {
    ledcWrite(pwmChannelRight1, 0);
    ledcWrite(pwmChannelRight2, speed);
  } else {
    ledcWrite(pwmChannelRight1, speed * -1);
    ledcWrite(pwmChannelRight2, 0);
  }

  blinkLed = speed != 0;
}

void turretMotor(int speed) {
  if (speed > 0) {
    ledcWrite(pwmChannelTurret1, 0);
    ledcWrite(pwmChannelTurret2, speed);
  } else {
    ledcWrite(pwmChannelTurret1, speed * -1);
    ledcWrite(pwmChannelTurret2, 0);
  }

  blinkLed = speed != 0;
}

void handleUDP() {
  int packetSize = udp.parsePacket();

  if (packetSize) {
    // Buffer to hold incoming packet
    char packetBuffer[255];
    udp.read(packetBuffer, 255);
    String data = String(packetBuffer);
    data.trim(); // Trim any whitespace or newline characters
    Serial.print("Received packet: ");
    Serial.println(data);

    if (data[0] == 'S') {
      stop();
      return;
    }

    if (data[0] == 'F') {
      digitalWrite(statusBarLed3, !digitalRead(statusBarLed3));
      digitalWrite(turretBarrelLed, !digitalRead(turretBarrelLed));
      return;
    }

    int commaIndex = data.indexOf(',');
    int leftSpeed = 0;
    int rightSpeed = 0;

    // Parse left and right speeds from data
    if (commaIndex != -1) {
      // Command contains speeds for both motors
      String leftPart = data.substring(1, commaIndex);
      String rightPart = data.substring(commaIndex + 2);
      leftSpeed = leftPart.toInt();
      rightSpeed = rightPart.toInt();

      leftMotor(leftSpeed);
      rightMotor(rightSpeed);
    } else {
      // Command contains speed for one motor
      if (data[0] == 'L') {
        leftSpeed = data.substring(1).toInt();
        leftMotor(leftSpeed);
      } else if (data[0] == 'R') {
        rightSpeed = data.substring(1).toInt();
        rightMotor(rightSpeed);
      } else if (data[0] == 'T') {
        int turretSpeed = data.substring(1).toInt();
        turretMotor(turretSpeed);
      } else {
        Serial.println("Invalid command");
        stop();
      }
    }
  }
}

void handleBlink() {
  if (blinkLed) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= 500) {
      lastBlinkTime = currentTime;
      digitalWrite(ledPin, !digitalRead(ledPin));

      // Toggle status bar LEDs 2 and 3
      digitalWrite(statusBarLed2, !digitalRead(statusBarLed2));
      digitalWrite(statusBarLed3, !digitalRead(statusBarLed3));
    }
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void handleWifiStatusLED() {
  // blink status bar LED 1 when no WiFi, turn on when connected
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() % 1000 < 500) {
      digitalWrite(statusBarLed1, !digitalRead(statusBarLed1));
    }
  } else {
    digitalWrite(statusBarLed1, LOW);
  }
}

void loop() {
  ArduinoOTA.handle();
  handleUDP();
  handleBlink();
  handleWifiStatusLED();
}
