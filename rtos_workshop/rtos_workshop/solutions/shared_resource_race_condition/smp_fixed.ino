// Experiment 1B: SMP - place independent workloads on different cores.
// Core affinity is a design choice; this example makes parallel placement visible.
#include <Arduino.h>

void cpuWorkForMs(uint32_t ms) {
  const uint32_t started = millis();
  volatile uint32_t sink = 0;
  while ((uint32_t)(millis() - started) < ms) {
    for (int i = 0; i < 100; ++i) sink += i;
  }
  (void)sink;
}

void workerA(void *parameter) {
  for (;;) {
    cpuWorkForMs(100);
    Serial.printf("Task A finished work on Core %d at %lu ms\n",
                  xPortGetCoreID(), (unsigned long)millis());
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void workerB(void *parameter) {
  for (;;) {
    cpuWorkForMs(100);
    Serial.printf("Task B finished work on Core %d at %lu ms\n",
                  xPortGetCoreID(), (unsigned long)millis());
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("SMP SOLUTION DEMO: Worker A on Core 0, Worker B on Core 1");
  xTaskCreatePinnedToCore(workerA, "WorkerA", 3072, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(workerB, "WorkerB", 3072, nullptr, 1, nullptr, 1);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
