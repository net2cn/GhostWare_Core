#pragma once
#include <WinSock2.h>
#include <string>

#pragma warning(disable : 4996)

bool SocketListenClient()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return false;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(16896);
	connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	//"<EOF>" is necessary.
	send(sockClient, "Inject Success<EOF>", strlen("Inject Success<EOF>") + 1, 0);
	char recvBuf[1024];
	recv(sockClient, recvBuf, 1024, 0);

	closesocket(sockClient);
	WSACleanup();

	//Make sure the identity is complete - by adding "<EOF>" mark we can know if the string is complete.
	std::string recvStringBuffer(recvBuf);
	int endPatternPosition = recvStringBuffer.find("<EOF>");
	std::string recvContent = recvStringBuffer.substr(0, endPatternPosition);

	if (recvContent == std::string("Server Ready"))
		return true;

	return false;
}
