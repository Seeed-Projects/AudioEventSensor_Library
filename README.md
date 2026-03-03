# AudioEventSensor Library

The `AudioEventSensor` library allows easy integration with an audio event sensor module (like the XU316) over UART, enabling event detection, threshold configuration, and more.

---

## Installation

1. **Download the library**:

   * Download the repository as a `.ZIP` file.
   * In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library**.
   * Select the `.ZIP` file you downloaded to install the library.

2. **Manual Installation**:

   * Download the repository.
   * Place the folder `AudioEventSensor` in your **Arduino libraries** folder:

     * **Windows**: `C:\Users\<your_username>\Documents\Arduino\libraries`
     * **macOS/Linux**: `~/Documents/Arduino/libraries`

---

## Functions Overview

### `AudioEventSensor(HardwareSerial &serial)`

* **Description**: Initializes the `AudioEventSensor` with the given serial port (e.g., `Serial1`).

### `void begin(uint32_t baud = 115200)`

* **Description**: Begins communication with the sensor at the specified baud rate.
* **Example**: `audio.begin(115200);`

### `bool available()`

* **Description**: Checks if the sensor has data available to read.
* **Returns**: `true` if data is available, `false` otherwise.

### `String readEvent()`

* **Description**: Reads and returns a formatted event string including the event name and confidence percentage (e.g., `"glass_break 78% confidence"`).
* **Returns**: A formatted string like `"glass_break 78% confidence"`, or an empty string if no valid event is available.

### `void clearBuffer()`

* **Description**: Clears any pending data in the serial receive buffer. Useful before sending a command to avoid reading stale responses.

### `bool getSupportedList(String &out)`

* **Description**: Queries the supported event types from the sensor.
* **Returns**: `true` if the list was fetched successfully, `false` otherwise.
* **Example output**: `"baby_cry,glass_break,gunshot,snore,T3,T4"`

### `bool getDetectTypes(String &out)`

* **Description**: Retrieves the event types currently being detected by the sensor.
* **Returns**: `true` if successfully fetched, `false` otherwise.
* **Example output**: `"gunshot,glass_break"`

### `bool setDetectTypes(const String &types)`

* **Description**: Sets the event types to detect (comma-separated).
* **Returns**: `true` if the types were set successfully, `false` otherwise.
* **Example**: `audio.setDetectTypes("gunshot,glass_break");`

### `bool getEventThreshold(const String &type, int &threshold)`

* **Description**: Retrieves the detection threshold for a specific event type.
* **Returns**: `true` if successful, `false` otherwise.
* **Example**: `audio.getEventThreshold("glass_break", threshold);` → `threshold = 50`

### `bool setEventThreshold(const String &type, int threshold)`

* **Description**: Sets the detection threshold (0–100) for a specific event type.
* **Returns**: `true` if the threshold was set successfully, `false` otherwise.
* **Example**: `audio.setEventThreshold("glass_break", 60);`

### `bool setOutputType(const String &type)`

* **Description**: Sets the output format type of the sensor.
* **Returns**: `true` if successful, `false` otherwise.

### `bool setIntMode(const String &mode)`

* **Description**: Sets the interrupt mode of the sensor.
* **Returns**: `true` if successful, `false` otherwise.

### `bool getIntMode(String &out)`

* **Description**: Retrieves the current interrupt mode of the sensor.
* **Returns**: `true` if successful, `false` otherwise.

### `bool resetInterrupt()`

* **Description**: Resets the interrupt signal on the sensor.
* **Returns**: `true` if successful, `false` otherwise.

### `bool saveConfig()`

* **Description**: Saves the current configuration (detect types, thresholds, etc.) to the sensor's persistent storage so settings survive a reboot.
* **Returns**: `true` if saved successfully, `false` otherwise.

### `bool resetDevice()`

* **Description**: Resets the sensor to its default settings and reboots it. Includes internal delays to allow the device to come back online.
* **Returns**: `true` if the reset command was sent successfully, `false` otherwise.

### `bool getFirmwareVersion(String &version)`

* **Description**: Retrieves the firmware version string from the sensor.
* **Returns**: `true` if successful, `false` otherwise.
* **Example output**: `"1.0.0"`

---

## Example Usage

```cpp
#include <AudioEventSensor.h>

#define UART1_TX 43
#define UART1_RX 44

AudioEventSensor audio(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
  audio.begin(115200);

  String supportedEvents;
  if (audio.getSupportedList(supportedEvents)) {
    Serial.println("Supported Events: " + supportedEvents);
  }

  audio.setDetectTypes("gunshot,glass_break");
  String currentDetectTypes;
  if (audio.getDetectTypes(currentDetectTypes)) {
    Serial.println("Currently Detecting: " + currentDetectTypes);
  }
}

void loop() {
  if (audio.available()) {
    String event = audio.readEvent();
    if (event.length() > 0) {
      Serial.println("Detected Event: " + event);
    }
  }
  delay(500);
}
```

---



This library is released under the MIT License.

---
