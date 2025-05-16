// Teste para Sensor INMP441 (Microfone I2S)
#include "driver/i2s.h"

// --- Configurações dos Pinos (I2S) ---
// VERIFIQUE A PINAGEM DA SUA PLACA ESP32-S3 E CONEXÕES DO MICROFONE!
#define I2S_WS_PIN  7  // Atualizado para GPIO7
#define I2S_SCK_PIN 6  // Atualizado para GPIO6
#define I2S_SD_PIN  5  // Atualizado para GPIO5

// --- Configurações do I2S ---
#define I2S_PORT_NUM I2S_NUM_0
#define I2S_SAMPLE_RATE 16000
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT
#define I2S_BUFFER_SIZE 1024     // Número de amostras no buffer
#define I2S_READ_TIMEOUT_MS 100 // Timeout para leitura do I2S (em milissegundos)

// --- Configurações da Comunicação Serial ---
#define BAUD_RATE 115200

// Função para calcular RMS (simplificada)
float calculate_rms(int32_t* samples, int count) {
    if (count == 0) return 0.0;
    double sum_sq = 0.0;
    for (int i = 0; i < count; i++) {
        // Os dados do INMP441 são 24-bit, lidos em um int32_t.
        // Para RMS, a magnitude relativa é o que importa.
        float sample_val = (float)samples[i];
        sum_sq += sample_val * sample_val;
    }
    return sqrt(sum_sq / count);
}

void setup_inmp441() {
    Serial.println("Configurando I2S para INMP441...");
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // INMP441 é mono
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
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
    Serial.println("Driver I2S e pinos configurados para INMP441.");
}

void setup() {
    Serial.begin(BAUD_RATE);
    while (!Serial) {
        delay(10);
    }
    Serial.println("Teste do Sensor INMP441 (Microfone I2S)");
    setup_inmp441();
}

void loop() {
    int32_t i2s_samples[I2S_BUFFER_SIZE];
    size_t bytes_read = 0;

    // Tenta ler dados do I2S
    esp_err_t err = i2s_read(I2S_PORT_NUM, 
                             (void*)i2s_samples, 
                             I2S_BUFFER_SIZE * sizeof(int32_t), // Tamanho total do buffer em bytes
                             &bytes_read, 
                             I2S_READ_TIMEOUT_MS);

    if (err == ESP_OK) {
        if (bytes_read > 0) {
            int samples_read = bytes_read / sizeof(int32_t);
            float rms_value = calculate_rms(i2s_samples, samples_read);
            Serial.print("Nível de Áudio (RMS): ");
            Serial.println(rms_value, 2);
        } else {
            Serial.println("Nenhum dado lido do I2S (bytes_read == 0).");
        }
    } else {
        Serial.printf("Erro na leitura do I2S: %d\n", err);
    }

    delay(500); // Lê a cada 0.5 segundos
} 