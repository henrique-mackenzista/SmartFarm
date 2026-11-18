# Arquitetura

O protótipo roda sem servidores próprios na nuvem: os dispositivos são simulados no Wokwi, o broker MQTT é público e o Node-RED roda no computador da equipe.

## Onde roda cada serviço

| Serviço | Onde roda | Endereço |
|---|---|---|
| Dispositivos A e B | Wokwi (rede Wokwi-GUEST) | saída pelo gateway do Wokwi |
| Broker MQTT | HiveMQ público | `broker.hivemq.com:1883` |
| Node-RED | computador da equipe | `http://localhost:1880` |
| InfluxDB | _a definir_ | _a preencher_ |
| Grafana | _a definir_ | _a preencher_ |
| OpenWeather e Telegram Bot API | serviços externos | `api.openweathermap.org`, `api.telegram.org` |

> O enunciado do projeto determina que broker, Node-RED, banco de dados e gerador de dashboards não fiquem na mesma máquina, a menos que sejam máquinas virtuais distintas. O broker já é externo (HiveMQ). Para o InfluxDB e o Grafana, as opções que respeitam essa regra sem instalar servidores são o **InfluxDB Cloud** e o **Grafana Cloud**, ambos com plano gratuito.

## Por que o broker é público

Os ESP32 simulados no Wokwi rodam na nuvem do Wokwi e acessam a internet pela rede Wokwi-GUEST. Eles não conseguem alcançar um broker instalado no computador da equipe. O HiveMQ público é acessível tanto pelo Wokwi quanto pelo Node-RED local. Para evitar colisões e acessos indevidos, os tópicos e client IDs usam o prefixo `mack10308763` ([detalhes](topicos-mqtt.md)).

## Fluxo dos dados

1. O Dispositivo A publica a telemetria no broker a cada 5 s.
2. O Node-RED recebe a telemetria, consulta o OpenWeather e aplica as [regras de negócio](regras-de-negocio.md).
3. Quando uma regra decide, o Node-RED publica o comando para o Dispositivo B, que aciona o relé e publica o novo estado da bomba.
4. O Node-RED grava telemetria, estados, consumo, clima, alertas e comandos no InfluxDB e envia os alertas pelo Telegram.
5. O Grafana lê o InfluxDB e mostra os [dashboards](dashboards.md).
6. Pelo [bot do Telegram](bot-telegram.md), o usuário consulta a situação do campo a qualquer momento; o Node-RED responde com os valores mais recentes.
