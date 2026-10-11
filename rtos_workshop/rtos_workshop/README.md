# RTOS Workshop Assignment — Beginner Project

This project contains **four problem-and-solution pairs** using the ESP32 and FreeRTOS. Each topic has two Arduino sketches:

- One sketch demonstrates a problem in a practical situation.
- One matching sketch demonstrates a way to address the problem with FreeRTOS.

The code uses descriptive folder names, file names, function names, and variable names. The examples simulate sensors and factory activities, so no physical sensors or other parts are required.

> **Important for your assignment:** Run the sketches yourself and write your own observations in the worksheet near the end of this README. The worksheet is intentionally blank so you can follow your instructor's instruction.

## 1. How to run the project in Wokwi

1. Open [Wokwi's ESP32 simulator](https://wokwi.com/projects/new/esp32).
2. Choose a **dual-core ESP32 board**, such as **ESP32 DevKit v1**. The symmetric multiprocessing example uses CPU Core 0 and CPU Core 1, so do not choose a single-core ESP32-C3 board for that experiment.
3. Make a separate Wokwi project for each sketch. Do not paste all eight sketches into one project because each sketch has its own `setup()` and `loop()` functions.
4. Open the `.ino` file for the experiment you want to run, copy all its contents into Wokwi's `sketch.ino`, and start the simulation.
5. Open the Serial Monitor. The sketches use **115200 baud**.
6. Run the problem sketch first. Watch its output and write your own observations.
7. Run its matching solution sketch. Compare the two outputs and write what changed and why.

The examples use the FreeRTOS functions built into the Arduino ESP32 environment. No extra Arduino library is required.

## 2. Project folder layout

- `problems/` contains the four deliberately flawed practical examples.
- `solutions/` contains one matching solution for each example.

Each topic folder has exactly two sketches in total across the two top-level folders: a problem sketch and a solution sketch.

### Symmetric multiprocessing

- Problem: `problems/symmetric_multiprocessing/symmetric_multiprocessing_problem.ino`
- Solution: `solutions/symmetric_multiprocessing/symmetric_multiprocessing_solution.ino`

### Priority-based task scheduling

- Problem: `problems/priority_based_task_scheduling/priority_based_task_scheduling_problem.ino`
- Solution: `solutions/priority_based_task_scheduling/priority_based_task_scheduling_solution.ino`

### Shared-resource race condition

- Problem: `problems/shared_resource_race_condition/shared_resource_race_condition_problem.ino`
- Solution: `solutions/shared_resource_race_condition/shared_resource_race_condition_solution.ino`

### Inter-task communication

- Problem: `problems/inter_task_communication/inter_task_communication_problem.ino`
- Solution: `solutions/inter_task_communication/inter_task_communication_solution.ino`

---

# 3. Topic 1 — Symmetric Multiprocessing (SMP)

## What does it mean?

A CPU core is a processing unit that runs instructions. A dual-core ESP32 has two CPU cores. Symmetric multiprocessing, usually shortened to **SMP**, allows the operating system to schedule eligible tasks across those cores.

Think of two people doing separate jobs. If both jobs are assigned to Person A while Person B is available, Person A has to handle both. Assigning independent jobs to different people can let both jobs progress at the same time.

SMP is not itself a fault. The example demonstrates a design problem: two independent tasks have both been pinned to the same core, so those tasks cannot use the other core for their own work.

## Five real-world situations where core placement matters

1. **Factory monitoring:** one task checks motor temperature while another counts products on a conveyor. If both are placed on one core, they compete for that core.
2. **Robot control:** one task processes distance-sensor readings while another prepares camera information. Independent work may be able to run on separate cores.
3. **Smart greenhouse:** one task checks temperature and humidity while another updates an information display and records readings.
4. **Drone monitoring:** one task processes status information while another handles communication and telemetry. Timing and safety requirements still need careful design.
5. **Network device:** one task processes incoming messages while another performs diagnostics. Placing eligible workloads carefully can help make use of both cores.

These are examples where multicore design may be useful. Using both cores does not automatically make every program faster; work, synchronization, and timing all matter.

## General solution process

1. List the tasks and identify which tasks can work independently.
2. Check whether the selected microcontroller actually has more than one CPU core.
3. Use task affinity when appropriate to allow independent work to run on separate cores. In these examples, `xTaskCreatePinnedToCore()` selects a core.
4. Do not split tasks that need frequent, unsafe access to the same data without planning how to protect that data.
5. Run the program and compare the core numbers printed by each task. Then consider whether the changed placement improves the actual work, rather than assuming it must.

## What the supplied experiment demonstrates

The factory cooling-system task and conveyor-production task both print the CPU core on which they run. In the problem sketch, both are assigned to Core 0. In the solution sketch, one is assigned to Core 0 and the other to Core 1.

---

# 4. Topic 2 — Priority-Based Task Scheduling

## What does it mean?

A task is a piece of work the RTOS can schedule. A task's priority tells the scheduler how urgently that task should run compared with other ready tasks. In FreeRTOS, a **larger priority number means a higher priority**.

For example, responding to a dangerous temperature may be more urgent than preparing a background report. If the background report is given the higher priority and keeps using the CPU, the alarm check can be delayed.

A task that regularly waits, or becomes **Blocked**, allows other tasks to run. A task that continuously stays Ready at a higher priority can prevent lower-priority tasks on the same core from running when they need to.

## Five real-world situations where priority matters

1. **Overheating machine:** the task that detects unsafe temperature should receive CPU time before a non-urgent report task.
2. **Robot emergency stop:** responding to an emergency-stop signal must not wait behind a lengthy screen animation.
3. **Industrial motor:** checking an overcurrent condition should take priority over collecting statistics for a dashboard.
4. **Medical monitor:** a task that detects a dangerous reading may need to run before a task that saves routine measurements to storage.
5. **Communication device:** processing a time-sensitive control message may need priority over refreshing a display or printing debug information.

The correct priorities depend on actual timing requirements. A high number does not automatically make code safe or correct.

## General solution process

1. Identify what happens if each task runs late. Give truly time-sensitive tasks higher priority than routine background tasks.
2. Set priorities consistently with those deadlines and consequences.
3. Keep high-priority work short and predictable. Avoid making an urgent task do unnecessary printing or lengthy calculations.
4. Use suitable delay or blocking calls when a task has nothing to do, rather than making it consume the CPU continuously.
5. Run the program and compare the alarm's response delay in the problem and solution versions. Do not judge the result from a single message only; look at repeated output.

## What the supplied experiment demonstrates

A factory temperature-alarm task checks a deadline, while a background task repeatedly calculates a report. The problem version gives the report the higher priority. The solution gives the alarm the higher priority and leaves the report at a lower priority. Both tasks use the same core to make priority effects easier to see.

---

# 5. Topic 3 — Shared-Resource Race Condition

## What does it mean?

A **shared resource** is information or hardware that more than one task can use. A **race condition** happens when the result depends on the timing or order of those tasks.

For example, imagine a shared count is 10. Task A reads 10, and Task B also reads 10. Both calculate 11 and write 11. Even though two updates were requested, the final result is 11 instead of 12. One update was lost.

The common operation that looks like “increase the value by one” actually has to read the old value, calculate a new value, and write it back. Those steps must be protected when several tasks can update the same value.

## Five real-world situations where race conditions happen

1. **Warehouse records:** two tasks record incoming and outgoing transactions by increasing the same transaction count.
2. **Production counter:** two sensors or tasks register products on a line and update one shared total.
3. **Shared device settings:** one task changes a motor setting while another task reads or changes the same setting.
4. **Shared message buffer:** a communication task writes incoming data while another task edits or processes the same buffer.
5. **Shared event history:** separate tasks add events to the same record list, and one update can overwrite or conflict with another.

## General solution process

1. Identify each shared variable or resource.
2. Find every place where a task reads or changes it. Pay special attention to operations that read a value and then write an updated value.
3. Protect the entire operation that must happen as one unit. A FreeRTOS mutex is suitable when tasks need exclusive access to a shared resource.
4. Take the mutex before the protected operation and release it immediately after the operation is complete.
5. Keep the protected section short. Do not wait for long periods, print large reports, or do unrelated work while holding the mutex.
6. Repeat the test and compare the expected count with the actual count.

`volatile` alone does **not** solve a race condition. It does not make a multi-step update indivisible and does not replace a mutex or another proper synchronization method.

## What the supplied experiment demonstrates

Two warehouse stations each record 300 transactions. The expected total is 600. The problem version reads and writes the shared count without protection, with a delay between those steps to make lost updates easier to see. The solution protects each update with a FreeRTOS mutex.

The actual count in the problem sketch depends on task timing and may vary between runs. The solution is expected to report 600 after both tasks finish.

---

# 6. Topic 4 — Inter-Task Communication

## What does it mean?

**Inter-task communication** means passing information or signals from one task to another. For example, one task reads a sensor while another task saves the readings.

A shared variable can hold only its latest value. If the producer creates a new reading every 100 milliseconds and the consumer processes a reading every 350 milliseconds, several readings may be replaced before the consumer gets to process them.

A FreeRTOS **queue** can hold several messages and deliver them in order. If the queue is full, the program must have a plan. In this example, the producer waits for room, which is called **backpressure**. This is suitable only when waiting is acceptable for the producer.

A queue moves data between tasks. A mutex protects a shared resource from simultaneous access. They serve different purposes, even though both are useful in task-based programs.

## Five real-world situations where inter-task communication matters

1. **Greenhouse monitoring:** the sensor task passes temperature and humidity readings to a history-logging task.
2. **Robot control:** a button-reading task sends movement commands to a motor-control task.
3. **Serial communication:** one task receives incoming characters while another task assembles and processes complete commands.
4. **Safety monitoring:** a vibration-sensor task sends an alert or measurement to a task that decides whether to stop a machine.
5. **Network device:** a task receiving network messages places them in a queue for a separate task to interpret and record.

## General solution process

1. Decide exactly what information must move between tasks. Make a message structure if several values belong together.
2. Choose the right FreeRTOS tool. Use a queue when you need to transfer and buffer messages; use a semaphore or notification for a signal or event; use a mutex to protect shared data.
3. Decide what should happen if the receiver is slow or a queue becomes full. Possible choices include waiting, dropping a new reading, replacing an old reading, or counting lost readings.
4. Make the producer send messages through the chosen mechanism and make the consumer receive or wait for them.
5. Test using a producer that is faster than the consumer. Confirm whether messages are preserved, dropped, or whether the producer has to wait.

## What the supplied experiment demonstrates

The simulated greenhouse sensor creates one reading every 100 milliseconds, while the history logger handles a reading every 350 milliseconds. In the problem sketch, a one-value shared mailbox stores only the newest reading, so intermediate readings can be overwritten. A mutex keeps the group of reading fields consistent, but it cannot preserve old readings that have already been replaced.

In the solution sketch, a FreeRTOS queue stores up to 20 readings and passes them to the logger in order. When that queue fills, the producer waits until space is available. Watch the sample numbers to see how this differs from the single-value mailbox.

---

# 7. Your observation worksheet

**Complete these sections yourself after running the simulations.** Write what you actually see in Wokwi, not what you expect to see. It is okay if your result is a little different from someone else's because timing can vary.

## Experiment 1 — Symmetric multiprocessing

- **Problem sketch output:**
- **What I observed about the CPU core numbers:**
- **Why I think this happened:**
- **Solution sketch output:**
- **What changed and why:**

## Experiment 2 — Priority-based task scheduling

- **Problem sketch output:**
- **What I observed about the temperature-alarm timing:**
- **Why I think this happened:**
- **Solution sketch output:**
- **What changed and why:**

## Experiment 3 — Shared-resource race condition

- **Problem sketch expected count:**
- **Problem sketch actual count:**
- **What I observed:**
- **Why I think an update was lost or preserved:**
- **Solution sketch actual count:**
- **What changed and why:**

## Experiment 4 — Inter-task communication

- **Problem sketch sample numbers:**
- **What I observed about overwritten readings:**
- **Why I think this happened:**
- **Solution sketch sample numbers:**
- **What changed and why:**
- **What happened when the queue became busy or full:**

---

# 8. Recommended learning resources

1. **Shawn Hymel / Digi-Key — Introduction to RTOS:** [course code, examples, and links to the video lessons](https://github.com/ShawnHymel/introduction-to-rtos). Recommended topics include RTOS basics, task scheduling, queues, mutexes, deadlock, priority inversion, and multicore systems.
2. **Wokwi ESP32 guide:** [run an ESP32 simulation](https://docs.wokwi.com/guides/esp32).
3. **Official FreeRTOS queue guide:** [learn how queues transfer messages and block tasks](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues).
4. **Official FreeRTOS mutex guide:** [learn how a mutex protects a shared resource](https://freertos.org/Real-time-embedded-RTOS-mutexes.html).

---

# 9. Checklist before submitting

- [ ] Ran all four problem sketches in Wokwi.
- [ ] Ran all four solution sketches in Wokwi.
- [ ] Wrote my own observations based on the actual output.
- [ ] Explained each problem and the solution in my own words.
- [ ] Saved screenshots or copied relevant Serial Monitor output if required by the instructor.
- [ ] Confirmed both `problems/` and `solutions/` folders contain the four matching topics.
- [ ] Uploaded the project to GitHub.

## Technical notes

- These are beginner-friendly teaching examples, not production-ready control firmware.
- SMP and race-condition sketches need a dual-core ESP32 target because they use CPU Core 0 and CPU Core 1.
- Serial output and simulator timing can affect exact timing and the actual problem counter value.
- The priority example uses a bounded burst of background work to demonstrate that a poorly chosen priority can delay an urgent task without deliberately running forever without a break.
- The queue example deliberately makes the consumer slower than the producer. Its 20-item queue eventually fills, and the producer then waits for room. In a real system, select the overflow policy according to the sensor or application requirements.
