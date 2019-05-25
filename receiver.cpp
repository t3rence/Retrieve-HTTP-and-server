// ---------------------------Receiver.cpp-----------------------------------
// Terence Ho CSS432
// Created: 02/04/19
// Last Modified: 02/09/19
// Purpose: The receiver sends a GET HTTP request to the server to open html
// 	files and go through a directory if the server allows it. 
//---------------------------------------------------------------------------
#include <iostream>		  // IO
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <strings.h>      // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <sys/time.h>	  // time
#include <stdlib.h>		  // atoi
#include <stdio.h>        /* printf, fgets */
#include <fstream>        // File Stream
#include <regex>          // Regular Expression

using namespace std;

class Reciever {
private:
    int port;
    const char *serverIP;
    string path;
    bool serverIsBad;
public:
	Reciever(string serverIp, string serverPath) {
		serverIP = serverIp.c_str();
		port = 80;
		path = serverPath;
	}

	Reciever(string serverIp, int serverPort, string serverPath, 
		bool serverBad) {
        serverIP = serverIp.c_str();
        port = serverPort;
        path = serverPath;
        serverIsBad = serverBad;
    }


    void startRequest() {
        //GET Request string
        string server(serverIP);
        string getHTTP;
        if (serverIsBad){
            getHTTP = "GET /" + path + " HTTP/1.1\r\nHost: " 
            + server + "\r\n\r\n";
        } else{
            getHTTP = "BAD /" + path + " HTTP/1.1\r\nHost: " 
            + server + "\r\n\r\n";
        }

        //get hostent structure from the server name
        struct hostent *host = gethostbyname(serverIP);

		//Create socket & establish a connection to a server
		// Declare a sockaddr_in structure
		sockaddr_in sendSockAddr;
		bzero( (char*)&sendSockAddr, sizeof( sendSockAddr ) );
		sendSockAddr.sin_family      = AF_INET; // Address Family Internet
		sendSockAddr.sin_addr.s_addr =
			inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
		sendSockAddr.sin_port        = htons( port );

		// Connect to the server
        //Open a stream-oriented socket with the Internet address family.
		int clientSd = socket( AF_INET, SOCK_STREAM, 0 ); //Socket Descriptor
		int connectStatus = connect( clientSd, ( sockaddr* )&sendSockAddr, sizeof( sendSockAddr ) );
		if (connectStatus < 0) {
			cerr << "Failed to connect to server" << endl;
		}

        //Connect input socket to the server
        connect(clientSd, (sockaddr * ) & sendSockAddr, sizeof(sendSockAddr));

		send(clientSd, getHTTP.c_str(), strlen(getHTTP.c_str()),0);

		//Prepare to receieve from server
        char echoBuff[40000];
        int totalBytesRcvd = 0;
        unsigned int echoStringLen;
        int bytesRecieved;
        string output;

        while (((bytesRecieved = recv(clientSd, echoBuff, sizeof(echoBuff),0)) > 0)) {
            if (bytesRecieved > 0){
                output.append(echoBuff, bytesRecieved);
            }
        }

		if (bytesRecieved < 0) {
			perror("Error on bytes received");
		}

		//Pull response from server and take output
        smatch m;
        regex err("HTTP\\/1.1 (.*)");
        regex_search(output, m, err);
        if (m[1].str().find("200 OK") != std::string::npos) {
            cout << output << endl;		// display message

            ofstream myfile;			// save output to file
            myfile.open("received.txt");
            myfile << output << endl;
            myfile.close();
        } else {
			// HTTP error code
			regex_search(output, m, err);
			cout << m[1].str() << endl;
        }

        close(clientSd);
    }
};

int main(int argc, char *argv[]) {
	// Your retriever takes in an input from the command line and 
	// parses the server address and file (web page) that is being requested.
    string urlStr = argv[1];
    smatch m;

	// IP with port
    regex ipRegEx("https?:\\/\\/([0-9]+(?:\\.[0-9]+){3}):?([0-9]+)?\\/?(.+)?");
    // server/domain name
    regex domainRegEx("https?:\\/\\/([^/]*)(.*)?");

    // Is an IP
    if (regex_match(urlStr, ipRegEx)) {
        regex_search(urlStr, m, ipRegEx);
	    if(argc != 2) {
	        perror("Incorrect number of parameters\n");
            Reciever(m[1], atoi(m[2].str().c_str()), m[3].str(), true).startRequest();
        } else {
            Reciever(m[1], atoi(m[2].str().c_str()), m[3].str(), false).startRequest();
        }

   	// Is a domain
    } else if (regex_match(urlStr, domainRegEx)) {
        regex_search(urlStr, m, domainRegEx);
        Reciever(m[1], m[3].str()).startRequest();
    } else {
    	perror("Invalid input");
    }
    return 0;
}
