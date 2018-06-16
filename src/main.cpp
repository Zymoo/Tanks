#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "uWS/uWS.h"
#include <condition_variable>
#include <iostream>
#include <math.h>

#define PI 3.14159265

std::mutex mutex;
std::mutex stateMutex;


struct State {
    int id;
    double positionX;
    double positionY;
};

struct Element {
    char messageType;
    int id;
    int number;
    std::string message;
};

//Element pop(std::queue<Element> &queue) {
//
//    Element element;
//    std::unique_lock<std::mutex> lock(mutex);
//    //   cond.wait(lock);
//    element = queue.front();
//    queue.pop();
//    lock.unlock();
//    return element;
//}
//
//void push(std::queue<Element> &queue, Element element) {
//    std::lock_guard<std::mutex> lock(mutex);
//    queue.push(element);
//    //   cond.notify_one();
//}

struct UserMetaData {
    int port;
    char id;
};

class Queue {
    
    std::vector<State> stateHistory;
    std::queue<Element> queue;
    State players[4];
    
public:
    Queue() {};
    ~Queue() {};
    
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
    
    void updateState(int playerId, int x, int y) {
        std::lock_guard<std::mutex> lock(stateMutex);
        players[playerId].positionX = x;
        players[playerId].positionY = y;
    }
    
    State getState(int playerId) {
        return players[playerId];
    }
    
    bool empty() { return queue.empty(); }
};

void saveToQueue(Element &element, Queue &queue, char* message, int playerId, int length) {
    
    char outMessage[30];
    outMessage[0] = message[0];
    outMessage[1] = playerId;
    memcpy(&outMessage[2], message + 1, length);
    
    element.message = outMessage;
    element.id = playerId;
    queue.push(element);
    
}
void clientMain(int port,
                Queue& queue,
                UserMetaData* userMetaData,
                std::vector<uWS::WebSocket<uWS::SERVER>*>& websockets) {
    uWS::Hub h;
    h.onConnection([port, &queue, userMetaData, &websockets](
                                                             uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
        std::cout << "server thread: connected on port " << port << '\n';
        //
        websockets.push_back(ws);
        ws->setUserData(userMetaData);
    });
    h.onMessage([&queue](uWS::WebSocket<uWS::SERVER>* ws, char* message,
                         size_t length, uWS::OpCode opCode) {
        message[length] = 0;
        std::cout << "server: recived message on " << message << '\n';
        UserMetaData* userMetaData = (UserMetaData*)ws->getUserData();
        char playerId = userMetaData->id;
        // this swich is a cringy do something about it
        // the messega we are storing in Element is in format
        // "messegatype,plyerID,resofthemessage" in restofthemessage delimiter is
        // ','
        // the message we are reciving from from in in format
        // "messageType,restofthemessage", delimiter is ','
        switch (message[0]) {
            case 'p':
                // position message, x, y on map
                double x, y;
                sscanf(&(message[2]), "%lf,%lf", &x, &y);
                printf("messege p x:%lf, y:%lf \n", x, y);
                queue.updateState(playerId, x, y);
                // handle messege
                break;
            case 'm':
                // move message
                double dir, val;
                sscanf(&(message[2]), "%lf,%lf", &dir, &val);
                printf("messege m dir:%lf, val:%lf \n", dir, val);
                // handle messege
                break;
            case 't':
                // move head of the tank
                double angle;
                sscanf(&(message[2]), "%lf", &angle);
                printf("messege t angle:%lf \n", angle);
                // handle messege
                break;
            case 's':
                // shot
                double shotX, shotY, shotDir,b,a,tmpX,tmpY,wynik;
                sscanf(&(message[2]), "%lf,%lf,%lf", &shotX, &shotY, &shotDir);
                printf("messege s shotX:%lf, shotY:%lf, shotDir:%lf \n", shotX, shotY,
                       shotDir);
                                double newX;
                                double newY;

                a = tan(shotDir-180.0);
                b=shotY-a*shotX;
                for(int i =0;i<4;i++){
                    tmpX = queue.getState(i).positionX;
                    tmpY = queue.getState(i).positionY;
                    wynik=a*tmpX+b;
                    if(wynik>tmpY-50 && wynik<tmpY-50){
                        printf("JEBLOO");
                    }
                }
                
                
                
                
                
                
                break;
        }
        // TODO implement the state of the game, after shot we should check if some
        // player is in line
        
        // remove this cringy char* do all on std::string
        // lenght+1 because we add the id of the player which is one addional
        // char
        Element element;
        saveToQueue(element, queue, message, playerId, length);
        
    });
    h.listen(port);
    h.run();
}

