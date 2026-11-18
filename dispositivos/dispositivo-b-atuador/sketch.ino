// SmartFarm - Dispositivo B (atuador)
// Assina mack10308763/smartfarm/dispB/comando, liga/desliga o rele da bomba (e o LED)
// e confirma o estado em mack10308763/smartfarm/dispB/status (mensagem retida).

#include <WiFi.h>
#include <PubSubClient.h>

// ---------- Configuracoes ----------
const char* SSID_NAME = "Wokwi-GUEST";
const char* SSID_PASS = "";

const char* BROKER_URL  = "broker.hivemq.com";  // broker MQTT publico
const int   BROKER_PORT = 1883;
const char* CLIENT_ID   = "mack10308763-dispB";  // deve ser diferente do Dispositivo A

const char* TOPICO_COMANDO = "mack10308763/smartfarm/dispB/comando";
const char* TOPICO_STATUS  = "mack10308763/smartfarm/dispB/status";

const int PIN_RELE = 26;  // modulo rele - IN (bomba)
const int PIN_LED  = 27;  // LED indicador da bomba

// ---------- Objetos ----------
WiFiClient espClient;
PubSubClient clienteMQTT(espClient);
bool bombaLigada = false;

void acionarBomba(bool ligar);  // declaracao antecipada

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

// Publica o estado atual da bomba (retido: o Node-RED recebe o ultimo estado ao se conectar)
void publicarStatus() {
  const char* status = bombaLigada ? "{\"bomba\":1}" : "{\"bomba\":0}";
  clienteMQTT.publish(TOPICO_STATUS, status, true);
  Serial.print("Status publicado: ");
  Serial.println(status);
}

void acionarBomba(bool ligar) {
  if (ligar == bombaLigada) return;  // ja esta no estado pedido
  bombaLigada = ligar;
  digitalWrite(PIN_RELE, ligar ? HIGH : LOW);
  digitalWrite(PIN_LED,  ligar ? HIGH : LOW);
  Serial.println(ligar ? "Bomba LIGADA" : "Bomba DESLIGADA");
  publicarStatus();
}

// Recebe {"acao":"ligar"} ou {"acao":"desligar"} no topico de comando
void aoReceberMensagem(char* topico, byte* payload, unsigned int tamanho) {
  String mensagem;
  for (unsigned int i = 0; i < tamanho; i++) {
    mensagem += (char)payload[i];
  }
  Serial.print("Comando recebido: ");
  Serial.println(mensagem);

  // "desligar" primeiro, porque a palavra contem "ligar"
  if (mensagem.indexOf("\"desligar\"") >= 0) {
    acionarBomba(false);
  } else if (mensagem.indexOf("\"ligar\"") >= 0) {
    acionarBomba(true);
  }
}

void conectarMQTT() {
  while (!clienteMQTT.connected()) {
    Serial.println("Conectando-se ao broker MQTT...");
    if (clienteMQTT.connect(CLIENT_ID)) {
      Serial.println("Broker MQTT conectado!");
      clienteMQTT.subscribe(TOPICO_COMANDO);
      publicarStatus();  // informa o estado atual ao conectar
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(". Nova tentativa em 5 segundos.");
      delay(5000);
    }
  }
}

// ---------- Setup e Loop ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELE, LOW);
  digitalWrite(PIN_LED, LOW);

  conectarWiFi();
  clienteMQTT.setServer(BROKER_URL, BROKER_PORT);
  clienteMQTT.setCallback(aoReceberMensagem);
}

void loop() {
  if (!clienteMQTT.connected()) {
    conectarMQTT();
  }
  clienteMQTT.loop();  // recebe os comandos e mantem a conexao ativa
}
