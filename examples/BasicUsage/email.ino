#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <AudioEventSensor.h>

// WiFi credentials
#define WIFI_SSID "my_pass"
#define WIFI_PASSWORD "my_ssid"

// Email credentials
#define SENDER_EMAIL "kasun@gmail.com"
#define SENDER_PASSWORD "abcd efgh ijkl xyza"
#define RECIPIENT_EMAIL "tangxue@gmail.com"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587

// UART pins for audio sensor
#define UART1_TX 43
#define UART1_RX 44

// Cooldown period in milliseconds (30 seconds)
#define EVENT_COOLDOWN_MS 30000

// Event tracking structure
struct EventTracker {
  String eventType;
  unsigned long lastTriggerTime;
  int detectionCount;
};

// Track multiple event types
EventTracker events[] = {
  {"baby_cry", 0, 0},
  {"glass_break", 0, 0},
  {"gunshot", 0, 0},
  {"snore", 0, 0},
  {"T3", 0, 0},
  {"T4", 0, 0}
};
const int EVENT_COUNT = 6;

SMTPSession smtp;
AudioEventSensor audio(Serial1);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
  audio.begin(115200);

  Serial.println("=================================");
  Serial.println("Audio Event Email Alert System");
  Serial.println("=================================\n");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize audio sensor
  Serial.println("Initializing audio sensor...");
  delay(2000);
  
  audio.resetDevice();
  delay(500);

  // Get firmware version
  String firmwareVersion;
  if (audio.getFirmwareVersion(firmwareVersion)) {
    Serial.print("Firmware version: ");
    Serial.println(firmwareVersion);
  }

  // Configure detection
  Serial.println("Setting detection types...");
  if (audio.setDetectTypes("gunshot,glass_break")) {
    Serial.println("✓ Detection types set: gunshot, glass_break");
  } else {
    Serial.println("✗ Failed to set detection types");
  }
  delay(500);

  // Set thresholds
  audio.setEventThreshold("glass_break", 60);
  audio.setEventThreshold("gunshot", 65);
  delay(500);

  // Save configuration
  audio.saveConfig();
  delay(500);

  Serial.println("\n=================================");
  Serial.println("System ready. Monitoring events...");
  Serial.println("=================================\n");
}

void loop() {
  // Check for audio events
  if (audio.available()) {
    String eventData = audio.readEvent();
    
    if (eventData.length() > 0) {
      Serial.print("[" + getTimestamp() + "] ");
      Serial.println(eventData);
      
      // Extract event type from the event string
      String eventType = extractEventType(eventData);
      int confidence = extractConfidence(eventData);
      
      // Check if we should send an email for this event
      if (shouldSendEmail(eventType)) {
        Serial.println("\n>>> ALERT: New " + eventType + " detected!");
        Serial.println(">>> Sending email notification...");
        
        sendEventEmail(eventType, confidence);
        
        Serial.println(">>> Email sent. Cooldown active for " + 
                      String(EVENT_COOLDOWN_MS/1000) + " seconds.\n");
      }
    }
  }
  
  delay(100);
}

// Extract event type from event string (e.g., "glass_break 95% confidence")
String extractEventType(String eventData) {
  int spaceIndex = eventData.indexOf(' ');
  if (spaceIndex > 0) {
    return eventData.substring(0, spaceIndex);
  }
  return eventData;
}

// Extract confidence from event string
int extractConfidence(String eventData) {
  int percentIndex = eventData.indexOf('%');
  if (percentIndex > 0) {
    // Find the last space before %
    int lastSpace = eventData.lastIndexOf(' ', percentIndex);
    if (lastSpace > 0) {
      String confStr = eventData.substring(lastSpace + 1, percentIndex);
      return confStr.toInt();
    }
  }
  return 0;
}

// Check if enough time has passed since last email for this event type
bool shouldSendEmail(String eventType) {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < EVENT_COUNT; i++) {
    if (events[i].eventType == eventType) {
      // Check if cooldown period has passed
      if (currentTime - events[i].lastTriggerTime >= EVENT_COOLDOWN_MS) {
        // Update last trigger time
        events[i].lastTriggerTime = currentTime;
        events[i].detectionCount++;
        return true;
      } else {
        // Still in cooldown period
        unsigned long timeRemaining = EVENT_COOLDOWN_MS - (currentTime - events[i].lastTriggerTime);
        Serial.println("    ⏳ Cooldown active. " + String(timeRemaining/1000) + "s remaining.");
        return false;
      }
    }
  }
  
  return false; // Unknown event type
}

// Send email notification
void sendEventEmail(String eventType, int confidence) {
  // Prepare email subject and body
  String subject = "🚨 ALERT: " + formatEventName(eventType) + " Detected!";
  
  String body = "SECURITY ALERT\n";
  body += "═══════════════════════════════\n\n";
  body += "Event Type: " + formatEventName(eventType) + "\n";
  body += "Confidence: " + String(confidence) + "%\n";
  body += "Time: " + getTimestamp() + "\n";
  body += "Device: ESP32 Audio Sensor\n";
  body += "Location: [Your Location]\n\n";
  body += "═══════════════════════════════\n";
  body += "This is an automated alert from your audio monitoring system.\n";
  
  // Send the email
  gmail_send(subject, body);
}

// Format event name for display
String formatEventName(String eventType) {
  if (eventType == "glass_break") return "Glass Breaking";
  if (eventType == "gunshot") return "Gunshot";
  if (eventType == "baby_cry") return "Baby Crying";
  if (eventType == "snore") return "Snoring";
  return eventType;
}

// Get formatted timestamp
String getTimestamp() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds = seconds % 60;
  minutes = minutes % 60;
  hours = hours % 24;
  
  char timestamp[12];
  sprintf(timestamp, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(timestamp);
}

// Gmail send function
void gmail_send(String subject, String textMsg) {
  MailClient.networkReconnect(true);
  
  smtp.debug(0); // Set to 1 for debug output
  smtp.callback(smtpCallback);
  
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = SENDER_EMAIL;
  config.login.password = SENDER_PASSWORD;
  config.login.user_domain = F("127.0.0.1");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = F("ESP32 Security System");
  message.sender.email = SENDER_EMAIL;
  message.subject = subject;
  message.addRecipient(F("Security Admin"), RECIPIENT_EMAIL);
  message.text.content = textMsg;
  message.text.transfer_encoding = "base64";
  message.text.charSet = F("utf-8");
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  message.addHeader(F("Message-ID: <esp32.security@gmail.com>"));

  if (!smtp.connect(&config)) {
    Serial.printf("✗ Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", 
                  smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.printf("✗ Send error, Status Code: %d, Error Code: %d, Reason: %s\n", 
                  smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  } else {
    Serial.println("✓ Email sent successfully!");
  }
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());

  if (status.success()) {
    Serial.println("────────────────────");
    Serial.printf("✓ Message sent: %d\n", status.completedCount());
    Serial.printf("✗ Message failed: %d\n", status.failedCount());
    Serial.println("────────────────────\n");
    smtp.sendingResult.clear();
  }
}
