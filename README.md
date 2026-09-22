# MPI Lab — C and Open MPI

Hands-on exercises for learning **message-passing parallel programming in C** using **Open MPI**, following the *MPI Programming Tutorial* from the Department of Computer Science and Electronics, Universitas Gadjah Mada.

Each program in this repository is a complete, self-contained exercise that demonstrates one core MPI concept — from basic rank/memory semantics through point-to-point protocols, collective operations, nonblocking communication, and parallel numerical integration.

📄 The full tutorial is included in this repository: **[`Tutorial MPI.pdf`](./Tutorial%20MPI.pdf)**

---

## Table of contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Building the programs](#building-the-programs)
- [Running the programs](#running-the-programs)
- [The exercises](#the-exercises)
  - [1. `hello.c` — ranks and private memory](#1-helloc--ranks-and-private-memory)
  - [2. `request_reply.c` — point-to-point request/reply](#2-request_replyc--point-to-point-requestreply)
  - [3. `ring.c` — deadlock avoidance with Sendrecv](#3-ringc--deadlock-avoidance-with-sendrecv)
  - [4. `broadcast.c` — broadcasting configuration](#4-broadcastc--broadcasting-configuration)
  - [5. `scatter_sum.c` — scatter, gather, and reduce](#5-scatter_sumc--scatter-gather-and-reduce)
  - [6. `allreduce.c` — making results available everywhere](#6-allreducec--making-results-available-everywhere)
  - [7. `nonblocking_ring.c` — nonblocking communication](#7-nonblocking_ringc--nonblocking-communication)
  - [8. `pi_mpi.c` — parallel numerical integration](#8-pi_mpic--parallel-numerical-integration)
- [Measuring performance](#measuring-performance)
- [Troubleshooting](#troubleshooting)
- [Practice tasks](#practice-tasks)

---

## Requirements

- Linux (native install or a VM — e.g. Ubuntu 24.04 LTS in VMware)
- A C compiler and Open MPI development files
- No cluster, GPU, or job scheduler needed — all main exercises run on a single machine

The tutorial suggests a VM with **2 vCPUs, 4 GB RAM, and 25 GB disk** if you are not running Linux natively. For the optional two-machine exercise, two VMs on the same NAT network are required.

## Installation

On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential openmpi-bin libopenmpi-dev
```

Verify the toolchain:

```bash
gcc --version
mpicc --showme:version
mpirun --version
nproc
hostname
```

What each piece provides:

| Package / tool | Purpose |
|---|---|
| `build-essential` | Basic C compilation tools |
| `openmpi-bin` | Open MPI runtime commands (`mpirun`, etc.) |
| `libopenmpi-dev` | MPI headers and development libraries |
| `mpicc` | Compiler wrapper that adds MPI compile/link flags |
| `mpirun` | Launches a job; `-np` sets the process count |

> ⚠️ Run MPI programs as your **normal user** — do not put `sudo` before `mpirun`.

## Building the programs

Every `.c` file compiles to its own executable. The tutorial uses consistent flags:

```bash
mpicc -std=c11 -O2 -Wall -Wextra <source>.c -o <program>
```

Example:

```bash
mpicc -std=c11 -O2 -Wall -Wextra hello.c -o hello
```

Precompiled binaries for aarch64 Linux are also present in this repository, but you should rebuild them for your own platform.

## Running the programs

```bash
mpirun -np <processes> ./<program> [args]
```

Useful options:

```bash
# More processes than available slots (functional testing on a small VM):
mpirun --oversubscribe -np 4 ./hello

# Launch across multiple hosts (optional advanced exercise):
mpirun --hostfile hosts.txt -np 4 ./hello
```

---

---

## The exercises

### 1. `hello.c` — ranks and private memory

**Concept:** execution model — SPMD, ranks, communicators, and separate memory.

Demonstrates that `mpirun -np N` starts N processes running the same program, each with its own rank and its own private copy of every variable. Rank 0 sets `x = 100`; other ranks keep `x = 0`.

```bash
mpicc -std=c11 -O2 -Wall -Wextra hello.c -o hello
mpirun -np 1 ./hello
mpirun -np 2 ./hello
mpirun -np 4 ./hello
```

Sample output (order is **not** guaranteed — independent processes and output forwarding affect line order):

```
rank=2 size=4 host=ubuntu x=0
rank=0 size=4 host=ubuntu x=100
rank=3 size=4 host=ubuntu x=0
rank=1 size=4 host=ubuntu x=0
```

**Key ideas:**
- `MPI_Init` / `MPI_Finalize` bracket all MPI usage.
- `MPI_Comm_rank` and `MPI_Comm_size` write into the addresses you provide.
- A global/static C variable is **not** shared between ranks — values move only via explicit MPI communication.
- Rank 0 has no special status unless your program gives it one; ranks do not imply execution order.

---

### 2. `request_reply.c` — point-to-point request/reply

**Concept:** blocking send/receive semantics and what they actually guarantee.

Rank 0 sends `21` with tag `10`; rank 1 receives it, doubles it, and returns `42` with tag `20`. Requires **exactly 2 processes**.

```bash
mpicc -std=c11 -O2 -Wall -Wextra request_reply.c -o request_reply
mpirun -np 2 ./request_reply
```

Expected output:

```
Rank 0 sent 21 and received 42
```

**What blocking actually guarantees:**
- After `MPI_Send` returns, the sender may reuse its buffer — this does **not** prove the receiver has processed the value.
- A standard send may complete via internal buffering or may wait for a matching receive. Correctness must not depend on buffering.
- After `MPI_Recv` returns, the data is in the receive buffer; until then the process waits at that call.
- Matching requires the correct source, tag, datatype, and a large-enough receive buffer. A datatype mismatch is an error, not a way to select a different message.

> Try changing the request to `35` and predict the reply. Also try making the receive expect tag `11` while the send uses tag `10` — the job will hang; stop it with `Ctrl+C`.

---

### 3. `ring.c` — deadlock avoidance with Sendrecv

**Concept:** circular waiting (deadlock) and how to avoid it.

Every rank sends its rank number to its right neighbor and receives from its left neighbor, using `MPI_Sendrecv` — a single blocking call combining send and receive. With 4 ranks the logical ring is `0 → 1 → 2 → 3 → 0`, so ranks 0, 1, 2, 3 receive values 3, 0, 1, 2 respectively.

```bash
mpicc -std=c11 -O2 -Wall -Wextra ring.c -o ring
mpirun -np 4 ./ring
```

**Why deadlock happens:** if both processes call `MPI_Recv` first, neither reaches its send and both wait forever. Simply reversing the order is *not* a general fix — small messages may appear to work due to buffering, while other sizes or implementations hang. `MPI_Sendrecv` avoids the problem by handling the pairing internally; separate send/receive calls still suit one-way transfers and request–reply protocols.

**Checkpoint:** why is the left neighbor computed as `(rank − 1 + size) mod size`? (It wraps rank 0 around to the last rank.)

---

### 4. `broadcast.c` — broadcasting configuration

**Concept:** collective communication — `MPI_Bcast`.

Rank 0 selects `iterations = 1000`; every rank needs the same value.

```bash
mpicc -std=c11 -O2 -Wall -Wextra broadcast.c -o broadcast
mpirun -np 4 ./broadcast
```

**Collective rules:**
- A collective involves **all** processes in the communicator — every rank must call matching collectives in the same order, and for rooted collectives, agree on the root.
- Only initialization belongs inside the `if (rank == 0)` branch — the `MPI_Bcast` call itself must be on every rank's execution path.

```c
/* Incorrect: other ranks never participate. */
if (rank == 0)
    MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);

/* Correct: */
if (rank == 0)
    value = 25;
MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);
```

---


### 5. `scatter_sum.c` — scatter, gather, and reduce

**Concept:** distributing data and combining results with `MPI_Scatter`, `MPI_Gather`, and `MPI_Reduce`.

Rank 0 owns the array `[1..12]`, scatters equal blocks, each rank sums its block, then rank 0 gathers the partial sums **and** computes the reduced total.

```bash
mpicc -std=c11 -O2 -Wall -Wextra scatter_sum.c -o scatter_sum
mpirun -np 4 ./scatter_sum
mpirun -np 3 ./scatter_sum
```

Expected output for 4 processes:

```
Gathered partial sums: 6 15 24 33
Reduced total: 78
```

> ⚠️ Requires the process count to divide 12 evenly — use **1, 2, 3, 4, 6, or 12** ranks. For arbitrary sizes use `MPI_Scatterv` with per-rank counts and offsets instead of silently dropping leftovers.

**Read the counts carefully:**
- In `MPI_Scatter`, `count` is the number of elements sent **to each rank** (not 12); the root also receives its own block.
- In `MPI_Gather`, each rank sends one partial sum, so the root's receive count is 1 per rank and the receive array needs room for `size` integers.
- Only rank 0 receives the valid global result from `MPI_Reduce`.

**Checkpoint:** replacing `MPI_Gather` with `MPI_Reduce` loses the per-rank partial sums — reduce combines them into one value.

---

### 6. `allreduce.c` — making results available everywhere

**Concept:** `MPI_Allreduce` — combine contributions and deliver the result to **every** rank.

Each rank holds `local = rank + 1.0`; all ranks end up with the same global sum and mean. There is **no root argument**.

```bash
mpicc -std=c11 -O2 -Wall -Wextra allreduce.c -o allreduce
mpirun -np 4 ./allreduce
```

With 4 ranks every rank prints `sum=10.0 mean=2.5`.

> Use `MPI_Allreduce` when every rank needs the combined result (e.g. a global error in an iterative algorithm). If ranks hold different numbers of samples, reduce the global sum **and** the global sample count — averaging local means directly can be wrong.
>
> **Checkpoint:** change `MPI_SUM` to `MPI_MAX` and predict the result before running.

---

### 7. `nonblocking_ring.c` — nonblocking communication

**Concept:** `MPI_Irecv` / `MPI_Isend` / `MPI_Waitall` and correct buffer lifetimes.

The same ring exchange as Exercise 3, but nonblocking: both operations are posted, independent work runs while they are in flight, then `MPI_Waitall` completes them.

```bash
mpicc -std=c11 -O2 -Wall -Wextra nonblocking_ring.c -o nonblocking_ring
mpirun -np 4 ./nonblocking_ring
```

The received values match Exercise 3.

**Buffer rules while an operation is pending:**
- Do **not** modify the send buffer and do **not** read the receive buffer.
- Keep both buffers allocated until completion.
- Independent computation may proceed in between — though useful overlap depends on the implementation and workload.

> ⚠️ Posting a nonblocking send and immediately waiting for it *before* posting a needed receive can reintroduce circular waiting.
>
> **Checkpoint:** assigning a new value to `outgoing` before `MPI_Waitall` is incorrect — the send may still be reading it. Printing `incoming` must wait until `MPI_Waitall` confirms the receive completed.


### 8. `pi_mpi.c` — parallel numerical integration

**Concept:** a complete parallel algorithm — midpoint-rule integration of

```
π = ∫₀¹ 4/(1+x²) dx ≈ (1/N) Σᵢ 4/(1+((i+0.5)/N)²)
```

Each index is one independent function evaluation. Ranks get contiguous index ranges and reduce their partial sums.

```bash
mpicc -std=c11 -O2 -Wall -Wextra pi_mpi.c -o pi_mpi
mpirun -np 4 ./pi_mpi 10000000
mpirun -np 1 ./pi_mpi 10000000
```

Usage: `./pi_mpi [N: 1..1000000000]` — defaults to `N = 10000000`.

Expected result: `pi ≈ 3.141592653589793`. The last digits may vary with process count because floating-point addition is not exactly associative. For `N = 1` the estimate is exactly `3.2` in real arithmetic — useful for checking the partition, not accuracy.

**Load balancing (even when N is not divisible by P):** with `q = ⌊N/P⌋` and `r = N mod P`,

```
rank k handles nₖ = q + 1 if k < r, else q iterations
starting at index sₖ = k·q + min(k, r)
```

This assigns every index exactly once. Example — `N = 10, P = 3`: ranges `[0,4)`, `[4,7)`, `[7,10)`.

**How the timing works:**
- Only rank 0 parses the command line, then broadcasts the chosen value.
- `MPI_Barrier` aligns ranks at the start of the measured phase.
- `MPI_Wtime` returns local wall-clock time; durations use two timestamps from the same rank (clocks on different nodes need not be synchronized).
- The measured interval covers the numerical computation and the first reduction. A second reduction reports the **maximum** local duration — a practical approximation of parallel phase time. It excludes MPI startup, the broadcast, the barrier, the timing reduction, printing, and finalization.

> **Checkpoint:** with `N = 10` and 3 processes, ranks with zero assigned iterations still participate correctly in the reductions — collectives involve all ranks regardless of workload.

---

## Measuring performance

Follow this procedure for reproducible numbers:

1. Record CPU model, physical core count, OS, Open MPI version, and whether Linux is native or virtual (with allocated vCPUs/RAM).
2. Pick a fixed `N` (start at `10⁷`; increase if timings are too short/unstable).
3. Use the same executable and flags for every run; keep the laptop plugged in and background activity minimal.
4. Run one warm-up, then **at least five measured runs** per process count. Compare 1, 2, and (if resources permit) 4 processes.
5. Report the **median** time and the min–max range. Label oversubscribed runs separately.

```bash
lscpu
mpirun --version

# One warm-up and five measured runs with two ranks:
mpirun -np 2 ./pi_mpi 10000000
for trial in 1 2 3 4 5; do
    mpirun -np 2 ./pi_mpi 10000000
done
```

**Speedup and efficiency** (with `T₁` = median time at 1 process, `T_P` at P processes):

```
S_P = T₁ / T_P          E_P = S_P / P
```

Holding `N` fixed while changing `P` is called **strong scaling**. This baseline is the 1-process MPI run, not a separately optimized serial implementation.

| Processes | Median time (s) | Range (s) | Speedup | Efficiency |
|---|---|---|---|---|
| 1 | | | 1.00 | 1.00 |
| 2 | | | | |
| 4 | | | | |

**Why more processes can be slower:** communication, synchronization, CPU competition, memory bandwidth, scheduling, and VM overhead can offset the benefit of dividing work. Two VMs on one laptop still share the same physical hardware. Amdahl's law gives a simplified upper bound `S_P ≤ 1 / (f + (1−f)/P)` for serial fraction `f`, before communication overhead is counted.

> **Checkpoint:** distinguish program *correctness* from *performance improvement* when explaining an observed slowdown.

---


## Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| `mpi.h not found` | Install `libopenmpi-dev`; compile with `mpicc`, not bare `gcc` |
| Undefined MPI references | Link MPI properly — use `mpicc` as the compiler |
| Not enough slots | Use fewer processes or `mpirun --oversubscribe` |
| Refusal to run as root | Run as a normal user; never `sudo mpirun` |
| Program appears stuck | Suspect deadlock (mismatched send/recv order or tags); stop with `Ctrl+C` |
| Invalid rank | Destination/source must be within the communicator's rank range |
| Truncated message | Receive buffer/count too small for the matching message |
| Unexpected output order | Normal for independent ranks — gather data and print at one rank for ordered reporting |
| Remote SSH asks for password | Check public-key install, key permissions (600), and SSH config |
| Remote executable not found | Deploy the file on every node at an identical absolute path |
| SSH works but MPI hangs | Check peer networking, firewall rules, MPI version consistency, extra interfaces |
| More ranks are slower | Check workload size, CPUs, oversubscription, communication cost, VM contention |

---

## Practice tasks

From Section 15 of the tutorial — good extensions once the main exercises pass:

1. **Array request–reply.** Extend Exercise 2 to send five integers to rank 1; rank 1 squares each element and returns the five results. Use a small input such as `[1, 2, 3, 4, 5]` you can verify by hand.
2. **Distributed statistics.** Extend Exercise 5 to compute the global sum **and** maximum of the input. Explain why these are reductions and identify the reduction operators (`MPI_SUM`, `MPI_MAX`).
3. **Uneven work.** For `N = 17, P = 4`, list each rank's start index and iteration count using Exercise 8's partitioning rule. Check for gaps and overlap.
4. **Performance study.** Measure Exercise 8 with at least two values of `N` and the process counts your laptop supports. Explain when additional processes help.
5. **Optional distributed execution.** Run the ring across two VMs and capture output demonstrating both hostnames (see Section 13 of the tutorial for the SSH/hostfile setup).

---

## Repository contents

| File | Description |
|---|---|
| `hello.c` | Ranks, size, hostname, private memory (Exercise 1) |
| `request_reply.c` | Blocking send/receive request–reply (Exercise 2) |
| `ring.c` | Ring exchange with `MPI_Sendrecv` (Exercise 3) |
| `broadcast.c` | `MPI_Bcast` configuration sharing (Exercise 4) |
| `scatter_sum.c` | `MPI_Scatter` / `MPI_Gather` / `MPI_Reduce` (Exercise 5) |
| `allreduce.c` | `MPI_Allreduce` global result (Exercise 6) |
| `nonblocking_ring.c` | `MPI_Irecv` / `MPI_Isend` / `MPI_Waitall` (Exercise 7) |
| `pi_mpi.c` | Parallel π via midpoint rule + timing (Exercise 8) |
| `Tutorial MPI.pdf` | Full tutorial document |
| `README.md` | This file |

---

## References

- *MPI Programming Tutorial — Parallel Programming on Your Own Laptop*, Department of Computer Science and Electronics, Universitas Gadjah Mada (included as `Tutorial MPI.pdf`)
- MPI Forum: MPI standard — <https://www.mpi-forum.org/>
- Open MPI documentation — <https://www.open-mpi.org/doc/>
