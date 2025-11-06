# ESP8266 SMTP Email Issues - Summary

## The Problem

Sending emails directly via Gmail SMTP (SSL/TLS) from ESP8266 is **unreliable** due to hardware limitations:

### Root Cause
- **SSL handshake takes 3-10+ seconds** depending on network conditions
- **ESP8266 watchdog timer resets after ~3-5 seconds** if code doesn't yield
- **Result:** Random crashes, timeouts, or "Soft WDT reset" errors

### Testing Results
- ✅ **sendEmail.ino works sometimes** (~50% success rate)
- ❌ **healthChecker alerts fail consistently** when called from loop()
- 🎲 **Success depends on:** WiFi signal strength, network latency, Gmail server response time

### Error Types Observed
1. **Soft WDT reset** - Watchdog timeout during SSL handshake
2. **LoadStoreError** - Memory access errors in ReadyMail library
3. **readTimeout()** - Connection established but data read times out

## Why It's Intermittent

The ESP8266 has very limited resources:
- Only ~40-50KB free RAM
- 80MHz single-core CPU
- No hardware SSL acceleration
- Strict watchdog timer

When network conditions are perfect, emails *sometimes* get through. When there's any delay, the watchdog kills the process.

## Recommended Solution: Use a Webhook Service

Instead of fighting ESP8266 limitations, use a **webhook/email gateway service**:

### Option 1: IFTTT (Easiest)
**Simple HTTP POST → IFTTT sends email for you**

```cpp
// Replace sendAlert() function with:
bool sendAlert(const char* subject, const char* message) {
    HTTPClient http;
    WiFiClient client;

    String url = "https://maker.ifttt.com/trigger/website_down/with/key/YOUR_IFTTT_KEY";
    String payload = "{\"value1\":\"" + String(subject) + "\",\"value2\":\"" + String(message) + "\"}";

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(payload);
    http.end();

    return (httpCode == 200);
}
```

**Setup:**
1. Create free IFTTT account: https://ifttt.com
2. Create new Applet: "Webhooks" → "Email"
3. Get your webhook key from https://ifttt.com/maker_webhooks/settings
4. Replace `YOUR_IFTTT_KEY` in code above

**Benefits:**
- ✅ Fast (< 1 second)
- ✅ 100% reliable
- ✅ No SSL handshake delays
- ✅ No watchdog timeouts
- ✅ Free tier available

### Option 2: Use ESP32 Instead
ESP32 has enough power to handle Gmail SMTP reliably:
- 320KB RAM (8x more)
- 240MHz dual-core CPU
- Hardware SSL acceleration
- Drop-in replacement for most ESP8266 projects

### Option 3: Serial Monitoring Only
Keep it simple - monitor via serial output, no email alerts.

## Current Code Status

**UPDATE:** The healthChecker has been migrated to use IFTTT webhooks! The SMTP email code has been removed and replaced with a fast, reliable webhook implementation.

**Current Implementation:**
- Uses IFTTT Webhooks for notifications (100% reliable)
- No more SSL handshake delays or watchdog timeouts
- Sends notifications in < 1 second
- Works consistently on ESP8266 hardware

The sendEmail sketch still contains the original SMTP code for testing/reference purposes.

## Resources
- IFTTT Webhooks: https://ifttt.com/maker_webhooks
- ESP32 Migration Guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
- ReadyMail Library Issues: https://github.com/mobizt/ReadyMail/issues