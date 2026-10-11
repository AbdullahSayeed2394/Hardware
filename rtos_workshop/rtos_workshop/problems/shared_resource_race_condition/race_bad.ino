// Experiment 3A: Race condition - two cores update one shared counter without a lock.
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static constexpr uint32_t ITERATIONS = 2000;
volatile uint32_t sharedCounter = 0;
SemaphoreHandle_t finished;

void incrementTask(void *parameter) {
  for (uint32_t i = 0; i < ITERATIONS; ++i) {
    uint32_t temporary = sharedCounter; // read
    delayMicroseconds(50);              // widen the race window for this demo
    sharedCounter = temporary + 1;      // write can overwrite the other task's update
  }
  xSemaphoreGive(finished);
  vTaskDelete(nullptr);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  finished = xSemaphoreCreateCounting(2, 0);
  if (finished == nullptr) {
    Serial.println("ERROR: could not create completion semaphore");
    return;
  }
  Serial.println("RACE BAD DEMO: expected final count = 4000");
  xTaskCreatePinnedToCore(incrementTask, "CounterA", 3072, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(incrementTask, "CounterB", 3072, nullptr, 1, nullptr, 1);
}

void loop() {
  static bool reported = false;
  if (!reported && finished != nullptr) {
    xSemaphoreTake(finished, portMAX_DELAY);
    xSemaphoreTake(finished, portMAX_DELAY);
    Serial.printf("Expected: %lu | Actual: %lu\n",
                  (unsigned long)(2 * ITERATIONS), (unsigned long)sharedCounter);
    Serial.println("Compare the actual count with the expected count.");
    reported = true;
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}
