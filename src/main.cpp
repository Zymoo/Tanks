#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "uWS/uWS.h"

#define PI 3.14159265

std::mutex mutex;
std::mutex stateMutex;
std::mutex escapeMutex;

struct UserMetaData {
    int port;
    char id;
};
struct State {
    int id;
    double positionX;
    double positionY;
    char myLatestKiller;
    int deaths=0;
    int frags=0;

};
class Queue {


    struct Element {
        char messageType;
        int id;
        int number;
        std::string message;
    };

    std::vector<State> stateHistory;
    std::queue<Element> queue;
public: State players[4];
    std::vector<uWS::WebSocket<uWS::SERVER>*> websockets;
    std::vector<char> freePlayerId = { '0', '1', '2', '3' };
    std::vector<int> ports = { 3001, 3002, 3003, 3004, 3005, 3006,
                               3007, 3008, 3009, 3010, 3011, 3012 };

public:
    Queue() {};
    ~Queue() {};

    void addConnection(uWS::WebSocket<uWS::SERVER>* ws) {
        websockets.push_back(ws);
    }

    Element pop() {
        Element element;
        std::unique_lock<std::mutex> lock(mutex);
        element = queue.front();
        queue.pop();
        lock.unlock();
        return element;
    }

    void push(Element element) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(element);
    }

    void updateState(char playerId, int x, int y) {
        std::lock_guard<std::mutex> lock(stateMutex);
        int id = playerId - '0';
        players[id].positionX = x;
        players[id].positionY = y;
    }

    void incrementDeaths(char playerId, char whoKilledMe) {
        std::lock_guard<std::mutex> lock(stateMutex);
        int id = playerId - '0';
        players[id].deaths++;
        players[id].myLatestKiller=whoKilledMe;

    }
    //                queue.players[whoKilledMe-'0'].frags++;

    void incrementFrags(char playerId) {
        std::lock_guard<std::mutex> lock(stateMutex);
        int id = playerId - '0';
        players[id].frags++;
    }
    State getState(int playerId) { return players[playerId]; }

    void broadcast() {
        // as long as there are messages in the queue sends data to all connected
        // clients (ws). Queue is synchonised and thread safe.

        Element tmp;
        UserMetaData* userMetaData;
        while (1) {
            if (!queue.empty()) {
                tmp = pop();
                for (uWS::WebSocket<uWS::SERVER>* ws : websockets) {
                    userMetaData = (UserMetaData*)ws->getUserData();
                    if (userMetaData->id != tmp.id) {
                        std::string str = tmp.message;
                        const char* astr = str.c_str();
                        ws->send(astr, tmp.message.length(), uWS::OpCode::TEXT);
                    }
                    //                printf("\n\n%s\n\n",tmp.message.c_str());
                }
            }
        }
    }

    void removeWebsocket(uWS::WebSocket<uWS::SERVER>* ws) {
        websockets.erase(std::remove(websockets.begin(), websockets.end(), ws),
                         websockets.end());
    }

    void saveToQueue(char* message, int playerId, int length) {
        Element element;
        char outMessage[30];
        outMessage[0] = message[0];
        outMessage[1] = playerId;
        memcpy(&outMessage[2], message + 1, length);
        element.message = outMessage;
        element.id = playerId;
        push(element);
    }

    char getFreeId() {
        char id = 0;
        std::unique_lock<std::mutex> lock(escapeMutex);
        if (!freePlayerId.empty()) {
            id = freePlayerId.back();
            freePlayerId.pop_back();
        }
        lock.unlock();
        return id;
    }

    int getFreePort() {
        int port = 0;
        std::unique_lock<std::mutex> lock(escapeMutex);
        if (!ports.empty()) {
            port = ports.back();
            ports.pop_back();
        }
        lock.unlock();
        return port;
    }

    void addNewPort(int port) {
        std::unique_lock<std::mutex> lock(escapeMutex);
        ports.push_back(port);
        lock.unlock();
    }
    void addFreeId(char id) {
        std::unique_lock<std::mutex> lock(escapeMutex);
        freePlayerId.push_back(id);
        lock.unlock();
    }
};

