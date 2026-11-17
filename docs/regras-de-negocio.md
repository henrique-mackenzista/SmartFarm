# Regras de negócio

As regras ficam no Node-RED e são avaliadas a cada telemetria do Dispositivo A (a cada 5 segundos). Os limites são variáveis de fluxo, para que possam ser ajustados sem alterar o fluxo.

| Parâmetro | Valor | Uso |
|---|---|---|
| Umidade mínima | 30% | abaixo disso, irrigar |
| Umidade alvo | 60% | ao atingir, desligar |
| Nível crítico do reservatório | 20% | abaixo disso, bloquear a bomba |
| Nível de liberação | 25% | acima disso, liberar o bloqueio |
| Probabilidade de chuva | 60% | a partir disso, adiar a irrigação |
| Tempo máximo de irrigação | 15 min | segurança contra falhas |
| Vazão nominal da bomba | 2 L/min | estimativa do volume usado |

Os limites de umidade foram definidos para a simulação e devem ser ajustados ao tipo de solo e à cultura irrigada.

Quando mais de uma regra se aplica ao mesmo tempo, as de segurança (R3, R4 e R5) têm prioridade sobre a de irrigação (R1).

## Regras

| Regra | Condição | Ação |
|---|---|---|
| **R1 – Irrigação por demanda** | umidade do solo < 30% | publica `{"acao":"ligar"}` em `smartfarm/dispB/comando`; desliga quando a umidade chega a 60% |
| **R2 – Adiamento por chuva** | probabilidade de chuva nas próximas 3 h ≥ 60% (OpenWeather, consulta a cada 5 min) | suspende a R1 e avisa pelo Telegram |
| **R3 – Proteção do reservatório** | nível < 20% | desliga e bloqueia a bomba, envia alerta; libera acima de 25% |
| **R4 – Tempo máximo de irrigação** | bomba ligada há 15 min sem o solo atingir 60% | desliga a bomba e alerta possível falha no sensor ou vazamento |
| **R5 – Falha de comunicação** | Dispositivo A sem publicar há mais de 60 s | desliga a bomba por segurança e avisa o usuário |
| **R6 – Validação do sensor** | temperatura do DHT22 difere mais de 5 °C da do OpenWeather por 30 min | alerta de possível falha no sensor |
| **R7 – Registro de consumo** | mudança em `smartfarm/dispB/status` | grava a duração do acionamento e o volume estimado (duração × 2 L/min) no InfluxDB |

A faixa entre 30% e 60% (histerese) evita que a bomba fique ligando e desligando. Cada tipo de alerta respeita um intervalo mínimo de 30 minutos entre envios, para não lotar o Telegram.

## Como validar cada regra no Wokwi

| Regra | Teste |
|---|---|
| R1 | girar o potenciômetro do Dispositivo A abaixo de 30%; depois acima de 60% |
| R2 | injetar no fluxo uma previsão de chuva fictícia com um nó inject |
| R3 | aumentar a distância do HC-SR04 até o nível ficar abaixo de 20% |
| R4 | reduzir o tempo máximo durante o teste e manter a umidade abaixo de 60% |
| R5 | parar a simulação do Dispositivo A |
| R6 | ajustar a temperatura do DHT22 no Wokwi para longe da temperatura real |
| R7 | conferir no InfluxDB/Grafana o registro gravado após um ciclo de irrigação |
