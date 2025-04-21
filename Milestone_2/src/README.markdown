# Operating Systems Simulator

## Overview
The Operating Systems Simulator is a C-based project that simulates process management and memory allocation for three processes (P1, P2, P3) with hardcoded memory ranges. It uses a hash table-based memory system (via `uthash`) for O(1) average-case access to store instructions, variables, and Process Control Blocks (PCBs). The simulator supports loading processes, fetching data (instructions, variables, PCBs), updating variables/PCBs, and managing a ready queue for process scheduling. Global variables (`job_pool`, `memory`, `index_table`, `ready_queue`) simplify function calls. This README explains the code structure, flow, key operations (fetching/updating), and usage, reflecting recent updates to global variables, ready queue integration, and dynamic key formatting for fetching data.

### Key Features
- **Process Management**: Tracks processes with states (`NEW`, `READY`, `RUNNING`, `WAITING`, `TERMINATED`) and enqueues them to a global `ready_queue`.
- **Memory Management**: Stores data as `char*` (instructions, variables) or `PCB*` (PCBs) in a `MemoryWord` hash table.
- **Data Access**: Fetches/updates data using keys (e.g., `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`).
- **Type Safety**: Uses `DataType` enum (`TYPE_STRING`, `TYPE_PCB`) to distinguish data types.
- **Ready Queue**: Processes are added to `ready_queue` with `ready_time` set when they become `READY`.
- **Debugging**: Provides `printMemory`, `printPCB`, and `displayMemoryRange` for inspection.

## Project Structure
### Files
- **Header Files**:
  - `memory.h`: Defines `MemoryWord` struct and memory operations.
  - `memory_manager.h`: Declares memory management functions (e.g., `fetchDataByIndex`, `updateDataByIndex`).
  - `process.h`: Defines `Process` struct and process functions.
  - `PCB.h`: Defines `PCB` struct and getters/setters.
- **Source Files**:
  - `memory.c`: Implements memory operations (e.g., `addMemoryData`, `printMemory`).
  - `memory_manager.c`: Implements memory population, fetching, and updating.
  - `process.c`: Implements process creation and display.
  - `PCB.c`: Implements PCB creation, accessors, and printing.
  - `main.c`: Test harness demonstrating memory operations and ready queue usage.
- **Assumed Files** (not provided but required):
  - `Queue.h`, `Queue.c`: Queue operations for `job_pool` and `ready_queue`.
  - `index.h`, `index.c`: Index operations for key-to-address mapping.
  - `uthash.h`: Hash table library.
- **Program Files**:
  - `../programs/Program_1.txt`, `Program_2.txt`, `Program_3.txt`: Instruction files (e.g., `"assign a 5"`, `"nop"`).

### Memory Layout
The simulator uses hardcoded memory ranges for three processes:
- **P1**: Instructions 0–6, Variables 7–8 (a, b), PCB 9
- **P2**: Instructions 15–21, Variables 22–23 (a, b), PCB 24
- **P3**: Instructions 30–38, Variables 39–40 (a, b), PCB 41

### Key Structures
- **MemoryWord** (`memory.h`):
  - Fields: `address` (0–59), `data` (`char*` for `TYPE_STRING`, `PCB*` for `TYPE_PCB`), `type` (`DataType`), `hh` (uthash handle).
- **Process** (`process.h`):
  - Fields: `pid`, `state`, `file_path`, `arrival_time`, `ready_time`, `burstTime`, `remainingTime`, `next`.
- **PCB** (`PCB.h`):
  - Fields: `id`, `state`, `priority`, `programCounter`, `memLowerBound`, `memUpperBound`.

## Code Flow
The simulator initializes processes, loads them into memory, sets PCB states to `READY`, and enqueues them to `ready_queue`. The flow, based on `main.c`, is:

1. **Initialization** (`main.c`):
   - Initializes globals: `job_pool`, `memory`, `index_table`, `ready_queue` (using `createQueue` for queues).
   - Creates `Process` structs for P1, P2, P3 with `pid`, `file_path` (`../programs/Program_%d.txt`), `arrival_time`, and `burstTime`.
   - Enqueues processes to `job_pool`.
   - Sets `memory` and `index_table` to `NULL` (populated later).

