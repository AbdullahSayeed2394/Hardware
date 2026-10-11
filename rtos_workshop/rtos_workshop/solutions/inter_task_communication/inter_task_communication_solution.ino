#include <Arduino.h>

// Practical scenario: a greenhouse has one task creating sensor readings and
// another task saving readings to a history log. A FreeRTOS queue buffers readings
// so the slower logger can process older samples in order.

struct GreenhouseSensorReading
{
  uint32_t sampleNumber;
  float temperatureCelsius;
  float humidityPercent;
};

QueueHandle_t greenhouseSensorReadingQueue = nullptr;

void createGreenhouseSensorReadings(void *taskParameter)
{
  (void)taskParameter;

  uint32_t nextSampleNumber = 0;

  while (true)
  {
    nextSampleNumber++;

    GreenhouseSensorReading newGreenhouseReading;
    newGreenhouseReading.sampleNumber = nextSampleNumber;
    newGreenhouseReading.temperatureCelsius = 24.0f + (nextSampleNumber % 12U) * 0.3f;
    newGreenhouseReading.humidityPercent = 55.0f + (nextSampleNumber % 10U) * 0.7f;

    // If the queue is full, the producer waits until there is room for this reading.
    if (xQueueSend(greenhouseSensorReadingQueue, &newGreenhouseReading, portMAX_DELAY) == pdPASS)
    {
      Serial.printf(
        "[Sensor producer] Added sample %lu to the queue\n",
        (unsigned long)newGreenhouseReading.sampleNumber
      );
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void saveGreenhouseSensorReadings(void *taskParameter)
{
  (void)taskParameter;

  uint32_t lastProcessedSampleNumber = 0;

  while (true)
  {
    GreenhouseSensorReading readingReadyToSave;

    // The consumer waits here until a complete reading is available.
    if (xQueueReceive(greenhouseSensorReadingQueue, &readingReadyToSave, portMAX_DELAY) == pdPASS)
    {
      if (lastProcessedSampleNumber > 0 &&
          readingReadyToSave.sampleNumber != lastProcessedSampleNumber + 1U)
      {
        Serial.println("[History logger] WARNING: A sample number was missing from the queue.");
      }

      Serial.printf(
        "[History logger] Saved sample %lu: %.1f C, %.1f percent humidity\n",
        (unsigned long)readingReadyToSave.sampleNumber,
        readingReadyToSave.temperatureCelsius,
        readingReadyToSave.humidityPercent
      );

      lastProcessedSampleNumber = readingReadyToSave.sampleNumber;

      // Slow processing deliberately makes the queue build up so you can observe buffering.
      vTaskDelay(pdMS_TO_TICKS(350));
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Inter-task communication solution: a FreeRTOS queue stores multiple sensor readings.");
  Serial.println("The queue can hold 20 readings and transfers them in order.");

  greenhouseSensorReadingQueue = xQueueCreate(20, sizeof(GreenhouseSensorReading));

  if (greenhouseSensorReadingQueue == nullptr)
  {
    Serial.println("ERROR: Could not create the greenhouse sensor-reading queue.");
    return;
  }

  xTaskCreatePinnedToCore(
    createGreenhouseSensorReadings,
    "GreenhouseSensorProducerTask",
    4096,
    nullptr,
    1,
    nullptr,
    0
  );

  xTaskCreatePinnedToCore(
    saveGreenhouseSensorReadings,
    "GreenhouseHistoryLoggerTask",
    4096,
    nullptr,
    1,
    nullptr,
    1
  );
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
