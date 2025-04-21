# Operating Systems Simulator

## Overview
The Operating Systems Simulator is a C-based project that simulates process management and memory allocation for three processes (P1, P2, P3) with hardcoded memory ranges. It stores instructions, variables, and Process Control Blocks (PCBs) in a hash table-based memory system, using `uthash` for O(1) average-case access. The simulator supports loading processes, fetching data (instructions, variables, PCBs), and updating variables or PCBs, with a test harness in `main.c` to demonstrate functionality. This README explains the code structure, flow, key operations (fetching/updating), and how to use the system effectively.

### Key Features
- **Process Management**: Creates and tracks processes with states (`NEW`, `READY`, `RUNNING`, `WAITING`, `TERMINATED`).
- **Memory Management**: Stores data as `char*` (instructions, variables) or `PCB*` (PCBs) in a `MemoryWord` hash table.
- **Data Access**: Fetches and updates data using keys (e.g., `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`).
- **Type Safety**: Uses `DataType` enum (`TYPE_STRING`, `TYPE_PCB`) to distinguish data types.
- **Debugging**: Provides `printMemory`, `printPCB`, and `displayMemoryRange` for inspection.

## Project Structure
### Files
- **Header Files**:
  - `memory.h`: Defines `MemoryWord` struct and memory operations.
  - `memory_manager.h`: Defines memory management functions (e.g., `fetchDataByIndex`, `updateDataByIndex`).
  - `process.h`: Defines `Process` struct and process functions.
  - `PCB.h`: Defines `PCB` struct and getters/setters.
- **Source Files**:
  - `memory.c`: Implements memory operations (e.g., `addMemoryData`, `printMemory`).
  - `memory_manager.c`: Implements memory population, fetching, and updating.
  - `process.c`: Implements process creation and display.
  - `PCB.c`: Implements PCB creation, accessors, and printing.
  - `main.c`: Test harness demonstrating memory operations.
- **Assumed Files** (not provided but required):
  - `Queue.h`, `Queue.c`: Queue operations for process management.
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
  ```c
  typedef enum { TYPE_STRING, TYPE_PCB } DataType;
  typedef struct {
      int address;        // Memory address (0 to 59)
      void *data;         // char* for TYPE_STRING, PCB* for TYPE_PCB
      DataType type;      // Type of data
      UT_hash_handle hh;  // uthash handle
  } MemoryWord;
  ```
- **Process** (`process.h`):
  - Fields: `pid`, `state`, `file_path`, `arrival_time`, `ready_time`, `burstTime`, `remainingTime`, `next`.
- **PCB** (`PCB.h`):
  - Fields: `id`, `state`, `priority`, `programCounter`, `memLowerBound`, `memUpperBound`.

## Code Flow
The simulator initializes processes, loads them into memory, and allows fetching/updating data. Here’s the flow based on `main.c`:

1. **Initialization** (`main.c`):
   - Creates a `Queue` (`job_pool`) to hold processes.
   - Creates `Process` structs for P1, P2, P3 with `pid`, `file_path` (`../programs/Program_%d.txt`), `arrival_time`, and `burstTime`.
   - Enqueues processes into `job_pool`.
   - Initializes empty `MemoryWord` and `IndexEntry` hash tables.

2. **Memory Population** (`memory_manager.c`):
   - `populateMemory(job_pool, &memory, &index, current_time)`:
     - Iterates through `job_pool` to find processes in `NEW` state with `arrival_time <= current_time`.
     - For each process (e.g., P1, `pid=1`):
       - Calls `readInstructions` to load instructions and variables from `Program_1.txt`.
       - Calls `populatePCB` to create and store a `PCB`.
       - Sets process state to `READY`.
   - `readInstructions`:
     - Reads lines from the program file (e.g., `"assign a 5"`) into addresses 0–6 (for P1).
     - Stores instructions as `TYPE_STRING` using `addMemoryData`.
     - Creates keys like `P1_Instruction_1` using `addIndexEntry`.
     - Extracts variable names (e.g., `a` from `"assign a 5"`) and stores at addresses 7–8.
     - Creates keys like `P1_Variable_1`.
   - `populatePCB`:
     - Creates a `PCB` with `pid`, `memLowerBound` (e.g., 0), `memUpperBound` (e.g., 14).
     - Stores at address 9 as `TYPE_PCB`.
     - Creates key `P1_PCB`.

3. **Data Access and Updates** (`main.c`, `memory_manager.c`):
   - Fetch data using `fetchDataByIndex(index, memory, key, &type)` to retrieve instructions, variables, or PCBs.
   - Update variables or PCBs using `updateDataByIndex(index, memory, key, new_data, type)`.
   - `main.c` tests demonstrate fetching `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`, updating `P1_PCB` fields, and replacing `P1_PCB`.

4. **Debugging**:
   - `printMemory(memory)` displays all memory contents (instructions, variables, PCBs).
   - `displayMemoryRange(pid)` shows memory ranges for a process or all processes.
   - `printPCB(pcb)` displays PCB details for debugging.

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
- Executes tests in `main.c`, populating memory at times 0 and 4, fetching data, updating PCBs, and printing memory contents.
- Output includes memory ranges, fetched data, and updated PCB states.

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
```c
void* fetchDataByIndex(IndexEntry *index, MemoryWord *memory, const char *key, DataType *type_out);
```
- **Parameters**:
  - `index`: `IndexEntry` hash table mapping keys to addresses.
  - `memory`: `MemoryWord` hash table storing data.
  - `key`: String key (e.g., `P1_Instruction_1`, `P1_Variable_1`, `P1_PCB`).
  - `type_out`: Pointer to `DataType` (`TYPE_STRING` or `TYPE_PCB`) for the returned data.
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

