#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define DHTPIN 15
#define DHTTYPE DHT22

#define PIN_SOLO 34
#define PIN_LDR 35
#define PIN_BOTAO 27

#define LED_IRRIGACAO 25
#define LED_ALERTA 26
#define LED_LUZ 33

#define PIN_BUZZER 14
#define PIN_SERVO 18

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 21
#define OLED_SCL 22

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// IP Network do Next.js no seu Mac
const char* dashboardApiUrl = "http://10.60.32.213:3000/api/iot";

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo servoVentilacao;

float temperatura = 0;
float umidadeAr = 0;
int umidadeSolo = 0;
int luminosidade = 0;

bool irrigacaoAtiva = false;
bool alertaCritico = false;
bool luzArtificialAtiva = false;

int anguloServo = 0;
int telaOLED = 0;

String statusEstufa = "NORMAL";

unsigned long ultimaLeitura = 0;
unsigned long ultimoClique = 0;
unsigned long ultimoEnvioDashboard = 0;

void lerSensores() {
  temperatura = dht.readTemperature();
  umidadeAr = dht.readHumidity();

  if (isnan(temperatura)) temperatura = 0;
  if (isnan(umidadeAr)) umidadeAr = 0;

  int leituraSolo = analogRead(PIN_SOLO);
  int leituraLdr = analogRead(PIN_LDR);

  umidadeSolo = map(leituraSolo, 0, 4095, 100, 0);
  luminosidade = map(leituraLdr, 0, 4095, 0, 100);

  umidadeSolo = constrain(umidadeSolo, 0, 100);
  luminosidade = constrain(luminosidade, 0, 100);
}

void verificarBotao() {
  if (digitalRead(PIN_BOTAO) == LOW && millis() - ultimoClique > 400) {
    telaOLED++;
    if (telaOLED > 2) telaOLED = 0;
    ultimoClique = millis();
  }
}

void controlarEstufa() {
  irrigacaoAtiva = umidadeSolo < 30;
  luzArtificialAtiva = luminosidade < 35;
  alertaCritico = temperatura > 35 || temperatura < 10 || umidadeSolo < 15 || umidadeAr < 25;

  anguloServo = temperatura > 30 ? 90 : 0;

  if (alertaCritico) {
    statusEstufa = "CRITICO";
  } else if (irrigacaoAtiva) {
    statusEstufa = "IRRIGANDO";
  } else if (luzArtificialAtiva) {
    statusEstufa = "LUZ ARTIFICIAL";
  } else if (anguloServo > 0) {
    statusEstufa = "VENTILACAO ATIVA";
  } else {
    statusEstufa = "NORMAL";
  }

  digitalWrite(LED_IRRIGACAO, irrigacaoAtiva ? HIGH : LOW);
  digitalWrite(LED_ALERTA, alertaCritico ? HIGH : LOW);
  digitalWrite(LED_LUZ, luzArtificialAtiva ? HIGH : LOW);

  servoVentilacao.write(anguloServo);

  if (alertaCritico) {
    tone(PIN_BUZZER, 1000);
  } else {
    noTone(PIN_BUZZER);
  }
}

String sensoresJSON() {
  String json = "{";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"umidadeAr\":" + String(umidadeAr, 1) + ",";
  json += "\"umidadeSolo\":" + String(umidadeSolo) + ",";
  json += "\"luminosidade\":" + String(luminosidade);
  json += "}";
  return json;
}

String statusJSON() {
  String json = "{";
  json += "\"projeto\":\"GS-Systhesis-IoT-BioEstufa\",";
  json += "\"status\":\"" + statusEstufa + "\",";
  json += "\"alertaCritico\":" + String(alertaCritico ? "true" : "false");
  json += "}";
  return json;
}

String atuadoresJSON() {
  String json = "{";
  json += "\"irrigacaoAtiva\":" + String(irrigacaoAtiva ? "true" : "false") + ",";
  json += "\"luzArtificialAtiva\":" + String(luzArtificialAtiva ? "true" : "false") + ",";
  json += "\"ledAlerta\":" + String(alertaCritico ? "true" : "false") + ",";
  json += "\"buzzerAtivo\":" + String(alertaCritico ? "true" : "false") + ",";
  json += "\"servoVentilacao\":" + String(anguloServo);
  json += "}";
  return json;
}

