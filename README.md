# SmartFarm

Irrigação automatizada de precisão com IoT para o uso eficiente da água, alinhada à **ODS 6, meta 6.4 da Agenda 2030 da ONU**.

Projeto da disciplina **Objetos Inteligentes Conectados** — Faculdade de Computação e Informática, Universidade Presbiteriana Mackenzie.
Professor: Wallace Rodrigues de Santana.

**Equipe:** Henrique Storai · Gabriel Athia · Pedro Kihara

> 🎥 **Vídeo de demonstração:** _link do YouTube será adicionado aqui_
> 🔌 **Simulação no Wokwi:** [Dispositivo A (sensores)](#) · [Dispositivo B (atuador)](#) — _links serão adicionados aqui_

---

## O problema

A agricultura responde por 72% das retiradas de água doce no mundo, principalmente para irrigação (FAO, 2021). No Brasil, a irrigação foi o principal uso da água em 2022, com 50,5% de todo o volume retirado (ANA, 2024). Boa parte dessa água é aplicada por rotinas de horário fixo, que irrigam mesmo quando o solo ainda está úmido ou quando vai chover.

## A proposta

O SmartFarm decide **quando irrigar com base na condição real do solo**:

- liga a bomba só quando a umidade do solo cai abaixo de 30% e desliga ao chegar a 60%;
- adia a irrigação quando a previsão do tempo indica chuva;
- protege o reservatório, bloqueando a bomba quando o nível fica crítico;
- registra o tempo de cada acionamento para estimar o volume de água usado e compará-lo com uma rotina fixa — o indicador de eficiência da meta 6.4;
- envia alertas pelo Telegram e mostra tudo em dashboards no Grafana;
- responde a consultas por um [bot no Telegram](docs/bot-telegram.md), com menu de botões: status geral, umidade, nível do reservatório, chuva prevista, estado da bomba e economia de água.

## Arquitetura

```mermaid
flowchart LR
    A["Dispositivo A<br/>ESP32 + DHT22,<br/>potenciômetro, HC-SR04"] -- "smartfarm/dispA/telemetria" --> B(("Broker MQTT<br/>Mosquitto"))
    B -- "smartfarm/dispA/telemetria<br/>smartfarm/dispB/status" --> N["Node-RED<br/>regras de negócio"]
    N -- "smartfarm/dispB/comando" --> B
    B -- "smartfarm/dispB/comando" --> D["Dispositivo B<br/>ESP32 + relé (bomba)"]
    D -- "smartfarm/dispB/status" --> B
    N <-- "HTTPS" --> W["OpenWeather<br/>clima e previsão"]
    N <-- "HTTPS" --> T["Telegram Bot API<br/>alertas e menu de consulta"]
    N -- "HTTP + token" --> I[("InfluxDB")]
    I --> G["Grafana<br/>dashboards"]
```

| Bloco | Tecnologia | Onde roda |
|---|---|---|
| Dispositivos | 2 × ESP32 simulados no Wokwi | Wokwi (rede Wokwi-GUEST) |
| Broker MQTT | Eclipse Mosquitto, porta 1883 | Instância EC2 (AWS Academy) com IP elástico |
| Regras de negócio | Node-RED, porta 1880 | Outra instância EC2 |
| Banco de dados | InfluxDB (séries temporais) | Máquina separada (ver [docs/arquitetura.md](docs/arquitetura.md)) |
| Dashboards | Grafana | Máquina separada (ver [docs/arquitetura.md](docs/arquitetura.md)) |
| APIs externas | OpenWeather, Telegram Bot API | Serviços de terceiros |

Broker, Node-RED, banco de dados e Grafana ficam em máquinas virtuais distintas, conforme exigido pelo enunciado do projeto.

## Estrutura do repositório

```
smartfarm/
├── README.md                      ← você está aqui
├── docs/
│   ├── arquitetura.md             máquinas, portas, FQDN/IP e regras de firewall
│   ├── topicos-mqtt.md            tópicos e formato das mensagens JSON
│   ├── regras-de-negocio.md       regras R1 a R7 e como validá-las
│   ├── banco-de-dados.md          o que é gravado no InfluxDB
│   ├── dashboards.md              dashboards e painéis do Grafana
│   ├── bot-telegram.md            comandos do bot, segurança e código do Node-RED
│   └── img/                       diagramas e capturas de tela
├── dispositivos/
│   ├── README.md                  ligações, adaptações e como usar no Wokwi
│   ├── dispositivo-a-sensores/    sketch.ino, diagram.json e libraries.txt (Wokwi)
│   └── dispositivo-b-atuador/     sketch.ino, diagram.json e libraries.txt (Wokwi)
├── node-red/
│   ├── flows.json                 fluxo exportado do Node-RED, sem credenciais (a adicionar)
│   └── README.md                  paletas necessárias e como importar
├── grafana/
│   ├── dashboard-monitoramento.json        (a adicionar)
│   ├── dashboard-historico-alertas.json    (a adicionar)
│   └── README.md                  como importar e configurar a fonte de dados
├── infraestrutura/
│   ├── README.md                  instalação do broker e do Node-RED nas EC2
│   └── mosquitto/mosquitto.conf   configuração do broker
├── .env.example                   variáveis secretas necessárias (sem valores reais)
├── .gitignore
└── LICENSE
```

Os arquivos marcados como "a adicionar" entram no repositório à medida que o protótipo for concluído.

## Como reproduzir o projeto

1. **Broker MQTT** — numa instância EC2 (Ubuntu), instale o Mosquitto seguindo [`infraestrutura/README.md`](infraestrutura/README.md) e associe um IP elástico. Libere a porta 1883 no security group (detalhes em [docs/arquitetura.md](docs/arquitetura.md)).
2. **Dispositivos** — crie dois projetos ESP32 no Wokwi e copie para cada um os arquivos `sketch.ino`, `diagram.json` e `libraries.txt` da pasta correspondente em [`dispositivos/`](dispositivos/). No topo de cada `sketch.ino`, troque `SEU_IP_ELASTICO` pelo IP elástico do broker. As ligações e adaptações estão em [`dispositivos/README.md`](dispositivos/README.md).
3. **Banco de dados** — crie um bucket `smartfarm` no InfluxDB e um token com permissão de escrita e leitura.
4. **Node-RED** — em outra instância EC2, instale o Node-RED e as paletas listadas em [`node-red/README.md`](node-red/README.md), importe `node-red/flows.json` e preencha as credenciais a partir de [`.env.example`](.env.example).
5. **Grafana** — adicione o InfluxDB como fonte de dados e importe os dois arquivos JSON de [`grafana/`](grafana/).
6. **Bot do Telegram** — crie o bot no @BotFather e configure-o no Node-RED conforme [docs/bot-telegram.md](docs/bot-telegram.md).
7. **Teste** — inicie as duas simulações no Wokwi e gire o potenciômetro abaixo de 30%: o Node-RED deve publicar o comando de ligar, o relé do Dispositivo B deve acionar e o acionamento deve aparecer no dashboard. Os demais testes estão em [docs/regras-de-negocio.md](docs/regras-de-negocio.md).

## Dados sensíveis

Chaves de API, tokens, IDs de chat e números de telefone **não** são publicados neste repositório. O fluxo do Node-RED é exportado sem credenciais, e os valores necessários estão listados, sem conteúdo real, em [`.env.example`](.env.example).

## Referências

- AGÊNCIA NACIONAL DE ÁGUAS E SANEAMENTO BÁSICO (ANA). *Relatório Conjuntura dos Recursos Hídricos no Brasil atualiza informações sobre águas do País*. Brasília, DF: ANA, 2 fev. 2024.
- FOOD AND AGRICULTURE ORGANIZATION OF THE UNITED NATIONS (FAO). *The State of the World's Land and Water Resources for Food and Agriculture 2021: facts and figures*. Roma: FAO, 2021.
- ORGANIZAÇÃO DAS NAÇÕES UNIDAS (ONU). *Agenda 2030 para o Desenvolvimento Sustentável*. Nações Unidas Brasil, [s.d.].

## Licença

Distribuído sob a licença MIT. Veja [LICENSE](LICENSE).
