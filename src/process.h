#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include <cstddef>

struct Process {
    int id;                 
    int arrival_time;       
    std::vector<int> bursts; 
    size_t current_burst_index; 
    
    // Metrics
    int completion_time;
    int turnaround_time;
    
    // Runtime state
    int remaining_time_in_burst; 
    int priority_queue;      
};

#endif