2. **Memory Population** (`memory_manager.c`):
   - `populateMemory(current_time)`:
     - Iterates through `job_pool` to find processes in `NEW` state with `arrival_time <= current_time`.
     - For each process (e.g., P1, `pid=1`):
       - Calls `readInstructions` to load instructions and variables from `Program_1.txt`.
       - Calls `populatePCB` to create and store a `PCB` with state `READY` in `memory`.
       - Fetches the `PCB` to confirm state `READY` (using formatted key, e.g., `P1_PCB`).
       - Sets `ready_time = current_time` and enqueues to `ready_queue`.
       - Sets `Process` state to `READY`.
   - `readInstructions`:
     - Reads lines from program file (e.g., `"assign a 5"`) into addresses 0–6 (for P1).
     - Stores instructions as `TYPE_STRING` using `addMemoryData`.
     - Creates keys like `P1_Instruction_1` using `addIndexEntry`.
     - Extracts variable names (e.g., `a`) and stores at addresses 7–8 with keys like `P1_Variable_1`.
   - `populatePCB`:
     - Creates a `PCB` with `pid`, `memLowerBound` (e.g., 0), `memUpperBound` (e.g., 14).
     - Sets state to `READY` using `setPCBState`.
     - Stores in `memory` at address 9 as `TYPE_PCB` with key `P1_PCB`.

3. **Data Access and Updates** (`main.c`, `memory_manager.c`):
   - Fetches data using `fetchDataByIndex(key, type_out)` for instructions, variables, or PCBs.
   - Updates variables/PCBs using `updateDataByIndex(key, new_data, type)`.
   - `main.c` tests demonstrate fetching `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`, updating `P1_PCB`, and replacing `P1_PCB`.

4. **Debugging**:
   - `printMemory()` displays all memory contents (instructions, variables, PCBs with states).
   - `displayMemoryRange(pid)` shows memory ranges for a process or all processes.
   - `printPCB(pcb)` displays PCB details.

## How to Utilize the Code
### Compilation
Ensure `uthash.h` and all source files are in the project directory, and program files exist in `../programs/`.
```bash
gcc -o simulator main.c process.c Queue.c PCB.c memory.c index.c memory_manager.c -I. -arch arm64
```

### Running
```bash
./simulator
```
- Executes tests in `main.c`, populating memory at times 0 and 4, fetching/updating data, enqueuing to `ready_queue`, and printing memory contents.
- Output includes memory ranges, fetched data, PCB states (should show `READY`), and test results.

### Program Files
- **Location**: `../programs/Program_1.txt`, `Program_2.txt`, `Program_3.txt`.
- **Format**: One instruction per line (e.g., `"assign a 5"`, `"nop"`).
- **Example** (`Program_1.txt`):
  ```
  assign a 5
  assign b 10
  nop
  ```
  - Loads instructions to addresses 0–2, variables `a`, `b` to 7–8.

## Fetching Data
Use `fetchDataByIndex` to retrieve data by key.

### Function
- **Signature**: `void* fetchDataByIndex(const char *key, DataType *type_out);`
- **Parameters**:
  - `key`: String key (e.g., `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`).
  - `type_out`: Pointer to `DataType` (`TYPE_STRING` or `TYPE_PCB`).
- **Returns**: `void*` (cast to `char*` for `TYPE_STRING`, `struct PCB*` for `TYPE_PCB`) or `NULL` on failure.
- **Errors**: Prints to `stderr` if key or data is not found.

### Key Formats
- **Instructions**: `P<pid>_Instruction_<n>`
  - `<pid>`: Process ID (1, 2, 3).
  - `<n>`: Instruction number (1 to `inst_count`, e.g., 1–7 for P1).
  - Example: `P1_Instruction_1` (address 0), `P2_Instruction_3` (address 17).
- **Variables**: `P<pid>_Variable_<n>`
  - `<pid>`: Process ID (1, 2, 3).
  - `<n>`: Variable index (1 or 2, for `a` or `b`).
  - Example: `P1_Variable_1` (address 7, variable `a`), `P3_Variable_2` (address 40, variable `b`).
- **PCB**: `P<pid>_PCB`
  - `<pid>`: Process ID (1, 2, 3).
  - Example: `P1_PCB` (address 9), `P2_PCB` (address 24).

### Dynamic Key Formatting
To fetch data for a specific process or part (e.g., PCB, instruction, variable) when the process ID (`pid`) or other parameters (e.g., instruction number) are stored in variables, you must dynamically format the key using `snprintf`. Hardcoding keys like `P1_PCB` works for a single process but fails when `pid` is a variable (e.g., `curr->pid` in a loop). Using a literal format string like `P%d_PCB` also fails because it’s not a valid key—it’s a template with `%d` unexpanded.

