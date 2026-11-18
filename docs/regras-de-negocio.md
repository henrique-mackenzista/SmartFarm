# Regras de negócio

As regras ficam no Node-RED (aba "1. Dispositivos e regras" do [`flows.json`](../node-red/flows.json)) e são avaliadas a cada telemetria do Dispositivo A, a cada 10 segundos e a cada nova previsão do tempo. Os limites ficam no nó "Parâmetros e estado inicial" e podem ser ajustados sem alterar a lógica.

| Parâmetro | Valor | Uso |
|---|---|---|
| Umidade mínima | 30% | abaixo disso, irrigar |
| Umidade alvo | 60% | ao atingir, desligar |
| Nível crítico do reservatório | 20% | abaixo disso, bloquear a bomba |
| Nível de liberação | 25% | acima disso, liberar o bloqueio |
| Probabilidade de chuva | 60% | a partir disso, adiar a irrigação |
| Tempo máximo de irrigação | 15 min | segurança contra falhas |
| Pausa após o tempo máximo | 60 min | evita religar a bomba logo após a R4 |
| Vazão nominal da bomba | 2 L/min | estimativa do volume usado |

Os limites de umidade foram definidos para a simulação e devem ser ajustados ao tipo de solo e à cultura irrigada.

Quando mais de uma regra se aplica ao mesmo tempo, as de segurança (R3, R4 e R5) têm prioridade sobre a de irrigação (R1).

## Regras

| Regra | Condição | Ação |
|---|---|---|
| **R1 – Irrigação por demanda** | umidade do solo < 30% | publica `{"acao":"ligar"}` em `.../dispB/comando`; desliga quando a umidade chega a 60% |
| **R2 – Adiamento por chuva** | probabilidade de chuva nas próximas 3 h ≥ 60% (OpenWeather, consulta a cada 5 min) | suspende a R1 e avisa pelo Telegram |
| **R3 – Proteção do reservatório** | nível < 20% | desliga e bloqueia a bomba, envia alerta; libera acima de 25% |
| **R4 – Tempo máximo de irrigação** | bomba ligada há 15 min sem o solo atingir 60% | desliga a bomba, pausa a irrigação automática por 60 min e alerta possível falha no sensor ou vazamento |
| **R5 – Falha de comunicação** | Dispositivo A sem publicar há mais de 60 s | desliga a bomba por segurança e avisa o usuário; avisa de novo quando a telemetria volta |
| **R6 – Validação do sensor** | temperatura do DHT22 difere mais de 5 °C da do OpenWeather por 30 min | alerta de possível falha no sensor |
| **R7 – Registro de consumo** | mudança em `.../dispB/status` | grava a duração do acionamento e o volume estimado (duração × 2 L/min) no InfluxDB |

A faixa entre 30% e 60% (histerese) evita que a bomba fique ligando e desligando. Cada tipo de alerta respeita um intervalo mínimo de 30 minutos entre envios, para não lotar o Telegram.

## Como validar cada regra no Wokwi

| Regra | Teste |
|---|---|
| R1 | girar o potenciômetro do Dispositivo A abaixo de 30%; depois acima de 60% |
| R2 | definir `SMARTFARM_PARAMETROS` com `{"chuvaLimite": 0}`, para que qualquer previsão conte como chuva, e deixar o solo abaixo de 30% |
| R3 | aumentar a distância do HC-SR04 até o nível ficar abaixo de 20% |
| R4 | reduzir o tempo máximo com `SMARTFARM_PARAMETROS` ([node-red/README.md](../node-red/README.md)) e manter a umidade abaixo de 60% |
| R5 | parar a simulação do Dispositivo A |
| R6 | ajustar a temperatura do DHT22 no Wokwi para longe da temperatura real |
| R7 | conferir no InfluxDB/Grafana o registro gravado após um ciclo de irrigação |
