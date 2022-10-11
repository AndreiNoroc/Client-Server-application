#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

using namespace std;

int main(int argc, char *argv[]) {
    // Dezactivam bufferingul la afisare
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Declaram variabilele si arrayurile utilizate
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr;
    int sockfd, n, ret;
    int val = 1;

    fd_set read_fds;
    fd_set tmp_fds;

    // Golim multimile de descriptori utilizate
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Verificam daca este rulat corect executabilul
    if (argc < 4) {
        cout << "Usage: ./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n";
        exit(0);
    }

    // Creeam socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Subscriber: Socket Error!\n";
        exit(1);
    }

    // Completam adresa serverului, familia de adrese si portul
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    ret = inet_aton(argv[2], &serv_addr.sin_addr);
    if (ret == 0) {
        cout << "Subscriber: inet_aton Error!\n";
        exit(1);
    }

    // Clientul se connecteaza la sever
    ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        cout << "Subscriber: Connect Error!\n";
        exit(1);
    }

    // Trimitem idul serverului
    ret = send(sockfd, argv[1], strlen(argv[1]), 0);
    if (ret < 0) {
        cout << "Client ID could not be send!\n";
        exit(1);
    }

    // Dezactivam algortimul lui Nagle
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
    if (ret < 0) {
        cout << "Server: setsockopt error!\n";
        exit(1);
    }

    // Se adauga noii descriptori in multimea read_fds
    FD_SET(sockfd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    while (1) {
        tmp_fds = read_fds;

        // Alege un socket pentru interactiune
        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        if (ret < 0) {
            cout << "Subscriber: Select error!\n";
            exit(1);
        }

        // Verificam daca socketul apartine multimii tmp_fds
        if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
            // Citim de la tastatura comenzile
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN, stdin);
            buffer[strlen(buffer) - 1] = '\0';

            char newbuffer[BUFLEN];
            memset(newbuffer, 0, BUFLEN);
            strcpy(newbuffer, buffer);

            string precommand(strtok(buffer, "\0"));

            // Verificam comanda
            if (precommand.size() == 4) {
                if (precommand.compare("exit") == 0) {
                    // Trimitem comanda severului si inchide clientul
                    n = send(sockfd, precommand.c_str(), 
                    strlen(precommand.c_str()), 0);
                    if (n < 0) {
                        cout << "Subscriber: Send error!\n";
                    }

                    close(sockfd);

                    return 0;
                }
            } else {
                // Verificam comanda
                string command(strtok(buffer, " "));

                if (command.compare("subscribe") == 0 || command.compare("unsubscribe") == 0) {
                    // Formam mesajul/comanda si o trimite catre server
                    command.append(" ");
                    if (command.compare("subscribe") == 0) {
                        command.append(strtok(NULL, " "));
                        command.append(" ");
                    }
                    command.append(strtok(NULL, "\0"));

                    n = send(sockfd, command.c_str(), 
                    strlen(command.c_str()), 0);
                    if (n < 0) {
                        cout << "Subscriber: Send error!\n";
                        exit(1);
                    }

                    // Afisam mesajul dorit
                    if (command.front() == 's') {
                        cout << "Subscribed to topic.\n";
                    } else {
                        cout << "Unsubscribed from topic.\n";
                    }
                } else {
                    cout << "Legit commands: subscribe <TOPIC> <SF> or unsubscribe <TOPIC> or exit!\n";
                }
            }
        }
        
        // Verificam daca socketul apartine multimii tmp_fds
        if (FD_ISSET(sockfd, &tmp_fds)) {
            // Primim structura de la server
            memset(buffer, 0, BUFLEN);
            n = recv(sockfd, buffer, sizeof(buffer), 0);
            if (n < 0) {
                cout << "Recv error!\n";
                exit(1);
            }

            // Verificam numarul de bytes primiti
            if (n == 0) {
                cout << "Server has been closed!\n";
                break;
            }

            // Afisam mesajul trimis de la clientul UDP
            msg_to_tcp *tcpmsg = (msg_to_tcp *)buffer;
            cout << tcpmsg->ip_address << ":" << tcpmsg->port 
            << " - " << tcpmsg->topic 
            << " - " << tcpmsg->data_type
            << " - " << tcpmsg->payload << "\n";
        }
    }
    
    // Inchidem socketul
    close(sockfd);

    return 0;
}
