#include <Arduino.h>

// Practical scenario: two factory stations record warehouse transactions.
// Both tasks increase the same transaction count without protecting the update.
// The delay between reading and writing makes lost updates easier to see.

volatile uint32_t sharedWarehouseTransactionCount = 0;
const uint32_t transactionsRecordedByEachStation = 300;
bool finalWarehouseReportHasBeenPrinted = false;

void recordIncomingDeliveryTransactions(void *taskParameter)
{
  (void)taskParameter;

  for (uint32_t transactionNumber = 0;
       transactionNumber < transactionsRecordedByEachStation;
       transactionNumber++)
  {
    uint32_t previouslyRecordedTransactionCount = sharedWarehouseTransactionCount;

    // Deliberately unsafe: another task can change the shared count during this wait.
    vTaskDelay(1);

    sharedWarehouseTransactionCount = previouslyRecordedTransactionCount + 1U;
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
    uint32_t previouslyRecordedTransactionCount = sharedWarehouseTransactionCount;

    // Deliberately unsafe: this task can overwrite the other task's update.
    vTaskDelay(1);

    sharedWarehouseTransactionCount = previouslyRecordedTransactionCount + 1U;
  }

  Serial.println("[Outgoing shipment station] Finished recording its transactions.");
  vTaskDelete(nullptr);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Race-condition problem: two stations update one shared count without a lock.");
  Serial.println("Expected total: 600 transactions. The actual total may be smaller.");

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
    // Give both workers time to finish all 300 updates.
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
