# 🌱 Systhesis BioEstufa Espacial IoT

## 👨‍💻 Integrantes

- João Victor Alcântara — RM562707
- Phillipo Barbosa — RM565399
- Eduardo Martins — RM562259

---

# 📌 Descrição do Projeto

Protótipo IoT desenvolvido para monitoramento e automação de uma bioestufa inteligente voltada para produção de alimentos em colônias espaciais.

O sistema utiliza sensores e atuadores conectados ao ESP32 para monitorar o ambiente e executar ações automáticas que auxiliam no cultivo.

---

# 🌎 Problema

A produção de alimentos em ambientes extremos exige monitoramento constante das condições ambientais.

Variações inadequadas de temperatura, umidade ou luminosidade podem comprometer totalmente uma plantação.

---

# ✅ Solução

O sistema realiza:

- Leitura de temperatura
- Leitura de umidade do ar
- Leitura de umidade do solo
- Leitura de luminosidade
- Irrigação automática
- Controle de ventilação
- Controle de iluminação artificial
- Alertas críticos

---

# 🛠️ Tecnologias Utilizadas

- ESP32
- Arduino Framework
- PlatformIO
- Wokwi
- DHT22
- Sensor de Umidade do Solo
- Sensor LDR
- OLED SSD1306
- Servo Motor
- LEDs
- Buzzer
- Wi-Fi

---

# 🌡️ Sensores

## DHT22

Responsável por:

- Temperatura
- Umidade do ar

---

## Sensor de Umidade do Solo

Responsável por:

- Controle da irrigação

---

## Sensor LDR

Responsável por:

- Controle de luminosidade

---

# ⚙️ Atuadores

- Servo Motor
- LEDs Indicadores
- Buzzer
- Display OLED

---

# 🔌 Comunicação

O ESP32 disponibiliza uma API REST local para consulta dos dados do sistema.

Exemplo:

```json
{
  "temperatura": 28.5,
  "umidadeAr": 45,
  "umidadeSolo": 62,
  "luminosidade": 40,
  "status": "NORMAL"
}
```

---

# 📁 Estrutura do Projeto

```txt
src/
 └── main.cpp

platformio.ini

diagram.json

wokwi.toml
```

---

# 🌐 Endpoints

```http
/api/sensores
```

```http
/api/status
```

```http
/ api/atuadores
```

```http
/api/geral
```

---

# ▶️ Como Executar

## 1. Abrir no Wokwi

Importar:

- diagram.json
- wokwi.toml
- src/main.cpp

---

## 2. Iniciar Simulação

Executar o projeto.

---

## 3. Alterar Sensores

Modificar valores dos sensores para observar:

- Irrigação automática
- Ventilação
- Luz artificial
- Alertas

---

# 📷 Evidências

## Circuito ESP32

Adicionar imagem do circuito.

---

## Simulação

Adicionar captura do Wokwi.

---

## OLED

Adicionar captura do display.

---

# 🔗 Dashboard Integrado

Deploy:

https://gs-systhesis-io-t-dashboard.vercel.app/

GitHub:

https://github.com/alc-joao/GS-Systhesis-IoT-Dashboard

---

# 🎥 Vídeo Pitch

Adicionar link do vídeo.

---

# 📦 Entrega

O projeto contém:

- ESP32
- Sensores
- Atuadores
- OLED
- API REST
- Dashboard
- Simulação Wokwi
- Documentação

---

# 🌱 Systhesis BioEstufa Espacial

Projeto acadêmico desenvolvido para a Global Solution FIAP.