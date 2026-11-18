# Arquitetura

O protótipo roda sem servidores próprios: os dispositivos são simulados no Wokwi, o broker MQTT é público, o Node-RED roda no computador da equipe (em um container Docker) e o banco de dados e os dashboards usam os planos gratuitos do InfluxDB Cloud e do Grafana Cloud.

## Onde roda cada serviço

| Serviço | Onde roda | Endereço |
|---|---|---|
| Dispositivos A e B | Wokwi (rede Wokwi-GUEST) | saída pelo gateway do Wokwi |
| Broker MQTT | HiveMQ público | `broker.hivemq.com:1883` |
| Node-RED | computador da equipe (container Docker `nodered/node-red`) | `http://localhost:1880` |
| InfluxDB | InfluxDB Cloud Serverless, plano Free (AWS us-east-1) | `https://us-east-1-1.aws.cloud2.influxdata.com`, bucket `smartfarm` |
| Grafana | Grafana Cloud, plano Free | lê o bucket `smartfarm` com um token só de leitura |
| OpenWeather e Telegram Bot API | serviços externos | `api.openweathermap.org`, `api.telegram.org` |

> O enunciado do projeto determina que broker, Node-RED, banco de dados e gerador de dashboards não fiquem na mesma máquina, a menos que sejam máquinas virtuais distintas. Com essa distribuição, cada serviço roda em uma infraestrutura diferente: o broker no HiveMQ, o Node-RED no computador da equipe, o banco no InfluxDB Cloud e os dashboards no Grafana Cloud.
>
> O plano gratuito do InfluxDB Cloud guarda os dados por até 30 dias, o suficiente para o protótipo.

## Por que o broker é público

Os ESP32 simulados no Wokwi rodam na nuvem do Wokwi e acessam a internet pela rede Wokwi-GUEST. Eles não conseguem alcançar um broker instalado no computador da equipe. O HiveMQ público é acessível tanto pelo Wokwi quanto pelo Node-RED local. Para evitar colisões e acessos indevidos, os tópicos e client IDs usam o prefixo `mack10308763` ([detalhes](topicos-mqtt.md)).

## Fluxo dos dados

1. O Dispositivo A publica a telemetria no broker a cada 5 s.
2. O Node-RED recebe a telemetria, consulta o OpenWeather e aplica as [regras de negócio](regras-de-negocio.md).
3. Quando uma regra decide, o Node-RED publica o comando para o Dispositivo B, que aciona o relé e publica o novo estado da bomba.
4. O Node-RED grava telemetria, estados, consumo, clima, alertas e comandos no InfluxDB e envia os alertas pelo Telegram.
5. O Grafana lê o InfluxDB e mostra os [dashboards](dashboards.md).
6. Pelo [bot do Telegram](bot-telegram.md), o usuário consulta a situação do campo a qualquer momento; o Node-RED responde com os valores mais recentes.
