#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* ssid = "Hakdog";
const char* password = "hakdog05";
const char* serverUrl = "http://192.168.13.246/integrative2/api.php";

#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        float temp = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temp) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        String payload = "{\"name\":\"Temperature and Humidity\", \"value\":\"" + String(temp) + "C, " + String(humidity) + "%\"}";
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            Serial.println(http.getString());
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }

    delay(5000);
}