Instead, create a formatted key:
1. Declare a character array to hold the key, e.g., `char formatted[20]` (20 bytes is sufficient for keys like `P1_PCB` or `P1_Instruction_1`).
2. Use `snprintf` to insert the variable (e.g., `pid`) into the key format, e.g., `snprintf(formatted, sizeof(formatted), "P%d_PCB", pid)` produces `P1_PCB` for `pid=1`.
3. Pass the formatted key to `fetchDataByIndex`, e.g., `fetchDataByIndex(formatted, &type)`.

This ensures the correct key (e.g., `P1_PCB`) is used to retrieve the data from `index_table` and `memory`. For example, in `populateMemory`, formatting the PCB key dynamically allows fetching the PCB for any process to confirm its state is `READY`.

**Examples**:
- **Fetch PCB** (e.g., for `pid=1`):
  ```c
  char formatted[20];
  snprintf(formatted, sizeof(formatted), "P%d_PCB", pid);
  void *data = fetchDataByIndex(formatted, &type);
  ```
  - Produces `P1_PCB`, fetches PCB at address 9.
- **Fetch Instruction** (e.g., 3rd instruction of `pid=2`):
  ```c
  char formatted[20];
  snprintf(formatted, sizeof(formatted), "P%d_Instruction_%d", pid, 3);
  void *data = fetchDataByIndex(formatted, &type);
  ```
  - Produces `P2_Instruction_3`, fetches instruction at address 17.
- **Fetch Variable** (e.g., variable `a` of `pid=3`):
  ```c
  char formatted[20];
  snprintf(formatted, sizeof(formatted), "P%d_Variable_%d", pid, 1);
  void *data = fetchDataByIndex(formatted, &type);
  ```
  - Produces `P3_Variable_1`, fetches variable at address 39.

**Note**: Always check `data != NULL` and `type` (e.g., `TYPE_PCB`) before casting, and use `sizeof(formatted)` to prevent buffer overflows.

### Example
Fetch an instruction:
```c
DataType type;
char formatted[20];
snprintf(formatted, sizeof(formatted), "P1_Instruction_1");
void *data = fetchDataByIndex(formatted, &type);
if (data && type == TYPE_STRING) {
    printf("Instruction: %s\n", (char*)data); // E.g., "assign a 5"
}
```

Fetch a PCB dynamically:
```c
int pid = 1; // Example PID
char formatted[20];
snprintf(formatted, sizeof(formatted), "P%d_PCB", pid);
void *data = fetchDataByIndex(formatted, &type);
if (data && type == TYPE_PCB) {
    struct PCB *pcb = (struct PCB*)data;
    printf("PCB: PID=%d, State=%d\n", getPCBId(pcb), getPCBState(pcb)); // State=READY
}
```

## Updating Data
Use `updateDataByIndex` to update variables or PCBs (instructions are read-only).

### Function
- **Signature**: `int updateDataByIndex(const char *key, void *new_data, DataType type);`
- **Parameters**:
  - `key`: String key (e.g., `P1_Variable_1`, `P1_PCB`).
  - `new_data`: `char*` for `TYPE_STRING`, `struct PCB*` for `TYPE_PCB`.
  - `type`: `DataType` (`TYPE_STRING` or `TYPE_PCB`).
- **Returns**: `0` on success, `-1` on failure (e.g., invalid key, type mismatch).
- **Errors**: Prints to `stderr` for invalid keys, types, or NULL data.
- **Notes**: Only `P<pid>_Variable_<n>` and `P<pid>_PCB` keys are allowed; `new_data` is duplicated.

### Example
Update a variable:
```c
char formatted[20];
snprintf(formatted, sizeof(formatted), "P1_Variable_1");
if (updateDataByIndex(formatted, "5", TYPE_STRING) == 0) {
    printf("Updated variable a to 5\n");
}
```

Update a PCB:
```c
char formatted[20];
snprintf(formatted, sizeof(formatted), "P1_PCB");
struct PCB *new_pcb = createPCBWithBounds(1, 0, 14);
setPCBState(new_pcb, RUNNING);
setPCBProgramCounter(new_pcb, 2);
if (updateDataByIndex(formatted, new_pcb, TYPE_PCB) == 0) {
    printf("Updated P1_PCB\n");
}
```

## Memory Ranges
Use `getProcessMemoryRange` or `displayMemoryRange` to understand memory allocation.

### Function
- **Signature**: `MemoryRange getProcessMemoryRange(int pid);`
- **Parameters**: `pid` (1, 2, 3).
- **Returns**: `MemoryRange` struct with `inst_start`, `inst_count`, `var_start`, `var_count`, `pcb_start`, `pcb_count`.

