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

void deleteFile(string filename)
{
	memset(Buffer, '\0', 1024);
	const char* myfilename = filename.c_str();
	send(MainSocket, myfilename, strlen(myfilename), 0);
	recv(MainSocket, Buffer, 1024, 0);
	cout << Buffer << endl;
	
}

bool file_exists(const string& name) {
	ifstream f(name.c_str());
	if (f.good()) {
		f.close();
		return true;
	}
	else {
		f.close();
		return false;
	}
}

void recieve(const char* filename){
	//open up new socket for file transfer
	FileSocket = ConnectSocket("127.0.0.1", 25001);

	//file size of file being receieved
	int fileSize;
	char* msg = "File Recieved";	//string to echo

	//clear the buffer 
	memset(Buffer, '\0', 1024);
	if (recv(FileSocket, Buffer, 1024, 0)) //receive file size
	{
		fileSize = atoi(Buffer);
	}
	//buffer for file
	char* RecieveBuffer = new char[fileSize];

	send(FileSocket, "OK", (int)strlen("OK"), 0);

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

	closesocket(FileSocket);
	WSACleanup();
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
	
	recv(MainSocket, Buffer, 1024, 0);
	cout << Buffer << endl;

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
		if (input == "PASS")
		{
			const char* myfilename = parameters.c_str();
			send(MainSocket, myfilename, strlen(myfilename), 0);
			memset(Buffer, '\0', 1024);
			recv(MainSocket, Buffer, 1024, 0);
			cout << Buffer << endl;
			
		}
		else if (input == "STOR"){
			if (file_exists(parameters)){
				// send a file to server
				send(parameters);
			}
			else{
				cout << "File does not exists" << endl;
			}
		}
		else if (input == "RETR"){
			const char *file = parameters.c_str();

			//send server the file name to be retrieved
			send(MainSocket, file, (int)strlen(file), 0);

			//recieve ok msg
			memset(Buffer, '\0', 1024);
			recv(MainSocket, Buffer, 1024, 0);

			string s = Buffer;
			//check for ack
			if (s == "OK")
			{
				// Receive until the peer closes the connection
				recieve(file);
			}
			else{
				cout << Buffer << endl;
			}
		}
		else if (input == "DELE"){
			deleteFile(parameters);
		}
		else if (input == "RNFR")
		{
			const char*	old = parameters.c_str();
			send(MainSocket, old, strlen(old), 0);
		}
		else if (input == "RNTO")
		{
			const char*	new1= parameters.c_str();
			send(MainSocket,new1, strlen(new1),0);
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