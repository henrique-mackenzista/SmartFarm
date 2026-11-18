# Bot do Telegram

Além de enviar alertas, o bot do SmartFarm responde a comandos: o usuário consulta a situação do campo pelo celular, sem precisar abrir o Grafana. O menu aparece como botões no teclado do Telegram, então não é preciso digitar.

## Comandos

| Comando | Resposta |
|---|---|
| `/status` | resumo: umidade do solo, reservatório, chuva prevista e estado da bomba |
| `/umidade` | umidade do solo atual e a faixa (seco, ideal ou acima do ideal) |
| `/nivel` | nível do reservatório e autonomia estimada |
| `/chuva` | probabilidade de chuva nas próximas 3 h e se a irrigação está adiada |
| `/bomba` | estado da bomba: ligada, desligada ou bloqueada (e o motivo) |
| `/economia` | água usada nos últimos 7 dias comparada à rotina fixa (meta 6.4) |
| `/ajuda` ou `/start` | lista de comandos e o menu de botões |

Exemplo:

```
Usuário: /status
Bot:     🌱 Umidade do solo: 43% (ideal)
         💧 Reservatório: 52% · autonomia ≈ 5,2 dias
         🌧️ Chuva prevista (3 h): 10%
         ⚙️ Bomba: desligada
         Última leitura: há 4 s
```

Os comandos são só de consulta: o bot não liga nem desliga a bomba. O acionamento continua exclusivamente com as [regras de negócio](regras-de-negocio.md).

## Segurança

- **Só chats autorizados recebem resposta.** Qualquer pessoa pode encontrar um bot no Telegram; por isso o Node-RED compara o ID do chat com a lista em `TELEGRAM_CHAT_IDS_AUTORIZADOS` e ignora os demais.
- **Leitura antiga é sinalizada.** Se o Dispositivo A estiver há mais de 60 s sem publicar, a resposta avisa que os valores podem estar desatualizados.
- **Nada sensível é publicado.** O token do bot e os IDs de chat ficam só no `settings.js` do Node-RED ([como configurar](../node-red/README.md)). No banco de dados, o ID do chat é gravado anonimizado.

## Como funciona no Node-RED

A aba **3. Telegram** do [`flows.json`](../node-red/flows.json) usa só o nó `http request`, sem paletas extras:

```
inject (a cada 3 s) → function "Busca mensagens" → http request (getUpdates)
                    → function "Responde comandos" → http request (sendMessage) → registro no InfluxDB
```

- A cada 3 s o Node-RED pergunta ao Telegram se há mensagens novas (método `getUpdates`). Como é o Node-RED que busca, não é preciso HTTPS nem porta aberta no computador.
- O nó "Responde comandos" lê os valores mais recentes guardados pelas regras de negócio (contexto global `telemetria`, `bomba`, `clima`, `irrigacoes` e `adiamentos`), monta o texto e o envia com o teclado de botões pelo método `sendMessage`.
- Os alertas das regras usam o mesmo `sendMessage`, enviados ao chat definido em `TELEGRAM_CHAT_ID`.
- Cada comando respondido é gravado no InfluxDB, na measurement `comandos` ([banco-de-dados.md](banco-de-dados.md)).

## Configuração do bot

1. No Telegram, converse com o **@BotFather**, crie o bot com `/newbot` e guarde o token.
2. Ainda no @BotFather, use `/setcommands` e cole a lista abaixo para os comandos aparecerem no botão **Menu** do app:
   ```
   status - Resumo do campo
   umidade - Umidade do solo
   nivel - Nível do reservatório
   chuva - Previsão de chuva
   bomba - Estado da bomba
   economia - Economia de água na semana
   ajuda - Lista de comandos
   ```
3. Envie uma mensagem ao bot pelo celular de cada integrante e descubra os IDs de chat (instruções em [node-red/README.md](../node-red/README.md)).
4. Defina `TELEGRAM_BOT_TOKEN`, `TELEGRAM_CHAT_ID` e `TELEGRAM_CHAT_IDS_AUTORIZADOS` no `settings.js` e reinicie o Node-RED.
