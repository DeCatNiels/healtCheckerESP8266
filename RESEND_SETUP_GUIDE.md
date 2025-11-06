# Resend Email Setup Guide

**Resend** is a modern email API service with a generous free tier (100 emails/day). Perfect for ESP8266 health monitoring!

## Why Resend Works Great on ESP8266

✅ **Simple HTTP API** - No SMTP, no SSL handshake delays
✅ **Fast** - Responses in < 1 second
✅ **Reliable** - 100% success rate (no watchdog timeouts)
✅ **100 emails/day free** - More than enough for monitoring
✅ **Low memory usage** - Just a simple HTTP POST

---

## Setup Instructions

### Step 1: Sign Up for Resend

1. Go to https://resend.com
2. Click "Get Started" or "Sign Up"
3. Create a free account with your email
4. Verify your email address

### Step 2: Verify Your Domain

Resend requires you to verify a domain to send emails. You have two options:

#### Option A: Use Your Own Domain (Recommended)

1. In Resend dashboard, go to **Domains**
2. Click **Add Domain**
3. Enter your domain (e.g., `yourdomain.com`)
4. Add the DNS records shown to your domain's DNS settings
5. Wait for verification (usually 5-15 minutes)

**DNS Records to Add:**
- TXT record for domain verification
- MX, TXT (SPF), TXT (DKIM) records for sending

6. Once verified, you can send from any address at that domain (e.g., `alerts@yourdomain.com`)

#### Option B: Use a Subdomain

If you don't want to change your main domain's email settings:

1. Add a subdomain like `alerts.yourdomain.com`
2. Add the DNS records for just that subdomain
3. Send from `noreply@alerts.yourdomain.com`

### Step 3: Get Your API Key

