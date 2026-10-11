#include <Arduino.h>

// Practical scenario: a factory controller must check an urgent temperature alarm.
// The solution gives the alarm the higher priority and the background report a lower priority.
// The alarm task also waits briefly between checks so background work can use spare CPU time.

volatile uint32_t backgroundReportCalculationResult = 0;

void generateBackgroundFactoryReport(void *taskParameter)
{
  (void)taskParameter;

  while (true)
  {
    uint32_t calculationResult = backgroundReportCalculationResult + 1U;
    uint32_t workBatchStartTime = millis();

    while (millis() - workBatchStartTime < 150U)
    {
      for (uint32_t calculationStep = 0; calculationStep < 2500; calculationStep++)
      {
        calculationResult = (calculationResult * 33U) ^ calculationStep;
      }
    }

    backgroundReportCalculationResult = calculationResult;

    Serial.printf(
      "[Background report] Finished a report batch on CPU core %d. Result: %lu\n",
      xPortGetCoreID(),
      (unsigned long)backgroundReportCalculationResult
    );

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void checkFactoryTemperatureAlarm(void *taskParameter)
{
  (void)taskParameter;

  uint32_t nextAlarmDeadline = millis() + 100U;

  while (true)
  {
    uint32_t currentTime = millis();

    if ((int32_t)(currentTime - nextAlarmDeadline) >= 0)
    {
      uint32_t alarmResponseDelay = currentTime - nextAlarmDeadline;

      if (alarmResponseDelay > 20U)
      {
        Serial.printf(
          "[Temperature alarm] LATE by %lu milliseconds!\n",
          (unsigned long)alarmResponseDelay
        );
      }
      else
      {
        Serial.printf(
          "[Temperature alarm] Checked on time; delay was %lu milliseconds.\n",
          (unsigned long)alarmResponseDelay
        );
      }

      nextAlarmDeadline = currentTime + 100U;
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Priority solution: the urgent temperature alarm has the higher priority.");
  Serial.println("Compare alarm timing with the problem version.");

  // Correct assignment: the alarm gets priority 4; background work gets priority 1.
  xTaskCreatePinnedToCore(
    generateBackgroundFactoryReport,
    "BackgroundFactoryReportTask",
    4096,
    nullptr,
    1,
    nullptr,
    0
  );

  xTaskCreatePinnedToCore(
    checkFactoryTemperatureAlarm,
    "FactoryTemperatureAlarmTask",
    4096,
    nullptr,
    4,
    nullptr,
    0
  );
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
