simulator: src/main.cpp src/scheduler.cpp
	g++ -std=c++11 -Wall src/main.cpp src/scheduler.cpp -o simulator

clean:
	rm simulator