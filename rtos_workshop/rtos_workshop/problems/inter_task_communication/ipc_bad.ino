// Experiment 4A: Bad inter-task communication - a shared variable keeps only latest data.
#include <Arduino.h>

volatile uint32_t latestReading = 0;

void producerTask(void *parameter) {
  uint32_t sample = 0;
  for (;;) {
    latestReading = ++sample; // intermediate values can be overwritten before the reader sees them
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void consumerTask(void *parameter) {
  uint32_t lastSeen = 0;
  for (;;) {
    uint32_t current = latestReading;
    if (current != lastSeen) {
      uint32_t skipped = (current > lastSeen) ? current - lastSeen - 1 : 0;
      Serial.printf("Consumer received latest=%lu, skipped=%lu sample(s)\n",
                    (unsigned long)current, (unsigned long)skipped);
      lastSeen = current;
    }
    vTaskDelay(pdMS_TO_TICKS(350));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("IPC BAD DEMO: producer writes faster than consumer reads");
  xTaskCreatePinnedToCore(producerTask, "Producer", 3072, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(consumerTask, "Consumer", 3072, nullptr, 1, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
