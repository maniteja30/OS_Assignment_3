#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "process.h"

using namespace std;

// Helper: check if all processes finished all burst cycles
bool all_processes_done(const vector<Process>& processes) {
    for (const auto& p : processes) {
        if (p.current_burst_index < p.bursts.size()) {
            return false;
        }
    }
    return true;
}

// Helper: print tick schedule state
void log_tick_schedule(int tick, int cpu1_pid, int cpu2_pid = -2) {
    cout << "Tick " << tick << ": ";
    if (cpu1_pid != -1) cout << "[CPU 0: P" << cpu1_pid << "] ";
    else cout << "[CPU 0: IDLE] ";

    if (cpu2_pid != -2) {
        if (cpu2_pid != -1) cout << "[CPU 1: P" << cpu2_pid << "] ";
        else cout << "[CPU 1: IDLE] ";
    }
    cout << "\n";
}

// =======================================================
// --- PART I: SINGLE-PROCESSOR ALGORITHMS ---
// =======================================================

int simulate_FIFO(vector<Process>& processes) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue;
    int cpu_active_process = -1;

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                ready_queue.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    ready_queue.push(p.id);
                } else {
                    p.completion_time = current_time;
                }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu_active_process == -1 && !ready_queue.empty()) {
            cpu_active_process = ready_queue.front();
            ready_queue.pop();
        }

        log_tick_schedule(current_time, cpu_active_process);

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;
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
        }
        current_time++;
    }
    return current_time;
}

int simulate_RR(vector<Process>& processes, int time_quantum) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue;
    int cpu_active_process = -1;
    int current_quantum_used = 0;

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                ready_queue.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    ready_queue.push(p.id);
                } else {
                    p.completion_time = current_time;
                }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu_active_process == -1 && !ready_queue.empty()) {
            cpu_active_process = ready_queue.front();
            ready_queue.pop();
            current_quantum_used = 0;
        }

        log_tick_schedule(current_time, cpu_active_process);

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;
            current_quantum_used++;

            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else {
                    p.completion_time = current_time;
                }
                cpu_active_process = -1;
            } else if (current_quantum_used == time_quantum) {
                ready_queue.push(p.id);
                cpu_active_process = -1;
            }
        }
        current_time++;
    }
    return current_time;
}

int simulate_MLFQ(vector<Process>& processes, bool enable_boost) {
    int current_time = 0;
    queue<int> q0, q1, q2;
    vector<int> waiting_queue;
    int cpu_active_process = -1;
    int current_quantum_used = 0;
    int quantum_limit = 2;

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        if (enable_boost && current_time > 0 && current_time % 20 == 0) {
            while (!q1.empty()) { q0.push(q1.front()); q1.pop(); }
            while (!q2.empty()) { q0.push(q2.front()); q2.pop(); }
            if (cpu_active_process != -1) {
                processes[cpu_active_process].priority_queue = 0;
            }
        }

        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                p.priority_queue = 0;
                q0.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    if (p.priority_queue == 0) q0.push(p.id);
                    else if (p.priority_queue == 1) q1.push(p.id);
                    else q2.push(p.id);
                } else {
                    p.completion_time = current_time;
                }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu_active_process == -1) {
            if (!q0.empty()) { cpu_active_process = q0.front(); q0.pop(); }
            else if (!q1.empty()) { cpu_active_process = q1.front(); q1.pop(); }
            else if (!q2.empty()) { cpu_active_process = q2.front(); q2.pop(); }
            current_quantum_used = 0;
        }

        log_tick_schedule(current_time, cpu_active_process);

        if (cpu_active_process != -1) {
            Process& p = processes[cpu_active_process];
            p.remaining_time_in_burst--;
            current_quantum_used++;

            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else {
                    p.completion_time = current_time;
                }
                cpu_active_process = -1;
            } else if (current_quantum_used == quantum_limit) {
                if (p.priority_queue < 2) p.priority_queue++;
                if (p.priority_queue == 1) q1.push(p.id);
                else q2.push(p.id);
                cpu_active_process = -1;
            }
        }
        current_time++;
    }
    return current_time;
}

// =======================================================
// --- PART II: DUAL-PROCESSOR ALGORITHMS ---
// =======================================================

int simulate_FIFO_dual(vector<Process>& processes) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue;
    int cpu1 = -1, cpu2 = -1;

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                ready_queue.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    ready_queue.push(p.id);
                } else {
                    p.completion_time = current_time;
                }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu1 == -1 && !ready_queue.empty()) { cpu1 = ready_queue.front(); ready_queue.pop(); }
        if (cpu2 == -1 && !ready_queue.empty()) { cpu2 = ready_queue.front(); ready_queue.pop(); }

        log_tick_schedule(current_time, cpu1, cpu2);

        if (cpu1 != -1) {
            Process& p = processes[cpu1];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu1 = -1;
            }
        }

        if (cpu2 != -1) {
            Process& p = processes[cpu2];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu2 = -1;
            }
        }
        current_time++;
    }
    return current_time;
}

