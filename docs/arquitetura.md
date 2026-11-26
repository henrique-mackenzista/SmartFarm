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

## Endereços (FQDN e IP)

Consultados por DNS em 26/11/2026. Os IPs dos serviços em nuvem podem mudar, pois eles usam balanceamento de carga.

| Elemento | FQDN | IP | Onde roda |
|---|---|---|---|
| Dispositivos A e B | wokwi.com (simulador) | 10.10.0.2 na rede Wokwi-GUEST; saída por 172.67.75.230 | servidores do Wokwi |
| Broker MQTT | broker.hivemq.com | 18.197.15.210 | nuvem da HiveMQ |
| Node-RED | localhost, porta 1880 | 192.168.0.11 (rede local) | contêiner Docker no computador da equipe |
| API de clima | api.openweathermap.org | 169.197.141.10 | nuvem do OpenWeather |
| API do bot | api.telegram.org | 149.154.166.110 | nuvem do Telegram |
| Banco de dados | us-east-1-1.aws.cloud2.influxdata.com | 34.196.233.7 | InfluxDB Cloud (AWS us-east-1) |
| Dashboards | radiantermine1510.grafana.net | 104.18.13.97 | Grafana Cloud |
