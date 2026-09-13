#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "process.h"

using namespace std;

// Declarations
int simulate_FIFO(vector<Process>& processes);
int simulate_RR(vector<Process>& processes, int time_quantum);
int simulate_MLFQ(vector<Process>& processes, bool enable_boost);
int simulate_FIFO_dual(vector<Process>& processes);
void print_metrics(const vector<Process>& processes, int total_run_time);

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        cerr << "Usage: ./simulator <algorithm> <path-to-workload> [cpus: 1 or 2]\n";
        return 1;
    }

    string algorithm = argv[1];
    string filepath = argv[2];
    int cpus = (argc == 4) ? stoi(argv[3]) : 1;

    vector<Process> processes;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filepath << endl;
        return 1;
    }

    int val;
    int pid = 0;
    while (file >> val) {
        Process p;
        p.id = pid++;
        p.arrival_time = val;
        p.current_burst_index = 0;
        p.priority_queue = 0;

        while (file >> val && val != -1) {
            p.bursts.push_back(val);
        }
        if (!p.bursts.empty()) {
            p.remaining_time_in_burst = p.bursts[0];
        }
        processes.push_back(p);
    }

    cout << "Loaded " << processes.size() << " processes successfully.\n";

    int total_run_time = 0;

    if (cpus == 2) {
        if (algorithm == "FIFO") {
            cout << "Running Dual-CPU FIFO Simulation...\n";
            total_run_time = simulate_FIFO_dual(processes);
        } else {
            cerr << "Algorithm " << algorithm << " with 2 CPUs running standard fallback...\n";
            total_run_time = simulate_FIFO_dual(processes);
        }
    } else {
        if (algorithm == "FIFO") {
            cout << "Running Single-CPU FIFO Simulation...\n";
            total_run_time = simulate_FIFO(processes);
        } else if (algorithm == "RR") {
            cout << "Running Round Robin Simulation (Quantum = 2)...\n";
            total_run_time = simulate_RR(processes, 2);
        } else if (algorithm == "MLFQ") {
            cout << "Running MLFQ Simulation...\n";
            total_run_time = simulate_MLFQ(processes, true);
        }
    }

    print_metrics(processes, total_run_time);
    return 0;
}