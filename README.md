# 👻 Ghost-ARM

A lightweight, bare-metal operating system for **ARM926EJ-S** architecture, built from scratch using C and Assembly.
This project implements preemptive multi-tasking, a priority-based scheduler, dynamic memory management, and an interactive shell interface.

## 🚀 Features

### 1. Kernel Core
* **Preemptive Multi-tasking:** Context switching using IRQ (Timer Interrupt).
* **Priority-Based Scheduler:** Round-Robin scheduling with weighted time slices (Quantum).
* **Process Lifecycle Management:** Supports `READY`, `WAIT`, and `DEAD` states.
* **Context Switching:** Full register context saving/restoring (R0-R12, SP, LR, PC, CPSR).

### 2. Memory Management (`mm.c`)
* **Dynamic Allocation:** Custom `malloc` and `free` implementation.
* **Heap Management:** Linked-list based memory block management with First-Fit algorithm.
* **Split & Coalesce:** Efficient memory block splitting logic.

### 3. Interactive Shell (`shell.c`)
* **CLI Interface:** UART-based command line interface.
* **Process Control Commands:**
    * `jobs`: List running tasks with PID, Priority, and State.
    * `kill <pid>`: Terminate a specific process.
    * `stop <pid>`: Suspend (pause) a process.
    * `resume <pid>`: Resume a suspended process.
    * `sleep <sec>`: Put the shell to sleep for N seconds (non-blocking for background tasks).
* **System Info:** `uptime`, `memtest`, `whoami`.
* **Concurrency Handling:** Atomic I/O operations with Critical Section Protection using `disable_irq` / `enable_irq`.

### 4. Hardware Abstraction
* **Target Architecture:** ARMv5TE (ARM926EJ-S) via QEMU VersatilePB.
* **Drivers:**
    * **UART0 (PL011):** Serial communication.
    * **Timer0 (SP804):** System tick generation.
    * **VIC (PL190):** Vector Interrupt Controller management.

---

## 📂 Project Structure

```bash
.
├── ghost.h      # Global headers, structures (TCB), and constants
├── main.c       # Entry point, hardware initialization, and task creation
├── kernel.c     # Scheduler, IRQ handlers, and context switching logic
├── shell.c      # Shell interface and command parsing
├── mm.c         # Memory Manager (malloc/free)
├── lib.c        # String utilities and hardware I/O helpers
├── startup.s    # Startup, IVT, and Assembly interrupt handlers
├── linker.ld    # Linker script for memory layout
└── Makefile     # Build script
```

---

## 🛠 How to Build & Run

### Prerequisites
* `arm-none-eabi-gcc` toolchain
* `qemu-system-arm`

### Build
```bash
make clean && make
# Cleans previous build and compiles ghost-arm.axf
```

### Run
```bash
qemu-system-arm -M versatilepb -m 128M -nographic -kernel ./ghost-arm.axf
```

---

## 💻 Shell Commands Manual

| Command | Description |
| :--- | :--- |
| `help` | Show available commands |
| `jobs` | Show list of running processes (PID, Prio, State) |
| `kill <pid>` | Terminate process with ID `<pid>` |
| `stop <pid>` | Pause (Suspend) process `<pid>` |
| `resume <pid>` | Resume paused process `<pid>` |
| `sleep <sec>` | Sleep shell for `<sec>` seconds |
| `memtest` | Test dynamic memory allocation (malloc/free) |
| `uptime` | Show system uptime in seconds |
| `clear` | Clear the terminal screen |

---

## 📝 Future Roadmap

* [ ] Implement **IPC (Inter-Process Communication)** via Message Queues.
* [ ] Dynamic Task Spawning (`spawn` command).
* [ ] File System support (Simple RAM Disk).
* [ ] User Mode separation (currently runs in SVC mode).