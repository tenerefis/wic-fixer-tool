#include "stdafx.h"

HTTP_Request::HTTP_Request()
{
	WSADATA wsaData;
	
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
		log_window(L"WSAStartup error");

	this->m_IpAddress = 0;
	this->m_Port = 0;
	memset(this->m_URL, 0, sizeof(this->m_URL));
	memset(this->m_Hostname, 0, sizeof(this->m_Hostname));
	memset(this->m_Filename, 0, sizeof(this->m_Filename));
	this->m_Socket = INVALID_SOCKET;
	memset(&this->m_Addr, 0, sizeof(this->m_Addr));
	memset(this->m_ResponseBody, 0, sizeof(this->m_ResponseBody));
	this->m_ResponseStatus = 0;
	this->m_ContentLength = 0;
}

HTTP_Request::~HTTP_Request()
{
	int iResult = WSACleanup();

	if (iResult != 0)
		log_window(L"WSACleanup error");
}

bool HTTP_Request::Init(unsigned long ulAddress, unsigned short nPort)
{
	if (ulAddress == 0 || nPort == 0)
		return false;

	this->m_IpAddress = ulAddress;
	this->m_Port = nPort;

	this->m_Addr.sin_family = AF_INET;
	this->m_Addr.sin_port = htons(this->m_Port);
	this->m_Addr.sin_addr.s_addr = this->m_IpAddress;
	
	return true;
}

bool HTTP_Request::Init(const char *pszURL)
{
	if (!strstr(pszURL, "://"))
		return false;

	// TODO: handle urls without schema(http/s), handle urls without filename
	const char *i;

	if ((i = strstr(pszURL, ".com")) || (i = strstr(pszURL, ".net")) || (i = strstr(pszURL, ".org")))
	{
		size_t index = (size_t)(strstr(pszURL, i) - pszURL);
		size_t index2 = (size_t)(strstr(pszURL, "://") - pszURL);

		strcpy_s(this->m_URL, pszURL);
		strncpy_s(this->m_Hostname, pszURL + index2 + 3, index - index2 + 1);
		strncpy_s(this->m_Filename, pszURL + index + 4, strlen(pszURL + index + 4));
	}

	if (strlen(this->m_Hostname) == 0 || strlen(this->m_Filename) == 0)
		return false;

	hostent *remotehost = gethostbyname(this->m_Hostname); // freed when winsock thread ends

	in_addr addr;
	memset(&addr, 0, sizeof(addr));

	if (!remotehost)
		return false;

	if (remotehost->h_addrtype == AF_INET)
		addr.s_addr = *(unsigned long*)remotehost->h_addr_list[0];

	if (addr.s_addr == 0)
		return false;

	if (!this->Init(addr.s_addr, 80))
		return false;

	return true;
}

bool HTTP_Request::SendGetRequest()
{
	this->m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->m_Socket == INVALID_SOCKET)
	{
		shutdown(this->m_Socket, 2);
		closesocket(this->m_Socket);
		return false;
	}

	// TODO: handle timeouts
	char sendBuf[1024] = "";
	char recvBuf[1024] = "";
	int iResult = 0;

	sprintf_s(sendBuf, "GET %s HTTP/1.1\r\nHost: %s\r\nAccept: text/plain\r\nConnection: close\r\n\r\n", this->m_Filename, this->m_Hostname);

	iResult = connect(this->m_Socket, (sockaddr*)&this->m_Addr, sizeof(this->m_Addr));

	if (iResult == SOCKET_ERROR)
	{
		shutdown(this->m_Socket, 2);
		closesocket(this->m_Socket);
		return false;
	}

	iResult = send(this->m_Socket, (const char*)sendBuf, (int)strlen(sendBuf), 0);

	if (iResult == SOCKET_ERROR)
	{
		shutdown(this->m_Socket, 2);
		closesocket(this->m_Socket);
		return false;
	}

	iResult = shutdown(this->m_Socket, 1); /* 1 = SD_SEND */

	if (iResult == SOCKET_ERROR)
	{
		shutdown(this->m_Socket, 2); /* 2 = SD_BOTH */
		closesocket(this->m_Socket);
		return false;
	}

	int bufLen = 0;

	do
	{
		bufLen = recv(this->m_Socket, recvBuf, sizeof(recvBuf), 0);
		if (bufLen > 0)
			printf("bytes received: %d", bufLen);
		else if (bufLen == 0)
			printf("connection closed");
		else
			printf("recv failed");
	} while (bufLen > 0);

	shutdown(this->m_Socket, 2);
	closesocket(this->m_Socket);

	// TODO: extract Content-Length
	if (!strstr(recvBuf, "HTTP/1.1 200 OK"))
		return false;

	this->m_ResponseStatus = 200;

	size_t index = (size_t)(strstr(recvBuf, "\r\n\r\n") - recvBuf);
	strcpy_s(this->m_ResponseBody, recvBuf + index + 4);

	return true;
}

bool HTTP_Request::Response(char *pszData, unsigned int uMaxLength)
{
	if (this->m_ResponseStatus != 200 || strlen(this->m_ResponseBody) == 0)
		return false;

	// should use content length
	strcpy_s(pszData, uMaxLength, this->m_ResponseBody);

	return true;
}