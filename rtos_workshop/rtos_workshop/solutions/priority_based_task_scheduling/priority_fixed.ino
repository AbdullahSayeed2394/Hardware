// Experiment 2B: Priority - give time-sensitive work the higher priority.
#include <Arduino.h>

void backgroundTask(void *parameter) {
  for (;;) {
    // A low-priority background job does a bounded amount of work.
    uint32_t started = millis();
    volatile uint32_t sink = 0;
    while ((uint32_t)(millis() - started) < 80) {
      for (int i = 0; i < 100; ++i) sink += i;
    }
    (void)sink;
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void alarmTask(void *parameter) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    Serial.printf("[IMPORTANT ALARM] ran at %lu ms\n", (unsigned long)millis());
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("PRIORITY SOLUTION DEMO: alarm=4, background=1 (same core)");
  xTaskCreatePinnedToCore(backgroundTask, "Background", 3072, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(alarmTask, "Alarm", 3072, nullptr, 4, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