### Example
Fetch an instruction:
```c
DataType type;
void *data = fetchDataByIndex(index, memory, "P1_Instruction_1", &type);
if (data && type == TYPE_STRING) {
    printf("Instruction: %s\n", (char*)data); // E.g., "assign a 5"
}
```

Fetch a PCB:
```c
void *data = fetchDataByIndex(index, memory, "P1_PCB", &type);
if (data && type == TYPE_PCB) {
    struct PCB *pcb = (struct PCB*)data;
    printf("PCB: PID=%d, State=%d\n", getPCBId(pcb), getPCBState(pcb));
}
```

## Updating Data
Use `updateDataByIndex` to update variables or PCBs. Instructions are read-only.

### Function
```c
int updateDataByIndex(IndexEntry *index, MemoryWord *memory, const char *key, void *new_data, DataType type);
```
- **Parameters**:
  - `index`: `IndexEntry` hash table.
  - `memory`: `MemoryWord` hash table.
  - `key`: String key (e.g., `P1_Variable_1`, `P1_PCB`).
  - `new_data`: `char*` for `TYPE_STRING`, `struct PCB*` for `TYPE_PCB`.
  - `type`: `DataType` (`TYPE_STRING` or `TYPE_PCB`).
- **Returns**: `0` on success, `-1` on failure (e.g., invalid key, type mismatch).
- **Errors**: Prints to `stderr` for invalid keys, types, or NULL data.
- **Notes**:
  - Only `P<pid>_Variable_<n>` and `P<pid>_PCB` keys are allowed.
  - `new_data` is duplicated (`strdup` for strings, direct pointer for PCBs).

### Key Formats
- **Variables**: `P<pid>_Variable_<n>` (e.g., `P1_Variable_1` for `a`).
- **PCB**: `P<pid>_PCB` (e.g., `P2_PCB`).

### Example
Update a variable:
```c
if (updateDataByIndex(index, memory, "P1_Variable_1", "5", TYPE_STRING) == 0) {
    printf("Updated variable a to 5\n");
}
```

Update a PCB:
```c
struct PCB *new_pcb = createPCBWithBounds(1, 0, 14);
setPCBState(new_pcb, RUNNING);
setPCBProgramCounter(new_pcb, 2);
if (updateDataByIndex(index, memory, "P1_PCB", new_pcb, TYPE_PCB) == 0) {
    printf("Updated P1_PCB\n");
}
```

## Memory Ranges
Use `getProcessMemoryRange` or `displayMemoryRange` to understand memory allocation.

### Function
```c
MemoryRange getProcessMemoryRange(int pid);
```
- **Parameters**:
  - `pid`: Process ID (1, 2, 3).
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

## Debugging
- **Print Memory**:
  ```c
  printMemory(memory);
  ```
  - Shows all addresses, types (`STRING`, `PCB`), and data (strings or PCB details).
- **Print PCB**:
  ```c
  struct PCB *pcb = fetchDataByIndex(index, memory, "P1_PCB", &type);
  if (pcb && type == TYPE_PCB) {
      printPCB(pcb);
  }
  ```
- **Check Ranges**:
  ```c
  displayMemoryRange(0);
  ```

## Notes for Teammates
- **Key Naming**:
  - Always use exact key formats: `P<pid>_Instruction_<n>`, `P<pid>_Variable_<n>`, `P<pid>_PCB`.
  - Variables are `1` (a) or `2` (b). Instructions depend on program file lines (up to `inst_count`).
- **Data Types**:
  - Check `type_out` from `fetchDataByIndex`:
    - `TYPE_STRING`: Cast to `char*`.
    - `TYPE_PCB`: Cast to `struct PCB*`, use getters (`getPCBId`, `getPCBState`).
  - Use correct `type` in `updateDataByIndex` (`TYPE_STRING` for variables, `TYPE_PCB` for PCBs).
- **PCB Access**:
  - PCBs are stored in memory, not `Process`. Fetch with `P<pid>_PCB`.
  - Use getters/setters for safety (e.g., `setPCBState(pcb, RUNNING)`).
- **Memory Management**:
  - `updateDataByIndex` frees old data automatically.
  - For cleanup, call `freeMemoryWord(memory)` and `freeIndex(index)` (currently commented in `main.c`).
  - Implement `freeProcess` in `process.c` for proper `Queue` cleanup.
- **Program Files**:
  - Ensure `../programs/Program_%d.txt` exist with valid instructions.
  - Test with `printMemory` to verify loading.

## Limitations
- Hardcoded for three processes (P1, P2, P3).
- `main.c` is a test harness, not a full scheduler (no instruction execution).
- Silent failure if program files are missing.
- Memory cleanup is commented out, risking leaks.

## Future Improvements
- Add a scheduler to execute instructions (e.g., parse `"assign a 5"`).
- Enhance error handling for program file failures.
- Support dynamic memory ranges for more processes.

## License
Educational project, uses `uthash` (BSD license).

*Generated on April 22, 2025, by Grok 3, built by xAI.*