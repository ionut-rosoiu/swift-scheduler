# Workstealing #
![http://swift-scheduler.googlecode.com/svn/wiki/img/workstealing.png](http://swift-scheduler.googlecode.com/svn/wiki/img/workstealing.png)

In a _work-stealing scheduling_ policy, each of the worker threads involved
maintains a pool of ready tasks to be executed, and executes tasks by removing them from this pool. The process of creating tasks is dynamic, each tasks that gets created is being added to the pool of the creating worker. While trying to get a task from it's own pool the worker discovers the pool is empty, it will pick another worker (the  victim_) and it will remove a task from it's associated pool, essentially_stealing_a task (thus becoming a_thief_). This type of scheduling is very efficient because the idle worker threads (i.e. the threads that do no useful work) do the load balancing._

# Allocator #
![http://swift-scheduler.googlecode.com/svn/wiki/img/allocator.png](http://swift-scheduler.googlecode.com/svn/wiki/img/allocator.png)

The allocator is based on the _buddy_ allocation scheme. The heap is divided into buckets, each bucket containing chunks of the same size. The buckets are initially empty and chunks are added to the buckets whenever memory is released, in order to reuse it. When a caller asks for a memory block of size _sz_, the allocator will round the requested size to the nearest power of two greater than the size. This is done to minimize the number of buckets needed. It will then look in the corresponding bucket to see if there are recycled chunks of memory of the requested size available to be reused. If so, the allocator will remove a chunk from the corresponding bucket and it will return it to the client. If there are no more chunks available, the allocator will use the heap to retrieve a chunk from it.

# Workqueue #

The workqueue is based on the recent work of Chase and Lev [Dynamic Circular WorkStealing Deque](http://research.sun.com/scalable/pubs/main-10.pdf) that was _"ported"_ to a non-garbage-collected, non-memory-modeled, plain-old C.

![http://swift-scheduler.googlecode.com/svn/wiki/img/workqueue.png](http://swift-scheduler.googlecode.com/svn/wiki/img/workqueue.png)

The workqueue has a contiguous space to hold available tasks, so it has a _bottom_ and a _top_. Three operations can be executed on it:
  * _push()_ which adds a task into the workqueue at the top,
  * _pop()_ which removes a task from the top of the workqueue and
  * _steal()_ which removes a task from the bottom of the workqueue.

Not all operations are available to every thread. The _push()_ and _pop()_ methods can be called only by the owner thread, so only the owner has access to the top of the workqueue. The _steal()_ operation can only be executed by other threads apart the owner thread, so only other threads work with the bottom of the workqueue.

![http://swift-scheduler.googlecode.com/svn/wiki/img/workqueue-internal.png](http://swift-scheduler.googlecode.com/svn/wiki/img/workqueue-internal.png)

There is a distinction between the logical and the physical indices: the logical indices are stored in the top and bottom variables and the physical ones are calculated by applying the modulus operator versus the arrays size. The size of the array itself is kept in the cyclic array object. Local _push()_ and _pop()_ operations are performed by changing the value of _bottom_, while stealing is done by incrementing _top_ by one.