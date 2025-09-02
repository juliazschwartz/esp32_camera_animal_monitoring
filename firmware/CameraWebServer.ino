// #include "esp_camera.h"
// #include <WiFi.h>
// #include <ArduinoWebsockets.h>
// #include "esp_timer.h"
// #include "img_converters.h"
// #include "fb_gfx.h"
// #include "soc/soc.h" //disable brownout problems
// #include "soc/rtc_cntl_reg.h"  //disable brownout problems
// #include "driver/gpio.h"

// //this one for AI_THINKER
// // #define PWDN_GPIO_NUM     32
// // #define RESET_GPIO_NUM    -1
// // #define XCLK_GPIO_NUM      0
// // #define SIOD_GPIO_NUM     26
// // #define SIOC_GPIO_NUM     27

// // #define Y9_GPIO_NUM       35
// // #define Y8_GPIO_NUM       34
// // #define Y7_GPIO_NUM       39
// // #define Y6_GPIO_NUM       36
// // #define Y5_GPIO_NUM       21
// // #define Y4_GPIO_NUM       19
// // #define Y3_GPIO_NUM       18
// // #define Y2_GPIO_NUM        5
// // #define VSYNC_GPIO_NUM    25
// // #define HREF_GPIO_NUM     23
// // #define PCLK_GPIO_NUM     22

// //THIS one fot Wroover
// #define PWDN_GPIO_NUM -1
// #define RESET_GPIO_NUM -1
// #define XCLK_GPIO_NUM 21
// #define SIOD_GPIO_NUM 26
// #define SIOC_GPIO_NUM 27
// #define Y9_GPIO_NUM 35
// #define Y8_GPIO_NUM 34
// #define Y7_GPIO_NUM 39
// #define Y6_GPIO_NUM 36
// #define Y5_GPIO_NUM 19
// #define Y4_GPIO_NUM 18
// #define Y3_GPIO_NUM 5
// #define Y2_GPIO_NUM 4
// #define VSYNC_GPIO_NUM 25
// #define HREF_GPIO_NUM 23
// #define PCLK_GPIO_NUM 22



// char * url = "ws://192.168.0.104:8080";
// // char * url = "ws://164.92.239.48:8080";


// camera_fb_t * fb = NULL;
// size_t _jpg_buf_len = 0;
// uint8_t * _jpg_buf = NULL;
// uint8_t state = 0;

// using namespace websockets;
// WebsocketsClient client;

// ///////////////////////////////////CALLBACK FUNCTIONS///////////////////////////////////
// void onMessageCallback(WebsocketsMessage message) {
//   Serial.print("Got Message: ");
//   Serial.println(message.data());
// }

// ///////////////////////////////////INITIALIZE FUNCTIONS///////////////////////////////////
// esp_err_t init_camera() {
//   camera_config_t config;
//   config.ledc_channel = LEDC_CHANNEL_0;
//   config.ledc_timer = LEDC_TIMER_0;
//   config.pin_d0 = Y2_GPIO_NUM;
//   config.pin_d1 = Y3_GPIO_NUM;
//   config.pin_d2 = Y4_GPIO_NUM;
//   config.pin_d3 = Y5_GPIO_NUM;
//   config.pin_d4 = Y6_GPIO_NUM;
//   config.pin_d5 = Y7_GPIO_NUM;
//   config.pin_d6 = Y8_GPIO_NUM;
//   config.pin_d7 = Y9_GPIO_NUM;
//   config.pin_xclk = XCLK_GPIO_NUM;
//   config.pin_pclk = PCLK_GPIO_NUM;
//   config.pin_vsync = VSYNC_GPIO_NUM;
//   config.pin_href = HREF_GPIO_NUM;
//   config.pin_sscb_sda = SIOD_GPIO_NUM;
//   config.pin_sscb_scl = SIOC_GPIO_NUM;
//   config.pin_pwdn = PWDN_GPIO_NUM;
//   config.pin_reset = RESET_GPIO_NUM;
//   config.xclk_freq_hz = 20000000;
//   config.pixel_format = PIXFORMAT_JPEG;
//   //init with high specs to pre-allocate larger buffers
//   if (psramFound()) {
//     config.frame_size = FRAMESIZE_XGA;
//     config.jpeg_quality = 12;
//     config.fb_count = 2;
//   } else {
//     config.frame_size = FRAMESIZE_SVGA;
//     config.jpeg_quality = 12;
//     config.fb_count = 1;
//   }
//   // Camera init
//   esp_err_t err = esp_camera_init(&config);
//   if (err != ESP_OK) {
//     Serial.printf("Camera init failed with error 0x%x", err);
//     return err;
//   }
//   sensor_t * s = esp_camera_sensor_get();
//   s->set_framesize(s, FRAMESIZE_VGA);
//   Serial.println("Cam Success init");
//   return ESP_OK;
// };


