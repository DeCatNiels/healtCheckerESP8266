# ESP8266 Health Checker

A website monitoring system for ESP8266 that checks website availability and sends email notifications when issues are detected.

## Projects

### 1. healthChecker (Recommended)
**Complete monitoring solution** that combines website health checks with automatic email alerts.

**Features:**
- Continuous website monitoring (60-second intervals)
- Automatic email alerts when site goes down
- Recovery notifications when site comes back online
- Smart failure detection (requires 3 consecutive failures before alerting)
- Detailed HTML and text email reports
- Serial output for real-time status monitoring

### 2. initialCheckSerial
Simple website health monitor that reports status via serial output only (no email alerts).

**Features:**
- WiFi connectivity
- HTTPS website monitoring
- Configurable check interval
- Serial output for status reports

### 3. sendEmail
Standalone email notification test system using Gmail SMTP.

**Features:**
- Gmail SMTP integration
- HTML and text email support
- Multi-language message support
- Configurable recipients
- Useful for testing email configuration

## Setup

### 1. Configure Your Secrets

Each sketch directory contains its own configuration files. Copy the example configuration file in the sketch you want to use:

```bash
# For healthChecker (recommended)
cp healthChecker/config.h.example healthChecker/config.h

# For initialCheckSerial
cp initialCheckSerial/config.h.example initialCheckSerial/config.h

# For sendEmail
cp sendEmail/config.h.example sendEmail/config.h
```

Edit the appropriate `config.h` file and set:
- `WIFI_SSID` - Your WiFi network name
- `WIFI_PASSWORD` - Your WiFi password
- `MONITOR_URL` - Website URL to monitor
- `AUTHOR_EMAIL` - Your Gmail address
- `AUTHOR_PASSWORD` - Gmail app-specific password ([How to create](https://support.google.com/accounts/answer/185833))
- `RECIPIENT_EMAIL` - Email address to receive alerts

### 2. Install Arduino CLI

```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

Add to your PATH or use `./bin/arduino-cli` from this directory.

### 3. Configure Arduino CLI

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core install esp8266:esp8266
```

### 4. Install Required Libraries

```bash
arduino-cli lib install ReadyMail
```

## Upload to ESP8266

### Quick Upload (Easiest Method)

Use the included upload script for automatic compile, upload, and monitoring:

```bash
./upload.sh                    # Upload healthChecker (default)
./upload.sh initialCheckSerial # Upload initialCheckSerial
./upload.sh sendEmail          # Upload sendEmail
```

The script will:
- Compile the sketch
- Upload it to your ESP8266
- Automatically open a colorized serial monitor
- Show success/error messages in color

### Manual Upload

### Compile and Upload healthChecker (Recommended)

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 healthChecker/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 healthChecker/
```

### Compile and Upload initialCheckSerial

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 initialCheckSerial/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 initialCheckSerial/
```

### Compile and Upload sendEmail

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 sendEmail/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 sendEmail/
```

**Note:** Replace `/dev/ttyUSB0` with your board's port. Find it with:
```bash
arduino-cli board list
```

### Board Types

If you're not using NodeMCU v2, change `nodemcuv2` to your board type:
- `nodemcuv2` - NodeMCU 1.0 (ESP-12E Module)
- `generic` - Generic ESP8266 Module
- `d1_mini` - WeMos D1 Mini
- `huzzah` - Adafruit HUZZAH ESP8266

List all available boards:
```bash
arduino-cli board listall esp8266
```

## Monitor Serial Output

```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Press `Ctrl+C` to exit the monitor.

## How It Works

The **healthChecker** sketch:
1. Connects to your WiFi network
2. Checks the website every 60 seconds
3. Tracks consecutive failures
4. After 3 consecutive failures, attempts to send an email alert
5. When the site recovers, sends a recovery notification
6. Continues monitoring indefinitely

## ⚠️ Important: Email Reliability Issues

**Gmail SMTP on ESP8266 is unreliable** due to SSL handshake timeouts and watchdog timer constraints. Emails work ~50% of the time.

**For reliable alerts, use a webhook service instead:**
- See [SMTP_ISSUES.md](SMTP_ISSUES.md) for detailed explanation
- IFTTT webhook implementation takes 5 minutes and works 100% reliably
- Or upgrade to ESP32 which can handle Gmail SMTP properly

## Security Notes

- Each sketch has its own `config.h` file containing credentials (gitignored)
- Never commit `config.h` files to version control
- The sketches use `setInsecure()` to bypass SSL certificate validation (acceptable for testing, but consider proper certificate handling for production)
- Use Gmail app-specific passwords, not your main password

## License

MIT
