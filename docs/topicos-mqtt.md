# Tópicos MQTT

A comunicação entre os dispositivos e o Node-RED usa o protocolo MQTT, no modelo publicação/assinatura, por meio do broker público **HiveMQ** (`broker.hivemq.com`, porta 1883).

Os ESP32 do Wokwi rodam na nuvem do Wokwi e não alcançam um broker instalado no computador da equipe; por isso o broker é público. Como qualquer pessoa pode usar um broker público, **todos os tópicos e client IDs começam com `mack10308763`**, para não colidir com outros usuários nem permitir que alguém publique na bomba por acaso.

Os tópicos seguem a hierarquia `mack10308763/smartfarm/<dispositivo>/<tipo>` e todas as mensagens são JSON.

| Tópico | Publica | Assina | Frequência | Retida |
|---|---|---|---|---|
| `mack10308763/smartfarm/dispA/telemetria` | Dispositivo A | Node-RED | a cada 5 s | não |
| `mack10308763/smartfarm/dispB/comando` | Node-RED | Dispositivo B | quando uma regra decide ligar ou desligar | não |
| `mack10308763/smartfarm/dispB/status` | Dispositivo B | Node-RED | ao conectar e a cada mudança | sim |

## Mensagens

### `.../dispA/telemetria`

```json
{"umidade_solo": 43.0, "temperatura": 27.4, "umidade_ar": 61.0, "nivel": 52.0}
```

| Campo | Unidade | Origem no Wokwi |
|---|---|---|
| `umidade_solo` | % | potenciômetro (o Wokwi não tem sensor de umidade de solo) |
| `temperatura` | °C | DHT22 |
| `umidade_ar` | % | DHT22 |
| `nivel` | % do reservatório | HC-SR04 medindo a distância até a superfície da água |

### `.../dispB/comando`

```json
{"acao": "ligar"}
```

`acao` aceita `"ligar"` ou `"desligar"`.

### `.../dispB/status`

```json
{"bomba": 1}
```

`1` = ligada, `0` = desligada. A mensagem é retida, então o Node-RED recebe o estado atual da bomba assim que se conecta.

## Identificadores de cliente

Cada cliente usa um identificador único no broker. Dois clientes com o mesmo identificador derrubam um ao outro.

| Cliente | Client ID |
|---|---|
| Dispositivo A | `mack10308763-dispA` |
| Dispositivo B | `mack10308763-dispB` |
| Node-RED | `mack10308763-nodered` |

## Acompanhar as mensagens

Com o [MQTT Explorer](https://mqtt-explorer.com), conecte em `broker.hivemq.com:1883` e assine `mack10308763/smartfarm/#` para ver todas as mensagens do projeto. Também é possível publicar por ali um `{"acao":"ligar"}` para testar o Dispositivo B sozinho.
