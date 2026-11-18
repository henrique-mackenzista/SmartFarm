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

- **Só chats autorizados recebem resposta.** Qualquer pessoa pode encontrar um bot no Telegram; por isso o Node-RED compara o ID do chat com a lista em `TELEGRAM_CHAT_IDS_AUTORIZADOS` (veja [`.env.example`](../.env.example)) e ignora os demais.
- **Leitura antiga é sinalizada.** Se o Dispositivo A estiver há mais de 60 s sem publicar, a resposta avisa que os valores podem estar desatualizados, em vez de mostrá-los como atuais.
- O token do bot e os IDs de chat não são publicados neste repositório.

## Como funciona no Node-RED

O bot usa a paleta **`node-red-contrib-telegrambot`**, em modo *polling*: o Node-RED consulta o Telegram periodicamente em busca de mensagens novas, então não é preciso HTTPS nem porta de entrada aberta na instância EC2.

```
telegram receiver → function "Responder comando" → telegram sender
```

As regras de negócio já mantêm no contexto do fluxo os valores mais recentes, e o nó `function` só lê esses valores e monta a resposta:

| Variável de fluxo | Conteúdo | Quem atualiza |
|---|---|---|
| `telemetria` | `{umidade_solo, temperatura, umidade_ar, nivel, ts}` | chegada de `smartfarm/dispA/telemetria` |
| `bomba` | `{estado, bloqueio}` — `bloqueio` = `"chuva"`, `"reservatorio"` ou `null` | chegada de `smartfarm/dispB/status` e regras R2/R3 |
| `clima` | `{prob_chuva, temperatura}` | consulta ao OpenWeather |
| `economia7d` | `{usado_l, rotina_l, adiadas}` | consulta ao InfluxDB a cada hora |

Cada comando recebido também é gravado no InfluxDB, na measurement `comandos` (veja [banco-de-dados.md](banco-de-dados.md)).

### Código do nó `function` "Responder comando"

```javascript
// Entrada: mensagem do nó "telegram receiver"
const chatId = msg.payload.chatId;
const autorizados = (env.get("TELEGRAM_CHAT_IDS_AUTORIZADOS") || "")
    .split(",").map(s => s.trim()).filter(Boolean);
if (!autorizados.includes(String(chatId))) return null;   // ignora chats desconhecidos

const comando = String(msg.payload.content || "").trim().split(/[\s@]/)[0].toLowerCase();

const t = flow.get("telemetria") || {};
const b = flow.get("bomba") || {};
const c = flow.get("clima") || {};
const e = flow.get("economia7d") || {};

const fmt = (v, casas = 0) => (v === undefined || v === null) ? "–" : Number(v).toFixed(casas).replace(".", ",");
const idadeS = t.ts ? Math.round((Date.now() - t.ts) / 1000) : null;
const aviso = (idadeS === null || idadeS > 60)
    ? "\n⚠️ Sem leitura recente do Dispositivo A: os valores podem estar desatualizados." : "";

function faixaUmidade(u) {
    if (u === undefined) return "";
    return u < 30 ? "seco" : (u <= 60 ? "ideal" : "acima do ideal");
}
function textoBomba() {
    if (b.bloqueio === "chuva") return "bloqueada (chuva prevista)";
    if (b.bloqueio === "reservatorio") return "bloqueada (reservatório crítico)";
    return b.estado === 1 ? "ligada" : "desligada";
}
const autonomia = (t.nivel !== undefined && e.usado_l)
    ? fmt((t.nivel / 100 * 200) / (e.usado_l / 7), 1) + " dias" : "–";   // reservatório de 200 L

const respostas = {
    "/status":
        `🌱 Umidade do solo: ${fmt(t.umidade_solo)}% (${faixaUmidade(t.umidade_solo)})\n` +
        `💧 Reservatório: ${fmt(t.nivel)}% · autonomia ≈ ${autonomia}\n` +
        `🌧️ Chuva prevista (3 h): ${fmt(c.prob_chuva)}%\n` +
        `⚙️ Bomba: ${textoBomba()}\n` +
        `Última leitura: há ${idadeS === null ? "–" : idadeS} s`,
    "/umidade": `🌱 Umidade do solo: ${fmt(t.umidade_solo)}% (${faixaUmidade(t.umidade_solo)}). Faixa ideal: 30% a 60%.`,
    "/nivel": `💧 Reservatório: ${fmt(t.nivel)}% · autonomia ≈ ${autonomia}` +
              (t.nivel < 20 ? "\n🔴 Nível crítico: bomba bloqueada." : ""),
    "/chuva": `🌧️ Chuva prevista nas próximas 3 h: ${fmt(c.prob_chuva)}%. ` +
              (c.prob_chuva >= 60 ? "Irrigação adiada." : "Irrigação liberada."),
    "/bomba": `⚙️ Bomba: ${textoBomba()}.`,
    "/economia": `📊 Últimos 7 dias: ${fmt(e.usado_l)} L usados × ${fmt(e.rotina_l)} L de uma rotina fixa. ` +
                 `Economia de ${e.rotina_l ? fmt((1 - e.usado_l / e.rotina_l) * 100) : "–"}%. ` +
                 `${fmt(e.adiadas)} irrigações adiadas por chuva.`
};
const ajuda = "Comandos disponíveis:\n/status · /umidade · /nivel · /chuva · /bomba · /economia";

let texto = respostas[comando];
if (!texto) texto = (comando === "/start" || comando === "/ajuda") ? ajuda : "Comando não reconhecido.\n" + ajuda;
if (comando !== "/start" && comando !== "/ajuda" && respostas[comando]) texto += aviso;

const teclado = {
    keyboard: [["/status"], ["/umidade", "/nivel"], ["/chuva", "/bomba"], ["/economia", "/ajuda"]],
    resize_keyboard: true
};

msg.payload = {
    chatId: chatId,
    type: "message",
    content: texto,
    options: { reply_markup: JSON.stringify(teclado) }
};
return msg;
```

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
3. No Node-RED, instale a paleta `node-red-contrib-telegrambot`, crie a configuração do bot com o token e ative o modo *polling*.
4. Defina a variável de ambiente `TELEGRAM_CHAT_IDS_AUTORIZADOS` com os IDs de chat do grupo, separados por vírgula, e reinicie o Node-RED.
