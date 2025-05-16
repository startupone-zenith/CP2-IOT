// Teste para Sensor SHT30
#include <Wire.h>
#include "Adafruit_SHT31.h"

// --- Configurações dos Pinos (I2C) ---
// VERIFIQUE A PINAGEM DA SUA PLACA ESP32-S3!
//pin 8 e 9 comunicacao i2c
#define SDA_PIN 47  // Atualizado para GPIO47
#define SCL_PIN 48  // Atualizado para GPIO48

// --- Configurações da Comunicação Serial ---
#define BAUD_RATE 115200

// Objeto do Sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
    Serial.begin(BAUD_RATE);
    while (!Serial) {
        delay(10); // Espera a serial estar pronta
    }
    Serial.println("Teste do Sensor SHT30");

    // Inicializa I2C com pinos especificados
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!sht31.begin(0x44)) { // Endereço I2C padrão 0x44 (ou 0x45)
        Serial.println("Erro ao encontrar o sensor SHT30. Verifique a conexão, endereço e pinos.");
        while (1) delay(1);
    }
    Serial.println("Sensor SHT30 inicializado com sucesso!");
}

void loop() {
    float temp = sht31.readTemperature();
    float hum = sht31.readHumidity();

    if (isnan(temp)) {  // isnan() verifica se o valor é "Not a Number"
        Serial.println("Falha ao ler temperatura do SHT30");
    } else {
        Serial.print("Temperatura: ");
        Serial.print(temp, 2); // Imprime com 2 casas decimais
        Serial.println(" °C");
    }

    if (isnan(hum)) {
        Serial.println("Falha ao ler umidade do SHT30");
    } else {
        Serial.print("Umidade: ");
        Serial.print(hum, 2); // Imprime com 2 casas decimais
        Serial.println(" %");
    }

    Serial.println("--------------------");
    delay(2000); // Espera 2 segundos entre as leituras
} 