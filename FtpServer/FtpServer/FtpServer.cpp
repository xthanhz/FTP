#include <string>
#include <iostream>
#include <WinSock2.h>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET MainSocket, ClientSocket, FileSocketListen, FileSocket;
WSADATA Winsock;
sockaddr_in Addr;
int Addrlen = sizeof(sockaddr_in);
char Buffer[1024];	//data buffer
char *prt = Buffer;		//pointer to buffer
sockaddr_in IncomingAddress;
int AddressLen = sizeof(IncomingAddress);
int SendResult;
int RecieveResult;

SOCKET OpenAndList(int port)
{
	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ZeroMemory(&Addr, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(port);
	bind(s, (sockaddr*)&Addr, sizeof(Addr));

	if (listen(s, 1) == SOCKET_ERROR)
	{
		cout << "Listening error" << endl;
	}
	else
	{
		cout << "Listening!" << endl;
	}

	return s;
}

SOCKET OpenFileTransferListener(int port)
{
	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ZeroMemory(&Addr, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(port);
	bind(s, (sockaddr*)&Addr, sizeof(Addr));

	if (listen(s, 1) == SOCKET_ERROR)
	{
		cout << "Listening error" << endl;
	}
	else
	{
		cout << "File transfer port opened!" << endl;
	}

	return s;
}

void recieve(){
	//opens new socket for file transfer
	FileSocketListen = OpenFileTransferListener(25001);
	SOCKET FileSocket = accept(FileSocketListen, NULL, NULL);

	//file size of file being receieved
	int fileSize; 
	const char* filename;
	char* msg = "File Recieved";	//string to echo

	//clear the buffer 
	memset(Buffer, '\0', 1024);
	if (recv(FileSocket, Buffer, 1024, 0)) //receive file size
	{
		fileSize = atoi(Buffer);
		//printf("File Size: %i\r\n", fileSize);
	}

	//buffer for file
	char* RecieveBuffer = new char[fileSize]; 
		
	//clear the buffer 
	memset(Buffer, '\0', 1024);
	if (recv(FileSocket, Buffer, 1024, 0)) //receive file name
	{
		filename = Buffer;
	}

	do {
		RecieveResult = recv(FileSocket, RecieveBuffer, fileSize, 0);
		if (RecieveResult > 0) {
			printf("Bytes received: %d\n", RecieveResult);

			// Echo the buffer back to the sender
			SendResult = send(FileSocket, msg, (int)strlen(msg), 0);

			//send error check
			if (SendResult == SOCKET_ERROR) {
				cout << "Sending Error" << endl;
				closesocket(FileSocket);
				WSACleanup();
				return;
			}
			printf("Bytes sent: %d\n", SendResult);

		}
		else if (RecieveResult == 0){
			//store the file
			FILE* outfile;
			outfile = fopen(filename, "wb"); //todo: change this to the file being stored
			fwrite(RecieveBuffer, 1, fileSize, outfile);
			fclose(outfile);

			printf("File Recieved, Connection closing...\n\n");
			closesocket(FileSocket);
		}
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(FileSocket);
			WSACleanup();
			return;
		}

	} while (RecieveResult > 0);

	closesocket(FileSocketListen);
	WSACleanup();
}

void send(string filename){
}

void acceptClients(){
	//Accepts client
	cout << "Accepting Clients" << endl;

	ClientSocket = accept(MainSocket, NULL, NULL);

	if (ClientSocket == INVALID_SOCKET) {
		cout << "Client Accepting Error" << endl;
		closesocket(MainSocket);
		WSACleanup();
		exit(0);
	}

	cout << "Client Accepted" << endl;
	
	string command = " "; 
	while (command != "Close"){
		//----------------------Get user command----------------------

		cout << "Waiting for client command " << endl;

		//clear the buffer 
		memset(Buffer, '\0', 1024);
		RecieveResult = recv(ClientSocket, Buffer, 1024, 0);
		if (RecieveResult != SOCKET_ERROR) //receive file name
		{
			command = Buffer;
			//cout << command << endl;
		}
		else{ // checks to see if client d/c on us
			command = "Close";
		}
		//----------------------Get user command-----------------------

		//TODO implement other commands
		//list of commands
		if (command == "STOR"){
			// Receive until the peer shuts down the connection
			recieve();
		}
		if (command == "RETR"){
			cout << "user wants to retrieve, test" << endl;
		}
		else{
			char* msg = "Invalid Command";
			SendResult = send(ClientSocket, msg, (int)strlen(msg), 0);
		}
	}

	cout << "Client Disconnected " << endl << endl; 
	//accept a new client
	acceptClients();
}

int main()
{
	//open the socket
	MainSocket = OpenAndList(25000);

	//accept the client and listen for action
	acceptClients();

	system("pause");

	//close socket
	closesocket(MainSocket);
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}