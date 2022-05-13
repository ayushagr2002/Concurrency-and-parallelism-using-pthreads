# Concurrency-and-parallelism-using-pthreads
Mimicking real word scenarios using pthreads and exploiting parallelism to achieve tasks without affecting the correctness of program

### Q1
An alternate course allocation system where a student can take trial classes of a course and can withdraw and opt for a different course if he/she does not like the course. Different labs in the college have been asked to provide students who can act as course TA mentors temporarily and take the trial tutorials.
Each student, course and TA is simulated using a thread. Locks are used at appropriate places to avoid race conditions.

### Q2
A simulation where people enter the stadium, buy tickets to a particular zone (stand), watch the match and then exit. 
Each person is simulated using a thread and events like buying a ticker, watching a match etc. happen in parallel.

### Q3
A simulation where multiple clients are making requests to a server which manipulates a single dictionary shared across all clients. 
Concepts of socket programming, pthreads and mutex locks are extensively used to ensure correct and efficient handling of client requests.
