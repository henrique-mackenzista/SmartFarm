# Grafana

Dois dashboards, descritos em detalhe em [docs/dashboards.md](../docs/dashboards.md):

| Arquivo | Dashboard |
|---|---|
| `dashboard-monitoramento.json` | Monitoramento em tempo real |
| `dashboard-historico-alertas.json` | Histórico e alertas |

> Os arquivos JSON serão adicionados aqui quando os dashboards estiverem montados com os dados reais.

## Como importar

1. **Connections → Data sources → Add data source → InfluxDB**: informe a URL, a organização, o bucket `smartfarm` e o token.
2. **Dashboards → New → Import**: envie cada arquivo JSON e selecione a fonte de dados criada no passo anterior.
