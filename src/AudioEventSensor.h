#pragma once
#include <Arduino.h>

class AudioEventSensor {
public:
    AudioEventSensor(HardwareSerial &serial);

    void begin(uint32_t baud=115200);
    bool available();
    String readEvent();             // Reads: +EVENT: <id>,<confidence>
    void clearBuffer();             // NEW: Clear serial buffer

    // ---- Basic Commands ----
    bool getDetectTypes(String &out);
    bool setDetectTypes(const String &types);

    bool getEventThreshold(const String &type, int &threshold);
    bool setEventThreshold(const String &type, int threshold);

    bool getSupportedList(String &out);

    bool setOutputType(const String &type);
    bool setIntMode(const String &mode);
    bool getIntMode(String &modeOut);

    bool resetInterrupt();
    bool saveConfig();
    bool resetDevice();

    bool getFirmwareVersion(String &version);

private:
    HardwareSerial *_serial;
    String readLine(uint32_t timeout=500); // Increased default timeout
    bool expectOK(const String &cmd, uint32_t timeout=500);
};
