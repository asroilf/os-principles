This program solves the readers-writers problem using pthread mutexes and condition variables. It gives priority to the writer, so when the writer wants to update the files, new readers must wait until the writer finishes. There are three replicas of the same file, and each reader is assigned to the replica with the fewest current readers to balance the load. The writer gets exclusive access and updates all three replicas at the same time. After every read or write operation, the program writes information to a log file, including reader counts, writer status, and file contents.

## How to run
`gcc -pthread thread.c -o rw`
`./rw`
