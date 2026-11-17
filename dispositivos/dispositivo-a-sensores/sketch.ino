// SmartFarm - Dispositivo A (sensores)
// ESP32 simulado no Wokwi. Lê umidade do solo (potenciômetro), temperatura e
// umidade do ar (DHT22) e nível do reservatório (HC-SR04) e publica tudo em JSON
// no tópico smartfarm/dispA/telemetria a cada 5 segundos.

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ---------- Configuração ----------
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* BROKER_HOST     = "SEU_IP_ELASTICO";   // IP público (elástico) do broker Mosquitto
const int   BROKER_PORT     = 1883;
const char* BROKER_USER     = "";                  // preencha se o broker exigir autenticação
const char* BROKER_PASSWORD = "";
const char* CLIENT_ID       = "smartfarm-dispA";   // precisa ser único no broker

const char* TOPICO_TELEMETRIA = "smartfarm/dispA/telemetria";
const unsigned long INTERVALO_PUBLICACAO_MS = 5000;

// ---------- Pinos ----------
#define PINO_DHT          15
#define TIPO_DHT          DHT22
#define PINO_UMIDADE_SOLO 34   // saída do potenciômetro (entrada analógica)
#define PINO_TRIG         5
#define PINO_ECHO         18

// ---------- Reservatório ----------
// O HC-SR04 fica na tampa do reservatório e mede a distância até a água.
// Reservatório cheio: distância mínima. Reservatório vazio: distância = altura.
const float ALTURA_RESERVATORIO_CM = 100.0;

WiFiClient espClient;
PubSubClient mqtt(espClient);
DHT dht(PINO_DHT, TIPO_DHT);
unsigned long ultimaPublicacao = 0;

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.print(" conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void conectarBroker() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    if (mqtt.connect(CLIENT_ID, BROKER_USER, BROKER_PASSWORD)) {
      Serial.println("conectado.");
    } else {
      Serial.print("falhou (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Nova tentativa em 5 s.");
      delay(5000);
    }
  }
}

float lerUmidadeSolo() {
  // O Wokwi não tem sensor de umidade de solo: o potenciômetro representa a leitura.
  int leitura = analogRead(PINO_UMIDADE_SOLO);   // 0 a 4095
  return leitura * 100.0 / 4095.0;
}

float lerNivelReservatorio() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);

  long duracao = pulseIn(PINO_ECHO, HIGH, 30000);  // timeout de 30 ms
  if (duracao == 0) return NAN;

  float distanciaCm = duracao * 0.0343 / 2.0;
  float nivel = (ALTURA_RESERVATORIO_CM - distanciaCm) * 100.0 / ALTURA_RESERVATORIO_CM;
  return constrain(nivel, 0.0f, 100.0f);
}

void publicarTelemetria() {
  float umidadeSolo = lerUmidadeSolo();
  float temperatura = dht.readTemperature();
  float umidadeAr   = dht.readHumidity();
  float nivel       = lerNivelReservatorio();

  if (isnan(temperatura) || isnan(umidadeAr) || isnan(nivel)) {
    Serial.println("Leitura inválida de sensor; publicação ignorada.");
    return;
  }

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"umidade_solo\":%.1f,\"temperatura\":%.1f,\"umidade_ar\":%.1f,\"nivel\":%.1f}",
           umidadeSolo, temperatura, umidadeAr, nivel);

  mqtt.publish(TOPICO_TELEMETRIA, payload);
  Serial.print("Publicado em ");
  Serial.print(TOPICO_TELEMETRIA);
  Serial.print(": ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  dht.begin();

  conectarWiFi();
  mqtt.setServer(BROKER_HOST, BROKER_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();
  if (!mqtt.connected()) conectarBroker();
  mqtt.loop();

  if (millis() - ultimaPublicacao >= INTERVALO_PUBLICACAO_MS) {
    ultimaPublicacao = millis();
    publicarTelemetria();
  }
}
