# Timers Example

This example creates a thread and a timer.
The timer callback is executed evry 1.5 second and prints a message. The task continues to run and prints a status message every 2 seconds.

## How to build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## How to Run

```bash
cd build # From the root directory of the example
./example-timers
```

## Example Output

```
Task 1 is running
Timer callback
Task 1 is running
Timer callback
Task 1 is running
Timer callback
Timer callback
Task 1 is running
```