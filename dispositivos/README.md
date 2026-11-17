# Dispositivos

O projeto usa dois ESP32 simulados no [Wokwi](https://wokwi.com). Conforme o enunciado, **todos os sensores ficam em um dispositivo e o atuador fica no outro**.

| Dispositivo | Componentes | Publica | Assina |
|---|---|---|---|
| [A – Sensores](dispositivo-a-sensores/) | DHT22, potenciômetro, HC-SR04 | `smartfarm/dispA/telemetria` | — |
| [B – Atuador](dispositivo-b-atuador/) | relé (bomba) e LED | `smartfarm/dispB/status` | `smartfarm/dispB/comando` |

## Ligações

**Dispositivo A**

| Componente | Pino do componente | Pino do ESP32 |
|---|---|---|
| DHT22 | VCC / SDA / GND | 3V3 / GPIO 15 / GND |
| Potenciômetro | VCC / SIG / GND | 3V3 / GPIO 34 / GND |
| HC-SR04 | VCC / TRIG / ECHO / GND | 5V / GPIO 5 / GPIO 18 / GND |

**Dispositivo B**

| Componente | Pino do componente | Pino do ESP32 |
|---|---|---|
| Módulo relé | VCC / IN / GND | 5V / GPIO 26 / GND |
| LED (com resistor de 220 Ω) | anodo / catodo | GPIO 27 / GND |

## Adaptações

- O Wokwi não tem sensor de umidade de solo: o **potenciômetro** representa a leitura (0–100%).
- O Wokwi não tem bomba de irrigação: o **relé** representa o acionamento da bomba, e o **LED** acende junto para facilitar a visualização.
- O **HC-SR04** mede a distância da tampa até a água de um reservatório de 100 cm de altura; o nível em % é calculado a partir dessa distância.

## Como usar no Wokwi

1. Crie um projeto novo com a placa **ESP32**.
2. Substitua o conteúdo de `sketch.ino` e `diagram.json` pelos arquivos da pasta do dispositivo.
3. Crie o arquivo `libraries.txt` com o conteúdo do arquivo da pasta (o Wokwi instala as bibliotecas sozinho).
4. No topo do `sketch.ino`, troque `SEU_IP_ELASTICO` pelo IP do broker MQTT.
5. Inicie a simulação e acompanhe as mensagens no monitor serial.
