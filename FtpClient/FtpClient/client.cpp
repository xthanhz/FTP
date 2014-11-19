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
char* str = "Trying to connect to server!";	//string to echo
char Buffer[1056];	//data buffer
char *prt = Buffer;		//pointer to buffer
int Addrlen = sizeof(Addr);
int Result;

void recieve(){
	do {

		Result = recv(Socket, Buffer, 256, 0);

		if (Result > 0){
			printf("Bytes received: %d\n", Result);
		}
		else if (Result == 0){
			printf("Message Recieve: ");
			for (int i = 0; i < sizeof(Buffer); i++)
				cout << Buffer[i];
			printf("\nConnection closed\n");
		}
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (Result > 0);
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

	Result = send(Socket, newfilename, (int)strlen(newfilename), 0);

	//check if sent correctly
	if (Result == SOCKET_ERROR) {
		cout << "Sending Fail" << endl;;
		closesocket(Socket);
		WSACleanup();
		system("exit");
	}

	//print byte being sent
	printf("Bytes Sent: %ld\n", Result);

	//Close sending
	Result = shutdown(Socket, SD_SEND);
	if (Result == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(Socket);
		WSACleanup();
		system("exit");
	}

	recieve();
}


int main(){

	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		// initalize socket 

	ZeroMemory(&Addr, sizeof(Addr));    // c lear the struct
	Addr.sin_family = AF_INET;    // set the address family
	
	Addr.sin_addr.s_addr = inet_addr("127.0.0.1");	// set the address
	Addr.sin_port = htons(25343);    // set the port

	//connect to server
	if (connect(Socket, (sockaddr*)&Addr, sizeof(Addr)) < 0)
	{
		cout << "Connection failed !" << endl;
		system("pause");
		return 0;
	}

	cout << "Connection successful !\n" << endl;

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