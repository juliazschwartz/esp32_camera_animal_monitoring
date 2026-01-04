#include <WiFi.h>
#include <WebSocketsClient.h>
#include "esp_camera.h"

// const char* ssid = "los_pollos";
// const char* password = "BlueMagic99%";

const char* ssid = "SCHWARTZ";
const char* password = "ararajuba";

WebSocketsClient webSocket;

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
// ===== Inicializa câmera =====
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size   = FRAMESIZE_QVGA;   // 320x240
  config.jpeg_quality = 12;               // 0–63 (menor = melhor)
  config.fb_count     = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erro ao iniciar câmera");
    return false;
  }

  return true;
}

// ===== Envia frame =====
void sendFrame() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Falha ao capturar imagem");
    return;
  }

  // envia como binário
  webSocket.sendBIN(fb->buf, fb->len);

  Serial.printf("Imagem enviada (%d bytes)\n", fb->len);
  esp_camera_fb_return(fb);
}

// ... (seus defines e initCamera permanecem iguais)

bool isConnected = false;

void setup() {
  Serial.begin(115200);
  delay(2000); // Aumente o delay inicial para dar tempo do Serial Monitor abrir
  Serial.println("\n=========== TESTE DE BOOT ===========");

  if (!initCamera()) {
    Serial.println("Travei na Camera!");
    return;
  }
  Serial.println("Camera OK!");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK!");

  // Tente primeiro SEM SSL (porta 9080) se o SSL continuar travando o log
  webSocket.beginSSL("mouse-monitoring.rennanfelipe.net", 9443, "/ws/");
  
  webSocket.onEvent([](WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
      Serial.println("Conectado ao Nginx!");
      isConnected = true;
    }
    if (type == WStype_DISCONNECTED) {
      Serial.println("Desconectado!");
      isConnected = false;
    }
  });
}

void loop() {
  webSocket.loop();

  // Envio contínuo independente de receber texto
  if (isConnected) {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 200) { // Envia a cada 200ms
      sendFrame();
      lastMillis = millis();
    }
  }
}
