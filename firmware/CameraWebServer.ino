#include <WebSocketsClient.h>
#include "esp_camera.h"
#include <WiFi.h>

// Pinos da Câmera (Wrover) permanecem os mesmos...
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

QueueHandle_t frameQueue;
WebSocketsClient webSocket;

void init_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Ajuste equilibrado: QVGA com qualidade moderada
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 15; // 10 a 20 é o ideal para WSS
  config.fb_count = 1;      // Reduzir para 1 economiza MUITA RAM para o SSL

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    while (true) delay(1000);
  }
}

void init_websocket() {
  webSocket.setReconnectInterval(5000);
  // Otimização para pacotes grandes
  webSocket.setExtraHeaders("Max-Payload-Size: 16384"); 
  webSocket.beginSSL("mouse-monitoring.rennanfelipe.net", 9443, "/ws/");
  
  webSocket.onEvent([](WStype_t type, uint8_t * payload, size_t length) {
    if(type == WStype_CONNECTED) Serial.println("Conectado ao Nginx!");
    if(type == WStype_DISCONNECTED) Serial.println("Desconectado!");
  });
}

void cameraTask(void* pvParameters) {
  for (;;) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      if (xQueueSend(frameQueue, &fb, 0) != pdTRUE) {
        esp_camera_fb_return(fb);
      }
    }
    // Aumente para 500ms (2 frames por segundo) para testar a estabilidade total
    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(115200);
  init_camera();

  frameQueue = xQueueCreate(2, sizeof(camera_fb_t*));
  
  WiFi.begin("los_pollos", "BlueMagic99%");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  init_websocket();
  xTaskCreate(cameraTask, "Camera Task", 4096, NULL, 1, NULL);
}

void loop() {
  webSocket.loop();

  camera_fb_t* fb = NULL;
  if (webSocket.isConnected() && xQueueReceive(frameQueue, &fb, 0)) {
    if (fb) {
      // Monitor de RAM para debug
      size_t freeHeap = ESP.getFreeHeap();
      
      if(freeHeap > 10000) { // Só tenta enviar se tiver mais de 10KB de RAM
        bool ok = webSocket.sendBIN(fb->buf, fb->len);
        if(!ok) Serial.println("Erro no envio SSL");
      } else {
        Serial.println("RAM baixa! Pulando frame...");
      }
      
      esp_camera_fb_return(fb);
    }
  }
  usleep(1000); // 1ms de descanso para o sistema
}
