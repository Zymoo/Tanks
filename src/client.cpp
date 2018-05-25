#define _CRT_SECURE_NO_WARNINGS
#include "uWS/uWS.h"
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>

class Queue {
	struct State {
		int positionX;
		int positionY;
		// ... other
	};
	std::vector<State> stateHistory;
	int numberClient;
	// send current state to all client
	void broadcast(State currentState) {};

public:
	Queue() {};
	~Queue() {};
	// checks all queue's loops, when current state is ready fire broadcast(), and
	// saves it to stateHistory
	void queueLoop() {
		State currentState;
		bool stateReady = false;
		while (1) {
			// checks queues and get new state
			if (stateReady) {
				broadcast(currentState);
				// broadcast(currentState);
				stateHistory.push_back(currentState);
			}
		}
	};
	void addNewClient(int port) {
		// add new queues
	};
	void handleMessage() {
		// add messeage to queses
	};
};

int getPortNum(char* string) {

	int number = 0;
	int factor = 1;

	for (int i = 0; i < 4; i++) {
		number = number + (int)(string[3 - i] - '0') * factor;
		factor = factor * 10;
	}
	return number;
}

int main() {

	// main thread waits on port 3000 to be accepted by server
	// when accepted changes connection to the port send by the server
	uWS::Hub h;
	int receivedPort = 0;

	h.onConnection([](uWS::WebSocket<uWS::CLIENT> *client, uWS::HttpRequest req)
	{
		std::cout << "Client main: connected with server " << '\n';
	});
	h.onMessage([&h, &receivedPort](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
		std::cout << "Received message from server: " << std::string(message, length) << std::endl;
		receivedPort = getPortNum(message);
		h.getDefaultGroup<uWS::CLIENT>().terminate();
	});
	h.connect("ws://localhost:3000", nullptr);
	h.run();

	h.onConnection([](uWS::WebSocket<uWS::CLIENT> *client, uWS::HttpRequest req)
	{
		std::cout << "Client main: connected with port given by server " << '\n';
	});
	h.connect("ws://localhost:" + std::to_string(receivedPort), nullptr);
	h.run();

	return 0;
}
