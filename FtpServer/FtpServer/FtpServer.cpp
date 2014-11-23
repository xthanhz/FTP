#include <string>
#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <cstdio>
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
bool PASSWORD = false;
char* password = "password";
string oldName;
string newName;

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
	//opens new socket for file transfer
	FileSocketListen = OpenFileTransferListener(25001);
	SOCKET FileSocket = accept(FileSocketListen, NULL, NULL);

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

		//clear the buffer 
		memset(Buffer, '\0', 1024);
		RecieveResult = recv(FileSocket, Buffer, 1024, 0);
		string returnmsg = Buffer;
		if (returnmsg != "OK")
			return;

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

		//clear the buffer 
		memset(Buffer, '\0', 1024);
		RecieveResult = recv(FileSocket, Buffer, 1024, 0);
		printf("Message Recieve: ");
		cout << Buffer << endl;
		printf("Connection closed\n\n");

		//Close sending
		SendResult = shutdown(FileSocket, SD_SEND);
		if (SendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(FileSocket);
			WSACleanup();
			exit(0);
		}
	}
	else{
		cout << "Invalid file" << endl;
	}
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
	char* s = "Server Requires Password. Please use PASS password to login.";
	SendResult = send(ClientSocket, s, (int)strlen(s), 0);
	cout << "Waiting on Password";
	
	cout << "Client Accepted" << endl;

	string command = " ";
	string filename = " ";
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
		if (command == "PASS"){
			recv(ClientSocket, Buffer, 1024, 0);
			if (strcmp(Buffer, password) == 0)
			{
				cout << "login successful!" << endl;
				char* msg = "Login Successful";
				send(ClientSocket, msg, (int)strlen(msg), 0);
				PASSWORD = true;
			}
			else
			{
				char* msg = "Incorrect password. You are being disconnected.";
				send(ClientSocket, msg, (int)strlen(msg), 0);
				closesocket(ClientSocket);
			}
		}
		else if (command == "STOR" && PASSWORD == true){
			// Receive until the peer shuts down the connection
			recieve();
		}
		else if (command == "RETR" && PASSWORD == true){
			//clear the buffer 
			memset(Buffer, '\0', 1024);
			recv(ClientSocket, Buffer, 1024, 0);//retrieve filename

			string s = Buffer;
			//check if file exists
			if (file_exists(s)){
				char* msg = "OK";
				//give ok msg
				send(ClientSocket, msg, (int)strlen(msg), 0);

				//send file
				send(s);
			}
			else{
				char* msg = "File does not exists";
				//give error msg
				send(ClientSocket, msg, (int)strlen(msg), 0);
			}
		}
		else if (command == "DELE" && PASSWORD == true){
			recv(ClientSocket, Buffer, 1024, 0);

			if (remove(Buffer) == 0)
			{
				printf("File %s has been deleted.\n", Buffer);
				char* msg = "File has been deleted.";
				send(ClientSocket, msg, (int)strlen(msg), 0);
			}
			else
				fprintf(stderr, "Error deleting file %s.\n", Buffer);
				char* msg = "Error deleting file.";
				send(ClientSocket, msg, (int)strlen(msg), 0);
		}
		else if (command == "RNFR" && PASSWORD == true)
		{
			recv(ClientSocket, Buffer, 1024, 0);
			oldName = Buffer;
			cout << oldName << endl;
		}
		else if (command == "RNTO" && PASSWORD == true)
		{
			recv(ClientSocket, Buffer, 1024, 0);
			newName = Buffer;
			cout << newName << endl;
			const char* oldName1 = oldName.c_str();
			const char* newName1 = newName.c_str();
			int result = rename(oldName1,newName1);
			if (result == 0)
				puts("File successfully renamed");
			else
				perror("Error renaming file");
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