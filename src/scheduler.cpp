#include <iostream>
#include <vector>
#include <queue>
#include "process.h"

using namespace std;

// Helper to check if all processes are fully complete (all bursts finished)
bool all_processes_done(const vector<Process>& processes) {
    for (const auto& p : processes) {
        if (p.current_burst_index < p.bursts.size()) {
            return false;
        }
    }
    return true;
}

// =======================================================
// --- DAY 2, ALGORITHM A: FIFO SCHEDULING ---
// =======================================================
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

// =======================================================
// --- DAY 2, ALGORITHM B: ROUND ROBIN SCHEDULING ---
// =======================================================
void simulate_RR(vector<Process>& processes, int time_quantum) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue; 
    
    int cpu_active_process = -1; 
    int current_quantum_used = 0; // Tracks how long the current process has been on the CPU

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
                it = waiting_queue.erase(it);
            } else {
                ++it;
            }
        }

        // 3. CPU Execution & Preemption
        if (cpu_active_process == -1 && !ready_queue.empty()) {
            cpu_active_process = ready_queue.front();
            ready_queue.pop();
            current_quantum_used = 0; // Reset quantum tracker for the new process
        }

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;
            current_quantum_used++;

            if (p.remaining_time_in_burst == 0) {
                // CPU burst naturally finished before or exactly when quantum expired
                p.current_burst_index++; 
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else {
                    p.completion_time = current_time; // Process is completely finished
                }
                cpu_active_process = -1; // CPU is free
            } 
            else if (current_quantum_used == time_quantum) {
                // Preemption: Burst isn't done, but the time quantum expired!
                ready_queue.push(p.id); // Push back to the end of the line
                cpu_active_process = -1; // Kick it off the CPU
            }
        }
        current_time++;
    }
}

// --- DAY 3: MLFQ SCHEDULING (Single Processor) ---
void simulate_MLFQ(vector<Process>& processes, bool enable_boost) {
    int current_time = 0;
    queue<int> q0, q1, q2; // Q0 is highest priority
    vector<int> waiting_queue;
    
    int cpu_active_process = -1;
    int current_quantum_used = 0;
    int quantum_limit = 2; // All queues have a time quantum of 2

    while (!all_processes_done(processes)) {
        // 1. Priority Boost (Every 20 time units)
        if (enable_boost && current_time > 0 && current_time % 20 == 0) {
            // Move everything from Q1 and Q2 back to Q0
            while (!q1.empty()) { q0.push(q1.front()); q1.pop(); }
            while (!q2.empty()) { q0.push(q2.front()); q2.pop(); }
            if (cpu_active_process != -1) {
                processes[cpu_active_process].priority_queue = 0;
            }
        }

        // 2. Check for new arrivals
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                p.priority_queue = 0;
                q0.push(p.id);
            }
        }

        // 3. Process I/O (Similar to FIFO)
        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    // Maintain current priority queue after I/O
                    if (p.priority_queue == 0) q0.push(p.id);
                    else if (p.priority_queue == 1) q1.push(p.id);
                    else q2.push(p.id);
                } else {
                    p.completion_time = current_time;
                }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        // 4. CPU Execution & Preemption
        if (cpu_active_process == -1) {
            if (!q0.empty()) { cpu_active_process = q0.front(); q0.pop(); }
            else if (!q1.empty()) { cpu_active_process = q1.front(); q1.pop(); }
            else if (!q2.empty()) { cpu_active_process = q2.front(); q2.pop(); }
            current_quantum_used = 0;
        }

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;
            current_quantum_used++;

            // CPU burst finished
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else {
                    p.completion_time = current_time;
                }
                cpu_active_process = -1;
            } 
            // Time Quantum Expired (Downgrade priority)
            else if (current_quantum_used == quantum_limit) {
                if (p.priority_queue < 2) p.priority_queue++;
                
                if (p.priority_queue == 1) q1.push(p.id);
                else q2.push(p.id);
                
                cpu_active_process = -1;
            }
        }
        current_time++;
    }
}

// --- DAY 4: METRICS AND OUTPUT ---
void print_metrics(const vector<Process>& processes, int total_run_time) {
    int total_turnaround = 0;
    int max_turnaround = 0;

    for (const auto& p : processes) {
        int turnaround = p.completion_time - p.arrival_time;
        total_turnaround += turnaround;
        if (turnaround > max_turnaround) {
            max_turnaround = turnaround;
        }
    }

    double avg_turnaround = (double)total_turnaround / processes.size();

    cout << "\n--- SCHEDULING METRICS ---\n";
    cout << "Average Turnaround Time: " << avg_turnaround << "\n";
    cout << "Maximum Turnaround Time: " << max_turnaround << "\n";
    cout << "Total Simulator Run Time (ticks): " << total_run_time << "\n";
    cout << "--------------------------\n";
}