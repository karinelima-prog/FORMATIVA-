
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "kalini";
const char* password = "12345678";

WebServer server(80);

float currentTemp = 0.0;
float currentWind = 0.0;
int windDirection = 0;

void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=-23.5113&longitude=-46.8761&current_weather=true";

    if (http.begin(client, apiUrl)) {
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          currentTemp = doc["current_weather"]["temperature"] | 0.0;
          currentWind = doc["current_weather"]["windspeed"] | 0.0;
          windDirection = doc["current_weather"]["winddirection"] | 0;

          Serial.printf(
            "Temperatura: %.1f C | Vento: %.1f km/h | Direcao: %d graus\n",
            currentTemp, currentWind, windDirection
          );
        } else {
          Serial.print("Falha ao ler JSON: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.printf("Erro HTTP: %d\n", httpCode);
      }

      http.end();
    }
  }
}

String getWindDirection(int degrees) {
  if (degrees >= 338 || degrees < 23) return "Norte";
  if (degrees < 68) return "Nordeste";
  if (degrees < 113) return "Leste";
  if (degrees < 158) return "Sudeste";
  if (degrees < 203) return "Sul";
  if (degrees < 248) return "Sudoeste";
  if (degrees < 293) return "Oeste";
  return "Noroeste";
}

void handleRoot() {
  fetchWeatherData();

  String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Painel Climático - ESP32</title>";

  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #0f172a; color: #f8fafc; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; }";
  html += ".card { background: #1e293b; padding: 2rem; border-radius: 12px; text-align: center; max-width: 400px; width: 90%; border: 1px solid #334155; box-shadow: 0 10px 25px rgba(0,0,0,0.3); }";
  html += "h1 { color: #38bdf8; font-size: 1.2rem; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 1.5rem; }";
  html += ".metric { font-size: 2.5rem; font-weight: bold; color: #f1f5f9; margin: 10px 0; }";
  html += ".sub-metric { font-size: 1.1rem; color: #94a3b8; margin-bottom: 12px; }";
  html += ".btn { background: #38bdf8; color: #0f172a; border: none; padding: 12px 24px; font-weight: bold; border-radius: 6px; cursor: pointer; transition: 0.2s; margin-top: 10px; }";
  html += ".btn:hover { background: #0284c7; color: #ffffff; }";
  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "<h1>Clima em Tempo Real (Barueri)</h1>";

  html += "<div class='metric'>" + String(currentTemp, 1) + " &deg;C</div>";

  html += "<div class='sub-metric'>Velocidade do vento: "
       + String(currentWind, 1) + " km/h</div>";

  html += "<div class='sub-metric'>Direção do vento: "
       + getWindDirection(windDirection)
       + " (" + String(windDirection) + "&deg;)</div>";

  html += "<button class='btn' onclick='location.reload()'>Atualizar Dados</button>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP do ESP32: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Servidor HTTP iniciado com sucesso.");
}

void loop() {
  server.handleClient();
}
