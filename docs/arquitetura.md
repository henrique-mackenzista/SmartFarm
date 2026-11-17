# Arquitetura e infraestrutura

O enunciado do projeto exige que o broker MQTT, o Node-RED, o banco de dados e o gerador de dashboards **não** fiquem na mesma máquina, a menos que sejam máquinas virtuais distintas. Por isso cada serviço roda numa máquina separada.

## Máquinas e serviços

> Os endereços abaixo serão preenchidos com os valores reais do protótipo.

| Serviço | Máquina | FQDN | IP | Portas |
|---|---|---|---|---|
| Broker MQTT (Mosquitto) | EC2 – AWS Academy, IP elástico | _a preencher_ | _a preencher_ | 1883 |
| Node-RED | EC2 – AWS Academy | _a preencher_ | _a preencher_ | 1880 |
| InfluxDB | _a definir_ | _a preencher_ | _a preencher_ | 8086 |
| Grafana | _a definir_ | _a preencher_ | _a preencher_ | 3000 |
| Dispositivos A e B | Wokwi (rede Wokwi-GUEST) | — | saída pelo gateway do Wokwi | — |

O broker usa IP elástico para que o endereço não mude quando a instância for reiniciada; assim o código dos dispositivos não precisa ser alterado.

## Regras de firewall (security groups)

| Máquina | Porta | Origem | Motivo |
|---|---|---|---|
| Broker | 1883/TCP | 0.0.0.0/0 | os dispositivos saem por um gateway do Wokwi de endereço desconhecido |
| Broker | 22/TCP | IP da equipe | administração |
| Node-RED | 1880/TCP | IP da equipe | editor do Node-RED |
| Node-RED | 22/TCP | IP da equipe | administração |

## Fluxo dos dados

1. O Dispositivo A publica a telemetria no broker a cada 5 s.
2. O Node-RED recebe a telemetria, consulta o OpenWeather e aplica as [regras de negócio](regras-de-negocio.md).
3. Quando uma regra decide, o Node-RED publica o comando para o Dispositivo B, que aciona o relé e publica o novo estado da bomba.
4. O Node-RED grava telemetria, estados, consumo, clima e alertas no InfluxDB e envia os alertas pelo Telegram.
5. O Grafana lê o InfluxDB e mostra os [dashboards](dashboards.md).