### Example
```c
MemoryRange range = getProcessMemoryRange(1);
printf("P1 Instructions: %d–%d\n", range.inst_start, range.inst_start + range.inst_count - 1);
```

### Display
```c
displayMemoryRange(0); // All processes
displayMemoryRange(1); // P1 only
```

## Ready Queue
- **Purpose**: `ready_queue` holds processes ready to run, populated by `populateMemory`.
- **Usage**:
  - Processes with `state == NEW` and `arrival_time <= current_time` are processed.
  - `populatePCB` sets PCB state to `READY` in `memory`.
  - `populateMemory` sets `ready_time = current_time`, enqueues to `ready_queue`, and sets `Process` state to `READY`.
- **Access**: Iterate `ready_queue->front` to access `Process` structs (e.g., check `pid`).

### Example
Check `ready_queue` contents:
```c
Process *curr = ready_queue->front;
while (curr) {
    printf("Ready PID: %d, Ready Time: %d\n", curr->pid, curr->ready_time);
    curr = curr->next;
}
```

## Debugging
- **Print Memory**:
  ```c
  printMemory();
  ```
  - Shows addresses, types (`STRING`, `PCB`), and data (strings or PCB details, e.g., `State=READY`).
- **Print PCB**:
  ```c
  char formatted[20];
  snprintf(formatted, sizeof(formatted), "P1_PCB");
  DataType type;
  struct PCB *pcb = (struct PCB*)fetchDataByIndex(formatted, &type);
  if (pcb && type == TYPE_PCB) {
      printPCB(pcb); // Shows PID, State=READY, etc.
  }
  ```
- **Check Ranges**:
  ```c
  displayMemoryRange(0);
  ```
- **Verify Ready Queue**:
  - After `populateMemory(0)`, `ready_queue` should contain P1 (ready_time=0).
  - After `populateMemory(4)`, it should include P2 (ready_time=4), P3 (ready_time=4).

## Notes for Teammates
- **Global Variables**:
  - Use `job_pool`, `memory`, `index_table`, `ready_queue` directly; no need to pass as parameters.
  - `index_table` renamed from `index` to avoid conflicts with `<strings.h>`.
- **Key Naming**:
  - Use exact formats: `P<pid>_Instruction_<n>`, `P<pid>_Variable_<n>`, `P<pid>_PCB`.
  - Format keys dynamically for variable `pid` or indices: `snprintf(formatted, sizeof(formatted), "P%d_PCB", pid)`.
- **Data Types**:
  - Check `type_out` from `fetchDataByIndex`: `TYPE_STRING` (cast to `char*`), `TYPE_PCB` (cast to `struct PCB*`).
  - Use correct `type` in `updateDataByIndex` (`TYPE_STRING` for variables, `TYPE_PCB` for PCBs).
- **PCB Access**:
  - PCBs are stored in `memory`, not `Process`. Fetch with `P<pid>_PCB`.
  - Use getters/setters (e.g., `setPCBState(pcb, RUNNING)`).
  - PCB state is set to `READY` in `populatePCB` and confirmed in `populateMemory`.
- **Memory Management**:
  - `updateDataByIndex` frees old data automatically.
  - Cleanup with `freeMemoryWord()`, `freeIndex(index_table)`, `freeQueue(job_pool)`, `freeQueue(ready_queue)` in `main.c`.
  - Implement `freeProcess` in `process.c` for proper queue cleanup.
- **Program Files**:
  - Ensure `../programs/Program_%d.txt` exist with valid instructions.
  - Verify loading with `printMemory`.

## Limitations
- Hardcoded for three processes (P1, P2, P3).
- `main.c` is a test harness, not a full scheduler (no instruction execution).
- Silent failure if program files are missing.
- Redundant `fetchDataByIndex` in `populateMemory` (since `populatePCB` sets `READY`).
- Missing `freeProcess` for queue cleanup, risking leaks.
- Commented-out Test 5 (variable update) in `main.c`.

## Future Improvements
- Remove redundant `fetchDataByIndex` in `populateMemory`.
- Implement `freeProcess` and uncomment cleanup.
- Add error handling for program file failures.
- Develop a scheduler using `ready_queue` (e.g., round-robin).
- Support dynamic memory ranges for more processes.

## License
Educational project, uses `uthash` (BSD license).

*Generated on April 24, 2025, by Grok 3, built by xAI.*