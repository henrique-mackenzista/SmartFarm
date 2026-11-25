# Node-RED

O arquivo [`flows.json`](flows.json) contém todo o fluxo do SmartFarm, dividido em quatro abas:

| Aba | O que faz |
|---|---|
| **1. Dispositivos e regras** | recebe a telemetria do Dispositivo A e o status do Dispositivo B, aplica as [regras R1 a R7](../docs/regras-de-negocio.md) e publica os comandos da bomba |
| **2. OpenWeather** | a cada 5 min consulta o clima atual (temperatura) e a previsão de chuva das próximas 3 h |
| **3. Telegram** | envia os alertas e responde aos comandos do [bot de consulta](../docs/bot-telegram.md) |
| **4. InfluxDB** | grava no banco tudo o que as outras abas geram ([measurements](../docs/banco-de-dados.md)) |

O fluxo usa só nós que já vêm com o Node-RED, mais a paleta `node-red-contrib-influxdb`. OpenWeather e Telegram são acessados pelo nó `http request`.

## 1. Instalar o Node-RED no computador

### Opção A: Docker (usada pela equipe)

Com o [Docker Desktop](https://www.docker.com/products/docker-desktop/) instalado, rode no PowerShell (troque os valores pelas suas chaves):

```powershell
docker run -d --name mynodered --restart unless-stopped -p 1880:1880 `
  -v C:\smartfarm\node-red:/data `
  -e TZ=America/Sao_Paulo `
  -e OPENWEATHER_API_KEY=sua_chave `
  -e "OPENWEATHER_CIDADE=Sao Paulo,BR" `
  -e TELEGRAM_BOT_TOKEN=token_do_bot `
  -e TELEGRAM_CHAT_ID=id_do_chat `
  -e TELEGRAM_CHAT_IDS_AUTORIZADOS=id_1,id_2 `
  -e 'SMARTFARM_PARAMETROS={"semTelemetriaS":180}' `
  nodered/node-red
```

O fluxo, as credenciais e a memória ficam na pasta `C:\smartfarm\node-red`, então o container pode ser removido e recriado sem perder nada. Para mudar uma variável: `docker stop mynodered`, `docker rm mynodered` e o mesmo `docker run` com o valor novo. A linha `SMARTFARM_PARAMETROS` é opcional (veja "Testar com tempos menores").

Dentro do container, `localhost` é o próprio container. Por isso o banco usa o InfluxDB Cloud, e não um InfluxDB instalado no computador.

Com Docker, as chaves já vão pelas variáveis `-e`: no passo 3, faça só a parte (b), editando o `settings.js` dentro de `C:\smartfarm\node-red`.

### Opção B: npm

Requer o [Node.js](https://nodejs.org) (versão LTS).

```bash
npm install -g --unsafe-perm node-red
node-red
```

O editor abre em http://localhost:1880. A pasta de configuração do Node-RED fica em `~/.node-red` (no Windows, `C:\Users\<usuario>\.node-red`).

## 2. Instalar a paleta do InfluxDB

Menu (☰) → **Manage palette** → **Install** → procure `node-red-contrib-influxdb` → **Install**.

## 3. Configurar as chaves e a memória

Abra o arquivo `settings.js` da pasta `.node-red` e faça duas alterações:

**a) No início do arquivo**, antes de `module.exports`, defina as variáveis com os valores reais. Elas ficam só no seu computador e não são exportadas junto com o fluxo:

```javascript
process.env.OPENWEATHER_API_KEY = "sua_chave_do_openweather";
process.env.OPENWEATHER_CIDADE = "Sao Paulo,BR";
process.env.TELEGRAM_BOT_TOKEN = "token_do_bot";
process.env.TELEGRAM_CHAT_ID = "id_do_chat_que_recebe_os_alertas";
process.env.TELEGRAM_CHAT_IDS_AUTORIZADOS = "id_1,id_2,id_3";   // quem pode consultar o bot
```

**b) Dentro de `module.exports`**, procure `contextStorage` e deixe assim, para que o histórico de irrigações e o estado da bomba não se percam quando o Node-RED reiniciar:

```javascript
contextStorage: {
    default: { module: "localfilesystem" }
},
```

Salve e reinicie o Node-RED (Ctrl+C no terminal e `node-red` de novo).

> Para descobrir o ID do seu chat no Telegram, envie qualquer mensagem ao bot e abra `https://api.telegram.org/bot<TOKEN>/getUpdates` no navegador: o número aparece em `"chat":{"id": ...}`. Faça isso **antes** de importar o fluxo, porque depois o próprio Node-RED passa a consumir essas mensagens.

## 4. Importar o fluxo

1. Menu (☰) → **Import** → **select a file to import** → escolha `flows.json` → **Import**.
2. Dê dois cliques em qualquer nó do InfluxDB (aba 4) e, no lápis ao lado de **Server**, informe:
   - **Version:** 2.0;
   - **URL** do InfluxDB Cloud (ex.: `https://us-east-1-1.aws.cloud2.influxdata.com`);
   - **Token** com permissão de leitura e escrita no bucket.

   No próprio nó `InfluxDB`, troque `SUA_ORGANIZACAO` na **Organization** pelo Organization ID (aparece na URL do console do InfluxDB Cloud, depois de `/orgs/`), o **Bucket** `smartfarm` e a **Time Precision** em milissegundos.
3. Clique em **Deploy**.

O broker já vem configurado com `broker.hivemq.com`, porta 1883, e o client ID `mack10308763-nodered`.

## 5. Conferir se está funcionando

| Onde olhar | O que deve aparecer |
|---|---|
| Aba 1, nó "Recebe telemetria" | o status abaixo do nó mostra a umidade e o nível mais recentes |
| Aba 1, nó "Regras R1 a R5" | o estado da bomba ou o motivo do bloqueio |
| Aba 2, nós "Guarda temperatura" e "Probabilidade de chuva" | a temperatura e a chance de chuva da última consulta |
| Painel de debug (🐞, à direita) | cada comando publicado para o Dispositivo B |
| Telegram | resposta ao enviar `/status` para o bot |
| InfluxDB Cloud, Data Explorer | `SELECT * FROM "telemetria" WHERE time >= now() - interval '1 hour'` retorna uma linha a cada ~5 s |

## Testar com tempos menores

Para testar as regras sem esperar 15 minutos, defina no `settings.js` a variável `SMARTFARM_PARAMETROS`, em JSON, com os valores que quer trocar. Exemplo para testar o tempo máximo (R4) em 1 minuto:

```javascript
process.env.SMARTFARM_PARAMETROS = '{"tempoMaxIrrigacaoMin": 1, "intervaloAlertaMin": 0}';
```

Os nomes dos parâmetros estão no nó "Parâmetros e estado inicial" (aba 1). Remova a variável depois dos testes.

## Exportar para o repositório

Ao atualizar este arquivo, use Menu → **Export** → **All flows** → **Download**. Antes de publicar, confira que nenhuma chave, token ou ID de chat foi digitado direto em algum nó: eles devem ficar só no `settings.js`. O token do InfluxDB fica nas credenciais do Node-RED e não é exportado.
