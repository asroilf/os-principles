# Memory Manager Simulation

The assignment demonstrates a simple simulation of dynamic memory allocation in C. It models how an operating system might manage a 256 MB memory space using a linked list, and lets you compare four classic allocation strategies side by side.

---

## How it works

Memory starts as one big free block. As processes come and go, it gets allocated into a free segment. When a process terminates, its block is freed and merged back with any adjacent free blocks to prevent fragmentation.

The whole thing is backed by a doubly linked list where each node is either `USED` (owned by a process) or `FREE` (available).

---

## Allocation strategies

**First Fit** — scans from the beginning and takes the first hole that's big enough for use. It is fast but tends to fragment the memory over time.

**Next Fit** — same idea as first fit, but picks up where it left off last time instead of starting from the top. It spreads allocations more evenly.

**Best Fit** — scans the whole list and picks the smallest hole that fits for the job. It leaves less wasted space per allocation, but also can create a lot of tiny unusable fragments.

**Worst Fit** — picks the largest available hole every time. The leftover space is more likely to be big enough to be useful for future requests.

---

## Building and running

```bash
gcc -o memory_manager memory_manager.c
./memory_manager
```

No dependencies, just a C compiler. Tested with gcc.

---

## Output

After each operation the program prints the current state of memory — start address, size, and who owns it:

```
  Allocate 40MB->p1
    Start    Size     Owner
    ---------------------
    0        40       P1
    40       216      [free]
```

All four strategies run the same workload so that you can directly compare where each one places things.

---

## Workload

The default workload in `main()` is hardcoded as an array of `Workload` structs. Each workload is either an allocation or a termination:

```c
Workload workload[] = {
    {1, "P1", 40},   // allocate 40 MB for P1
    {0, "P1",  0},   // terminate P1
};
```
