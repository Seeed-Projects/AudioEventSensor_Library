#include <AudioEventSensor.h>

#define UART1_TX 43
#define UART1_RX 44

AudioEventSensor audio(Serial1);

void setup() {
  // Start the serial communication for both USB and UART1
  Serial.begin(115200);
  delay(1000); // Wait for Serial to be ready
  
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  // Initialize the sensor
  audio.begin(115200);

  Serial.println("Audio Event Sensor Test Script");
  Serial.println("Waiting for device to be ready...");
  delay(2000); // Give device time to stabilize

  // -------------------------
  // Optional: Reset device at start for clean state
  // -------------------------
  Serial.println("Resetting device...");
  audio.resetDevice(); 
  // resetDevice() now includes proper delays internally
  Serial.println("Device reset complete.");

  // -------------------------
  // Get and print firmware version (good first test)
  // -------------------------
  String firmwareVersion;
  if (audio.getFirmwareVersion(firmwareVersion)) {
    Serial.print("Firmware version: ");
    Serial.println(firmwareVersion);
  } else {
    Serial.println("Failed to get firmware version.");
  }
  delay(500);

  // -------------------------
  // Get and print supported events
  // -------------------------
  String supportedEvents;
  if (audio.getSupportedList(supportedEvents)) {
    Serial.println("Supported Events:");
    Serial.println(supportedEvents);
  } else {
    Serial.println("Failed to get supported events.");
  }
  delay(500);

  // -------------------------
  // Set detection types (example: gunshot, glass_break)
  // -------------------------
  Serial.println("Setting detection types...");
  if (audio.setDetectTypes("gunshot,glass_break")) {
    Serial.println("Detection types set to: gunshot, glass_break");
  } else {
    Serial.println("Failed to set detection types.");
  }
  delay(500);

  // -------------------------
  // Verify detection types were set
  // -------------------------
  String currentDetectTypes;
  if (audio.getDetectTypes(currentDetectTypes)) {
    Serial.println("Currently detecting:");
    Serial.println(currentDetectTypes);
  } else {
    Serial.println("Failed to get detection types.");
  }
  delay(500);

  // -------------------------
  // Set threshold for glass_break
  // -------------------------
  Serial.println("Setting threshold for glass_break...");
  if (audio.setEventThreshold("glass_break", 60)) {
    Serial.println("Threshold for glass_break set to 60.");
  } else {
    Serial.println("Failed to set event threshold.");
  }
  delay(500);

  // -------------------------
  // Verify threshold was set
  // -------------------------
  int threshold;
  if (audio.getEventThreshold("glass_break", threshold)) {
    Serial.print("Verified threshold for glass_break: ");
    Serial.println(threshold);
  } else {
    Serial.println("Failed to get event threshold.");
  }
  delay(500);

  // -------------------------
  // Set threshold for gunshot
  // -------------------------
  Serial.println("Setting threshold for gunshot...");
  if (audio.setEventThreshold("gunshot", 65)) {
    Serial.println("Threshold for gunshot set to 65.");
  } else {
    Serial.println("Failed to set gunshot threshold.");
  }
  delay(500);

  // -------------------------
  // Save configuration to persist settings
  // -------------------------
  Serial.println("Saving configuration...");
  if (audio.saveConfig()) {
    Serial.println("Configuration saved successfully.");
  } else {
    Serial.println("Failed to save configuration.");
  }
  delay(500);

  Serial.println("\nSetup complete. Monitoring for events...\n");
}

void loop() {
  // Continuously check for events and print to serial
  if (audio.available()) {
    String event = audio.readEvent();
    if (event.length() > 0) {
      Serial.print("Detected Event: ");
      Serial.println(event);
    }
  }

  delay(100); // Reduced from 500ms for faster response
}
