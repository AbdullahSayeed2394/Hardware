// Experiment 4B: Use a FreeRTOS queue to buffer messages between tasks.
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static constexpr UBaseType_t QUEUE_LENGTH = 20;
QueueHandle_t sampleQueue;

void producerTask(void *parameter) {
  uint32_t sample = 0;
  for (;;) {
    uint32_t reading = ++sample;
    // Wait for space instead of silently overwriting a previous sample.
    if (xQueueSend(sampleQueue, &reading, portMAX_DELAY) != pdPASS) {
      Serial.println("Queue send failed");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void consumerTask(void *parameter) {
  uint32_t reading = 0;
  for (;;) {
    if (xQueueReceive(sampleQueue, &reading, portMAX_DELAY) == pdPASS) {
      Serial.printf("Consumer received sample=%lu\n", (unsigned long)reading);
      // Simulate slower processing; the queue stores waiting messages.
      vTaskDelay(pdMS_TO_TICKS(350));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  sampleQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
  if (sampleQueue == nullptr) {
    Serial.println("ERROR: could not create sample queue");
    return;
  }
  Serial.println("IPC FIXED DEMO: producer sends each sample through a queue");
  xTaskCreatePinnedToCore(producerTask, "Producer", 3072, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(consumerTask, "Consumer", 3072, nullptr, 1, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
