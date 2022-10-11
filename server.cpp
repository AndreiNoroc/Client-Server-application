#include <iostream>
#include <queue>
#include <math.h>
#include <string.h>
#include <climits>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"

using namespace std;

int main(int argc, char *argv[]) {
    // Dezactivam bufferingul la afisare
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Declaram variabilele si arrayurile utilizate
    char buffer[BUFLEN];

    int tcp_socket, tcp_newsocket, udp_socket, portno;
    int n, i, ret, fdmax;
    int val = 1;

    struct sockaddr_in serv_addr, serv_addr_udp, new_serv_addr;
    socklen_t sizeofsockaddr = sizeof(sockaddr);

    vector<Topic> topics;
    vector<Client> clients;

    fd_set read_fds;
    fd_set tmp_fds;

    // Golim multimile de descriptori utilizate
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Verificam daca este rulat corect executabilul
    if (argc < 2) {
        cout << "./server <PORT_DORIT>\n";
        exit(0);
    }

    // Creeam socketii tcp si udp
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        cout << "TCP socket error!\n";
        exit(1);
    }

    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        cout << "UDP socket error!\n";
        exit(1);
    }

    // Retinem portul
    portno = atoi(argv[1]);
    if (portno == 0) {
        cout << "Port number error!\n";
        exit(1);
    }

    // Completam adresa serverului, familia de adrese si portul
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    memset((char *) &serv_addr_udp, 0, sizeof(serv_addr_udp));
    serv_addr_udp.sin_family = AF_INET;
    serv_addr_udp.sin_port = htons(portno);
    serv_addr_udp.sin_addr.s_addr = INADDR_ANY;

    // Asociam adresa cu socketul tcp
    ret = bind(tcp_socket, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
    if (ret < 0) {
        cout << "Bind error!\n";
        exit(1);
    }

    // Ascultati pentru socketul tcp
    ret = listen(tcp_socket, INT_MAX);
    if (ret < 0) {
        cout << "Listen error!\n";
        exit(1);
    }

    // Asociam adresa cu socketul udp
    ret = bind(udp_socket, (struct sockaddr *) &serv_addr_udp, sizeof(struct sockaddr));
    if (ret < 0) {
        cout << "Bind error!\n";
        exit(1);
    }

    // Dezactivam algortimul lui Nagle
    ret = setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    if (ret < 0) {
        cout << "Server: setsockopt error!\n";
        exit(1);
    }

    // Se adauga noii descriptori in multimea read_fds
    FD_SET(tcp_socket, &read_fds);
    FD_SET(udp_socket, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    // Actualizam socket maxim
    fdmax = max(tcp_socket, udp_socket);

    while (1) {
        tmp_fds = read_fds;

        // Asteptam conectarea unui client
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        if (ret < 0) {
            cout << "SERVER: Select error!\n";
            exit(1);
        }

        // Parcugem socketii si verificam 
        // daca acestia apartin multimii tmp_fds
        for (i = 0 ; i <= fdmax ; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == STDIN_FILENO) {
                    // Citim comenzile de la tastatura
                    memset(buffer, 0, BUFLEN);
                    fgets(buffer, BUFLEN, stdin);
                    buffer[strlen(buffer) - 1] = '\0';

                    // Verificam tipul comenzii primite de la tastatura,
                    // inchidem socketii si sterge descriptorii din lista
                    if (strcmp(buffer, "exit") == 0) {
                        // Inchidem severul
                        for (i = 0 ; i <= fdmax ; i++) {
                            if (FD_ISSET(i, &read_fds)) {
                                close(i);
                                FD_CLR(i, &read_fds);
                            }
                        }

                        // Inchide socketii udp si tcp
                        close(tcp_socket);
                        close(udp_socket);

                        FD_ZERO(&read_fds);
                        FD_ZERO(&tmp_fds);

                        return 0;
                    }
                } else if (i == udp_socket) {
                    // Primim mesajele de la clientul UDP
                    memset(buffer, 0, BUFLEN);
                    ret = recvfrom(udp_socket, buffer, BUFLEN, 0, (struct sockaddr *) &serv_addr_udp, &sizeofsockaddr);
                    if (ret < 0) {
                        cout << "Nothing from UDP client";
                        exit(1);
                    }

                    // Extragem mesajul in structura
                    msg_from_udp *udpmsg = (msg_from_udp *)buffer;
                    
                    // Parsam mesajul si formam structura
                    // utilizata pentru a trimite mesajul
                    // la clientii TCP
                    msg_to_tcp tcpmsg;
                    memset(&tcpmsg, 0, sizeof(msg_to_tcp));

                    strcpy(tcpmsg.ip_address, inet_ntoa(serv_addr_udp.sin_addr));
                    tcpmsg.port = ntohs(serv_addr_udp.sin_port);
                    strcpy(tcpmsg.topic, udpmsg->topic);

                    int sign;
                    if (udpmsg->data_type == 0) {
                        strcpy(tcpmsg.data_type, "INT");
                        sign = (udpmsg->payload[0]) ? -1 : 1;
                        sprintf(tcpmsg.payload, "%d", sign * ntohl(*(uint32_t *)(udpmsg->payload + 1)));
                    } else if (udpmsg->data_type == 1) {
                        strcpy(tcpmsg.data_type, "SHORT_REAL");
                        sprintf(tcpmsg.payload, "%.2f", 1.0 * ntohs(*(uint16_t *)(udpmsg->payload)) / 100);
                    } else if (udpmsg->data_type == 2) {
                        strcpy(tcpmsg.data_type, "FLOAT");
                        sign = (udpmsg->payload[0]) ? -1 : 1;
                        sprintf(tcpmsg.payload, "%lf", 1.0 * sign * ntohl(*(uint32_t *)(udpmsg->payload + 1)) / pow(10, udpmsg->payload[5]));
                    } else if (udpmsg->data_type == 3) {
                        strcpy(tcpmsg.data_type, "STRING");
                        memset(tcpmsg.payload, 0, 1501);
                        strcpy(tcpmsg.payload, udpmsg->payload);
                    }

                    // Parcurgem topicurile clientii,
                    // facem verificarile aperente
                    // si trimitem mesajele sau le salvam pentru
                    // a fii trimise atunci cand clientul TCP este activ
                    for (Topic &t : topics) {
                        if (t.name.compare(udpmsg->topic) == 0) {
                            for (Client &c : t.clients) {
                                if (c.connected == true) {
                                    n = send(c.sock, &tcpmsg, BUFLEN, 0);
                                } else if (c.storeandforward == true) {
                                    c.pend_msg.push(tcpmsg);

                                    for (Client &cn : clients) {
                                        if (cn.id.compare(c.id) == 0) {
                                            cn.pend_msg.push(tcpmsg);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (i == tcp_socket) {
                    // Citim de pe socketul i
                    tcp_newsocket = accept(i, (struct sockaddr *) &new_serv_addr, &sizeofsockaddr);
                    if (tcp_newsocket < 0) {
                        cout << "Accept new client error!\n";
                        exit(1);
                    }

                    // Dezactivam algoritmul lui Nagle
                    setsockopt(tcp_newsocket, IPPROTO_TCP, TCP_NODELAY, (char*) &val, sizeof(int));

                    // Adaugam socketul la lista si actualizam fdmax
                    FD_SET(tcp_newsocket, &read_fds);
                    fdmax = max(fdmax, tcp_newsocket);

                    // Primim id ul clientului TCP
                    memset(buffer, 0, BUFLEN);
                    ret = recv(tcp_newsocket, buffer, BUFLEN, 0);
                    if (ret < 0) {
                        cout << "ID client has not been received!\n";
                        exit(1);
                    }

                    // Verificam daca clientul este deja conectat,
                    // daca este inactiv si il reconectam si trimitem mesajele
                    // retinute pentru a le primi daca storeandforward este activ
                    bool found = false;
                    for (Client &c : clients) {
                        if (c.id.compare(buffer) == 0) {
                            if (c.connected == true) {
                                cout << "Client " << buffer << " already connected.\n";
                                
                                close(tcp_newsocket);
                                FD_CLR(tcp_newsocket, &read_fds);
                            } else {
                                cout << "New client " << buffer
                                << " connected from " << inet_ntoa(new_serv_addr.sin_addr)
                                << ":" << ntohs(new_serv_addr.sin_port) << ".\n";

                                for (Topic &t : topics) {
                                    for (Client &ct : t.clients) {
                                        if (ct.id.compare(buffer) == 0) {
                                            ct.connected = true;
                                            ct.sock = tcp_newsocket;

                                            c.connected = true;
                                            c.sock = tcp_newsocket;

                                            while (!c.pend_msg.empty()) {
                                                n = send(c.sock, &c.pend_msg.front(), BUFLEN, 0);
                                                if (n < 0) {
                                                    cout << "Pending message could not be send!";
                                                    break;
                                                }

                                                c.pend_msg.pop(); 
                                                ct.pend_msg.pop();
                                            }

                                            break;
                                        }
                                    }
                                }
                            }

                            found = true;
                            break;
                        }
                    }

                    // Daca nu exista creeam un client nou si il adaugam
                    // in vectorii utilizati si afisam mesajul dorit
                    if (found == false) {
                        Client new_client;
                        new_client.connected = true;
                        new_client.id = buffer;
                        new_client.sock = tcp_newsocket;
                        new_client.storeandforward = false;
                        clients.push_back(new_client);

                        cout << "New client " << buffer
                        << " connected from " << inet_ntoa(new_serv_addr.sin_addr)
                        << ":" << ntohs(new_serv_addr.sin_port) << ".\n";
                    } else {
                        continue;
                    }
                } else {
                    // Primim comenzile date in subscriber la citirea
                    // de la tastatura si efectuam schimbarile necesare
                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, BUFLEN, 0);
                    if (n < 0) {
                        cout << "No command received from TCP client!\n";
                        exit(1);
                    }

                    // Verificam daca numarul de bytes primiti
                    if (n == 0) {
                        // Dezactivam socketul il scoatem din lista de descriptori
                        close(i);
                        
                        FD_CLR(i, &read_fds);
                        
                        // Facem schimbarile necesare clientilor cu socketul respectiv
                        for (Topic &t : topics) {
                            for (Client &c : t.clients) {
                                if (c.connected == true && c.sock == i) {
                                    c.connected = false;
                                    c.sock = -1;
                                }
                            }
                        }

                        for (Client &c : clients) {
                            if (c.sock == i) {
                                c.connected = false;
                                c.sock = -1;
                                cout << "Client " << c.id << " disconnected.\n";
                                break;
                            }
                        }
                    } else {
                        // Verificam tipul comenzii primite
                        if (strlen(buffer) == 4 && strcmp(buffer, "exit") == 0) {
                            // Deconectam clientul
                            for (Client &c : clients) {
                                if (c.sock == i) {

                                    for (Topic &t : topics) {
                                        for (Client &ct : t.clients) {
                                            if (ct.id.compare(c.id) == 0) {
                                                ct.sock = -1;
                                                ct.connected = false;

                                                break;
                                            }
                                        }
                                    }

                                    cout << "Client " << c.id << " disconnected.\n";
                                    c.sock = -1;
                                    c.connected = false;

                                    break;
                                }
                            }
                        } else {
                            // Abonam clientul
                            string type_command(strtok(buffer, " "));

                            if (type_command.compare("subscribe") == 0) {
                                string tp_name(strtok(NULL, " "));
                                string sf(strtok(NULL, "\0"));

                                // Verificam existenta topicului si adaugam clientul
                                // in topicul dorit
                                bool existtopic = false;
                                for (Topic &t : topics) {
                                    if (t.name.compare(tp_name) == 0) {
                                        for (Client &c : clients) {
                                            if (c.sock == i) {
                                                bool isintopic = false;
                                                for (Client &ct : t.clients) {
                                                    if (ct.id.compare(c.id) == 0) {
                                                         isintopic = true;
                                                    }
                                                }

                                                if (isintopic == false) {
                                                    if (sf.compare("1") == 0) {
                                                        c.storeandforward = true;
                                                    } else {
                                                        c.storeandforward = false;
                                                    }

                                                    t.clients.push_back(c);
                                                }
                                                break;
                                            }
                                        }

                                        existtopic = true;
                                        break;
                                    }
                                }

                                // Creeam un nou topic si adaugam clientul in el
                                if (existtopic == false) {
                                    Topic new_topic;
                                    new_topic.name = tp_name;

                                    for (Client &c : clients) {
                                        if (c.sock == i) {
                                            if (sf.compare("1") == 0) {
                                                c.storeandforward = true;
                                            } else {
                                                c.storeandforward = false;
                                            }

                                            new_topic.clients.push_back(c);
                                            break;
                                        }
                                    }

                                    topics.push_back(new_topic);
                                }
                            } else if (type_command.compare("unsubscribe") == 0) {
                                // Dezabonam clientul
                                string tp_name(strtok(NULL, "\0"));

                                // Parcurgem topicurile si eliminam clientul din acestea
                                int j;
                                for (Topic &t : topics) {
                                    if (t.name.compare(tp_name) == 0) {

                                        j = -1;
                                        bool findcl = false;
                                        for (Client &c : t.clients) {
                                            j++;
                                            if (c.sock == i) {
                                                findcl = true;
                                                break;
                                            }
                                        }

                                        if (findcl == true) {
                                            t.clients.erase(t.clients.begin() + j);
                                        }
                                        
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
