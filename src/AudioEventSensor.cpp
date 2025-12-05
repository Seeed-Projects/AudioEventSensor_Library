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
}

bool AudioEventSensor::available() {
    return _serial->available();
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

bool AudioEventSensor::expectOK(const String &cmd) {
    _serial->println(cmd);
    String resp = readLine();
    return resp == "OK";
}

// ---------- Command Functions --------------

bool AudioEventSensor::getDetectTypes(String &out) {
    _serial->println("AT+GETDETECT");
    String resp = readLine();
    if (resp.startsWith("+GETDETECT:")) {
        out = resp.substring(12);
        return true;
    }
    return false;
}

bool AudioEventSensor::setDetectTypes(const String &types) {
    return expectOK("AT+SETDETECT=" + types);
}

bool AudioEventSensor::getEventThreshold(const String &type, int &threshold) {
    _serial->println("AT+GETEVENTTHRESHOLD=" + type);
    String resp = readLine();
    if (resp.startsWith("+GETEVENTTHRESHOLD:")) {
        int comma = resp.lastIndexOf(',');
        threshold = resp.substring(comma + 1).toInt();
        return true;
    }
    return false;
}

bool AudioEventSensor::setEventThreshold(const String &type, int threshold) {
    return expectOK("AT+SETEVENTTHRESHOLD=" + type + "," + String(threshold));
}

bool AudioEventSensor::getSupportedList(String &out) {
    _serial->println("AT+GETSUPPORTEDLIST");
    String resp = readLine();
    Serial.println("Response from device: " + resp); // Add debug output here
    if (resp.startsWith("+GETSUPPORTEDLIST:")) {
        out = resp.substring(18);
        return true;
    }
    return false;
}


bool AudioEventSensor::setOutputType(const String &type) {
    return expectOK("AT+SETOUTPUTTYPE=" + type);
}

bool AudioEventSensor::setIntMode(const String &mode) {
    return expectOK("AT+SETINTMODE=" + mode);
}

bool AudioEventSensor::getIntMode(String &out) {
    _serial->println("AT+GETINTMODE");
    String resp = readLine();
    if (resp.startsWith("+GETINTMODE:")) {
        out = resp.substring(12);
        return true;
    }
    return false;
}

bool AudioEventSensor::resetInterrupt() {
    return expectOK("AT+RESETINT");
}

bool AudioEventSensor::saveConfig() {
    return expectOK("AT+SAVECONFIG");
}

bool AudioEventSensor::resetDevice() {
    return expectOK("AT+RESET");
}

bool AudioEventSensor::getFirmwareVersion(String &version) {
    _serial->println("AT+GETFWVERSION");
    String resp = readLine();
    if (resp.startsWith("+GETFWVERSION:")) {
        version = resp.substring(14);
        return true;
    }
    return false;
}
