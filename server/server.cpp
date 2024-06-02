//
// Gal Melman id- 319051950
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ctime>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")

#define TIME_PORT 27015
#define BUFFER_SIZE 255

// Function to get the current time formatted according to the provided format
void getCurrentTime(char* response, const char* format) {
	time_t currentTime;
	time(&currentTime);
	struct tm* timeinfo = localtime(&currentTime);
	strftime(response, BUFFER_SIZE, format, timeinfo);
}

// Function to handle the "MeasureTimeLap" request
void handleMeasureTimeLap(char* response) {
	static time_t startTime = 0;
	static bool isFirstMeasurement = true;
	time_t now = time(nullptr);
	double timeLapse;

	if (isFirstMeasurement) {
		strcpy(response, "First measurement started.");
		startTime = now;
		isFirstMeasurement = false;
	}
	else if (difftime(now, startTime) > 180) {
		strcpy(response, "Second measurement started.");
		startTime = now;
		isFirstMeasurement = true;
	}
	else {
		timeLapse = difftime(now, startTime);
		sprintf(response, "Time lapse since last measurement: %.2f seconds", timeLapse);
		isFirstMeasurement = true;
	}
}


/// Function to handle the "GetTimeWithoutDateInCity" request
void handleTimeWithoutDateInCity(const char* request, char* response) {
	// Extract city from the received message
	char city[BUFFER_SIZE];
	strcpy(city, strchr(request, ' ') + 1);
	time_t now = time(nullptr);
	char timeStr[BUFFER_SIZE];  // used to convert time format to string

	// Define time zone offsets in seconds
	const int UTC_Doha = 3 * 3600;    // UTC+3 (Doha)
	const int UTC_Prague = 2 * 3600;  // UTC+2 (Prague)
	const int UTC_NewYork = -4 * 3600; // UTC-4 (New York)
	const int UTC_Berlin = 2 * 3600;   // UTC+2 (Berlin)

	int offset = 0;
	bool validCity = true;

	if (strcmp(city, "Doha") == 0) {
		offset = UTC_Doha;
	}
	else if (strcmp(city, "Prague") == 0) {
		offset = UTC_Prague;
	}
	else if (strcmp(city, "New York") == 0) {
		offset = UTC_NewYork;
	}
	else if (strcmp(city, "Berlin") == 0) {
		offset = UTC_Berlin;
	}
	else {
		validCity = false;
	}

	

	if (validCity) {
		now += offset;
		struct tm* localTime = gmtime(&now);
		strftime(timeStr, BUFFER_SIZE, "%H:%M:%S", localTime);
		snprintf(response, BUFFER_SIZE, "Current time in %s: %s", city, timeStr);
	}
	else {
		struct tm* localTime = localtime(&now);
		strftime(timeStr, BUFFER_SIZE, "%H:%M:%S", localTime);
		snprintf(response, BUFFER_SIZE, "Invalid city: %s. Current local time: %s", city, timeStr);
	}
}

void handleRequest(const char* request, char* response) {
	if (strcmp(request, "GetTime") == 0) {
		getCurrentTime(response, "Current time: %Y-%m-%d %H:%M:%S");
	}
	else if (strcmp(request, "GetTimeWithoutDate") == 0) {
		getCurrentTime(response, "Current time without date: %H:%M:%S");
	}
	else if (strcmp(request, "GetTimeSinceEpoch") == 0) {
		time_t currentTime;
		time(&currentTime);
		sprintf(response, "Time since epoch: %ld seconds", currentTime);
	}
	else if (strcmp(request, "GetClientToServerDelayEstimation") == 0) {
		// Return server time in milliseconds
		DWORD serverTime = GetTickCount();
		sprintf(response, "Server time: %lu ms", serverTime);
	}
	else if (strcmp(request, "MeasureRTT") == 0) {
		strcpy(response, "Round-trip time measurement");
	}
	else if (strcmp(request, "GetTimeWithoutDateOrSeconds") == 0) {
		getCurrentTime(response, "Current time without date or seconds: %H:%M");
	}
	else if (strcmp(request, "GetYear") == 0) {
		getCurrentTime(response, "Current year: %Y");
	}
	else if (strcmp(request, "GetMonthAndDay") == 0) {
		getCurrentTime(response, "Current month and day: %m-%d");
	}
	else if (strcmp(request, "GetSecondsSinceBeginingOfMonth") == 0) {
		time_t currentTime;
		time(&currentTime);
		struct tm* timeinfo = localtime(&currentTime);
		int secondsSinceBeginning = (timeinfo->tm_mday - 1) * 86400 + timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;
		sprintf(response, "Seconds since beginning of month: %d", secondsSinceBeginning);
	}
	else if (strcmp(request, "GetWeekOfYear") == 0) {
		time_t currentTime;
		time(&currentTime);
		struct tm* timeinfo = localtime(&currentTime);
		int weekOfYear = (timeinfo->tm_yday + 7 - timeinfo->tm_wday) / 7;
		sprintf(response, "Current week of year: %d", weekOfYear);
	}
	else if (strcmp(request, "GetDaylightSavings") == 0) {
		time_t currentTime;
		time(&currentTime);
		struct tm* timeinfo = localtime(&currentTime);
		if (timeinfo->tm_isdst > 0) {
			strcpy(response, "Daylight savings time is currently in effect");
		}
		else {
			strcpy(response, "Daylight savings time is currently not in effect");
		}
	}
	else if (strncmp(request, "GetTimeWithoutDateInCity", strlen("GetTimeWithoutDateInCity")) == 0) {
	handleTimeWithoutDateInCity(request, response);
	}
	else if (strcmp(request, "MeasureTimeLap") == 0) {
		handleMeasureTimeLap(response);
		}
}

void main() {
	// Initialize Winsock (Windows Sockets)
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		std::cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Create a socket
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Time Server: Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	// Bind the socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(TIME_PORT);

	if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Time Server: Error at bind(): " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	// Wait for client requests
	char recvBuff[255];
	char sendBuff[255];
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	std::cout << "Time Server: Waiting for client requests...\n";

	while (true) {
		// Receive request from client
		int bytesRecv = recvfrom(serverSocket, recvBuff, sizeof(recvBuff), 0, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
		if (bytesRecv == SOCKET_ERROR) {
			std::cerr << "Time Server: Error at recvfrom(): " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return;
		}
		recvBuff[bytesRecv] = '\0';

		std::cout << "Time Server: Received \"" << recvBuff << " bytes of \"" << recvBuff << "\" message.\n";

		// Handle the received request
		handleRequest(recvBuff, sendBuff);

		// Send the response to the client
		int bytesSent = sendto(serverSocket, sendBuff, strlen(sendBuff), 0, reinterpret_cast<sockaddr*>(&clientAddr), clientAddrLen);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Time Server: Error at sendto(): " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return;
		}
		std::cout << "Time Server: Sent \"" << sendBuff << "\" to client.\n";
	}

	// Clean up
	closesocket(serverSocket);
	WSACleanup();
	return;
}


