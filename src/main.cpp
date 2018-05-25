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

void clientMain(int port, Queue &queue) {

  uWS::Hub h;

  h.onConnection(
    [port, &queue](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
    std::cout << "server thread: connected on port " << port << '\n';
    queue.addNewClient(port);
  });
  h.onMessage([&queue](uWS::WebSocket<uWS::SERVER> *ws, char *message,
    size_t length,
    uWS::OpCode opCode) { queue.handleMessage(); });

  h.listen(port);
  h.run();
}

void copyPortNum(int number, char* string) {

  for (int i = 0; i < 4; i++) {
    string[3 - i] = number % 10 + '0';
    number = number / 10;
  }
}

int main() {

  // main thread waits on port 3000 for clients, when client connects new thread
  // starts running and handles the client on new port

  uWS::Hub h;
  Queue q = Queue();
  int clientId = 0;
  int clientPort[] = { 3001, 3002, 3003, 3004, 3005, 3006,
    3007, 3008, 3009, 3010, 3011, 3012 };
  std::vector<std::thread> threadVector;

  h.onConnection([&q, &clientId, clientPort, &threadVector](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) 
  {
    std::cout << "server main: connected with client " << clientId << '\n';
    char *message = new char[4];
    copyPortNum(clientPort[clientId], message);
    ws->send(message, 4, uWS::OpCode::TEXT);
    threadVector.push_back(std::thread(&clientMain, clientPort[clientId], std::ref(q)));
    clientId++;
  });


  h.listen(3000);
  h.run();
  
  return 0;
}
