# concurrency

## overview

practising c++ concurrency, moving away from dp since dp is mostly
interview-specific, this is genuinely used across most real workplace codebases
(servers, background jobs, anything handling multiple things at once)

--------------------------------------------------------------------------------

## race condition warmup

### goal

witness a genuine race condition on a shared counter, then fix it two different
ways, first with a mutex, then with std::atomic, to understand both tools and
when each is appropriate

### what i learnt

my first attempt at witnessing the race condition didn't actually work, i passed
the counter into each thread by value, so each thread got its own private copy,
no shared state meant no race was even possible, and the original counter in
main never changed at all. to genuinely share state between threads, the
variable needs to be passed by reference, and std::thread requires std::ref()
explicitly to do this, since it copies arguments by default as a safety measure
against dangling references once i witnessed the actual race (inconsistent,
wrong totals across runs), i fixed it with a mutex, but made the same mistake as
with the counter, i declared the mutex locally inside the function being called
by both threads, meaning each thread got its own separate mutex instance, giving
zero actual coordination between them. a mutex has to be shared between threads
to protect shared data, same reasoning as the counter itself i learnt
std::lock_guard is the raii wrapper around a mutex, it locks on construction and
unlocks automatically on destruction, guaranteeing the mutex is released even if
an exception is thrown or the function returns early, this is why raw
mtx.lock()/mtx.unlock() is considered unsafe practice, a thrown exception
between lock and unlock would leave the mutex permanently locked i also learnt
about lock granularity, locking around the entire loop (rather than just each
individual increment) technically works and gives the correct answer, but
effectively serialises the two threads, one runs its whole loop before the other
can start, removing most of the actual concurrency finally i converted the same
problem to use std::atomic<int> instead of a mutex, for a simple shared counter
this is the more idiomatic and lighter weight tool, no explicit locking required
at all, the atomicity guarantee is built into the type itself. this version also
let the two threads genuinely interleave their individual increments, rather
than running sequentially like the coarse-grained mutex version did

### challenges

i had two rounds of the same underlying mistake, forgetting to actually share
the resource between threads, first with the counter (passed by value instead of
reference), then with the mutex (declared locally instead of at a shared scope).
the pattern is now clear to me: anything a thread needs to coordinate on must be
reachable by both threads, not recreated separately inside each one

### changes for next time

before writing any concurrent code, explicitly check every piece of shared state
and confirm it's genuinely shared (passed by reference or living at a scope both
threads can reach), not accidentally duplicated attempt the full exercise next,
a thread-safe queue using mutex and condition_variable, with a producer thread
pushing and a consumer thread popping, blocking rather than busy-waiting when
empty

--------------------------------------------------------------------------------

## thread-safe queue

### goal

build a queue that multiple threads can safely push to and pop from at the same
time, a producer thread pushing values in, a consumer thread popping them out,
with the consumer blocking (not busy-waiting) when the queue is empty

### what i learnt

this needed both a mutex (protecting the underlying queue from being touched by
two threads at once) and a condition_variable (letting the consumer sleep
efficiently instead of repeatedly checking if the queue is empty in a loop,
wasting cpu) i learnt cv.wait() needs a std::unique_lock, not a std::lock_guard,
because it has to unlock the mutex while the thread sleeps and relock it when
woken, lock_guard doesn't support that. i also learnt about spurious wakeups, a
thread can wake from wait() even without a notify call, which is why wait()
takes a predicate and rechecks it every time it wakes, rather than trusting a
single wakeup means the condition is actually true i initially tried a shared
bool "done" flag to signal when production had finished, without properly
synchronising it, reading and writing it outside the lock. this was the same
category of mistake as my very first unprotected counter exercise, a plain
shared bool touched from multiple threads without protection is a data race
regardless of what it's used for. fixed by only ever reading/writing done while
holding the same mutex used for the queue itself i also repeated the mutex
warmup's over-locking mistake in a new form: my first working version of
produce() held the lock for the entire push loop, so all items got pushed and a
single notify fired only once everything was already done. it technically
worked, but collapsed into "wait once, drain everything", not genuine
interleaving between producer and consumer. fixing this meant locking around
each individual push separately, and notifying after releasing the lock rather
than while still holding it (so the woken thread isn't stuck immediately
blocking on the lock i'm still holding)

### challenges

i needed help getting the boolean stopping condition right and still don't feel
fully confident reasoning through condition_variable behaviour unaided,
particularly predicting exactly when a waiting thread wakes versus stays asleep.
this is a genuine gap, not just a syntax gap, i can get the code working but
couldn't yet write it from a blank file without referring back to what i've
already built

### changes for next time

attempt a bounded buffer version next (capped queue size, producer must also
wait if the queue is full, needing a second condition variable for "not full"
alongside the existing one for "not empty"), this is the natural next step and
forces reasoning about waiting from both directions, not just the consumer side
try implementing this same thread-safe queue again from a blank file, with no
reference to this version, purely to test whether the mechanism has actually
sunk in
