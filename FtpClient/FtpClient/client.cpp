#include <string>
#include <sstream>
#include <WinSock2.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//main variables
SOCKET MainSocket, FileSocket;
WSADATA Winsock;
sockaddr_in Addr;	//address
char Buffer[1024];	//data buffer
char *prt = Buffer;		//pointer to buffer
int Addrlen = sizeof(Addr);
int SendResult;
int RecieveResult;

SOCKET ConnectSocket(char* address, int port)
{
	WSAStartup(MAKEWORD(2, 2), &Winsock);    // Start Winsock

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		// initalize socket 

	ZeroMemory(&Addr, sizeof(Addr));    // c lear the struct
	Addr.sin_family = AF_INET;    // set the address family

	Addr.sin_addr.s_addr = inet_addr(address);	// set the address
	Addr.sin_port = htons(port);    // set the port

	//connect to server
	if (connect(s, (sockaddr*)&Addr, sizeof(Addr)) < 0)
	{
		cout << "Connection failed !" << endl;
		system("pause");
		system("exit");
	}

	cout << "Connection successful !\n" << endl;

	return s;
}

void recieve(){
}

void send(string filename){
	//open up new socket for file transfer
	FileSocket = ConnectSocket("127.0.0.1", 25001);

	char *newfilename;
	unsigned long iFileSize = 0;
	long size;     //file size

	ifstream infile(filename, ios::in | ios::binary | ios::ate);

	//convert string to char
	if (infile.is_open()){
		size = infile.tellg();     //retrieve get pointer position
		infile.seekg(0, ios::beg);     //position get pointer at the begining of the file
		newfilename = new char[size];     //initialize the buffer
		infile.read(newfilename, size);     //read file to buffer
		infile.close();     //close file

		string s = to_string(size);
		const char* mysize = s.c_str();
		//send file size
		send(FileSocket, mysize, size, 0);
		
		//send file name
		const char* myfilename = filename.c_str();
		send(FileSocket, myfilename, strlen(myfilename), 0);

		//send file
		SendResult = send(FileSocket, newfilename, size, 0);

		//check if sent correctly
		if (SendResult == SOCKET_ERROR) {
			cout << "Sending Fail" << endl;;
			closesocket(FileSocket);
			WSACleanup();
			exit(0);
		}

		//print byte being sent
		printf("Bytes Sent: %ld\n", SendResult);

		//Close sending
		SendResult = shutdown(FileSocket, SD_SEND);
		if (SendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(FileSocket);
			WSACleanup();
			exit(0);
		}
		//clear the buffer 
		memset(Buffer, '\0', 1024);
		do {
			RecieveResult = recv(FileSocket, Buffer, 1024, 0);

			if (RecieveResult > 0){
				printf("Bytes received: %d\n", RecieveResult);
			}
			else if (RecieveResult == 0){

				printf("Message Recieve: ");
				cout << Buffer << endl;
				printf("Connection closed\n\n");
			}
			else
				printf("recv failed with error: %d\n", WSAGetLastError());

		} while (RecieveResult > 0);
	}
	else{
		cout << "Invalid file" << endl;
	}
}


int main(){
	MainSocket = ConnectSocket("127.0.0.1", 25000);

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

		// send command
		const char* msg = input.c_str();

		//send server request and let server know we want to store
		send(MainSocket, msg, (int)strlen(msg), 0);

		if (input == "STOR"){
			// send a file to server
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
	closesocket(MainSocket);
	WSACleanup();
	return 0;
}