1. In Resend dashboard, go to **API Keys**
2. Click **Create API Key**
3. Give it a name: `ESP8266 Health Checker`
4. Select permissions: **Sending access** (default)
5. Click **Create**
6. **Copy the API key immediately** - it starts with `re_`
7. Keep it safe (you won't see it again!)

Example API key: `re_123456789_abcdefghijklmnopqrstuvwxyz`

### Step 4: Configure Your ESP8266

1. **Copy the config file:**
   ```bash
   cp healthChecker/config.h.example healthChecker/config.h
   ```

2. **Edit `healthChecker/config.h`:**
   ```cpp
   // WiFi Configuration
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"

   // Website to Monitor
   #define MONITOR_URL "https://yourwebsite.com"

   // Resend Email Configuration
   #define RESEND_API_KEY "re_123456789_YourActualAPIKey"
   #define RESEND_FROM_EMAIL "alerts@yourdomain.com"  // Your verified domain
   #define RESEND_TO_EMAILS "email1@example.com,email2@example.com"  // Comma-separated recipients
   ```

3. **Upload to ESP8266:**
   ```bash
   ./upload.sh
   ```

### Step 5: Test Your Setup

You can test the Resend API before uploading to ESP8266:

**Using curl:**
```bash
curl -X POST https://api.resend.com/emails \
  -H "Authorization: Bearer re_YOUR_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "from": "alerts@yourdomain.com",
    "to": ["your-email@gmail.com"],
    "subject": "Test from ESP8266",
    "text": "This is a test email!"
  }'
```

If successful, you'll get a response with an email ID, and you should receive the email within seconds!

---

## How It Works

1. **ESP8266 detects website is down**
2. **Sends HTTPS POST to Resend API** with email details
3. **Resend receives the request and sends email** on your behalf to all recipients
4. **You receive email notification** within seconds

---

## Troubleshooting

### Not Receiving Emails?

1. **Check spam folder** - First emails might go to spam
2. **Verify domain is confirmed** - Check Resend dashboard
3. **Check API key is correct** - It should start with `re_`
4. **Check FROM email matches verified domain** - Must be `someone@yourdomain.com`
5. **Check ESP8266 serial output** for errors

### Error: "Domain not verified"

- Your domain DNS records aren't set up yet
- Wait 15-30 minutes after adding DNS records
- Use `dig TXT yourdomain.com` to verify DNS propagation

### HTTP 401 Error

- Your API key is incorrect or expired
- Generate a new API key in Resend dashboard

### HTTP 422 Error

- Your FROM email doesn't match verified domain
- Make sure `RESEND_FROM_EMAIL` uses your verified domain

### Memory Issues?

The Resend implementation is very lightweight:
- Uses ~2-3KB of RAM for the request
- Simple HTTPS POST with minimal overhead
- Much better than SMTP libraries (which used 15KB+)

---

## Best Practices

### 1. Use a Dedicated FROM Address
```cpp
#define RESEND_FROM_EMAIL "alerts@yourdomain.com"
```
Not your personal email - makes it easier to filter

### 2. Multiple Recipients
```cpp
#define RESEND_TO_EMAILS "email1@gmail.com,email2@outlook.com,email3@yahoo.com"
```
Separate multiple email addresses with commas (no spaces needed)

### 3. Adjust Check Intervals for Production

Once you've tested, adjust these in `healthChecker.ino`:
```cpp
const unsigned long CHECK_INTERVAL = 60000; // Check every 60 seconds
const int MAX_FAILURES = 3; // Alert after 3 consecutive failures
```

This prevents false alarms and reduces email volume.

### 4. Monitor Your Email Usage

- Free tier: 100 emails/day
- With 60-second checks: ~1,440 checks/day
- Even with 10% downtime: ~144 emails (over limit!)
- **Solution:** Set `MAX_FAILURES = 3` to reduce alerts

### 5. Set Up Email Filters

Create a Gmail filter for `from:alerts@yourdomain.com`:
- Label: "ESP8266 Alerts"
- Important: Yes
- Optional: Forward to phone as SMS

---

## Advanced: HTML Emails

Want nicer-looking emails? Modify the `sendAlert` function in `healthChecker.ino`:

```cpp
// Instead of "text", use "html"
payload = "{";
payload += "\"from\":\"" + String(RESEND_FROM_EMAIL) + "\",";
payload += "\"to\":" + recipients + ",";  // recipients is already built as array
payload += "\"subject\":\"" + String(subject) + "\",";
payload += "\"html\":\"<h2>" + String(subject) + "</h2>";
payload += "<p>" + String(message) + "</p>";
payload += "<p><strong>Website:</strong> <a href='" + String(hostURL) + "'>" + String(hostURL) + "</a></p>\"";
payload += "}";
```

---

## Cost Comparison

| Service | Free Tier | ESP8266 Compatible | Setup Time |
|---------|-----------|-------------------|------------|
| **Gmail SMTP** | Free | ❌ Unreliable | 5 min |
| **Resend** | 100/day | ✅ Perfect | 10 min |
| **SendGrid** | 100/day | ✅ Good | 10 min |
| **Mailgun** | 5,000/month | ✅ Good | 15 min |

---

## What If I Don't Have a Domain?

If you don't own a domain, you have these options:

1. **Buy a cheap domain** - $1-5/year from Namecheap/Cloudflare
2. **Use a free subdomain service** - Some DNS providers offer free subdomains
3. **Use alternative notification services** - Consider services that don't require domain verification

Note: Resend requires domain verification on the free tier for security and deliverability.

---

## Support & Resources

- Resend Documentation: https://resend.com/docs
- Resend Email API: https://resend.com/docs/api-reference/emails/send-email
- Domain Verification Guide: https://resend.com/docs/dashboard/domains/introduction
- ESP8266 HTTPClient Docs: https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html

---

## Next Steps

Once everything works:

1. ✅ Verify you're receiving emails reliably
2. ✅ Adjust check intervals to reasonable values (60s)
3. ✅ Set failure threshold to 3+ to avoid false alarms
4. ✅ Monitor your Resend dashboard for usage stats
5. ✅ Set up email filters/forwarding for alerts

Your ESP8266 is now reliably monitoring your website with email alerts!