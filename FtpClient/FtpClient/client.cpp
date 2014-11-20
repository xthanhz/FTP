#include <string>
#include <sstream>
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//main variables
SOCKET Socket;
WSADATA Winsock;
sockaddr_in Addr;	//address
char Buffer[1056];	//data buffer
char *prt = Buffer;		//pointer to buffer
int Addrlen = sizeof(Addr);
int SendResult;
int RecieveResult;

void recieve(){
	char* msg = "File Recieved";	//string to echo
	do {
		RecieveResult = recv(Socket, Buffer, 256, 0);
		if (RecieveResult > 0) {
			printf("Bytes received: %d\n", RecieveResult);

			// Echo the buffer back to the sender
			SendResult = send(Socket, msg, (int)strlen(msg), 0);

			//send error check
			if (SendResult == SOCKET_ERROR) {
				cout << "Sending Error" << endl;
				closesocket(Socket);
				WSACleanup();
				system("exit");
			}
			printf("Bytes sent: %d\n", SendResult);

		}
		else if (RecieveResult == 0){
			printf("Message Recieve: ");
			for (int i = 0; i < sizeof(Buffer); i++)
				cout << Buffer[i];

			printf("\nConnection closing...\n");
			printf("\nListening to new clients...\n");
			closesocket(Socket);
		}
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(Socket);
			WSACleanup();
			system("exit");
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

	SendResult = send(Socket, newfilename, (int)strlen(newfilename), 0);

	//check if sent correctly
	if (SendResult == SOCKET_ERROR) {
		cout << "Sending Fail" << endl;;
		closesocket(Socket);
		WSACleanup();
		system("exit");
	}

	//print byte being sent
	printf("Bytes Sent: %ld\n", SendResult);

	//Close sending
	SendResult = shutdown(Socket, SD_SEND);
	if (SendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		system("exit");
	}

	do {

		RecieveResult = recv(Socket, Buffer, 256, 0);

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

void open_socket(const char* address, int port, SOCKET soc){
	soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		// initalize socket 

	ZeroMemory(&Addr, sizeof(Addr));    // c lear the struct
	Addr.sin_family = AF_INET;    // set the address family

	Addr.sin_addr.s_addr = inet_addr(address);	// set the address
	Addr.sin_port = htons(port);    // set the port

	//connect to server
	if (connect(soc, (sockaddr*)&Addr, sizeof(Addr)) < 0)
	{
		cout << "Connection failed !" << endl;
		system("pause");
		system("exit");
	}

	cout << "Connection successful !\n" << endl;
}

int main(){

	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	open_socket("127.0.0.1", 25000, Socket);

	string input, parameters;
	//get user input
	cout << "Please enter a message or command to send " << endl;
	cin >> input >> parameters;
	
	while (input != "exit"){
		if (input == "HELP"){
			cout << "Avaliable commands:" << endl;
			cout << "1. STOR filename" << endl;
			cout << "2. RETR filename" << endl;
		}
		if (input == "STOR"){
			//send request
			send(parameters);
		}
		else if (input == "RETR"){
			// Receive until the peer closes the connection
			recieve();
		}
		cout << "Please enter a message or command to send " << endl;
		cin >> input >> parameters;
	}
	system("pause");

	//close socket and cleanup
	closesocket(Socket);
	WSACleanup();
	return 0;
}