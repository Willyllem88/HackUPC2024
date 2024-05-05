#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "libpq-fe.h"

#define BACK_LOG_SIZE 5
#define PORT 8090

char *host = "localhost";
char *port = "5432";
char *dataBase = "grafana";
char *user = "postgres";
char *passwd = "hackupc";

//Error handling
int serverfd;


void sigint_handler(int s) {
    printf("Cerrando server\n");
    close(serverfd);
    exit(0);
}

void error_exit(char * msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

//Creates a server to handle TCP/IP conections to the port PORT
//Pre: true
//Post: returns the fd of the socket
int createSocket()
{   
    int option = 1;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (socketfd < 0)
        error_exit("error creating fd");

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(socketfd, (struct sockaddr*)&address, addrlen) < 0)
        error_exit("error binding socket");
    
    if (listen(socketfd, BACK_LOG_SIZE))
        error_exit("error listening socket");

    return socketfd;
}

/*DataBase modifier methods*/
//Closes a given database connection
void closeDBConnection(PGconn *connection, int code) {
  PQfinish(connection);
  exit(code);
}

// If command not performed
void commandErrorHandler(PGconn *conn, PGresult *res) {
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "%s\n", PQresultErrorMessage(res));
    PQclear(res);
    closeDBConnection(conn, 1);
  }
  else printf("Columnas enviadas\n");
}

void clearDataDB()
{
    char * sqlQuery = "DELETE FROM data;";

    PGconn *conn = PQsetdbLogin(host, port, NULL, NULL, dataBase, user, passwd);

    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        closeDBConnection(conn, 1);
    }
    else printf("Connected\n");


    // Begin the transaction
    PGresult *response = PQexec(conn, "BEGIN");
    commandErrorHandler(conn, response);
    PQclear(response);

    //Insert the given data
    printf("Provando reset\n");
    response = PQexec(conn, sqlQuery);
    commandErrorHandler(conn, response);
    PQclear(response);

    // Commit the transaction
    response = PQexec(conn, "COMMIT");
    commandErrorHandler(conn, response);
    PQclear(response);

    PQfinish(conn);
}

//Handles the conection to the server. Parses data and updates DB
//Pre: connectionfd is a valid fd and the given data follows the format: "timpeStamp,degrees,humidity,decibles"
//Post: updates the DB
void insertData2DB(char * data)
{
    char sqlQuery[25] = "INSERT INTO data VALUES(";
    strcat(sqlQuery, data);
    strcat(sqlQuery, ");");
    printf("%s\n", data);
    printf("--> %s\n", sqlQuery);

    PGconn *conn = PQsetdbLogin(host, port, NULL, NULL, dataBase, user, passwd);

    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        closeDBConnection(conn, 1);
    }
    else printf("Connected\n");


    // Begin the transaction
    PGresult *response = PQexec(conn, "BEGIN");
    commandErrorHandler(conn, response);
    PQclear(response);

    //Insert the given data
    printf("Provando envio\n");
    response = PQexec(conn, sqlQuery);
    commandErrorHandler(conn, response);
    PQclear(response);

    // Commit the transaction
    response = PQexec(conn, "COMMIT");
    commandErrorHandler(conn, response);
    PQclear(response);

    closeDBConnection(conn, 0);
}

void handleConection(int connectionfd)
{
    char buff[128];
    recv(connectionfd, buff, sizeof(buff), 0);
    printf("Client : %s\n", buff);

    insertData2DB(buff);
    /*Child dies*/
    close(connectionfd);
    exit(0);
}



int main(int argc, char * argv[]) {
    serverfd = createSocket();
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    int status;

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) 
        error_exit("Error closing server");

    clearDataDB();

    while (1) {
        int connectionfd = accept(serverfd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (connectionfd < 0)
            error_exit("error accepting connection");
        
        else {
            printf("Connection accepted\n");
            int cpid = fork();
            //Fork error/
            if (cpid < 0)
                error_exit("forking error");
            //Child process the conection/
            else if (cpid == 0)
                handleConection(connectionfd);
            //Parent closes the fd/
            else
                close(connectionfd);
        }
        while(waitpid(-1, &status, WNOHANG) > 0);
    }
}