#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "process.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./simulator <algorithm> <path-to-workload>\n";
        return 1;
    }

    string algorithm = argv[1];
    string filepath = argv[2];
    vector<Process> processes;

    ifstream file(filepath);
    int val;
    int pid = 0;

    // Each row is one process, ending with -1
    while (file >> val) {
        Process p;
        p.id = pid++;
        p.arrival_time = val;
        p.current_burst_index = 0;
        p.priority_queue = 0; // For MLFQ

        // Read CPU and I/O bursts until -1 is found
        while (file >> val && val != -1) {
            p.bursts.push_back(val);
        }
        if (!p.bursts.empty()) {
            p.remaining_time_in_burst = p.bursts[0];
        }
        processes.push_back(p);
    }

    cout << "Loaded " << processes.size() << " processes successfully.\n";
    // TODO: Pass 'processes' to your scheduler function based on 'algorithm'

    return 0;
}