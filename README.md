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
