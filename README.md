# Intelligent Transport Network Management System (ITNMS)

A console-based smart-city transport network simulator built for **CS221 – Data Structures & Algorithms** (Fall 2025). ITNMS models vehicles, passengers, routes, stations, ticketing, shortest paths, and traffic operations — with every data structure implemented manually from scratch (no STL, only `<iostream>`).

## Features

- **Graph (Route & Station Management)** — add/delete stations and routes, BFS & DFS traversals, Dijkstra's shortest path, Prim's Minimum Spanning Tree, cycle detection, busiest station/route stats
- **Queue (Passenger Ticketing)** — FIFO linked-list queue for ticket requests, enqueue/dequeue/display
- **Hash Table (Vehicle Database)** — chained hashing for vehicle insert/search/remove
- **Stack (History & Undo)** — tracks recent actions (add vehicle, add passenger, add route, add station) and supports undo
- **BST (Station Directory)** — stores station metadata for fast lookup and in-order display
- **Min-Heap** — used in Dijkstra's algorithm for priority-based vertex selection
- **Sorting & Searching Module** — Bubble, Selection, Insertion, Merge, Quick, and Heap sort; Linear and Binary search, selectable at runtime on user-provided data

## Project Structure

```
itnms-repo/
├── itnms.cpp     # Full source (single-file implementation)
├── README.md     # This file
└── .gitignore
```

All data structures (`TicketQueue`, `HistoryStack`, `VehicleHashTable`, `BST`, `MinHeap`, `Graph`, `SortingModule`) and the driver class `ITNMS` live in `itnms.cpp`.

## Building & Running

Requires a C++ compiler (g++, clang++, or MSVC) with C++11 or later.

```bash
g++ -std=c++11 -O2 -o itnms itnms.cpp
./itnms
```

On Windows (MinGW):

```bash
g++ -std=c++11 -O2 -o itnms.exe itnms.cpp
itnms.exe
```

On startup, the program prompts for a number of stations to initialize (default: 5), then presents a menu-driven interface covering all 20 system operations (add/remove vehicles, manage the passenger queue, add routes, run BFS/DFS, Dijkstra, MST, undo, sorting/searching demos, cycle detection, and traffic analytics).

## Course Context

Built for the CS221 Data Structures & Algorithms semester project, which required manual implementations of arrays, linked lists, stacks, queues, trees, heaps, graphs, hash tables, and classic sorting/searching algorithms within one integrated system.

## License

This project was created for academic purposes as part of a university course.