void clientMain(int port, Queue& queue, UserMetaData* userMetaData) {
    uWS::Hub h;
    h.onConnection([port, &queue, userMetaData](uWS::WebSocket<uWS::SERVER>* ws,
                                                uWS::HttpRequest req) {
        std::cout << "server thread: connected on port " << port << '\n';
        //
        queue.addConnection(ws);
        ws->setUserData(userMetaData);
    });
    h.onMessage([&queue](uWS::WebSocket<uWS::SERVER>* ws, char* message,
                         size_t length, uWS::OpCode opCode) {
        message[length] = 0;
        UserMetaData* userMetaData = (UserMetaData*)ws->getUserData();
        char playerId = userMetaData->id;
        //        printf("%s\n",message);
        // this swich is a cringy do something about it
        // the messega we are storing in Element is in format
        // "messegatype,plyerID,resofthemessage" in restofthemessage delimiter is
        // ','
        // the message we are reciving from from in in format
        // "messageType,restofthemessage", delimiter is ','
        switch (message[0]) {
            case 'u':
                // position message, x, y on map
                double x, y;
                sscanf(&(message[2]), "%lf,%lf", &x, &y);
                //                printf("messege p x:%lf, y:%lf \n", x, y);
                queue.updateState(playerId, x, y);
                // handle messege
                break;
            case 'm':
                // move message
                double dir, val;
                sscanf(&(message[2]), "%lf,%lf", &dir, &val);
                // handle messege
                break;
            case 't':
                // move head of the tank
                double angle;
                sscanf(&(message[2]), "%lf", &angle);
                // handle messege
                break;
            case 's':
                // shot
                double shotX, shotY, shotDir, b, a, tmpX, tmpY, wynik;
                sscanf(&(message[2]), "%lf,%lf,%lf", &shotX, &shotY, &shotDir);
                a = tan(shotDir*PI / 180.0);
                b = shotY - a * shotX;
                for (int i = 0; i<4; i++) {
                    //TODO: shotDir 2 IFs - y Left or Right side
                    //TODO: STOP on first target.
                    //TODO: sizeof(dlaSpajkiego) +1
                    tmpX = queue.getState(i).positionX;
                    tmpY = queue.getState(i).positionY;
                    //                    printf("%lf,%lf\n", tmpX,tmpY);
                    wynik = a * tmpX + b;

                    if (wynik>tmpY - 100 && wynik<tmpY + 100 && playerId - '0' != i && tmpX>1.0 && tmpY>1.0) {
                        char* dlaSpajkiego = new char[40];
                        sprintf(dlaSpajkiego, "b%d,%lf,%lf,%c", i, tmpX, tmpY, playerId);
                        queue.saveToQueue(dlaSpajkiego, i, 30);
                        delete[] dlaSpajkiego;
                    }
                }

                break;
            case 'd':
                queue.incrementDeaths(playerId,message[1]);
                queue.incrementFrags(message[1]);
                char* dlaSpajkiego = new char[40];
                sprintf(dlaSpajkiego, "k,%d,%d,%d,%d,%d,%d,%d,%d",
                        queue.players[0].frags,queue.players[0].deaths,
                        queue.players[1].frags,queue.players[1].deaths,
                        queue.players[2].frags,queue.players[2].deaths,
                        queue.players[3].frags,queue.players[3].deaths);
                queue.saveToQueue(dlaSpajkiego, 0, 30);
                queue.saveToQueue(dlaSpajkiego, 1, 30);
                queue.saveToQueue(dlaSpajkiego, 2, 30);
                queue.saveToQueue(dlaSpajkiego, 3, 30);
                delete[] dlaSpajkiego;

                break;
        }
        queue.saveToQueue(message, playerId, length);
    });
    h.onDisconnection([&queue](uWS::WebSocket<uWS::SERVER>* ws, int code,
                               char* message, size_t length) {
        UserMetaData* userMetaData = (UserMetaData*)ws->getUserData();
        char playerId = userMetaData->id;
        int port = userMetaData->port;
        queue.addNewPort(port);
        queue.addFreeId(playerId);
        queue.removeWebsocket(ws);
    });
    h.listen(port);
    h.run();
}

int main() {
    // main thread waits on port 3000 for clients, when client connects new thread
    // starts running and handles the client on new port
    std::vector<std::string> welcomeMessage = {
            std::string("hullDir:315,turretDir:315,x:3190,y:2540"),
            std::string("hullDir:45,turretDir:45,x:400,y:2540"),
            std::string("hullDir:135,turretDir:135,x:400,y:430"),
            std::string("hullDir:225,turretDir:225,x:3190,y:430") };
    Queue queue = Queue();
    // all threads in program
    std::vector<std::thread> threadVector;
    // start the broadcast thread that sends messages to all client in game
    threadVector.push_back(std::thread(&Queue::broadcast, &queue));
    uWS::Hub h;

    // the port we are initially wait on clients.
    int port = 3000;
    std::cout << "waiting on port: " << port << '\n';
    h.onConnection([&queue, &welcomeMessage, &threadVector](
            uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
        UserMetaData* userData = new UserMetaData;
        // get one of the free id player and remove it from list
        // if (freePlayerId.empty)
        //   handle if there is already 4 players;
        // else
        userData->id = queue.getFreeId();
        if (userData->id == 0)
            std::cout << "za duzo graczy do somthing about it";

        userData->port = queue.getFreePort();
        if (userData->id == 0)
            std::cout << "nie ma wolnych portow";

        // instead of this shamefull code with char* and magic number create a
        // std::string and just before ws->send() transrofm it to char*;
        // the form of the message is "port,userid,welcomeMessage"
        std::string tmpWelcomeMessage = welcomeMessage.back();
        welcomeMessage.pop_back();
        int messageSize = tmpWelcomeMessage.length();
        char* message = new char[messageSize + 2 + 6 + 1];
        sprintf(message, "%d,", userData->port);
        message[5] = userData->id;
        message[6] = ',';
        std::strcpy(message + 7, tmpWelcomeMessage.c_str());
        message[messageSize + 9] = 0;
        // get welcome message
        ws->send(message, messageSize + 9, uWS::OpCode::TEXT);
        threadVector.push_back(
                std::thread(&clientMain, userData->port, std::ref(queue), userData));
        delete[] message;
    });

    h.onMessage([port, &queue](uWS::WebSocket<uWS::SERVER>* ws, char* message,
                               size_t length, uWS::OpCode opCode) {});

    h.onDisconnection([port, &queue](uWS::WebSocket<uWS::SERVER>* ws, int code,
                                     char* message, size_t length) {
        std::cout << "server: ws disconnected on " << port << '\n';
    });

    h.listen(port);
    h.run();

    return 0;
}