// esp_err_t init_wifi() {
//   WiFi.begin("Zamitello_2G", "coruja2024");
//   Serial.println("Starting Wifi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("Connecting to websocket");
//   client.onMessage(onMessageCallback);
//   bool connected = client.connect(url);
//   if (!connected) {
//     Serial.println("Cannot connect to websocket server!");
//     state = 3;
//     return ESP_FAIL;
//   }
//   if (state == 3) {
//     return ESP_FAIL;
//   }

//   Serial.println("Websocket Connected!");
//   client.send("deviceId"); // for verification
//   return ESP_OK;
// };


// ///////////////////////////////////SETUP///////////////////////////////////
// void setup() {
//   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
//   //  disableCore0WDT();

//   Serial.begin(115200);
//   Serial.setDebugOutput(true);
//   init_camera();
//   init_wifi();
// }

// ///////////////////////////////////MAIN LOOP///////////////////////////////////
// void loop() {
//   if (client.available()) {
//     camera_fb_t *fb = esp_camera_fb_get();
//     if (!fb) {
//       Serial.println("Camera capture failed");
//       esp_camera_fb_return(fb);
//       ESP.restart();
//     }
//     client.sendBinary((const char*) fb->buf, fb->len);
//     Serial.println("MJPG sent");
//     esp_camera_fb_return(fb);
//     client.poll();
//   }
// }


#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "DHT.h"
#include <WiFiManager.h>

#define DHTPIN 33
#define DHTTYPE DHT22
#define MQ137_PIN 32

// Camera pins Wrover
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

const char* WIFI_SSID = "Zamitello_2G";
const char* WIFI_PASS = "coruja2024";
const char* WS_URL = "ws://164.92.239.48:8080";

using namespace websockets;
WebsocketsClient client;

DHT dht(DHTPIN, DHTTYPE);

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
}

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

  // config.frame_size = FRAMESIZE_VGA;
  config.frame_size = FRAMESIZE_QVGA; // menor que VGA

  config.jpeg_quality = 25;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    while (true) delay(1000);
  }
  Serial.println("Camera initialized");
}

void init_websocket() {
  client.onMessage(onMessageCallback);

  int retries = 0;
  while (!client.connect(WS_URL)) {
    Serial.println("WebSocket falhou, tentando novamente...");
    retries++;
    if (retries > 5) {
      Serial.println("Muitas tentativas, aguardando 5s...");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      retries = 0;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  Serial.println("WebSocket conectado");
  client.send("{\"type\":\"info\",\"data\":\"deviceId\"}");
}

void cameraTask(void* pvParameters) {
  for (;;) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      if (client.available()) {
        client.sendBinary((const char*)fb->buf, fb->len);
        
      }
      esp_camera_fb_return(fb);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS); // 5 FPS

  }
}

void sensorTask(void* pvParameters) {
  for (;;) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int mq137 = analogRead(MQ137_PIN);

    // Verifica se leituras são válidas
    if (isnan(h) || isnan(t)) {
      Serial.println("Falha na leitura do DHT");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.print("Temp: "); Serial.print(t);
    Serial.print(" C, Hum: "); Serial.print(h);
    Serial.print(" %, MQ137: "); Serial.println(mq137);

    if (client.available()) {
      // Monta payload JSON completo com tipo
      String payload = "{";
      payload += "\"type\":\"sensor\",";
      payload += "\"data\":{";
      payload += "\"temperature\":" + String(t, 2) + ",";
      payload += "\"humidity\":" + String(h, 2) + ",";
      payload += "\"mq137\":" + String(mq137);
      payload += "}}";

      client.send(payload);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ137_PIN, INPUT);

  init_camera();
  WiFiManager wm;
  // força esquecer redes salvas
  // wm.resetSettings();

  // Se não conseguir conectar, abre AP "ESP_AP"
  wm.autoConnect("ESP_AP");
  Serial.println("Conectado: " + WiFi.SSID());
  init_websocket();

  // Envia deviceId como JSON
  if (client.available()) {
    String idPayload = "{\"type\":\"info\",\"data\":\"deviceId\"}";
    client.send(idPayload);
  }

  xTaskCreate(cameraTask, "Camera Task", 8192, NULL, 1, NULL);
  xTaskCreate(sensorTask, "Sensor Task", 4096, NULL, 1, NULL);
}
void loop() {
  client.poll();
    // Se desconectou, tenta reconectar
  if (!client.available()) {
    Serial.println("WebSocket desconectado, tentando reconectar...");
    init_websocket();
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);
}


