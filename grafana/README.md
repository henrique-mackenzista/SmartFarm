# Grafana

Dois dashboards, descritos em detalhe em [docs/dashboards.md](../docs/dashboards.md):

| Arquivo | Dashboard |
|---|---|
| `dashboard-monitoramento.json` | Monitoramento em tempo real |
| `dashboard-historico-alertas.json` | Histórico e alertas |

Os arquivos foram exportados do Grafana Cloud no formato de dashboard v2 (`dashboard.grafana.app/v2`) e todas as consultas usam SQL sobre o bucket `smartfarm`. Eles não contêm tokens nem senhas.

> Os painéis referenciam a fonte de dados pelo identificador que ela tem no Grafana de origem. Depois de importar, se algum painel mostrar erro de fonte de dados, abra o painel em **Edit** e selecione a sua fonte InfluxDB, ou substitua no JSON o valor `dg564ekasyqrka` pelo identificador (UID) da sua fonte antes de importar.

## Como importar

1. **Connections → Data sources → Add data source → InfluxDB** e preencha:

   | Campo | Valor |
   |---|---|
   | Query language | SQL |
   | URL | URL do InfluxDB Cloud (ex.: `https://us-east-1-1.aws.cloud2.influxdata.com`) |
   | Database | `smartfarm` |
   | Token | token só de leitura do bucket `smartfarm` |

   Clique em **Save & test**.
2. **Dashboards → New → Import**: envie cada arquivo JSON e selecione a fonte de dados criada no passo anterior.