// this probably should be inside the queue object
void broadcast(std::vector<uWS::WebSocket<uWS::SERVER>*>& websockets,
               Queue& queue) {
    // as long as there are messages in the queue sends data to all connected
    // clients (ws). Queue is synchonised and thread safe.
    
    while (1) {
        Element tmp;
        UserMetaData* userMetaData;
        if (!queue.empty()) {
            tmp = queue.pop();
            for (uWS::WebSocket<uWS::SERVER>* ws : websockets) {
                userMetaData = (UserMetaData*)ws->getUserData();
                if (userMetaData->id != tmp.id) {
                    std::string str = tmp.message;
                    const char *astr = str.c_str();
                    ws->send(astr, tmp.message.length(), uWS::OpCode::TEXT);
                }
                //                printf("\n\n%s\n\n",tmp.message.c_str());
            }
        }
    }
}

int main() {
    // main thread waits on port 3000 for clients, when client connects new thread
    // starts running and handles the client on new port
    std::vector<std::string> welcomeMessage = {
        std::string("hullDir:315,turretDir:315,x:3190,y:2540"),
        std::string("hullDir:45,turretDir:45,x:400,y:2540"),
        std::string("hullDir:135,turretDir:135,x:400,y:430"),
        std::string("hullDir:225,turretDir:225,x:3190,y:430") };
    std::vector<char> freePlayerId = { '0', '1', '2', '3' };
    Queue queue = Queue();
    // available ports
    std::vector<int> ports = { 3001, 3002, 3003, 3004, 3005, 3006,
        3007, 3008, 3009, 3010, 3011, 3012 };
    // all threads in program
    std::vector<std::thread> threadVector;
    // every ws is stored in this vector, the thread broadcast send messages to ws
    // in this vector
    std::vector<uWS::WebSocket<uWS::SERVER>*> websockets;
    // start the broadcast thread that sends messages to all client in game
    threadVector.push_back(
                           std::thread(broadcast, std::ref(websockets), std::ref(queue)));
    uWS::Hub h;
    
    // the port we are initially wait on clients.
    int port = 3000;
    std::cout << "waiting on port: " << port << '\n';
    h.onConnection(
                   [&ports, &queue, &freePlayerId, &welcomeMessage, &threadVector,
                    &websockets](uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
                       // issue poprawić to bo mnie oczy bola jak na to patrze ale szybko
                       // musialem napisac
                       
                       // napisac co sie dzieje jak client usuwa sie
                       // napisac co jak jest zly port
                       // napisac co jak za duzo clientow wiecej niz 4
                       
                       // nowy client dochodzi musze to wysłać do innych graczy,
                       // o,id,jego_poczatkowa_pozycja.
                       
                       UserMetaData* userData = new UserMetaData;
                       // get one of the free id player and remove it from list
                       // if (freePlayerId.empty)
                       //   handle if there is already 4 players;
                       // else
                       userData->id = freePlayerId.back();
                       freePlayerId.pop_back();
                       
                       // get one of the free ports and remove from availble list
                       // if (ports.empty)
                       //   handle if there is already 4 players;
                       // else
                       // note: we shoult probably talk with front what happen when the port is
                       //       unavailable
                       userData->port = ports.back();
                       ports.pop_back();
                       
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
                       threadVector.push_back(std::thread(&clientMain, userData->port,
                                                          std::ref(queue), userData,
                                                          std::ref(websockets)));
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
