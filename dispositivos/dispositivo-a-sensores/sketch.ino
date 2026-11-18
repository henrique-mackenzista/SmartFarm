// SmartFarm - Dispositivo A (sensores)
// Le DHT22 (temperatura e umidade do ar), potenciometro (umidade do solo) e HC-SR04
// (nivel do reservatorio) e publica tudo em JSON no topico
// mack10308763/smartfarm/dispA/telemetria a cada 5 segundos.

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

// ---------- Configuracoes ----------
const char* SSID_NAME = "Wokwi-GUEST";
const char* SSID_PASS = "";

const char* BROKER_URL  = "broker.hivemq.com";  // broker MQTT publico
const int   BROKER_PORT = 1883;
const char* CLIENT_ID   = "mack10308763-dispA";  // deve ser diferente do Dispositivo B

const char* TOPICO_TELEMETRIA = "mack10308763/smartfarm/dispA/telemetria";

const int PIN_DHT  = 15;  // DHT22 - SDA
const int PIN_POT  = 34;  // potenciometro - SIG (umidade do solo)
const int PIN_TRIG = 5;   // HC-SR04 - TRIG
const int PIN_ECHO = 18;  // HC-SR04 - ECHO

const float ALTURA_RESERVATORIO_CM = 100.0;   // distancia da tampa ate o fundo
const unsigned long INTERVALO_MS   = 5000;    // intervalo entre publicacoes

// ---------- Objetos ----------
WiFiClient espClient;
PubSubClient clienteMQTT(espClient);
DHTesp dht;
unsigned long ultimaPublicacao = 0;

// ---------- Funcoes ----------
void conectarWiFi() {
  Serial.print("Conectando-se ao Wi-Fi");
  WiFi.begin(SSID_NAME, SSID_PASS, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  while (!clienteMQTT.connected()) {
    Serial.println("Conectando-se ao broker MQTT...");
    if (clienteMQTT.connect(CLIENT_ID)) {
      Serial.println("Broker MQTT conectado!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(". Nova tentativa em 5 segundos.");
      delay(5000);
    }
  }
}

// Umidade do solo simulada pelo potenciometro (0 a 100%)
int lerUmidadeSolo() {
  return map(analogRead(PIN_POT), 0, 4095, 0, 100);
}

// Nivel do reservatorio em % a partir da distancia medida pelo HC-SR04
float lerNivelReservatorio() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracao = pulseIn(PIN_ECHO, HIGH, 30000);
  float distanciaCm = duracao * 0.0343 / 2.0;
  float nivel = (ALTURA_RESERVATORIO_CM - distanciaCm) / ALTURA_RESERVATORIO_CM * 100.0;
  return constrain(nivel, 0.0f, 100.0f);
}

void publicarTelemetria() {
  float temperatura = dht.getTemperature();
  float umidadeAr   = dht.getHumidity();
  int umidadeSolo   = lerUmidadeSolo();
  float nivel       = lerNivelReservatorio();

  if (isnan(temperatura) || isnan(umidadeAr)) {
    Serial.println("Falha na leitura do DHT22, publicacao ignorada.");
    return;
  }

  char mensagem[160];
  snprintf(mensagem, sizeof(mensagem),
           "{\"umidade_solo\":%d,\"temperatura\":%.1f,\"umidade_ar\":%.1f,\"nivel\":%.0f}",
           umidadeSolo, temperatura, umidadeAr, nivel);

  clienteMQTT.publish(TOPICO_TELEMETRIA, mensagem);
  Serial.print("Telemetria publicada: ");
  Serial.println(mensagem);
}

// ---------- Setup e Loop ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  dht.setup(PIN_DHT, DHTesp::DHT22);

  conectarWiFi();
  clienteMQTT.setServer(BROKER_URL, BROKER_PORT);
}

void loop() {
  if (!clienteMQTT.connected()) {
    conectarMQTT();
  }
  clienteMQTT.loop();  // mantem a conexao ativa

  if (millis() - ultimaPublicacao >= INTERVALO_MS) {
    ultimaPublicacao = millis();
    publicarTelemetria();
  }
}
