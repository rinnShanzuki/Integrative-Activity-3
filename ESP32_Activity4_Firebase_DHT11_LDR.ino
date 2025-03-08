#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>

#define DHTPIN 26      
#define DHTTYPE DHT11  
#define LDRPIN 33      

const char* ssid = "Hakdog";
const char* password = "hakdog05";
const char* firestoreUrl = "https://firestore.googleapis.com/v1/projects/firstproject-2cda0/databases/(default)/documents/sensorsdata?key=AIzaSyBXhD1PNpXNJ8Ce6GYbuWKsiZyliqAGcxE";
const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();

    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to Wi-Fi");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int lightValue = analogRead(LDRPIN);
        int lightStatus = (lightValue < 2500) ? 1 : 0;

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time");
            return;
        }
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

        HTTPClient http;
        http.begin(firestoreUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonData = String("{") +
            "\"fields\": {" +
            "\"light\": {\"integerValue\": " + String(lightStatus) + "}," +
            "\"temperature\": {\"stringValue\": \"" + String(temperature) + "\"}," +
            "\"humidity\": {\"stringValue\": \"" + String(humidity) + "\"}," +
            "\"timestamp\": {\"stringValue\": \"" + timestamp + "\"}" +
            "}" +
            "}";

        int httpResponseCode = http.POST(jsonData);
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response);
        } else {
            Serial.println("Error: " + http.errorToString(httpResponseCode));
        }

        http.end();
    } else {
        Serial.println("WiFi disconnected. Reconnecting...");
        WiFi.begin(ssid, password);
    }

    delay(5000);
}
