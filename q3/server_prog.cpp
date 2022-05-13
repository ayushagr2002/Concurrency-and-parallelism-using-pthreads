#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>

/////////////////////////////
#include <iostream>
#include <assert.h>
#include <tuple>
#include <bits/stdc++.h>
using namespace std;
/////////////////////////////

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 100
#define PORT_ARG 8001

const int initial_msg_len = 256;

////////////////////////////////////

const LL buff_sz = 1048576;
string dictionary[101];


int N;
#define MAX_SIZE 100
pthread_t *workers;
sem_t queuesem;
queue<int> clientrequests;
pthread_mutex_t dictlock[101];
pthread_mutex_t queuelock;
pthread_mutex_t printlock = PTHREAD_MUTEX_INITIALIZER;
///////////////////////////////////////////////////
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    //debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

///////////////////////////////

void handle_connection(int client_socket_fd)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################

    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;

    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        cout << "Client sent : " << cmd << endl;
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        string msg_to_send_back = "Ack: " + cmd;

        ////////////////////////////////////////
        // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
        // Yes. The client will get the data that was sent before the FIN packet that closes the socket.

        int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
        // debug(sent_to_client);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }
    }

close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    // return NULL;
}

void *worker_func(void *idx)
{
    //int id = *((int*)idx);
    printf("%ld\n",pthread_self());
    int fd;
    while (1)
    {
        //printf("ENtered thread\n");
        sem_wait(&queuesem);
        int sent_to_client;
        string msg_to_send_back;
        char *part1;
        const char *command;
        int reqindex;

        pthread_mutex_lock(&queuelock);
        fd = clientrequests.front();
        clientrequests.pop();
        pthread_mutex_unlock(&queuelock);

        // int client_socket_fd = *((int *)client_socket_fd_ptr);
        //####################################################

        int received_num, sent_num;

        /* read message from client */
        int ret_val = 1;

        string cmd;
        tie(cmd, received_num) = read_string_from_socket(fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        //cout << "Client sent : " << cmd << endl;
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }

        command = cmd.c_str();
        char command2[MAX_SIZE];
        strcpy(command2, command);
        char *saveptr;
        part1 = strtok_r(command2," ", &saveptr);
        reqindex = atoi(part1);
        part1 = strtok_r(NULL," ",&saveptr);

        if (strcmp(part1, "insert") == 0)
        {
            int inserted = 0;
            part1 = strtok_r(NULL, " ", &saveptr);
            int index = atoi(part1);

            pthread_mutex_lock(&dictlock[index]);
            if (dictionary[index].size() == 0)
            {
                part1 = strtok_r(NULL, " ", &saveptr);
                string word(part1);
                dictionary[index] = word;
                inserted = 1;
            }
            pthread_mutex_unlock(&dictlock[index]);
            if (inserted == 1)
            {
                msg_to_send_back = "Insertion Successful";
            }
            else
            {
                msg_to_send_back = "Key Already Exists";
            }
        }
        else if (strcmp(part1, "delete") == 0)
        {
            int deleted = 0;
            part1 = strtok_r(NULL, " ", &saveptr);
            int index = atoi(part1);
            pthread_mutex_lock(&dictlock[index]);
            if (dictionary[index].size() == 0)
                deleted = 0;
            else
            {
                dictionary[index].clear();
                deleted = 1;
            }
            pthread_mutex_unlock(&dictlock[index]);
            if (deleted == 1)
            {
                msg_to_send_back = "Deletion Successful";
            }
            else
            {
                msg_to_send_back = "No such key exists";
            }
        }
        else if (strcmp(part1, "update") == 0)
        {
            int updated = 0;
            string valtosend;
            part1 = strtok_r(NULL, " ", &saveptr);
            int index = atoi(part1);
            pthread_mutex_lock(&dictlock[index]);
            if (dictionary[index].size() == 0)
                updated = 0;
            else
            {
                part1 = strtok_r(NULL, " ", &saveptr);
                string val(part1);
                valtosend = val;
                dictionary[index] = val;
                updated = 1;
            }
            pthread_mutex_unlock(&dictlock[index]);
            if (updated == 1)
            {
                msg_to_send_back = valtosend;
            }
            else
            {
                msg_to_send_back = "Key does not exists";
            }
        }
        else if (strcmp(part1, "concat") == 0)
        {
            int concatenated = 1;
            string concat;
            part1 = strtok_r(NULL, " ", &saveptr);
            int index1 = atoi(part1);
            part1 = strtok_r(NULL, " ", &saveptr);
            int index2 = atoi(part1);
            pthread_mutex_lock(&dictlock[index1]);
            pthread_mutex_lock(&dictlock[index2]);
            if (dictionary[index1].size() == 0)
                concatenated = 0;
            if (dictionary[index2].size() == 0)
                concatenated = 0;
            if(concatenated == 1)
            {
                string val1 = dictionary[index1];
                string val2 = dictionary[index2];
                dictionary[index1] = val1 + val2;
                dictionary[index2] = val2 + val1;
                concatenated = 1;
            }
            pthread_mutex_unlock(&dictlock[index1]);
            pthread_mutex_unlock(&dictlock[index2]);
            if (concatenated == 1)
            {
                msg_to_send_back = dictionary[index2];
            }
            else
            {
                msg_to_send_back = "Concat failed as atleast one of the keys does not exists";
            }
        }
        else if (strcmp(part1, "fetch") == 0)
        {
            int fetched = 0;
            string val;
            part1 = strtok_r(NULL, " ", &saveptr);
            int index = atoi(part1);
            pthread_mutex_lock(&dictlock[index]);
            if (dictionary[index].size() == 0)
                fetched = 0;
            else
            {
                val = dictionary[index];
                fetched = 1;
            }
            pthread_mutex_unlock(&dictlock[index]);
            if (fetched == 1)
            {
                msg_to_send_back = val;
            }
            else
            {
                msg_to_send_back = "Key does not exists";
            }
        }
        else
        {
            msg_to_send_back = "Invalid operation requested";
        }

        pthread_t pthid = pthread_self();
        msg_to_send_back = to_string(reqindex) + ":" + to_string(pthid) + ":" + msg_to_send_back;
        ////////////////////////////////////////
        // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
        // Yes. The client will get the data that was sent before the FIN packet that closes the socket.
        sleep(2);
        sent_to_client = send_string_on_socket(fd, msg_to_send_back);
        //debug(sent_to_client);
        //cout << msg_to_send_back;
        // debug(sent_to_client);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }

        
    }
close_client_socket_ceremony:
    close(fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    return NULL;
}

int main(int argc, char *argv[])
{

    int i, j, k, t, n;

    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact 
    from a client process running on an arbitrary host
    */
    //get welcoming socket
    //get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    //CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    // initialise the semaphore to 0 
    sem_init(&queuesem, 0, 0);
    pthread_mutex_init(&queuelock, NULL);
    N = atoi(argv[1]);
    cout << "Threads spawned: " << N << "\n";
    workers = new pthread_t[N];
    // spawn n threads
    for (int i = 0; i < N; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&workers[i], NULL, worker_func, (void *)idx);
    }
    // clear the dictionary
    for(int i=0;i<101;i++)
    {
        dictionary[i].clear();
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);
    while (1)
    {
        /* accept a new request, create a client_socket_fd */
        /*
        During the three-way handshake, the client process knocks on the welcoming door
of the server process. When the server “hears” the knocking, it creates a new door—
more precisely, a new socket that is dedicated to that particular client. 
        */
        //accept is a blocking call
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        //push the request on the queue
        pthread_mutex_lock(&queuelock);
        clientrequests.push(client_socket_fd);
        pthread_mutex_unlock(&queuelock);
        // increment the semaphore
        sem_post(&queuesem);
    }

    close(wel_socket_fd);
    return 0;
}