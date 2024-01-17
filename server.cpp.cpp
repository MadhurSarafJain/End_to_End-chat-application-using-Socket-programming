#include <iostream>
#include <WinSock2.h>//include this library for socket programming
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>//to store all the clients that are sending mssgs to server
using namespace std;


#pragma comment(lib,"ws2_32.lib")

/*
we will first initialize the winsock library, create the socket and then get IP and port(so that other client also connects on this port)
then we bind the socket with ip / port
listen on the socket
accept(see we are creating blocking message passing in which blocks the sender until the message is received at the receiver’s end)
recv and send
close the socket
cleanup winsock
*/

bool Initialise() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;

}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients){
	//send/rcv
	cout << "client connected";
	char buffer[4096];

	while (1) {
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesrecvd <= 0) {
			cout << "client disconnected" << endl;
			break;
		}
		string message(buffer, bytesrecvd);
		cout << "message for client: " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
			
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
	}

	
	

	closesocket(clientSocket);

}

int main() {
	if (!Initialise()) {
		cout << "Winsock initialization failed";
		return 1;
	}


	cout << "Server Program" << endl;
	//create socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (listenSocket == INVALID_SOCKET) {
		cout << "socket initialisation failed";
		return 1;
	}

	//create address structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	//covert the ipaddress (0.0.0.0) put it inside the sin_family in binary form 
	if (InetPton(AF_INET,_T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address struture failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//bind
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR){
		cout << "bind failed";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "server has started listening on port: " << port << endl;
	vector<SOCKET> clients;

	//accept
	while (1) {
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

		if (clientSocket == INVALID_SOCKET) {
			cout << " Invalid client socket" << endl;
		}

		clients.push_back(clientSocket);

		thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}
	

	

	closesocket(listenSocket);
	
	
	WSACleanup();
	return 0;
}