String geralJSON() {
  String json = "{";
  json += "\"projeto\":\"GS-Systhesis-IoT-BioEstufa\",";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"umidadeAr\":" + String(umidadeAr, 1) + ",";
  json += "\"umidadeSolo\":" + String(umidadeSolo) + ",";
  json += "\"luminosidade\":" + String(luminosidade) + ",";
  json += "\"irrigacaoAtiva\":" + String(irrigacaoAtiva ? "true" : "false") + ",";
  json += "\"luzArtificialAtiva\":" + String(luzArtificialAtiva ? "true" : "false") + ",";
  json += "\"alertaCritico\":" + String(alertaCritico ? "true" : "false") + ",";
  json += "\"servoVentilacao\":" + String(anguloServo) + ",";
  json += "\"status\":\"" + statusEstufa + "\"";
  json += "}";
  return json;
}

void enviarDadosDashboard() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Dashboard: Wi-Fi desconectado");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String json = geralJSON();

  Serial.println("Enviando dados para o dashboard Next...");
  Serial.println(json);

  http.begin(client, dashboardApiUrl);
  http.addHeader("Content-Type", "application/json");

  int codigoResposta = http.POST(json);

  Serial.print("Resposta Dashboard Next: ");
  Serial.println(codigoResposta);

  if (codigoResposta > 0) {
    Serial.println(http.getString());
  } else {
    Serial.println("Erro ao enviar para dashboard Next");
  }

  http.end();
}

void enviarJSON(String json) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "application/json", json);
}

void atualizarOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (telaOLED == 0) {
    display.println("SYSTHESIS BIOESTUFA");
    display.println();
    display.print("Temp: ");
    display.print(temperatura, 1);
    display.println(" C");
    display.print("Umid Ar: ");
    display.print(umidadeAr, 1);
    display.println(" %");
    display.print("Solo: ");
    display.print(umidadeSolo);
    display.println(" %");
    display.print("Luz: ");
    display.print(luminosidade);
    display.println(" %");
  } else if (telaOLED == 1) {
    display.println("ATUADORES");
    display.println();
    display.print("Irrigacao: ");
    display.println(irrigacaoAtiva ? "ON" : "OFF");
    display.print("Luz Artif.: ");
    display.println(luzArtificialAtiva ? "ON" : "OFF");
    display.print("Servo: ");
    display.print(anguloServo);
    display.println(" graus");
    display.print("Buzzer: ");
    display.println(alertaCritico ? "ON" : "OFF");
  } else {
    display.println("STATUS GERAL");
    display.println();
    display.print("Status: ");
    display.println(statusEstufa);
    display.print("Alerta: ");
    display.println(alertaCritico ? "SIM" : "NAO");
    display.println();
    display.println("POST Next API");
  }

  display.display();
}

void handleDashboard() {
  String html = "<html><body><h1>Systhesis BioEstufa</h1><pre>";
  html += geralJSON();
  html += "</pre></body></html>";
  server.send(200, "text/html", html);
}

void handleSensores() {
  enviarJSON(sensoresJSON());
}

void handleStatus() {
  enviarJSON(statusJSON());
}

void handleAtuadores() {
  enviarJSON(atuadoresJSON());
}

void handleGeral() {
  enviarJSON(geralJSON());
}

void handleNaoEncontrado() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "application/json", "{\"erro\":\"Endpoint nao encontrado\"}");
}

void configurarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP Local ESP32: ");
  Serial.println(WiFi.localIP());
  Serial.print("Dashboard Next API: ");
  Serial.println(dashboardApiUrl);
}

void configurarRotas() {
  server.on("/", handleDashboard);
  server.on("/api/sensores", handleSensores);
  server.on("/api/status", handleStatus);
  server.on("/api/atuadores", handleAtuadores);
  server.on("/api/geral", handleGeral);
  server.onNotFound(handleNaoEncontrado);
  server.begin();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_SOLO, INPUT);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  pinMode(LED_IRRIGACAO, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);
  pinMode(LED_LUZ, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  dht.begin();

  servoVentilacao.attach(PIN_SERVO);
  servoVentilacao.write(0);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao iniciar OLED");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("SYSTHESIS");
  display.println("BioEstufa IoT");
  display.println("Iniciando...");
  display.display();

  configurarWiFi();
  configurarRotas();

  lerSensores();
  controlarEstufa();
  atualizarOLED();
  enviarDadosDashboard();

  Serial.println("Sistema iniciado com sucesso.");
}

void loop() {
  server.handleClient();
  verificarBotao();

  if (millis() - ultimaLeitura >= 1000) {
    ultimaLeitura = millis();

    lerSensores();
    controlarEstufa();
    atualizarOLED();

    Serial.println(geralJSON());
    Serial.print("IP Local ESP32: ");
    Serial.println(WiFi.localIP());
  }

  if (millis() - ultimoEnvioDashboard >= 1000) {
    ultimoEnvioDashboard = millis();
    enviarDadosDashboard();
  }
}