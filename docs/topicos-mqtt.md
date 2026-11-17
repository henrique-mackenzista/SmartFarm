# Tópicos MQTT

A comunicação entre os dispositivos e o Node-RED usa o protocolo MQTT, no modelo publicação/assinatura, por meio de um broker Eclipse Mosquitto na porta 1883.

Os tópicos seguem a hierarquia `smartfarm/<dispositivo>/<tipo>` e todas as mensagens são JSON.

| Tópico | Publica | Assina | Frequência | Retida |
|---|---|---|---|---|
| `smartfarm/dispA/telemetria` | Dispositivo A | Node-RED | a cada 5 s | não |
| `smartfarm/dispB/comando` | Node-RED | Dispositivo B | quando uma regra decide ligar ou desligar | não |
| `smartfarm/dispB/status` | Dispositivo B | Node-RED | ao conectar e a cada mudança | sim |

## Mensagens

### `smartfarm/dispA/telemetria`

```json
{"umidade_solo": 43.0, "temperatura": 27.4, "umidade_ar": 61.0, "nivel": 52.0}
```

| Campo | Unidade | Origem no Wokwi |
|---|---|---|
| `umidade_solo` | % | potenciômetro (o Wokwi não tem sensor de umidade de solo) |
| `temperatura` | °C | DHT22 |
| `umidade_ar` | % | DHT22 |
| `nivel` | % do reservatório | HC-SR04 medindo a distância até a superfície da água |

### `smartfarm/dispB/comando`

```json
{"acao": "ligar"}
```

`acao` aceita `"ligar"` ou `"desligar"`.

### `smartfarm/dispB/status`

```json
{"bomba": 1}
```

`1` = ligada, `0` = desligada. A mensagem é retida, então o Node-RED recebe o estado atual da bomba assim que se conecta.

## Identificadores de cliente

Cada cliente usa um identificador único no broker. Dois clientes com o mesmo identificador derrubam um ao outro.

| Cliente | Client ID |
|---|---|
| Dispositivo A | `smartfarm-dispA` |
| Dispositivo B | `smartfarm-dispB` |
| Node-RED | `smartfarm-nodered` |
