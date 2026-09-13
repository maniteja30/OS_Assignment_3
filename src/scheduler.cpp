#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include "process.h"

using namespace std;

// Helper to check if all processes are fully complete (all bursts finished)
bool all_processes_done(const vector<Process>& processes) {
    for (const auto& p : processes) {
        if (p.current_burst_index < p.bursts.size()) return false;
    }
    return true;
}

// --- DAY 2: FIFO SCHEDULING ---
void simulate_FIFO(vector<Process>& processes) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue; // Store IDs of processes in I/O
    
    int cpu_active_process = -1; // -1 means CPU is idle

    while (!all_processes_done(processes)) {
        // 1. Check for new arrivals at the current time
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                ready_queue.push(p.id);
            }
        }

        // 2. Process I/O (Waiting Queue)
        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            
            if (p.remaining_time_in_burst == 0) {
                // I/O is done. Move to next CPU burst.
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    ready_queue.push(p.id); // Back to ready queue
                } else {
                    p.completion_time = current_time; 
                }
                it = waiting_queue.erase(it); // Remove from waiting queue
            } else {
                ++it;
            }
        }

        // 3. CPU Execution
        if (cpu_active_process == -1 && !ready_queue.empty()) {
            cpu_active_process = ready_queue.front();
            ready_queue.pop();
        }

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;

            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++; // Move to I/O burst
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else {
                    p.completion_time = current_time; // Process is completely finished
                }
                cpu_active_process = -1; // CPU is free again
            }
        }
        current_time++;
    }
}