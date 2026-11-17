# Infraestrutura

## Broker MQTT (Mosquitto) numa instância EC2 Ubuntu

```bash
sudo apt update && sudo apt install -y mosquitto mosquitto-clients
sudo cp mosquitto/mosquitto.conf /etc/mosquitto/conf.d/smartfarm.conf
sudo systemctl enable --now mosquitto
sudo systemctl restart mosquitto
```

Teste, de qualquer máquina com o cliente Mosquitto:

```bash
mosquitto_sub -h <IP_DO_BROKER> -t 'smartfarm/#' -v
```

Associe um **IP elástico** à instância (EC2 → Elastic IPs → Allocate → Associate) para que o endereço não mude a cada reinício, e libere a porta 1883 no security group, conforme [docs/arquitetura.md](../docs/arquitetura.md).

## Node-RED numa instância EC2 Ubuntu

Para que o Node-RED volte a funcionar sozinho depois de um reinício da instância, instale-o como serviço:

```bash
bash <(curl -sL https://github.com/node-red/linux-installers/releases/latest/download/update-nodejs-and-nodered-deb) --confirm-install --confirm-root
sudo systemctl enable --now nodered.service
```
