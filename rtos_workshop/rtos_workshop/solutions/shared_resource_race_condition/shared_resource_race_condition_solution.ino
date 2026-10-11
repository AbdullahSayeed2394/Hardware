#include <Arduino.h>

// Practical scenario: two factory stations record warehouse transactions.
// A FreeRTOS mutex protects the complete shared-count update.

volatile uint32_t sharedWarehouseTransactionCount = 0;
const uint32_t transactionsRecordedByEachStation = 300;
SemaphoreHandle_t warehouseTransactionCountMutex = nullptr;
bool finalWarehouseReportHasBeenPrinted = false;

void recordIncomingDeliveryTransactions(void *taskParameter)
{
  (void)taskParameter;

  for (uint32_t transactionNumber = 0;
       transactionNumber < transactionsRecordedByEachStation;
       transactionNumber++)
  {
    if (xSemaphoreTake(warehouseTransactionCountMutex, portMAX_DELAY) == pdTRUE)
    {
      sharedWarehouseTransactionCount = sharedWarehouseTransactionCount + 1U;
      xSemaphoreGive(warehouseTransactionCountMutex);
    }

    // Wait only after releasing the mutex so the other task can use the shared count.
    vTaskDelay(1);
  }

  Serial.println("[Incoming delivery station] Finished recording its transactions.");
  vTaskDelete(nullptr);
}

void recordOutgoingShipmentTransactions(void *taskParameter)
{
  (void)taskParameter;

  for (uint32_t transactionNumber = 0;
       transactionNumber < transactionsRecordedByEachStation;
       transactionNumber++)
  {
    if (xSemaphoreTake(warehouseTransactionCountMutex, portMAX_DELAY) == pdTRUE)
    {
      sharedWarehouseTransactionCount = sharedWarehouseTransactionCount + 1U;
      xSemaphoreGive(warehouseTransactionCountMutex);
    }

    vTaskDelay(1);
  }

  Serial.println("[Outgoing shipment station] Finished recording its transactions.");
  vTaskDelete(nullptr);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Race-condition solution: a mutex protects each transaction-count update.");

  warehouseTransactionCountMutex = xSemaphoreCreateMutex();

  if (warehouseTransactionCountMutex == nullptr)
  {
    Serial.println("ERROR: Could not create the warehouse transaction mutex.");
    return;
  }

  xTaskCreatePinnedToCore(
    recordIncomingDeliveryTransactions,
    "IncomingDeliveryTransactionTask",
    4096,
    nullptr,
    2,
    nullptr,
    0
  );

  xTaskCreatePinnedToCore(
    recordOutgoingShipmentTransactions,
    "OutgoingShipmentTransactionTask",
    4096,
    nullptr,
    2,
    nullptr,
    1
  );
}

void loop()
{
  if (!finalWarehouseReportHasBeenPrinted)
  {
    vTaskDelay(pdMS_TO_TICKS(5000));

    Serial.printf(
      "[Warehouse report] Expected transaction count: %lu\n",
      (unsigned long)(transactionsRecordedByEachStation * 2U)
    );
    Serial.printf(
      "[Warehouse report] Actual shared transaction count: %lu\n",
      (unsigned long)sharedWarehouseTransactionCount
    );

    finalWarehouseReportHasBeenPrinted = true;
  }

  vTaskDelay(pdMS_TO_TICKS(1000));
}
