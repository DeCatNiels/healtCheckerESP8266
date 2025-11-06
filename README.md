# ESP8266 Health Checker

A website monitoring system for ESP8266 that checks website availability and sends email notifications when issues are detected.

## Projects

### 1. healthChecker (Recommended)

**Complete monitoring solution** that combines website health checks with automatic email notifications via Resend API.

**Features:**

- Continuous website monitoring (configurable intervals)
- Automatic email alerts when site goes down
- Recovery notifications when site comes back online
- Smart failure detection (configurable consecutive failures threshold)
- Multiple recipient support
- Fast and reliable notifications via Resend REST API
- No SMTP issues or memory constraints
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

**For healthChecker:**

- `WIFI_SSID` - Your WiFi network name
- `WIFI_PASSWORD` - Your WiFi password
- `MONITOR_URL` - Website URL to monitor
- `RESEND_API_KEY` - Your Resend API key from https://resend.com
- `RESEND_FROM_EMAIL` - Sender email (must use verified domain)
- `RESEND_TO_EMAILS` - Comma-separated list of recipient emails

See [RESEND_SETUP_GUIDE.md](RESEND_SETUP_GUIDE.md) for detailed setup instructions.

**For sendEmail (legacy SMTP test):**

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

**For healthChecker:** No additional libraries needed (uses built-in HTTPClient)

**For sendEmail (legacy SMTP test only):**

```bash
arduino-cli lib install ReadyMail
```

## Upload to ESP8266

### Quick Upload (Easiest Method)

In case there is an port monitor error: command 'open' failed: Permission denied. you can run the following to change the permissions:

```bash
sudo chmod a+rw /dev/ttyUSB0
```

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
2. Checks the website at configurable intervals (default: 60 seconds)
3. Tracks consecutive failures
4. After reaching the failure threshold (default: 3), sends email alerts via Resend API
5. When the site recovers, sends a recovery notification email
6. Continues monitoring indefinitely

## Setting Up Resend Email Notifications

1. Create a free account at https://resend.com (100 emails/day free)
2. Add and verify your domain in the Resend dashboard
3. Add the required DNS records (SPF, DKIM) to your domain via your DNS provider
4. Create an API key in Resend
5. Configure your `config.h` with:
   - Your API key
   - Sender email using your verified domain (e.g., `alerts@yourdomain.com`)
   - Recipient email addresses (comma-separated for multiple recipients)

For detailed instructions, see [RESEND_SETUP_GUIDE.md](RESEND_SETUP_GUIDE.md).

For why we use Resend instead of SMTP, see [SMTP_ISSUES.md](SMTP_ISSUES.md).

## Security Notes

- Each sketch has its own `config.h` file containing credentials (gitignored)
- Never commit `config.h` files to version control
- The sketches use `setInsecure()` to bypass SSL certificate validation (acceptable for testing, but consider proper certificate handling for production)
- Use Gmail app-specific passwords, not your main password

## License

MIT
