# Threading Example

In this example two threads are created. 
The first thread vTask1 prints a startup message and then status messages every 2 seconds. After 4 iteration the task deletes itself.
The second thread vTask2 prints a startup message and then status messages every 1 second.

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
./example-threading
```

## Example Output

```
Task 1 started
Task 1 is running
Task 2 started
Task 2 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 2 is running
Task 1 is running
Task 2 is running
Task 2 is running
Task 1 stopped
Task 2 is running
Task 2 is running
```