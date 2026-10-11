#include <Arduino.h>

// Practical scenario: a greenhouse has one task creating sensor readings and
// another task saving readings to a history log. The shared mailbox stores only
// the newest reading, so intermediate readings can be overwritten before processing.

struct GreenhouseSensorReading
{
  uint32_t sampleNumber;
  float temperatureCelsius;
  float humidityPercent;
};

GreenhouseSensorReading newestGreenhouseSensorReading = {0, 0.0f, 0.0f};
SemaphoreHandle_t greenhouseReadingMutex = nullptr;

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

    if (xSemaphoreTake(greenhouseReadingMutex, portMAX_DELAY) == pdTRUE)
    {
      newestGreenhouseSensorReading = newGreenhouseReading;
      xSemaphoreGive(greenhouseReadingMutex);
    }

    Serial.printf(
      "[Sensor producer] Created sample %lu\n",
      (unsigned long)newGreenhouseReading.sampleNumber
    );

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void saveGreenhouseSensorReadings(void *taskParameter)
{
  (void)taskParameter;

  uint32_t lastProcessedSampleNumber = 0;

  while (true)
  {
    GreenhouseSensorReading readingReadyToSave = {0, 0.0f, 0.0f};

    if (xSemaphoreTake(greenhouseReadingMutex, portMAX_DELAY) == pdTRUE)
    {
      readingReadyToSave = newestGreenhouseSensorReading;
      xSemaphoreGive(greenhouseReadingMutex);
    }

    if (readingReadyToSave.sampleNumber > 0)
    {
      if (lastProcessedSampleNumber > 0 &&
          readingReadyToSave.sampleNumber > lastProcessedSampleNumber + 1U)
      {
        uint32_t overwrittenReadingCount =
          readingReadyToSave.sampleNumber - lastProcessedSampleNumber - 1U;

        Serial.printf(
          "[History logger] WARNING: %lu readings were overwritten before they were saved.\n",
          (unsigned long)overwrittenReadingCount
        );
      }

      Serial.printf(
        "[History logger] Saved sample %lu: %.1f C, %.1f percent humidity\n",
        (unsigned long)readingReadyToSave.sampleNumber,
        readingReadyToSave.temperatureCelsius,
        readingReadyToSave.humidityPercent
      );

      lastProcessedSampleNumber = readingReadyToSave.sampleNumber;
    }

    // The logger is deliberately slower than the sensor-reading task.
    vTaskDelay(pdMS_TO_TICKS(350));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Inter-task communication problem: one shared value cannot keep every sample.");
  Serial.println("The simulated sensor creates a sample every 100 milliseconds.");
  Serial.println("The history logger reads a sample every 350 milliseconds.");

  greenhouseReadingMutex = xSemaphoreCreateMutex();

  if (greenhouseReadingMutex == nullptr)
  {
    Serial.println("ERROR: Could not create the greenhouse reading mutex.");
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
