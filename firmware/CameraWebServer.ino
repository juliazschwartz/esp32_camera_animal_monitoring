#include <WiFi.h>
#include <WebSocketsClient.h>
#include "esp_camera.h"
#include "esp_bt.h" // Vamos desligar o BT para garantir estabilidade

const char* ssid = "los_pollos2.4GHz";
const char* password = "BlueMagic99%";

WebSocketsClient webSocket;
bool isConnected = false;

// Pinos AI-Thinker (seus originais)
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

// --- TASK DO WEBSOCKET (Roda no Core 0) ---
void webSocketTask(void *pvParameters) {
  for (;;) {
    webSocket.loop();
    // Delay de 1ms para não travar o watchdog do Core 0 (Protocol Core)
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

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

  config.frame_size   = FRAMESIZE_QQVGA; 
  config.jpeg_quality = 20; // Qualidade 20 é mais leve para o buffer que 60
  config.fb_count     = 1; 

  if (esp_camera_init(&config) != ESP_OK) return false;
  return true;
}

void setup() {
  Serial.begin(115200);
  btStop(); // Desliga Bluetooth para liberar RAM para o SSL

  if (!initCamera()) {
    Serial.println("Erro Camera!");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi OK!");

  // Configurações de estabilidade antes de iniciar a Task
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(10000, 3000, 2); 

  webSocket.beginSSL("mouse-monitoring.rennanfelipe.net", 9443, "/ws/");
  
  webSocket.onEvent([](WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
      Serial.println("Conectado!");
      isConnected = true;
    } else if (type == WStype_DISCONNECTED) {
      Serial.println("Desconectado!");
      isConnected = false;
    }
  });

  // Cria a Task no Core 0 (Onde roda o WiFi)
  // Stack de 6k é suficiente para o loop do WS se não processarmos texto pesado
  xTaskCreatePinnedToCore(webSocketTask, "WS_Task", 6144, NULL, 1, NULL, 0);
}

void loop() {
  // O loop principal roda no Core 1 por padrão
  
  if (isConnected) {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 200) { 
      
      camera_fb_t * fb = esp_camera_fb_get();
      if (fb) {
        // O envio via SSL é bloqueante, mas aqui roda no Core 1
        // enquanto o Core 0 continua respondendo Pings do servidor
        if(webSocket.sendBIN(fb->buf, fb->len)) {
          Serial.printf("Enviado: %d bytes | RAM: %u\n", fb->len, ESP.getFreeHeap());
        }
        esp_camera_fb_return(fb);
      }
      
      lastMillis = millis();
    }
  } else {
    // Se não estiver conectado, dá uma folga pro processador
    delay(100);
  }
}
