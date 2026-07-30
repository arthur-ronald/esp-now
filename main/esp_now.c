#include "esp_now.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip_types.h"
#include "nvs_flash.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>

#define ESP_CHANNEL 1

static uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0xe8, 0xf6, 0x0a,
                                             0x8c, 0xa9, 0x54};

static const char *TAG = "esp_now_init";

void recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data,
             int data_len) {
  ESP_LOGI(TAG, "data received: " MACSTR "%s", MAC2STR(esp_now_info->src_addr),
           data);
}

void send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    ESP_LOGI(TAG, "ESP_NOW_SEND_SUCCESS");
  } else {
    ESP_LOGI(TAG, "ESP_NOW_SEND_FAIL");
  }
}

static esp_err_t init_wifi(void) {

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

  esp_netif_init();
  esp_event_loop_create_default();
  nvs_flash_init();
  esp_wifi_init(&wifi_init_config);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_storage(WIFI_STORAGE_FLASH);
  esp_wifi_start();

  ESP_LOGI(TAG, "wifi init completed");

  return ESP_OK;
}

static esp_err_t init_esp_now(void) {
  esp_now_init();
  esp_now_register_recv_cb(recv_cb);
  esp_now_register_send_cb(send_cb);

  ESP_LOGI(TAG, "esp now init completed");

  return ESP_OK;
}

static esp_err_t register_peer(uint8_t *peer_addr) {

  esp_now_peer_info_t esp_now_peer_info = {};
  memcpy(esp_now_peer_info.peer_addr, peer_addr, ESP_NOW_ETH_ALEN);
  esp_now_peer_info.channel = ESP_CHANNEL;
  esp_now_peer_info.ifidx = WIFI_IF_STA;

  esp_now_add_peer(&esp_now_peer_info);
  return ESP_OK;
}

static esp_err_t esp_now_send_data(const uint8_t *peer_addr,
                                   const uint8_t *data, uint8_t len) {

  esp_now_send(peer_addr, data, len);
  return ESP_OK;
}

void app_main(void) {
  ESP_ERROR_CHECK(init_wifi());
  ESP_ERROR_CHECK(init_esp_now());
  ESP_ERROR_CHECK(register_peer(peer_mac));

  uint8_t dataR[] = "255|0|0";
  uint8_t dataG[] = "0|255|0";
  uint8_t dataB[] = "0|0|255";

  uint8_t count = 0;

  while (true) {
    count++;
    if (count > 2) {
      count = 0;
    }
    switch (count) {
    case 0:
      esp_now_send_data(peer_mac, dataR, 32);
      break;
    case 1:
      esp_now_send_data(peer_mac, dataG, 32);
      break;
    case 2:
      esp_now_send_data(peer_mac, dataB, 32);
      break;
    default:
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
