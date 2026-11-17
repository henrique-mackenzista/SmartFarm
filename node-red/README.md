# Node-RED

O fluxo do Node-RED implementa as [regras de negócio](../docs/regras-de-negocio.md), integra as APIs (OpenWeather e Telegram) e grava os dados no InfluxDB.

> O arquivo `flows.json` será adicionado aqui quando o fluxo estiver concluído.

## Paletas necessárias

Instale pelo menu **Manage palette** do Node-RED:

| Paleta | Uso |
|---|---|
| `node-red-contrib-influxdb` | nó `influxdb out`, que grava no InfluxDB |

Os nós `mqtt in`, `mqtt out`, `http request`, `inject`, `change`, `join`, `function` e `debug` já vêm com o Node-RED.

## Como importar

1. Menu → **Import** → selecione `flows.json`.
2. Abra o nó de configuração do broker MQTT e informe o IP do broker (porta 1883, client ID `smartfarm-nodered`).
3. Abra o nó de configuração do InfluxDB e informe a URL, a organização, o bucket `smartfarm` e o token.
4. Nos nós `http request`, troque os marcadores pelos valores reais:
   - `OPENWEATHER_API_KEY` na URL do OpenWeather;
   - `TELEGRAM_BOT_TOKEN` na URL do Telegram e `TELEGRAM_CHAT_ID` no corpo da mensagem.
5. Clique em **Deploy**.

## Exportação sem dados sensíveis

Ao exportar o fluxo para este repositório, use **Export → All flows** e confira antes de publicar que chaves, tokens e IDs de chat foram substituídos pelos marcadores acima. O Node-RED não exporta as credenciais guardadas nos nós de configuração, mas valores digitados direto em URLs e em nós `function` são exportados.
