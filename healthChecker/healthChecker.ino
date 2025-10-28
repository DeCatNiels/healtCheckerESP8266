/**
 * ESP8266 Health Checker with Email Alerts
 * Monitors a website and sends email notifications when it goes down
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "config.h"

#define ENABLE_SMTP
#define ENABLE_DEBUG
#define READYMAIL_DEBUG_PORT Serial

#if defined(ESP32) || defined(ESP8266)
#define READYMAIL_TIME_SOURCE time(nullptr);
#endif

#include <ReadyMail.h>

// Configuration
const char* hostURL = MONITOR_URL;
const unsigned long CHECK_INTERVAL = 10000; // Check every 10 seconds (for testing)
const int MAX_FAILURES = 1; // Send alert after 1 failure (for testing)

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

// SMTP callback for debugging
void smtpCb(SMTPStatus status) {
    if (status.progress.available)
        ReadyMail.printf("ReadyMail[smtp][%d] Uploading file %s, %d %% completed\n",
                         status.state, status.progress.filename.c_str(), status.progress.value);
    else
        ReadyMail.printf("ReadyMail[smtp][%d]%s\n", status.state, status.text.c_str());
}

// Send email alert
bool sendAlert(const char* subject, const char* message) {
    Serial.println("\n=== Sending Email Alert ===");
    Serial.printf("Free heap before: %d bytes\n", ESP.getFreeHeap());

    // Force garbage collection
    delay(100);

    // Check if we have enough memory
    if (ESP.getFreeHeap() < 15000) {
        Serial.println("Not enough free memory to send email!");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();
    client.setBufferSizes(512, 512);

    SMTPClient smtp(client);

    smtp.connect(SMTP_HOST, SMTP_PORT, smtpCb, true);
    if (!smtp.isConnected()) {
        Serial.println("Failed to connect to SMTP server");
        return false;
    }

    smtp.authenticate(AUTHOR_EMAIL, AUTHOR_PASSWORD, readymail_auth_password);
    if (!smtp.isAuthenticated()) {
        Serial.println("Failed to authenticate");
        return false;
    }

    SMTPMessage msg;
    msg.headers.add(rfc822_subject, subject);
    msg.headers.add(rfc822_from, AUTHOR_EMAIL);
    msg.headers.add(rfc822_to, RECIPIENT_EMAIL);

    // Use C strings to save memory
    char body[200];
    snprintf(body, sizeof(body), "%s\n\nWebsite: %s", message, hostURL);
    msg.text.body(body);

    bool success = smtp.send(msg, "");

    if (success) {
        Serial.println("Email sent successfully!");
    } else {
        Serial.println("Failed to send email");
    }

    Serial.printf("Free heap after: %d bytes\n", ESP.getFreeHeap());
    return success;
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
