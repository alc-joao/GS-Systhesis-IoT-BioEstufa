#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 15
#define DHTTYPE DHT22

#define PIN_SOLO 34
#define LED_IRRIGACAO 25
#define LED_ALERTA 26

const char* ssid = "Wokwi-GUEST";
const char* password = "";

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperatura = 0;
float umidadeAr = 0;
int umidadeSolo = 0;
bool irrigacaoAtiva = false;
bool alertaCritico = false;
String statusEstufa = "NORMAL";

void lerSensores() {
  temperatura = dht.readTemperature();
  umidadeAr = dht.readHumidity();

  if (isnan(temperatura)) temperatura = 0;
  if (isnan(umidadeAr)) umidadeAr = 0;

  int leituraSolo = analogRead(PIN_SOLO);
  umidadeSolo = map(leituraSolo, 0, 4095, 0, 100);
}

void controlarEstufa() {
  irrigacaoAtiva = umidadeSolo < 30;
  alertaCritico = temperatura > 35 || umidadeSolo < 15;

  if (alertaCritico) {
    statusEstufa = "CRITICO";
  } else if (irrigacaoAtiva) {
    statusEstufa = "IRRIGANDO";
  } else {
    statusEstufa = "NORMAL";
  }

  digitalWrite(LED_IRRIGACAO, irrigacaoAtiva ? HIGH : LOW);
  digitalWrite(LED_ALERTA, alertaCritico ? HIGH : LOW);
}

String statusJSON() {
  String json = "{";
  json += "\"projeto\":\"GS-Systhesis-IoT-BioEstufa\",";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"umidadeAr\":" + String(umidadeAr, 1) + ",";
  json += "\"umidadeSolo\":" + String(umidadeSolo) + ",";
  json += "\"irrigacaoAtiva\":" + String(irrigacaoAtiva ? "true" : "false") + ",";
  json += "\"alertaCritico\":" + String(alertaCritico ? "true" : "false") + ",";
  json += "\"status\":\"" + statusEstufa + "\"";
  json += "}";
  return json;
}

void handleStatus() {
  server.send(200, "application/json", statusJSON());
}

void handleAlertas() {
  String json = "{";
  json += "\"alertaCritico\":" + String(alertaCritico ? "true" : "false") + ",";
  json += "\"status\":\"" + statusEstufa + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleIrrigacao() {
  String json = "{";
  json += "\"irrigacaoAtiva\":" + String(irrigacaoAtiva ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleDashboard() {
  String html = "<html><head><meta charset='UTF-8'>";
  html += "<title>GS-Systhesis-IoT-BioEstufa</title>";
  html += "<style>body{background:#050816;color:white;font-family:Arial;text-align:center;padding:30px}";
  html += ".card{background:#111827;margin:15px auto;padding:20px;border-radius:15px;max-width:400px}";
  html += "h1{color:#7df9ff}.v{font-size:30px;color:#38bdf8;font-weight:bold}</style></head><body>";
  html += "<h1>GS-Systhesis-IoT-BioEstufa</h1>";
  html += "<p>Bioestufa inteligente para colonização espacial</p>";
  html += "<div class='card'>Temperatura<br><span class='v'>" + String(temperatura, 1) + " C</span></div>";
  html += "<div class='card'>Umidade do Ar<br><span class='v'>" + String(umidadeAr, 1) + " %</span></div>";
  html += "<div class='card'>Umidade do Solo<br><span class='v'>" + String(umidadeSolo) + " %</span></div>";
  html += "<div class='card'>Status<br><span class='v'>" + statusEstufa + "</span></div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_IRRIGACAO, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);

  dht.begin();

  WiFi.begin(ssid, password);

  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleDashboard);
  server.on("/api/status", handleStatus);
  server.on("/api/alertas", handleAlertas);
  server.on("/api/irrigacao", handleIrrigacao);

  server.begin();
}

void loop() {
  server.handleClient();

  lerSensores();
  controlarEstufa();

  Serial.println(statusJSON());

  delay(2000);
}