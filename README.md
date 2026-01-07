# Chronicle

Chronicle is a high-performance, lock-free logging library for C++ designed for multi-threaded systems.
It allows multiple producer threads to emit log messages concurrently while a dedicated consumer thread asynchronously writes them to disk.

Chronicle is built around a lock-free MPSC ring buffer.
