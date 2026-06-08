#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
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
    display.println("Botao troca tela");
  }

  display.display();
}

void handleDashboard() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Systhesis BioEstufa</title>
<style>
body{margin:0;background:radial-gradient(circle at top,#172554,#020617 65%);color:white;font-family:Arial,sans-serif;padding:24px;text-align:center}
h1{color:#7df9ff}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:16px;max-width:900px;margin:0 auto}
.card{background:#0f172a;border:1px solid rgba(125,249,255,.25);border-radius:18px;padding:20px}
.label{color:#94a3b8;font-size:14px}
.value{font-size:28px;font-weight:bold;color:#38bdf8}
.status{margin-top:24px;font-size:24px;font-weight:bold;color:#22c55e}
.critico{color:#ef4444}
.footer{margin-top:26px;color:#94a3b8;font-size:13px}
</style>
</head>
<body>
<h1>SYSTHESIS BIOESTUFA</h1>
<p>Dashboard IoT para cultivo em ambientes extremos</p>

<div class="grid">
<div class="card"><div class="label">Temperatura</div><div class="value" id="temperatura">--</div></div>
<div class="card"><div class="label">Umidade do Ar</div><div class="value" id="umidadeAr">--</div></div>
<div class="card"><div class="label">Umidade do Solo</div><div class="value" id="umidadeSolo">--</div></div>
<div class="card"><div class="label">Luminosidade</div><div class="value" id="luminosidade">--</div></div>
<div class="card"><div class="label">Irrigação</div><div class="value" id="irrigacao">--</div></div>
<div class="card"><div class="label">Luz Artificial</div><div class="value" id="luzArtificial">--</div></div>
<div class="card"><div class="label">Servo Ventilação</div><div class="value" id="servo">--</div></div>
<div class="card"><div class="label">Alerta</div><div class="value" id="alerta">--</div></div>
</div>

<div class="status" id="status">Carregando...</div>
<div class="footer">Endpoints JSON: /api/sensores | /api/status | /api/atuadores | /api/geral</div>

<script>
async function atualizarDashboard(){
const r=await fetch('/api/geral');
const d=await r.json();

temperatura.innerText=d.temperatura+' °C';
umidadeAr.innerText=d.umidadeAr+' %';
umidadeSolo.innerText=d.umidadeSolo+' %';
luminosidade.innerText=d.luminosidade+' %';
irrigacao.innerText=d.irrigacaoAtiva?'ON':'OFF';
luzArtificial.innerText=d.luzArtificialAtiva?'ON':'OFF';
servo.innerText=d.servoVentilacao+'°';
alerta.innerText=d.alertaCritico?'SIM':'NAO';

status.innerText='Status: '+d.status;
if(d.alertaCritico){status.classList.add('critico')}else{status.classList.remove('critico')}
}
atualizarDashboard();
setInterval(atualizarDashboard,2000);
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleSensores() {
  server.send(200, "application/json", sensoresJSON());
}

void handleStatus() {
  server.send(200, "application/json", statusJSON());
}

void handleAtuadores() {
  server.send(200, "application/json", atuadoresJSON());
}

void handleGeral() {
  server.send(200, "application/json", geralJSON());
}

void handleNaoEncontrado() {
  server.send(404, "application/json", "{\"erro\":\"Endpoint nao encontrado\"}");
}

void configurarWiFi() {
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP Dashboard: ");
  Serial.println(WiFi.localIP());
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

  Serial.println("Sistema iniciado com sucesso.");
}

void loop() {
  server.handleClient();
  verificarBotao();

  if (millis() - ultimaLeitura >= 2000) {
    ultimaLeitura = millis();

    lerSensores();
    controlarEstufa();
    atualizarOLED();

    Serial.println(geralJSON());
    Serial.print("IP Dashboard: ");
    Serial.println(WiFi.localIP());
  }
}