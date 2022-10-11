#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <string>
#include <vector>
#include <queue>

#define BUFLEN 1552

using namespace std;

// Structura mesajului primit de la UDP
typedef struct msg_from_udp {
    char topic[50];
    char data_type;
    char payload[1500];
} msg_from_udp;

// Structura mesajului care se trimite
// clientului TCP
typedef struct msg_to_tcp {
    char ip_address[17];
    uint16_t port;
    char topic[51];
    char data_type[11];
    char payload[1501];
} msg_to_tcp;

// Structura clientului TCP
typedef struct Client {
    string id;
    int sock;
    bool connected;
    bool storeandforward;
    queue<msg_to_tcp> pend_msg;
} Client;

// Structura topicului
typedef struct Topic{
    string name;
    vector<Client> clients;
} Topic;

#endif
