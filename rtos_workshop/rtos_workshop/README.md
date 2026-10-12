Problems Related to RTOS (Real-Time Operating System)
These problems are faced in real-life projects or systems that do several works at the same time or where the works depend on different parameters. Problems might arise when several systems or parts of a system work with the same thing or the same data or the same entry.
4 named problems are:

1) Symmetric Multiprocessing
The condition when the same person is given 2 or more tasks that are continuous, and one work hampers the other work from being done at the same time. Here, in the hardware system, the person is a core in the processor.
An example: the same person or core is given the work of security and customer management in a bank, where both works need continuous attention. For this, two different cores need to be used.

Function useful in this case: xTaskCreatePinnedToCore() // xTaskCreate(), millis()
Never use delay() in your life, as it blocks the core from doing work for that time. Rather, use vTaskDelay(), as it gives other functions the opportunity to work during the delayed time or break time.
General things to remember:
a) Tasks that are independent and continuous need to be considered for this problem.
b) The solution to this problem is using a multicore processor.
c) Need to be careful about functions that work with the same data, as they might corrupt the data or work with wrong data in some cases.

2) Priority-Based Task Scheduling
The situation in a hospital where there are normal patients and suddenly new patients with emergency needs come, or an emergency need arises for already admitted patients. In this case, priority has to be given to the emergency case.
Another example might be the system of diagnosis, where, when a diagnosis is occurring and some serious problem is observed, the system would continue the work and take immediate measures according to the problem observed immediately.
Another example: a robot doing a work is facing an internal error and needs to shut down, but should not still be doing another work.
Example of an alarm, fire, or sleep.

Function useful for the solution of this case: fix the priority of work.
General things for this case:
a) Set priority according to weight and deadline.
b) Make the high-priority works compact, small, and fast, as they are keeping other works in the queue behind them.
c) Need a system to see if the priority work I am intending to work on is or will be solved or handled by a similar work in the queue.

3) Shared Resource Race Condition
Two different cores counting the number of visitors or doing entries in a log may write for the same incident. It may occur that they update it twice or do not update it. The same variable used for the settings of two motors doing different work may cause a problem.

Useful function or method: mutex.
Don't use volatile, as it can't handle the situation.
General solution:
a) Create a protected function that doesn't take much time and also use a FreeRTOS mutex to take the variable and work with it, and pause any other work that can be using the variable. Thus, no other variable will have wrong or non-updated data.
b) For functions that read, then process, then write, do not directly update.

4) Inter-Task Communication
It is necessary in almost every system, where one function depends on a value sent or updated by another function or work. But the problem is that the rate of receiving and updating and the rate of sending might not be the same, thus creating a problem and latency.
This is solved by a queue or buffer, but it creates another problem: a priority command or data might be lost or late to reach due to the buffer. This needs to be resolved.

General solution: buffer or queue.
Things to remember:
a) Keep enough space in the buffer for priority things, and also try to keep a priority-on-time and on-weight system.
b) Check the system by increasing the speed of the processing signal.
