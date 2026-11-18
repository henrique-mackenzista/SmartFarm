# Banco de dados

O banco de dados é o **InfluxDB**, um banco de séries temporais.

Quem grava nele é o **Node-RED**, não os dispositivos. Os ESP32 publicam no broker MQTT; o Node-RED assina os tópicos, aplica as regras de negócio e grava o resultado com o nó `influxdb out` (paleta `node-red-contrib-influxdb`).

A conexão usa a API HTTP do InfluxDB, autenticada por token, com a organização e o bucket `smartfarm` definidos na configuração do nó. O token não é publicado neste repositório (veja [`.env.example`](../.env.example)). O Grafana lê o mesmo bucket para montar os dashboards.

## O que é gravado

Cada registro é gravado com o horário e uma tag que identifica a origem.

| Measurement | Tags | Campos | Quando |
|---|---|---|---|
| `telemetria` | `dispositivo=dispA` | `umidade_solo`, `umidade_ar`, `temperatura`, `nivel` | a cada 5 s |
| `bomba` | `dispositivo=dispB`, `motivo` | `estado` (1 = ligada, 0 = desligada) | a cada mudança |
| `irrigacao` | `dispositivo=dispB` | `duracao_s`, `litros` | quando a bomba desliga |
| `clima` | `fonte=openweather` | `temperatura`, `prob_chuva` | a cada 5 min |
| `alertas` | `tipo` | `mensagem`, `enviado` (1 = enviado ao Telegram) | quando um alerta é gerado |
| `comandos` | `comando` | `chat` (ID anonimizado), `respondido` (1 = sim) | quando o bot recebe um comando |

Valores da tag `motivo`: `demanda`, `chuva`, `reservatorio`, `tempo_maximo`, `falha`.

Valores da tag `tipo`: `chuva_prevista`, `reservatorio_critico`, `sem_telemetria`, `sensor_divergente`, `tempo_maximo`.
