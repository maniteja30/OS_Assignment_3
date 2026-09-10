#ifndef PROCESS_H
#define PROCESS_H
#include <vector>

struct Process {
    int id;                 // Process ID (0, 1, 2...)
    int arrival_time;       // When the process arrives
    std::vector<int> bursts; // Alternating CPU and I/O bursts
    int current_burst_index; // Tracks which burst we are currently executing

    // Metrics for reporting
    int completion_time;
    int turnaround_time;

    // Runtime state
    int remaining_time_in_burst; 
    int priority_queue;      // Used later for MLFQ (Q0, Q1, Q2)
};
#endif