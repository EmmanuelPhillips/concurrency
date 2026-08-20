# Concurrency

## Overview

## Practising C++ concurrency, moving away from DP since DP is mostly interview-specific. Concurrency is genuinely used across most real workplace codebases: servers, background jobs, anything handling multiple things at once

## Race Condition Warmup

### Goal

Witness a genuine race condition on a shared counter, then fix it two different
ways: first with a mutex, then with `std::atomic`, to understand both tools and
when each is appropriate.

### What I Learnt

My first attempt at witnessing the race condition didn't actually work. I passed
the counter into each thread by value, so each thread got its own private copy.
No shared state meant no race was even possible, and the original counter in
`main` never changed at all. To genuinely share state between threads, the
variable needs to be passed by reference, and `std::thread` requires
`std::ref()` explicitly to do this, since it copies arguments by default as a
safety measure against dangling references. Once I witnessed the actual race
(inconsistent, wrong totals across runs), I fixed it with a mutex, but made the
same mistake as with the counter: I declared the mutex locally inside the
function being called by both threads. This meant each thread got its own
separate mutex instance, giving zero actual coordination between them. A mutex
has to be shared between threads to protect shared data, for the same reason as
the counter itself. I learnt that `std::lock_guard` is the RAII wrapper around a
mutex. It locks on construction and unlocks automatically on destruction,
guaranteeing the mutex is released even if an exception is thrown or the
function returns early. This is why raw `mtx.lock()` / `mtx.unlock()` is
considered unsafe practice: a thrown exception between `lock()` and `unlock()`
would leave the mutex permanently locked. I also learnt about lock granularity.
Locking around the entire loop, rather than just each individual increment,
technically works and gives the correct answer, but effectively serialises the
two threads. One runs its whole loop before the other can start, removing most
of the actual concurrency. Finally, I converted the same problem to use
`std::atomic<int>` instead of a mutex. For a simple shared counter, this is the
more idiomatic and lightweight tool. No explicit locking is required at all; the
atomicity guarantee is built into the type itself. This version also let the two
threads genuinely interleave their individual increments, rather than running
sequentially like the coarse-grained mutex version did.

### Challenges

I had two rounds of the same underlying mistake: forgetting to actually share
the resource between threads.

1. The counter was passed by value instead of by reference.
1. The mutex was declared locally instead of at a shared scope. The pattern is
   now clear to me: anything a thread needs to coordinate on must be reachable
   by both threads, not recreated separately inside each one.

### Changes for Next Time

- Before writing any concurrent code, explicitly check every piece of shared
  state and confirm it's genuinely shared (passed by reference or living at a
  scope both threads can reach), rather than accidentally duplicated.
- Attempt the full exercise next: a thread-safe queue using `mutex` and
  `condition_variable`, with a producer thread pushing and a consumer thread
  popping, blocking rather than busy-waiting when empty.

--------------------------------------------------------------------------------

## Thread-Safe Queue

### Goal

Build a queue that multiple threads can safely push to and pop from at the same
time, with a producer thread pushing values in and a consumer thread popping
them out, while the consumer blocks rather than busy-waiting when the queue is
empty.

### What I Learnt

This needed both a mutex, protecting the underlying queue from being touched by
two threads at once, and a `condition_variable`, letting the consumer sleep
efficiently instead of repeatedly checking if the queue is empty in a loop and
wasting CPU. I learnt that `cv.wait()` needs a `std::unique_lock`, not a
`std::lock_guard`, because it has to unlock the mutex while the thread sleeps
and relock it when woken. `lock_guard` doesn't support that. I also learnt about
spurious wakeups. A thread can wake from `wait()` even without a `notify` call,
which is why `wait()` takes a predicate and rechecks it every time it wakes,
rather than trusting that a single wakeup means the condition is actually true.
I initially tried a shared `bool` called `done` to signal when production had
finished, without properly synchronising it and reading and writing it outside
the lock. This was the same category of mistake as my very first unprotected
counter exercise: a plain shared `bool` touched from multiple threads without
protection is a data race, regardless of what it's being used for. I fixed this
by only ever reading or writing `done` while holding the same mutex used for the
queue itself. I also repeated the mutex warmup's over-locking mistake in a new
form. My first working version of `produce()` held the lock for the entire push
loop, so all items got pushed and a single `notify` fired only once everything
was already done. It technically worked, but collapsed into "wait once, drain
everything", rather than genuine interleaving between producer and consumer.
Fixing this meant locking around each individual push separately, and notifying
after releasing the lock rather than while still holding it. This means the
woken thread isn't immediately stuck waiting for the lock I'm still holding.

### Challenges

I needed help getting the boolean stopping condition right and still don't feel
fully confident reasoning through `condition_variable` behaviour unaided. In
particular, I struggle with predicting exactly when a waiting thread wakes
versus stays asleep. This is a genuine gap, not just a syntax gap. I can get the
code working, but I couldn't yet write it from a blank file without referring
back to what I've already built.

### Changes for Next Time

- Attempt a bounded buffer version next: a capped queue where the producer must
  also wait if the queue is full, requiring a second `condition_variable` for
  "not full" alongside the existing one for "not empty".
- Try implementing this same thread-safe queue again from a blank file, with no
  reference to this version, purely to test whether the mechanism has actually
  sunk in.

--------------------------------------------------------------------------------

## Thread-Safe Queue V2

### What I Learnt

This V2 was much easier to understand than my first attempt. I now have a much
better grasp of the roles of the mutex, `unique_lock`, and `condition_variable`,
especially how `unique_lock` allows `cv.wait()` to release the mutex while
sleeping and reacquire it when woken. I changed the producer to lock around each
individual push and added a short `chrono` sleep so the producer and consumer
visibly interleave. The `done` flag is now also protected by the same mutex, and
the consumer correctly waits until either the queue has data or production has
finished. I still don't fully understand the lambda passed to `cv.wait()`,
particularly the `[this]` capture. This is mainly because my understanding of
the `this` keyword is still weak.

### Challenges

This attempt went significantly better. I could understand the overall mechanism
rather than just following the code, but still needed help with the lambda and
`this` keyword.

### Changes for Next Time

- Rewrite the same queue from a blank file without referring to this version.
- Move onto the bounded buffer version, where both the producer and consumer can
  block.
