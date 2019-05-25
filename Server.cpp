// -----------------------------Server.cpp-----------------------------------
// Terence Ho CSS432
// Created: 02/04/19
// Last Modified: 02/09/19
// Purpose: This server is used with the receiver in order to respond to the
//  GET HTTP requests that the receiver sends. 
// "GET http://www.website.com/folder/index.html HTTP/1.1"
//---------------------------------------------------------------------------
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <stdlib.h>       // atoi
#include <stdio.h>        /* printf, fgets */
#include <iostream>       // IO
#include <fstream>        // File Stream
#include <regex>          // Regular Expression
#include <pthread.h>      // pthread
#include <stdexcept>      //Excpetions
#include <sys/time.h>     // time

using namespace std;

class Server {
private:
    int port;

public:
    Server(int serverPort) {
        // Reject invalid ports
        if((serverPort > 65535) || (serverPort < 1024)) {
            perror("Please enter a port number between 1024 - 65535");
        }
        port = serverPort;
    }

    void startServer() {
        pthread_t thread1;
        // Declare a sockaddr_in structure
        sockaddr_in acceptSockAddr;
        
        // zero-initialize it by calling bzero 
        bzero((char*) &acceptSockAddr, sizeof(acceptSockAddr));
        
        // and set its data members
        acceptSockAddr.sin_family       = AF_INET; // Address Family Internet
        acceptSockAddr.sin_addr.s_addr  = htonl( INADDR_ANY );
        acceptSockAddr.sin_port         = htons( port );
        

        //create socket
        int serverSd = -1;
        serverSd = socket( AF_INET, SOCK_STREAM, 0 );                               //IPPROTO_TCP?
        
        // socket is opened
        if(serverSd < 0) {
            perror("Cannot open socket");
        }

        // Set the SO_REUSEADDR option. (Note this option is useful to prompt OS to release the server port as soon as your server process is terminated.)
        const int on = 1;
        setsockopt( serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, 
                    sizeof( int ) );

        // Bind this socket to its local address by calling bind as passing the following arguments: the socket descriptor, 
        // the sockaddr_in structure defined above, and its data size. 
        bind(serverSd, (sockaddr * ) & acceptSockAddr, sizeof(acceptSockAddr));

        //Looping for new clients attemping to connect
        while (true) {
            listen(serverSd, 5);

            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);
            int newSd = accept(serverSd, (sockaddr * ) & newSockAddr,
                               &newSockAddrSize);
            int arr[1] = {newSd};
            pthread_create(&thread1, NULL, threadRequestHandler, (void *) arr);
        }
        close(serverSd);
    }

    static void *threadRequestHandler(void *vptr_value) {
        //HTTP Response Headers strings
        string http_OK = "HTTP/1.1 200 OK\r\nConnection: Closed\r\nContent-Type: text/html;\n\n";
        string http_NotFound = "HTTP/1.1 404 Not Found\r\nConnection: Closed\r\nContent-Type: text/html;\n\n";
        string http_BAD = "HTTP/1.1 400 Bad Request\n\n";
        string http_UnAuth = "HTTP/1.1 401 Unauthorized\n\n";
        string http_FORBID = "HTTP/1.1 403 Forbidden\n\n";

        int *arr = (int *) vptr_value;
        char dataBuff[4000];
        void *retVal = malloc(sizeof(int));


        int nRead = 0;
        string request = "";

        // Get Request, read bytes
        int bytesRead = recv(arr[0], dataBuff, 4000 - nRead, 0);
        if (bytesRead <= 0) {
            shutdown(arr[0], SHUT_RDWR);
            close(arr[0]);
            return retVal;
        }
        nRead += bytesRead;
        request.append(dataBuff, bytesRead);

        smatch m;
        regex path("(GET) (.*) .*"); //pull request path
        if (regex_search(request, m, path)) {
            fstream fs;                   // filestream
            string file = m[2].str();     //use file path
            file.erase(file.begin());     // remove "/"
            fs.open(file, fstream::in);   // open file
            string line;

            // Default page
            if (strcmp(m[2].str().c_str(), "/") == 0) {
                send(arr[0], http_OK.c_str(), strlen(http_OK.c_str()), 0);
                send(arr[0], "Welcome!", strlen("Welcome"),0);
            // If the HTTP request is for SecretFile.html
            } else if (strcmp(m[2].str().c_str(), "/SecretFile.html") == 0) {
                send(arr[0], http_UnAuth.c_str(), strlen(http_UnAuth.c_str()), 0);
            // If the request is for a file that is above the directory structure where your web server is running 
            // ( for example, "GET ../../../etc/passwd" ), you should return a 403 Forbidden.               
            } else if (file.find("..") != std::string::npos) {
                send(arr[0], http_FORBID.c_str(), strlen(http_FORBID.c_str()), 0);
            } else {
                if (fs.is_open()) { // found
                    send(arr[0], http_OK.c_str(), strlen(http_OK.c_str()), 0);
                    while (getline(fs, line)) {
                        send(arr[0], line.c_str(), strlen(line.c_str()), 0);
                    }
                    fs.close();
                } else { // doesn't exist in directory
                    send(arr[0], http_NotFound.c_str(), strlen(http_NotFound.c_str()), 0);
                    fs.open("NotFound.html", fstream::in);
                    while (getline(fs, line)) {
                        send(arr[0], line.c_str(), strlen(line.c_str()), 0);
                    }
                    fs.close();

                }
            }
        } else { // Bad request 400 Error
            send(arr[0], http_BAD.c_str(), strlen(http_BAD.c_str()), 0);
        }

        //close socket
        shutdown(arr[0], SHUT_RDWR);
        close(arr[0]);
    }


};

int main(int argc, char *argv[]) {
    if (argc > 2) {
        perror("Invalid argument input");
        return -1;
    } else {
        Server(atoi(argv[1])).startServer();
    }
    return 1;
}

// int fileDirectoryCheck(const char* const pathname) {
//     struct stat info;
//  int statRC = stat( pathname, &info );
//  if( statRC != 0 ) {
//      printf( "cannot access %s\n", pathname );
//      return -1;
//  } else if( info.st_mode & S_IFDIR ) {
//      printf( "%s is a directory\n", pathname );
//      return 1;
//  } else {
//      printf( "%s is no directory\n", pathname );
//      return 0;
//  }
// }