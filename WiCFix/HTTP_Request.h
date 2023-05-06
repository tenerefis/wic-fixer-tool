#pragma once

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

class HTTP_Request
{
private:
	unsigned long m_IpAddress;
	unsigned short m_Port;
	char m_URL[MAX_STRING_LENGTH];
	char m_Hostname[MAX_STRING_LENGTH];
	char m_Filename[MAX_STRING_LENGTH];
	SOCKET m_Socket;
	sockaddr_in	m_Addr;
	char m_ResponseBody[MAX_STRING_LENGTH];
	unsigned int m_ResponseStatus;
	unsigned int m_ContentLength;

public:

private:
	bool Init(unsigned long ulAddress, unsigned short nPort);

public:
	HTTP_Request();
	~HTTP_Request();

	bool Init(const char *pszURL);
	bool SendGetRequest();
	bool Response(char *pszData, unsigned int uMaxLength);
};

#endif