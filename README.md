# Operating Systems Course Project

## Project Title

## Course Information
- **Course Name:** [CSEN 602]
- **Instructor:** [Dr. Catherine M. Elias]

## Team Members
- [Noureldin Salaheldin] ([noureldin.gamaleldin@gmail.com])
- [Salma Tarek Soliman] ([salmaaburahmah@gmail.com])
- [Habiba Mahmoud] ([hrefaee72004@gmail.com])
- [Layla Khaled] ([laila.khaled.04@gmail.com])
- [Lama Abdeldayem] ([lamaadayaem1111@gmail.com])
- [Yasmeen Tarek] ([yasmeen.tarek.elabsawy@gmail.com])
- [Mai Hazem] ([maihazem607@gmail.com])

## Project Overview
The OS Scheduler Simulation is a C-based application that simulates various operating system scheduling algorithms, including First-Come-First-Serve (FCFS), Round Robin (RR), and Multi-Level Feedback Queue (MLFQ). The project provides a graphical user interface (GUI) built with GTK 4, allowing users to visualize the scheduling process, manage processes, and interact with the simulation in real-time. It also includes memory management, process control blocks (PCBs), mutex handling, and a console for logging and user interaction.

## Features

Scheduling Algorithms:

  First-Come-First-Serve (FCFS)
  Round Robin (RR) with configurable quantum
  Multi-Level Feedback Queue (MLFQ) with dynamic priority adjustment

Graphical Interface:
  
  Displays process states, ready queues, and blocked queues
  Visualizes memory allocation for processes
  Interactive controls for stepping through the simulation, running automatically, pausing, and resetting

Memory Management:

  Simulates memory allocation for processes, including PCBs, instructions, and variables
  Tracks memory ranges and usage

Process Management:

  Supports process creation with instruction files
  Manages process states (NEW, READY, RUNNING, BLOCKED, TERMINATED)
  Handles mutexes for resource synchronization (userInput, userOutput, file)

Console:
  Logs simulation events and errors
  Allows command-line interaction (e.g., creating processes)


## Technologies Used
- [C]
- [GTK4]

## Project Structure
The project is organized into several source files, each handling a specific component of the simulation:

main.c: Entry point of the application, initializes GTK, and sets up the main window and components.
controller.c: Manages the simulation controls (step, run, pause, reset) and updates the GUI.
unified_controller.c: Coordinates between the dashboard and simulator views, manages process creation.
dashboard_view.c: Displays the process list and simulation status.
simulator_view.c: Visualizes memory and handles process input.
memory_manager.c: Manages memory allocation, deallocation, and process data storage.
memory.c: Implements the memory hash table for storing data (PCBs, instructions, variables).
index.c: Manages an index table for quick access to memory locations.
process.c: Defines the Process structure and handles process creation and management.
PCB.c: Implements the Process Control Block (PCB) structure and related functions.
Queue.c: Implements a queue data structure for managing ready and blocked queues.
MLFQ.c: Implements the Multi-Level Feedback Queue scheduling algorithm.
RoundRobin.c: Implements the Round Robin scheduling algorithm.
FCFS.c: Implements the First-Come-First-Serve scheduling algorithm.
mutex.c: Manages mutexes for resource synchronization.
parser.c: Parses and executes process instructions.
instruction.c: Defines instruction types and handlers for process execution.
console_view.c, console_controller.c, console_model.c: Handle console logging and user input.
clock_controller.c: Manages the simulation clock.
view.c: Contains GUI-related functions for updating the display.
Header files for each source file are located in the include/ directory.

## Usage

Run the Simulator: After compiling, execute the binary:

./simulator

Interact with the GUI:
Dashboard View: Displays the list of processes, their states, and simulation metrics (e.g., clock cycle, algorithm).
Simulator View: Shows memory allocation and allows process creation by specifying a file path and arrival time.

Controls:
Step: Execute one cycle of the simulation.
Run: Automatically run the simulation at a fixed interval.
Pause: Pause automatic execution.
Reset: Reset the simulation to its initial state, including the process ID counter.

Scheduler Selection: Choose between MLFQ, FCFS, or Round Robin (with quantum input for RR).

Console: View logs and enter commands (e.g., create <file_path> <arrival_time>).

Create a Process:
In the simulator view, enter the path to a program file (e.g., ../programs/Program_1.txt) and an arrival time.
Click "Create Process" to add the process to the job pool.
Example program files should contain instructions like:
print Hello
assign x 10
semWait userInput

Monitor the Simulation:
Watch the process states update in the dashboard.
Observe memory allocation in the simulator view.
Check the console for detailed logs of process execution, scheduling decisions, and errors.

Example

Start the simulator:
./simulator

Create a process:
File Path: ../programs/Program_1.txt

Arrival Time: 0

Click "Create Process".

Select a scheduler (e.g., MLFQ) and click "Step" or "Run" to execute the simulation.

Monitor the console output:

[SYSTEM] OS Scheduler Simulation started
[PROCESS] Created process PID 1 with arrival time 0
[EXEC] Process 1 executing instruction at PC=0

Reset the simulation to start over:
Click the "Reset" button to clear all processes and reset the process ID counter.
