// Inclusão de Bibliotecas
#include <Wire.h>
#include "Adafruit_SHT31.h" // Para o sensor SHT30/SHT31
#include "driver/i2s.h"     // Para o microfone INMP441

// --- Configurações dos Pinos ---
// Sensor SHT30 (I2C) - VERIFIQUE A PINAGEM DA SUA PLACA!
#define SDA_PIN 47  // Atualizado
#define SCL_PIN 48  // Atualizado

// Sensor INMP441 (I2S) - VERIFIQUE A PINAGEM E CONEXÕES!
#define I2S_WS_PIN  7 // Atualizado
#define I2S_SCK_PIN 6 // Atualizado
#define I2S_SD_PIN  5 // Atualizado

// --- Configurações do I2S (INMP441) ---
#define I2S_PORT_NUM I2S_NUM_0 // Porta I2S a ser usada (0 ou 1)
#define I2S_SAMPLE_RATE 16000 // Taxa de amostragem (Hz)
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT // INMP441 envia 24 bits, mas lemos como 32
#define I2S_BUFFER_SIZE 1024 // Tamanho do buffer de amostras I2S
#define I2S_READ_TIMEOUT_MS 100 // Timeout para leitura do I2S

// --- Configurações da Comunicação Serial ---
#define BAUD_RATE 115200

// --- Intervalo de Leitura ---
#define READ_INTERVAL_MS 1000 // Enviar dados a cada 1 segundo

// Objetos dos Sensores
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Variáveis Globais
unsigned long previousMillis = 0;

void setup_sht30() {
    Wire.begin(SDA_PIN, SCL_PIN); // Inicializa I2C com pinos especificados
    if (!sht31.begin(0x44)) { // Endereço I2C padrão do SHT30 é 0x44 (ou 0x45)
        Serial.println("Erro ao encontrar o sensor SHT30. Verifique a conexão e o endereço.");
        while (1) delay(1);
    }
    Serial.println("Sensor SHT30 inicializado.");
}

void setup_inmp441() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Mestre, apenas recepção (RX)
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // Lemos como 32 bits
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // INMP441 é mono, geralmente mapeado para o canal esquerdo
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Nível de interrupção
        .dma_buf_count = 8,                       // Número de buffers DMA
        .dma_buf_len = 64,                        // Tamanho de cada buffer DMA
        .use_apll = false,                        // Usar APLL para clock mais preciso (pode ser true)
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE, // Não usado para RX
        .data_in_num = I2S_SD_PIN
    };

    esp_err_t err = i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Erro ao instalar driver I2S: %d\n", err);
        while (1) delay(1);
    }

    err = i2s_set_pin(I2S_PORT_NUM, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Erro ao configurar pinos I2S: %d\n", err);
        while (1) delay(1);
    }
    Serial.println("Sensor INMP441 (I2S) inicializado.");
}

void setup() {
    Serial.begin(BAUD_RATE);
    while (!Serial) {
        delay(10); // Espera a serial estar pronta (importante para algumas placas ESP32-S3)
    }
    Serial.println("Sistema de Coleta de Dados - ESP32-S3");

    setup_sht30();
    setup_inmp441();
}

float read_temperature() {
    float t = sht31.readTemperature();
    if (isnan(t)) {
        Serial.println("Falha ao ler temperatura do SHT30");
        return -999.0; // Valor de erro
    }
    return t;
}

float read_humidity() {
    float h = sht31.readHumidity();
    if (isnan(h)) {
        Serial.println("Falha ao ler umidade do SHT30");
        return -999.0; // Valor de erro
    }
    return h;
}

// Calcula o valor RMS de um buffer de amostras de áudio
float calculate_rms(int32_t* samples, int count) {
    if (count == 0) return 0.0;
    double sum_sq = 0.0;
    for (int i = 0; i < count; i++) {
        // O INMP441 fornece dados de 24 bits, mas são lidos em um int32_t.
        // Os dados estão nos bits mais significativos. Vamos usar o valor como está.
        // Para uma normalização mais precisa, seria necessário conhecer a escala exata.
        // Esta é uma simplificação. O valor é tipicamente left-justified.
        float sample_val = (float)samples[i]; 
        sum_sq += sample_val * sample_val;
    }
    return sqrt(sum_sq / count);
}


float read_audio_level() {
    int32_t i2s_samples[I2S_BUFFER_SIZE];
    size_t bytes_read = 0;

    // Limpa o buffer de DMA antigo, se houver.
    // i2s_zero_dma_buffer(I2S_PORT_NUM); // Comente se causar problemas ou se não for necessário.

    esp_err_t err = i2s_read(I2S_PORT_NUM, (void*)i2s_samples, 
                             I2S_BUFFER_SIZE * sizeof(int32_t), 
                             &bytes_read, 
                             I2S_READ_TIMEOUT_MS);

    if (err != ESP_OK) {
        Serial.printf("Erro na leitura do I2S: %d\n", err);
        return 0.0;
    }

    if (bytes_read == 0) {
        Serial.println("Nenhum dado lido do I2S.");
        return 0.0;
    }
    
    int samples_read = bytes_read / sizeof(int32_t);
    
    // Para o INMP441, os dados são 24-bit left-justified em um buffer de 32-bit.
    // Precisamos deslocar para a direita para obter o valor correto se quisermos o valor exato,
    // mas para RMS, a magnitude relativa é mais importante.
    // A leitura direta do int32_t (que contém os 24 bits MSB) pode ser usada,
    // mas tenha em mente que o valor absoluto pode ser grande.
    // Para um RMS mais "puro", você pode precisar escalar ou processar os bits.
    // Exemplo simples de ajuste (opcional, dependendo da interpretação desejada):
    // for (int i = 0; i < samples_read; i++) {
    //    i2s_samples[i] = i2s_samples[i] >> 8; // Desloca para alinhar se necessário
    // }


    return calculate_rms(i2s_samples, samples_read);
}


void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= READ_INTERVAL_MS) {
        previousMillis = currentMillis;

        float temperature = read_temperature();
        float humidity = read_humidity();
        float audio_level = read_audio_level();

        // Formato CSV: timestamp,temperatura,umidade,nivel_audio
        String data_csv = String(currentMillis) + "," +
                          String(temperature, 2) + "," + // 2 casas decimais para temperatura
                          String(humidity, 2) + "," +    // 2 casas decimais para umidade
                          String(audio_level, 2);        // 2 casas decimais para nível de áudio

        Serial.println(data_csv);
    }
} 