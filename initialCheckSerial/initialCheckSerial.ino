


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "config.h"

// De ENIGE URL die je hoeft aan te passen (moet met https:// beginnen)
const char* hostURL = MONITOR_URL; 


// Functie om de pure domeinnaam (hostName) uit de volledige URL te halen
String getHostName(const char* fullUrl) {
    String url = String(fullUrl);
    // Verwijder "https://" of "http://"
    int start = url.indexOf("://");
    if (start != -1) {
        start += 3; // +3 om na '://' te beginnen
    } else {
        start = 0;
    }
    
    // Zoek het eerste '/' teken dat het einde van de domeinnaam markeert
    int end = url.indexOf('/', start);
    if (end == -1) {
        end = url.length(); // Als er geen pad is, neem de rest van de string
    }
    
    return url.substring(start, end);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbinden met WiFi...");
  }
  Serial.println("Verbonden met WiFi.");
  Serial.print("IP-adres: ");
  Serial.println(WiFi.localIP());
}

void checkWebsite() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client; 
    
    // Ophalen van de HostName (bijv. "jouwwebsite.com") uit de hostURL
    String hostName = getHostName(hostURL);
    
    // Simpele Oplossing: Certificaatvalidatie overslaan (NIET VEILIG!)
    client.setInsecure(); 
    
    // Start het beveiligde verzoek met de volledige URL
    http.begin(client, hostURL); 

    // Voeg de Host-header toe met de pure domeinnaam
    http.addHeader("Host", hostName); 

    Serial.print("Controleren: ");
    Serial.println(hostURL);

    int httpCode = http.GET(); 
    
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == 200) { 
        Serial.println("Website is ONLINE! ✅");
      } else {
        Serial.printf("Website is MOGELIJK OFFLINE of heeft een fout! Code: %d\n", httpCode);
      }
    } else {
      Serial.printf("[HTTP] GET mislukt, foutcode: %s\n", http.errorToString(httpCode).c_str());
      Serial.println("Website is OFFLINE of er is een netwerkfout. ❌");
    }
    
    http.end(); 
  } else {
    Serial.println("Wi-Fi is verbroken. Probeer opnieuw te verbinden...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void loop() {
  checkWebsite();
  delay(60000); // Controleer elke 60 seconden
}