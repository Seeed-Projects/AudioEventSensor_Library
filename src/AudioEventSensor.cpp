#include "AudioEventSensor.h"

// -------- EVENT LABEL MAP ----------
struct EventLabel {
    int id;
    const char* name;
};

EventLabel LABEL_MAP[] = {
    {1, "baby_cry"},
    {2, "glass_break"},
    {3, "gunshot"},
    {4, "snore"},
    {5, "T3"},
    {6, "T4"}
};
const int LABEL_COUNT = 6;

String getEventNameFromID(int id) {
    for (int i = 0; i < LABEL_COUNT; i++) {
        if (LABEL_MAP[i].id == id) return LABEL_MAP[i].name;
    }
    return "unknown";
}

AudioEventSensor::AudioEventSensor(HardwareSerial &serial) {
    _serial = &serial;
}

void AudioEventSensor::begin(uint32_t baud) {
    _serial->begin(baud);
    delay(100); // Give serial time to initialize
    clearBuffer(); // Clear any startup garbage
}

bool AudioEventSensor::available() {
    return _serial->available();
}

// NEW: Clear serial buffer
void AudioEventSensor::clearBuffer() {
    while (_serial->available()) {
        _serial->read();
    }
}

String AudioEventSensor::readEvent() {
    if (!_serial->available()) return "";

    String line = _serial->readStringUntil('\n');
    line.trim();

    if (!line.startsWith("+EVENT:")) {
        return "";
    }

    // Example raw format:  +EVENT: 2,0.78
    line.remove(0, 7);  // Remove "+EVENT:"
    line.trim();

    int commaIndex = line.indexOf(',');
    if (commaIndex == -1) return "";

    int eventID = line.substring(0, commaIndex).toInt();
    float conf = line.substring(commaIndex + 1).toFloat();

    String eventName = getEventNameFromID(eventID);
    int percentage = (int)(conf);

    // Final formatted output
    String output = eventName + " " + String(percentage) + "% confidence";

    return output;
}

String AudioEventSensor::readLine(uint32_t timeout) {
    uint32_t start = millis();
    while (!_serial->available()) {
        if (millis() - start > timeout) return "";
    }
    String line = _serial->readStringUntil('\n');
    line.trim();
    return line;
}

bool AudioEventSensor::expectOK(const String &cmd, uint32_t timeout) {
    clearBuffer(); // Clear before sending
    _serial->println(cmd);
    delay(50); // Give device time to process
    
    String resp = readLine(timeout);
    
    // Some devices might send extra lines, read until OK or timeout
    uint32_t start = millis();
    while (resp != "OK" && resp != "" && (millis() - start < timeout)) {
        resp = readLine(timeout - (millis() - start));
    }
    
    return resp == "OK";
}

// ---------- Command Functions --------------

bool AudioEventSensor::getDetectTypes(String &out) {
    clearBuffer();
    _serial->println("AT+GETDETECT");
    delay(50);
    
    String resp = readLine(500);
    if (resp.startsWith("+GETDETECT:")) {
        out = resp.substring(12);
        // Read and discard the OK response
        readLine(200);
        return true;
    }
    return false;
}

bool AudioEventSensor::setDetectTypes(const String &types) {
    return expectOK("AT+SETDETECT=" + types, 500);
}

bool AudioEventSensor::getEventThreshold(const String &type, int &threshold) {
    clearBuffer();
    _serial->println("AT+GETEVENTTHRESHOLD=" + type);
    delay(50);
    
    String resp = readLine(500);
    if (resp.startsWith("+GETEVENTTHRESHOLD:")) {
        int comma = resp.lastIndexOf(',');
        threshold = resp.substring(comma + 1).toInt();
        // Read and discard the OK response
        readLine(200);
        return true;
    }
    return false;
}

bool AudioEventSensor::setEventThreshold(const String &type, int threshold) {
    return expectOK("AT+SETEVENTTHRESHOLD=" + type + "," + String(threshold), 500);
}

bool AudioEventSensor::getSupportedList(String &out) {
    clearBuffer();
    _serial->println("AT+GETSUPPORTEDLIST");
    delay(50);
    
    String resp = readLine(500);
    Serial.println("Response from device: " + resp); // Debug output
    
    if (resp.startsWith("+GETSUPPORTEDLIST:")) {
        out = resp.substring(18);
        // Read and discard the OK response
        readLine(200);
        return true;
    }
    return false;
}

bool AudioEventSensor::setOutputType(const String &type) {
    return expectOK("AT+SETOUTPUTTYPE=" + type, 500);
}

bool AudioEventSensor::setIntMode(const String &mode) {
    return expectOK("AT+SETINTMODE=" + mode, 500);
}

bool AudioEventSensor::getIntMode(String &out) {
    clearBuffer();
    _serial->println("AT+GETINTMODE");
    delay(50);
    
    String resp = readLine(500);
    if (resp.startsWith("+GETINTMODE:")) {
        out = resp.substring(12);
        // Read and discard the OK response
        readLine(200);
        return true;
    }
    return false;
}

bool AudioEventSensor::resetInterrupt() {
    return expectOK("AT+RESETINT", 500);
}

bool AudioEventSensor::saveConfig() {
    return expectOK("AT+SAVECONFIG", 1000); // Saving might take longer
}

bool AudioEventSensor::resetDevice() {
    clearBuffer();
    _serial->println("AT+RESET");
    delay(100);
    
    // Device will reboot, wait for it to come back online
    delay(2000); // Adjust based on your device's boot time
    clearBuffer(); // Clear any boot messages
    
    return true; // Can't really verify OK after reset
}

bool AudioEventSensor::getFirmwareVersion(String &version) {
    clearBuffer();
    _serial->println("AT+GETFWVERSION");
    delay(50);
    
    String resp = readLine(500);
    if (resp.startsWith("+GETFWVERSION:")) {
        version = resp.substring(14);
        // Read and discard the OK response
        readLine(200);
        return true;
    }
    return false;
}
