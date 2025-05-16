
# BeeTools

Este repositório contém o código e dados para o projeto BeeTools.

## Conteúdo do Repositório

O projeto atualmente inclui os seguintes arquivos e pastas:

* `dados_sensores.csv`: Um arquivo CSV contendo dados coletados por sensores.
* `python_data_logger.py`: Um script Python, possivelmente para registrar dados dos sensores.
* `esp32_sensor_code/`: Uma pasta que provavelmente contém código para microcontroladores ESP32 relacionados a sensores.
* `test_inmp441/`: Uma pasta de teste, possivelmente para o microfone INMP441.
* `test_sht30/`: Uma pasta de teste, possivelmente para o sensor SHT30 (temperatura e umidade).


# Mapeamento de Pinos Utilizados no Projeto com ESP32-S3

Este documento descreve a pinagem utilizada para conectar os sensores SHT30 (temperatura e umidade) e INMP441 (microfone) à placa controladora Heltec WiFi LoRa 32 V3 (ESP32-S3), conforme definido no código Arduino fornecido.

## 1. Placa Controladora

* **Controlador:** ESP32-S3 (integrado na placa Heltec WiFi LoRa 32 V3)

## 2. Sensor de Temperatura e Umidade SHT30

* **Protocolo de Comunicação:** I2C
* **Pinos Utilizados no Código:**
    * `#define SDA_PIN 47`
    * `#define SCL_PIN 48`
* **Conexões Físicas:**
    * **Pino SDA do SHT30** é conectado ao **GPIO 47** da placa ESP32-S3.
    * **Pino SCL do SHT30** é conectado ao **GPIO 48** da placa ESP32-S3.
* **Alimentação (essencial para o funcionamento, não definida como GPIO no código de dados):**
    * **Pino VDD (ou VCC) do SHT30** deve ser conectado a uma saída de **3.3V** da placa ESP32-S3.
    * **Pino GND do SHT30** deve ser conectado a um pino **GND** (Terra) da placa ESP32-S3.

## 3. Sensor de Microfone INMP441

* **Protocolo de Comunicação:** I2S
* **Pinos Utilizados no Código:**
    * `#define I2S_WS_PIN  7`  (Word Select / Left-Right Clock)
    * `#define I2S_SCK_PIN 6`  (Serial Clock / Bit Clock)
    * `#define I2S_SD_PIN  5`  (Serial Data)
* **Conexões Físicas:**
    * **Pino WS (ou LRCL) do INMP441** é conectado ao **GPIO 7** da placa ESP32-S3.
    * **Pino SCK (ou BCLK) do INMP441** é conectado ao **GPIO 6** da placa ESP32-S3.
    * **Pino SD (ou DOUT) do INMP441** é conectado ao **GPIO 5** da placa ESP32-S3.
* **Alimentação (essencial para o funcionamento, não definida como GPIO no código de dados):**
    * **Pino VDD do INMP441** (e VDDIO, se presente no módulo) deve ser conectado a uma saída de **3.3V** da placa ESP32-S3.
    * **Pino GND do INMP441** deve ser conectado a um pino **GND** (Terra) da placa ESP32-S3.

## 4. Resumo da Pinagem no ESP32-S3

| GPIO no ESP32-S3 | Sensor        | Pino no Sensor | Função no Sensor     | Protocolo |
| :--------------- | :------------ | :------------- | :------------------- | :-------- |
| **GPIO 47** | SHT30         | SDA            | Dados                | I2C       |
| **GPIO 48** | SHT30         | SCL            | Clock                | I2C       |
| **GPIO 7** | INMP441       | WS / LRCL      | Word Select          | I2S       |
| **GPIO 6** | INMP441       | SCK / BCLK     | Serial Clock         | I2S       |
| **GPIO 5** | INMP441       | SD / DOUT      | Serial Data (Entrada)| I2S       |
| **3.3V** | SHT30, INMP441| VDD / VCC      | Alimentação Positiva | -         |
| **GND** | SHT30, INMP441| GND            | Terra                | -         |

**Observação:** A correta localização física destes GPIOs na placa Heltec WiFi LoRa 32 V3 deve ser verificada utilizando o diagrama de pinos específico da placa.

## 5. Diagrama Visual da Pinagem da Placa (Referência)

Para auxiliar na localização física dos pinos na placa Heltec WiFi LoRa 32 V3, consulte o diagrama de pinos da placa. Abaixo está uma referência para incluir a imagem:

![Diagrama de Pinos da Placa Heltec WiFi LoRa 32 V3](C:\Users\Paulin\Desktop\FIAP\IOT\img\pin.png)


