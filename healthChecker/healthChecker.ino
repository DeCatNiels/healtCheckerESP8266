/**
 * ESP8266 Health Checker with Email Alerts via Resend
 * Monitors a website and sends email notifications when it goes down or recovers
 *
 * Uses Resend API for reliable email delivery (no SMTP issues!)
 * See RESEND_SETUP_GUIDE.md for configuration instructions
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "config.h"

// Configuration
const char* hostURL = MONITOR_URL;
const unsigned long CHECK_INTERVAL = 60000; // Check every 60 seconds
const int MAX_FAILURES = 3; // Send alert after 3 failures

// State tracking
int consecutiveFailures = 0;
bool wasDown = false;
unsigned long lastCheckTime = 0;

// Extract hostname from URL for Host header
String getHostName(const char* fullUrl) {
    String url = String(fullUrl);
    int start = url.indexOf("://");
    if (start != -1) {
        start += 3;
    } else {
        start = 0;
    }

    int end = url.indexOf('/', start);
    if (end == -1) {
        end = url.length();
    }

    return url.substring(start, end);
}

// Send email notification via Resend API to all recipients
bool sendAlert(const char* subject, const char* message) {
    HTTPClient http;
    WiFiClientSecure client;

    Serial.println("\n=== Sending Email via Resend ===");

    String url = "https://api.resend.com/emails";

    // Parse comma-separated email addresses and build JSON array
    String emailList = String(RESEND_TO_EMAILS);
    String recipients = "[";

    int startPos = 0;
    int commaPos = emailList.indexOf(',');
    bool firstEmail = true;

    while (commaPos != -1) {
        String email = emailList.substring(startPos, commaPos);
        email.trim();
        if (email.length() > 0) {
            if (!firstEmail) recipients += ", ";
            recipients += "\"" + email + "\"";
            firstEmail = false;
            Serial.println("Adding recipient: " + email);
        }
        startPos = commaPos + 1;
        commaPos = emailList.indexOf(',', startPos);
    }

    // Add the last email address
    String lastEmail = emailList.substring(startPos);
    lastEmail.trim();
    if (lastEmail.length() > 0) {
        if (!firstEmail) recipients += ", ";
        recipients += "\"" + lastEmail + "\"";
        Serial.println("Adding recipient: " + lastEmail);
    }
    recipients += "]";

    // Build JSON payload
    String payload = "{";
    payload += "\"from\":\"" + String(RESEND_FROM_EMAIL) + "\",";
    payload += "\"to\":" + recipients + ",";
    payload += "\"subject\":\"" + String(subject) + "\",";
    payload += "\"text\":\"" + String(message) + "\\n\\nWebsite: " + String(hostURL) + "\"";
    payload += "}";

    Serial.println("\nFull Payload: " + payload);

    // Configure SSL client
    client.setInsecure();
    client.setTimeout(15000);

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(RESEND_API_KEY));
    http.setTimeout(15000);

    int httpCode = http.POST(payload);
    String response = http.getString();
    http.end();

    if (httpCode == 200) {
        Serial.printf("✓ Email sent successfully! HTTP code: %d\n", httpCode);
        Serial.println("Response: " + response);
        return true;
    } else {
        Serial.printf("✗ Failed to send email. HTTP code: %d\n", httpCode);
        Serial.println("Response: " + response);
        return false;
    }
}

// Check website health
bool checkWebsite() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi is disconnected. Reconnecting...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        return false;
    }

    HTTPClient http;
    WiFiClientSecure client;

    String hostName = getHostName(hostURL);
    client.setInsecure();

    http.begin(client, hostURL);
    http.addHeader("Host", hostName);
    http.setTimeout(10000); // 10 second timeout

    Serial.print("\nChecking: ");
    Serial.println(hostURL);

    int httpCode = http.GET();
    bool isOnline = false;

    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == 200) {
            Serial.println("Website is ONLINE! ✓");
            isOnline = true;
        } else {
            Serial.printf("Website returned error code: %d\n", httpCode);
        }
    } else {
        Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        Serial.println("Website is OFFLINE or unreachable ✗");
    }

    http.end();
    return isOnline;
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP8266 Health Checker Starting ===");

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Monitoring: ");
        Serial.println(hostURL);
        Serial.printf("Check interval: %lu seconds\n", CHECK_INTERVAL / 1000);
        Serial.printf("Alert threshold: %d consecutive failures\n\n", MAX_FAILURES);
    } else {
        Serial.println("\nFailed to connect to WiFi!");
    }
}

void loop() {
    unsigned long currentTime = millis();

    // Check if it's time to run another health check
    if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
        lastCheckTime = currentTime;

        bool isOnline = checkWebsite();

        if (isOnline) {
            // Website is online
            if (wasDown) {
                // Website just came back online
                Serial.println("\n*** Website RECOVERED! ***");
                sendAlert("Website is BACK ONLINE",
                         "The website has recovered");
                wasDown = false;
            }
            consecutiveFailures = 0;

        } else {
            // Website is down or unreachable
            consecutiveFailures++;
            Serial.printf("Consecutive failures: %d/%d\n", consecutiveFailures, MAX_FAILURES);

            if (consecutiveFailures >= MAX_FAILURES && !wasDown) {
                // Website is down, send alert
                Serial.println("\n*** ALERT: Website is DOWN! ***");
                sendAlert("Website is DOWN", "The website appears to be offline");
                wasDown = true;
            }
        }
    }

    // Small delay to prevent tight looping
    delay(100);
}