int simulate_RR_dual(vector<Process>& processes, int time_quantum) {
    int current_time = 0;
    queue<int> ready_queue;
    vector<int> waiting_queue;
    int cpu1 = -1, cpu2 = -1;
    int cpu1_quantum = 0, cpu2_quantum = 0;

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                ready_queue.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    ready_queue.push(p.id);
                } else { p.completion_time = current_time; }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu1 == -1 && !ready_queue.empty()) { cpu1 = ready_queue.front(); ready_queue.pop(); cpu1_quantum = 0; }
        if (cpu2 == -1 && !ready_queue.empty()) { cpu2 = ready_queue.front(); ready_queue.pop(); cpu2_quantum = 0; }

        log_tick_schedule(current_time, cpu1, cpu2);

        if (cpu1 != -1) {
            Process& p = processes[cpu1];
            p.remaining_time_in_burst--; cpu1_quantum++;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu1 = -1;
            } else if (cpu1_quantum == time_quantum) {
                ready_queue.push(p.id); cpu1 = -1;
            }
        }

        if (cpu2 != -1) {
            Process& p = processes[cpu2];
            p.remaining_time_in_burst--; cpu2_quantum++;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu2 = -1;
            } else if (cpu2_quantum == time_quantum) {
                ready_queue.push(p.id); cpu2 = -1;
            }
        }
        current_time++;
    }
    return current_time;
}

int simulate_MLFQ_dual(vector<Process>& processes, bool enable_boost) {
    int current_time = 0;
    queue<int> q0, q1, q2;
    vector<int> waiting_queue;
    int cpu1 = -1, cpu2 = -1;
    int cpu1_quantum = 0, cpu2_quantum = 0;
    int quantum_limit = 2;

    auto fetch_next = [&]() {
        int pid = -1;
        if (!q0.empty()) { pid = q0.front(); q0.pop(); }
        else if (!q1.empty()) { pid = q1.front(); q1.pop(); }
        else if (!q2.empty()) { pid = q2.front(); q2.pop(); }
        return pid;
    };

    cout << "\n--- SCHEDULE TIMELINE ---\n";
    while (!all_processes_done(processes)) {
        if (enable_boost && current_time > 0 && current_time % 20 == 0) {
            while (!q1.empty()) { q0.push(q1.front()); q1.pop(); }
            while (!q2.empty()) { q0.push(q2.front()); q2.pop(); }
            if (cpu1 != -1) processes[cpu1].priority_queue = 0;
            if (cpu2 != -1) processes[cpu2].priority_queue = 0;
        }

        for (auto& p : processes) {
            if (p.arrival_time == current_time && p.current_burst_index == 0) {
                p.priority_queue = 0; q0.push(p.id);
            }
        }

        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            Process& p = processes[*it];
            p.remaining_time_in_burst--;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    if (p.priority_queue == 0) q0.push(p.id);
                    else if (p.priority_queue == 1) q1.push(p.id);
                    else q2.push(p.id);
                } else { p.completion_time = current_time; }
                it = waiting_queue.erase(it);
            } else { ++it; }
        }

        if (cpu1 == -1) { cpu1 = fetch_next(); cpu1_quantum = 0; }
        if (cpu2 == -1) { cpu2 = fetch_next(); cpu2_quantum = 0; }

        log_tick_schedule(current_time, cpu1, cpu2);

        if (cpu1 != -1) {
            Process& p = processes[cpu1];
            p.remaining_time_in_burst--; cpu1_quantum++;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu1 = -1;
            } else if (cpu1_quantum == quantum_limit) {
                if (p.priority_queue < 2) p.priority_queue++;
                if (p.priority_queue == 1) q1.push(p.id);
                else q2.push(p.id);
                cpu1 = -1;
            }
        }

        if (cpu2 != -1) {
            Process& p = processes[cpu2];
            p.remaining_time_in_burst--; cpu2_quantum++;
            if (p.remaining_time_in_burst == 0) {
                p.current_burst_index++;
                if (p.current_burst_index < p.bursts.size()) {
                    p.remaining_time_in_burst = p.bursts[p.current_burst_index];
                    waiting_queue.push_back(p.id);
                } else { p.completion_time = current_time; }
                cpu2 = -1;
            } else if (cpu2_quantum == quantum_limit) {
                if (p.priority_queue < 2) p.priority_queue++;
                if (p.priority_queue == 1) q1.push(p.id);
                else q2.push(p.id);
                cpu2 = -1;
            }
        }
        current_time++;
    }
    return current_time;
}

// =======================================================
// --- METRICS DISPLAY ---
// =======================================================

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