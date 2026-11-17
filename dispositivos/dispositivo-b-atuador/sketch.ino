// SmartFarm - Dispositivo B (atuador)
// ESP32 simulado no Wokwi. Assina smartfarm/dispB/comando e liga ou desliga o relé
// que representa a bomba de irrigação. A cada mudança publica o estado em
// smartfarm/dispB/status como mensagem retida.

#include <WiFi.h>
#include <PubSubClient.h>

// ---------- Configuração ----------
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* BROKER_HOST     = "SEU_IP_ELASTICO";   // IP público (elástico) do broker Mosquitto
const int   BROKER_PORT     = 1883;
const char* BROKER_USER     = "";                  // preencha se o broker exigir autenticação
const char* BROKER_PASSWORD = "";
const char* CLIENT_ID       = "smartfarm-dispB";   // precisa ser único no broker

const char* TOPICO_COMANDO = "smartfarm/dispB/comando";
const char* TOPICO_STATUS  = "smartfarm/dispB/status";

// ---------- Pinos ----------
#define PINO_RELE 26   // relé que representa a bomba
#define PINO_LED  27   // LED que acende junto com a bomba

WiFiClient espClient;
PubSubClient mqtt(espClient);
bool bombaLigada = false;

void publicarStatus() {
  const char* payload = bombaLigada ? "{\"bomba\":1}" : "{\"bomba\":0}";
  mqtt.publish(TOPICO_STATUS, payload, true);   // retida: quem conectar depois recebe o estado atual
  Serial.print("Status publicado: ");
  Serial.println(payload);
}

void definirBomba(bool ligar) {
  if (ligar == bombaLigada) return;   // nada mudou
  bombaLigada = ligar;
  digitalWrite(PINO_RELE, ligar ? HIGH : LOW);
  digitalWrite(PINO_LED, ligar ? HIGH : LOW);
  Serial.println(ligar ? "Bomba LIGADA" : "Bomba DESLIGADA");
  publicarStatus();
}

void aoReceberMensagem(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem;
  for (unsigned int i = 0; i < tamanho; i++) mensagem += (char)conteudo[i];
  Serial.print("Comando recebido: ");
  Serial.println(mensagem);

  // Formato esperado: {"acao":"ligar"} ou {"acao":"desligar"}
  if (mensagem.indexOf("\"desligar\"") >= 0) {
    definirBomba(false);
  } else if (mensagem.indexOf("\"ligar\"") >= 0) {
    definirBomba(true);
  } else {
    Serial.println("Comando desconhecido; ignorado.");
  }
}

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
      mqtt.subscribe(TOPICO_COMANDO);
      publicarStatus();   // informa o estado atual a cada (re)conexão
    } else {
      Serial.print("falhou (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Nova tentativa em 5 s.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_RELE, OUTPUT);
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_RELE, LOW);   // começa desligada, por segurança
  digitalWrite(PINO_LED, LOW);

  conectarWiFi();
  mqtt.setServer(BROKER_HOST, BROKER_PORT);
  mqtt.setCallback(aoReceberMensagem);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();
  if (!mqtt.connected()) conectarBroker();
  mqtt.loop();
}
