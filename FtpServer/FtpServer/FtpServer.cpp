#include <string>
#include <iostream>
#include <WinSock2.h>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET Socket, ClientSocket;
WSADATA Winsock;
sockaddr_in Addr;
int Addrlen = sizeof(sockaddr_in);
char Buffer[256];	//data buffer
char *prt = Buffer;		//pointer to buffer
sockaddr_in IncomingAddress;
int AddressLen = sizeof(IncomingAddress);
int SendResult;
int RecieveResult;

void recieve(){
	char* msg = "File Recieved";	//string to echo
	do {
		RecieveResult = recv(ClientSocket, Buffer, 256, 0);
		if (RecieveResult > 0) {
			printf("Bytes received: %d\n", RecieveResult);

			// Echo the buffer back to the sender
			SendResult = send(ClientSocket, msg, (int)strlen(msg), 0);

			//send error check
			if (SendResult == SOCKET_ERROR) {
				cout << "Sending Error" << endl;
				closesocket(ClientSocket);
				WSACleanup();
				return;
			}
			printf("Bytes sent: %d\n", SendResult);

		}
		else if (RecieveResult == 0){
			printf("Message Recieve: ");
			for (int i = 0; i < sizeof(Buffer); i++)
				cout << Buffer[i];

			printf("\nConnection closing...\n");
			printf("\nListening to new clients...\n");
			closesocket(ClientSocket);
		}
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (RecieveResult > 0);
}

void send(string filename){
	char *newfilename;
	unsigned long iFileSize = 0;
	long size;     //file size

	ifstream infile(filename, ios::in | ios::binary | ios::ate);

	//convert string to char
	size = infile.tellg();     //retrieve get pointer position
	infile.seekg(0, ios::beg);     //position get pointer at the begining of the file
	newfilename = new char[size];     //initialize the buffer
	infile.read(newfilename, size);     //read file to buffer
	infile.close();     //close file

	SendResult = send(ClientSocket, newfilename, (int)strlen(newfilename), 0);

	//check if sent correctly
	if (SendResult == SOCKET_ERROR) {
		cout << "Sending Fail" << endl;;
		closesocket(ClientSocket);
		WSACleanup();
		system("exit");
	}

	//print byte being sent
	printf("Bytes Sent: %ld\n", SendResult);

	//Close sending
	SendResult = shutdown(ClientSocket, SD_SEND);
	if (SendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		system("exit");
	}

	do {

		RecieveResult = recv(ClientSocket, Buffer, 256, 0);

		if (RecieveResult > 0){
			printf("Bytes received: %d\n", RecieveResult);
		}
		else if (RecieveResult == 0){
			printf("Message Recieve: ");
			for (int i = 0; i < sizeof(Buffer); i++)
				cout << Buffer[i];
			printf("\nConnection closed\n");
		}
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (RecieveResult > 0);
}

void open_socket(int port, SOCKET soc){
	soc= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ZeroMemory(&Addr, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(port);
	bind(soc, (sockaddr*)&Addr, sizeof(Addr));

	if (listen(soc, 1) == SOCKET_ERROR)
	{
		cout << "Listening error" << endl;
	}
	else
	{
		cout << "Listening!" << endl;
	}
}

int main()
{
	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	open_socket(25000, Socket);

	while (true){
		//Accepts client
		ClientSocket = accept(Socket, NULL, NULL);

		if (ClientSocket == INVALID_SOCKET) {
			cout << "Client Accepting Error" << endl;
			closesocket(Socket);
			WSACleanup();
			return 1;
		}

		cout << "Client Accepted" << endl;

		// Receive until the peer shuts down the connection
		recieve();
	}
	system("pause");

	//close socket
	closesocket(Socket);
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}