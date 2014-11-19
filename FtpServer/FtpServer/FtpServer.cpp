#include <string>
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET Socket, ClientSocket;
WSADATA Winsock;
sockaddr_in Addr;
int Addrlen = sizeof(sockaddr_in);
char* str = "Hi Client";	//string to echo
char Buffer[256];	//data buffer
char *prt = Buffer;		//pointer to buffer
sockaddr_in IncomingAddress;
int AddressLen = sizeof(IncomingAddress);
int SendResult;
int RecieveResult;


int main()
{
	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ZeroMemory(&Addr, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(25343);
	bind(Socket, (sockaddr*)&Addr, sizeof(Addr));

	if (listen(Socket, 1) == SOCKET_ERROR)
	{
		cout << "Listening error" << endl;
	}
	else
	{
		cout << "Listening!" << endl;
	}

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
		do {

			RecieveResult = recv(ClientSocket, Buffer, 256, 0);
			if (RecieveResult > 0) {
				printf("Bytes received: %d\n", RecieveResult);

				// Echo the buffer back to the sender
				SendResult = send(ClientSocket, str, (int)strlen(str) , 0);
				if (SendResult == SOCKET_ERROR) {
					cout << "Sending Error" << endl;
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
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
				return 1;
			}

		} while (RecieveResult > 0);

	}
	system("pause");

	//close socket
	closesocket(Socket);
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}