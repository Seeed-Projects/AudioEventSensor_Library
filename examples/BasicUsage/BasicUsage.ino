#include <AudioEventSensor.h>

#define UART1_TX 43
#define UART1_RX 44

AudioEventSensor audio(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  audio.begin(115200);

  Serial.println("Audio Event Sensor Ready");

  // Example: set detection types
  audio.setDetectTypes("gunshot,glass_break");

  // Example: set threshold
  audio.setEventThreshold("glass_break", 50);
}

void loop() {
  if (audio.available()) {
    String evt = audio.readEvent();
    if (evt.length() > 0) {
      Serial.print("Detected: ");
      Serial.println(evt);   // Already formatted: "glass_break 78% confidence"
    }
  }
}
