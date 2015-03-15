Multithreading has become an increasingly popular way to implement dynamic, highly asynchronous, concurrent programs. A multithreaded system provides the programmer with means to create, synchronize, and schedule threads.

# Task scheduling #

For efficient execution of a dynamically growing multithreaded computation on aparallel computer, a scheduling algorithm must ensure that enough threads are ac-tive concurrently to keep the processors busy. Simultaneously, it should ensure that the number of concurrently active threads remains within reasonable limits. More-over, the scheduler should also try to maintain related threads on the same processor, if possible, so that communication between them can be minimized.

Two scheduling paradigms have arisen to address the problem of scheduling multithreaded computations: work sharing and work stealing. In work sharing, when-ever a processor generates new threads, the scheduler attempts to migrate some of them to other processors in hopes of distributing the work to underutilized processors. A common implementation of the work sharing paradigm utilises the master-workers pattern. The master has a pool of work, sends pieces of work to the workers, manages the work and the workers. A typical worker gets a piece of work from the master, does the computation and sends the result back.

In work stealing, however, underutilized processors take the initiative: they attempt to steal threads from other processors. Intuitively, the migration of threads occurs less frequently with work stealing than with work sharing, since when all proc-essors have work to do, no threads are migrated by a work-stealing scheduler, but threads are always migrated by a work-sharing scheduler.

# Modelling multithreaded computations #

In writing a parallel application in a multithreaded language, a programmer expresses parallelism by coding instructions in a partial execution order, and the pro-grammer or the compiler partitions these instructions into totally ordered sequences called threads. The programmer need not specify which processors of a parallel com-puter execute which threads nor exactly when each thread should be executed. These scheduling decisions are made automatically by the runtime system’s scheduler, and the program only requires that each instruction of each thread is executed by some processor at a time consistent with the partial order.

A multithreaded language provides programmers with means to create and synchronize multiple computational threads, and the runtime system for such a lan-guage automatically schedules the execution of these threads on the processors of a parallel computer. To execute a multithreaded program efficiently, the runtime sys-tem’s scheduler must keep the processors busy doing work in order to realize parallel speedup, and simultaneously, it must maintain memory usage to within reasonable limits and avoid interprocessor communication to the greatest extent possible.

A multithreaded computation is composed of a set of threads, each of which is a sequential ordering of unit-size instructions. A processor takes one unit of time to execute one instruction. The instructions are connected by dependency edges, which provide a partial ordering on which instructions must execute before which other in-structions. In order to execute a thread, we allocate for it a chunk of memory called an activation frame that the instructions of the thread can use to store the values on which they compute.

An execution schedule for a multithreaded computation determines which processors of a parallel computer execute which instructions at each step. In any given step of an execution schedule, each processor either executes a single instruc-tion or sits idle. An execution schedule depends on the particular multithreaded com-putation and the number of processors. Specifically, if an instruction has a predeces-sor in its thread, then no processor may execute that instruction until after the predecessor has been executed.

During the course of its execution, a thread may create or spawn other threads. Spawning a thread is like a subroutine call, except that the spawning thread can operate concurrently with the spawned thread. We consider spawned threads to be children of the thread that did the spawning, and a thread may spawn as many chil-dren as it desires. In this way, threads are organized into a spawn tree. The spawn tree is the parallel analog of a call tree.

Each spawn edge goes from a specific instruction (the instruction that actu-ally does the spawn operation in the parent thread) to the first instruction of the child thread. An execution schedule must obey this edge in that no processor may execute an instruction in a spawned child thread until after the spawning instruction in the par-ent thread has been executed.

In addition to the continue and spawn edges, a multithreaded computation-may also contain dependency edges. Dependency edges model the data and control dependencies between threads. As an example of a data dependency, consider an in-struction that produces a data value consumed by another instruction. Such a pro-ducer/consumer relationship precludes the consuming instruction from executing until after the producing instruction. A dependency edge from the producing instruction to the consuming instruction enforces this ordering. An execution schedule must obey this edge in that no processor may execute the consuming instruction until after the producing instruction has been executed.

An execution schedule must obey the constraints given by the dependency, spawn, and continue edges of the computation. These edges form a directed graph of instructions, and no processor may execute an instruction until after all of the instruc-tion’s predecessors in this graph have been executed. So that execution schedules ex-ist, this graphmust be acyclic. That is, itmust be a directed acyclic graph, or dag. At any given step of an execution schedule, an instruction is ready if all of its predeces-sors in the dag have been executed. Only ready instructions may be executed.

# Work-sharing policy #

In a work stealing scheduler, each worker thread maintains a local pool of tasks that are ready to execute. Tasks are created dynamically and are added to the pool of the creating worker. A worker executes tasks form its own pool if possible, but if the pool of a worker is empty, the worker steals a task from the local pool of a randomly chosen worker (the victim).

The algorithm is distributed across the processors. Specifically, each proces-sor maintains a ready deque data structure of threads. The ready deque has two ends: a top and a bottom. Threads can be inserted on the bottom and removed from either end. A processor treats its ready deque like a call stack: pushing and popping from the bottom. Threads that are migrated to other processors are removed from the top. In general, a processor obtains work by removing the thread at the bottom of its ready deque. It starts working on the thread and continues executing the thread's instructions until the thread spawns, stalls, dies or enables a stalled thread, in which case it performs according to the following rules:
Spawns: If the thread spawns a child then the parent thread is placed on the bottom of the ready deque and the processor commences work on the child thread.
Stalls: If the thread stalls, its processor checks the ready deque. If the deque-contains any threads, then the processor removes and begins work on the bottommost thread. If the ready deque is empty, however, the processor be-gins work stealing: itsteals the topmost thread from the ready deque of a ran-domly chosen processor and begins work on it.
Dies: If the thread dies then the processor acts as in the case of a stalling.
Enables: If the thread enables a stalled thread the now-ready thread is placed on the bottom of the ready deque of the initial thread's processor. A thread can simultaneously enable a stalled thread and stall or die, in which case we first perform the rule for enabling and then rule for stalling or the rule for dy-ing.

The Work-Stealing Algorithm begins with all ready deques empty. The root thread of the multithreaded computation is placed in the ready deque of one proces-sor, while the other processors start work stealing. When a processor begins work stealing, it operates as follows: The processor becomes a thief and attempts to steal work from a victim processor chosen uniformly at random. The thief queries the ready deque of the victim and, if it is nonempty, the thief removes and begins work on the top thread. If the victim's ready deque is empty, however, the thief tries again picking another victim at random.

Task stealing is efficient since it is the processors that have nothing useful to do that do most of the load balancing.