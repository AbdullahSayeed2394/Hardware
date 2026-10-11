# Beginner Learning Guide: RTOS and FreeRTOS

## First understand the basic words

- **Microcontroller:** a small computer used inside a device, such as a sensor, robot, or appliance.
- **Operating system:** software that manages work and hardware resources.
- **RTOS:** a real-time operating system. It helps organize tasks that need to respond within time limits. It does not automatically make every program real-time or correct.
- **FreeRTOS:** an RTOS commonly used in embedded systems, including the ESP32 Arduino environment.
- **Task:** a piece of work that FreeRTOS can schedule. Each task has its own execution context and stack.
- **Scheduler:** the part of the RTOS that chooses which ready task should run.
- **Priority:** a number that tells the scheduler which ready task is more urgent. In FreeRTOS, a larger number means a higher priority.
- **Blocked task:** a task waiting for time, data, a lock, or an event. It does not need to use the CPU while it waits.
- **Concurrency:** tasks make progress over overlapping periods.
- **Parallelism:** tasks actually run at the same time on separate CPU cores.

## Topic 1 — Symmetric multiprocessing (SMP)

A dual-core ESP32 has two CPU cores. SMP scheduling allows eligible tasks to run across the cores. If independent workloads are pinned to the same core, they compete for that core even if another core is available to the application. The experiment reports each task's core number. Compare the problem and solution versions to see how task placement changes. Separate-core placement can enable parallel execution, but it does not guarantee a speed improvement for every workload.

The supplied sketches use `xTaskCreatePinnedToCore()` to choose a core. Use a dual-core ESP32 board such as the ESP32 DevKit v1 in Wokwi.

## Topic 2 — Priority-based task scheduling

FreeRTOS normally runs the highest-priority task that is ready to run on a core. A higher-priority ready task can preempt a lower-priority one. A high-priority task that remains ready for a long time can delay lower-priority tasks on the same core. A task that delays or waits becomes blocked and allows other work to run.

The experiment models factory report generation and a temperature-alarm check. The problem version gives the report the higher priority. The solution gives the alarm the higher priority. In real systems, choose task priority based on actual deadlines and consequences, keep urgent work brief, and measure the timing.

## Topic 3 — Shared-resource race condition

A race condition occurs when the result depends on the order or timing of tasks that use shared state. An update such as increasing a count usually involves reading the old count, calculating a new value, and writing it back. If two tasks perform those steps together, one update can overwrite the other.

The problem sketch has two warehouse stations updating one shared transaction count. The solution uses a FreeRTOS mutex. A task takes the mutex before updating the value and gives the mutex back immediately afterward. `volatile` alone does not make a shared update safe. In production firmware, never keep a mutex locked during long delays or unrelated work.

## Topic 4 — Inter-task communication

Inter-task communication transfers data or signals from one task to another. One task might produce sensor readings while another saves them. A single shared latest-value variable can be overwritten when the producer is faster than the consumer. A queue can buffer multiple messages and preserve their order until the receiver processes them, as long as the queue does not lose data due to an overflow policy.

The problem sketch uses a protected single-value mailbox. The mutex keeps the reading's fields consistent but cannot save previous readings that have been overwritten. The solution uses a queue with room for 20 readings. The consumer is deliberately slower, so the queue eventually fills and the producer waits for space. That waiting is called backpressure. In a real sensor system, decide whether waiting, dropping a reading, overwriting old data, or counting loss is appropriate.

## Suggested study order

1. Learn what an RTOS is and how a task differs from a normal function call.
2. Learn task states, priorities, preemption, and `vTaskDelay()`.
3. Learn how dual-core scheduling works and when core affinity is useful.
4. Learn why shared reads and writes can create race conditions.
5. Learn mutexes for protecting resources and queues for passing messages.
6. Run the problem sketch, describe the output in your own words, then run the solution sketch and compare.

## Useful resources

- Shawn Hymel / Digi-Key, Introduction to RTOS: https://github.com/ShawnHymel/introduction-to-rtos
- Wokwi ESP32 guide: https://docs.wokwi.com/guides/esp32
- FreeRTOS queue documentation: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues
- FreeRTOS mutex documentation: https://freertos.org/Real-time-embedded-RTOS-mutexes.html
