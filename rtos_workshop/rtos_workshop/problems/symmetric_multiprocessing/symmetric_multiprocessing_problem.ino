#include <Arduino.h>

// Practical scenario: a small factory monitors a cooling system and a conveyor.
// This problem version puts both independent tasks on CPU Core 0.
// The second core is not used by these two application tasks.

volatile uint32_t coolingSystemHealthScore = 0;
volatile uint32_t conveyorProductionHealthScore = 0;

uint32_t calculateCoolingSystemHealthScore(uint32_t startingValue)
{
  uint32_t calculatedValue = startingValue;

  for (uint32_t calculationStep = 0; calculationStep < 160000; calculationStep++)
  {
    calculatedValue = (calculatedValue * 33U) ^ calculationStep;
  }

  return calculatedValue;
}

uint32_t calculateConveyorProductionHealthScore(uint32_t startingValue)
{
  uint32_t calculatedValue = startingValue + 17U;

  for (uint32_t calculationStep = 0; calculationStep < 160000; calculationStep++)
  {
    calculatedValue = (calculatedValue * 31U) + calculationStep;
  }

  return calculatedValue;
}

void monitorFactoryCoolingSystem(void *taskParameter)
{
  (void)taskParameter;

  while (true)
  {
    coolingSystemHealthScore = calculateCoolingSystemHealthScore(millis());

    Serial.printf(
      "[Cooling system] Running on CPU core %d. Health score: %lu\n",
      xPortGetCoreID(),
      (unsigned long)coolingSystemHealthScore
    );

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void monitorConveyorProduction(void *taskParameter)
{
  (void)taskParameter;

  while (true)
  {
    conveyorProductionHealthScore = calculateConveyorProductionHealthScore(millis());

    Serial.printf(
      "[Conveyor production] Running on CPU core %d. Health score: %lu\n",
      xPortGetCoreID(),
      (unsigned long)conveyorProductionHealthScore
    );

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("SMP problem: two independent factory tasks share CPU Core 0.");
  Serial.println("Watch the CPU core number printed by each task.");

  if (xTaskCreatePinnedToCore(
        monitorFactoryCoolingSystem,
        "FactoryCoolingSystemMonitoringTask",
        4096,
        nullptr,
        1,
        nullptr,
        0) != pdPASS)
  {
    Serial.println("Could not create the cooling system monitoring task.");
  }

  if (xTaskCreatePinnedToCore(
        monitorConveyorProduction,
        "ConveyorProductionMonitoringTask",
        4096,
        nullptr,
        1,
        nullptr,
        0) != pdPASS)
  {
    Serial.println("Could not create the conveyor production monitoring task.");
  }
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}
