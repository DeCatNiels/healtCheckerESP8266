# ESP8266 Health Checker

A website monitoring system for ESP8266 that checks website availability and can send email notifications.

## Projects

### 1. initialCheckSerial
Monitors a website's health by sending HTTPS requests every 60 seconds and reports status via serial output.

**Features:**
- WiFi connectivity
- HTTPS website monitoring
- Configurable check interval
- Serial output for status reports

### 2. sendEmail
Email notification system using Gmail SMTP to send alerts.

**Features:**
- Gmail SMTP integration
- HTML and text email support
- Multi-language message support
- Configurable recipients

## Setup

### 1. Configure Your Secrets

Copy the example configuration file and add your credentials:

```bash
cp config.h.example config.h
```

Edit `config.h` and set:
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

## Security Notes

- `config.h` contains your credentials and is gitignored
- Never commit `config.h` to version control
- The sketches use `setInsecure()` to bypass SSL certificate validation (acceptable for testing, but consider proper certificate handling for production)
- Use Gmail app-specific passwords, not your main password

## License

MIT
