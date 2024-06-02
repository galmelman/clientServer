//
// Gal Melman id- 319051950
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <ctime>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define TIME_PORT 27015
#define BUFFER_SIZE 255

void send_request(SOCKET connSocket, sockaddr_in server, const char* request, bool print_response = true) {
	int bytesSent = sendto(connSocket, request, (int)strlen(request), 0, (const sockaddr*)&server, sizeof(server));
	if (SOCKET_ERROR == bytesSent) {
		std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
		closesocket(connSocket);
		WSACleanup();
		exit(1);
	}
	std::cout << "Time Client: Sent: " << bytesSent << "/" << strlen(request) << " bytes of \"" << request << "\" message.\n";

	if (print_response) {
		char recvBuff[255];
		int bytesRecv = recv(connSocket, recvBuff, 255, 0);
		if (SOCKET_ERROR == bytesRecv) {
			std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
			closesocket(connSocket);
			WSACleanup();
			exit(1);
		}

		recvBuff[bytesRecv] = '\0'; // Add null-terminator
		std::cout << "Time Client: Received: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
	}
}

// Function to prompt the user to choose a city from a list
void chooseCity(char* city) {
		std::cout << "Choose a city:\n";
		std::cout << "1. Doha (Qatar)\n";
		std::cout << "2. Prague (Czech Republic)\n";
		std::cout << "3. New York (USA)\n";
		std::cout << "4. Berlin (Germany)\n";
		std::cout << "Enter the city number: ";
		std::cout << "\n";

		int cityChoice;
		std::cin >> cityChoice;

		switch (cityChoice) {
		case 1:
			strcpy(city, "Doha");
			break;
		case 2:
			strcpy(city, "Prague");
			break;
		case 3:
			strcpy(city, "New York");
			break;
		case 4:
			strcpy(city, "Berlin");

			break;
		default:
			std::cout << "Invalid city choice.\n";
			strcpy(city, "invalid city");
			break;
		}

}

// Function to measure the RoundTripTime
void measureRTT(SOCKET connSocket, const sockaddr_in& server) {
	const char* sendBuff = "MeasureRTT";
	char recvBuff[255];
	DWORD startTicks[100], endTicks[100];
	double rttSum = 0;

	for (int i = 0; i < 100; ++i) {
		// Record the time just before sending the request
		startTicks[i] = GetTickCount();

		// Send the request to the server
		int bytesSent = sendto(connSocket, sendBuff, strlen(sendBuff), 0, (sockaddr*)&server, sizeof(server));
		if (bytesSent == SOCKET_ERROR) {
			std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
			closesocket(connSocket);
			WSACleanup();
			return;
		}

		// Wait for the response from the server
		int serverAddrSize = sizeof(server);
		int bytesRecv = recvfrom(connSocket, recvBuff, sizeof(recvBuff) - 1, 0, (sockaddr*)&server, &serverAddrSize);
		if (bytesRecv == SOCKET_ERROR) {
			std::cout << "Time Client: Error at recvfrom(): " << WSAGetLastError() << std::endl;
			closesocket(connSocket);
			WSACleanup();
			return;
		}
		recvBuff[bytesRecv] = '\0'; // Null-terminate the received data

		// Record the time just after receiving the response
		endTicks[i] = GetTickCount();

		// Calculate RTT for this request-response pair
		rttSum += (endTicks[i] - startTicks[i]);
	}

	// Calculate the average RTT
	double averageRTT = rttSum / 100.0;
	std::cout << "Average RTT: " << averageRTT << " ms\n";
}

// Function to measure the client to server delay
void measureClientToServerDelay(SOCKET connSocket, sockaddr_in server, char* sendBuff) {
	// Prepare the request message
	strcpy(sendBuff, "GetClientToServerDelayEstimation");

	// Collect request and measure delay
	DWORD startTicks[100], serverTicks[100];
	for (int i = 0; i < 100; ++i) {
		// Record the time before sending the request
		startTicks[i] = GetTickCount();
		// Send the request
		int bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&server, sizeof(server));
		if (SOCKET_ERROR == bytesSent) {
			std::cout << "Time Client: Error at sendto(): " << WSAGetLastError() << std::endl;
			closesocket(connSocket);
			WSACleanup();
			exit(1);
		}
		std::cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message number " << i + 1 << " .\n";
	}

	// Collect responses and measure delay
	for (int i = 0; i < 100; ++i) {
		// Receive the response
		char recvBuff[BUFFER_SIZE];
		int bytesRecv = recv(connSocket, recvBuff, BUFFER_SIZE, 0);
		// Check for errors in receiving
		if (SOCKET_ERROR == bytesRecv) {
			std::cout << "Time Client: Error at recv(): " << WSAGetLastError() << std::endl;
			closesocket(connSocket);
			WSACleanup();
			return;
		}
		recvBuff[bytesRecv] = '\0';
		std::cout << "Time Client: Received: " << bytesRecv << " bytes of \"" << recvBuff << "\" message number " << i + 1 << " .\n";

		// Extract server time from the response
		sscanf(recvBuff, "Server time: %lu ms", &serverTicks[i]);
	}

	// Calculate average delay
	double totalDelay = 0;
	for (int i = 0; i < 100; ++i) {
		// Calculate delay for each request and accumulate
		totalDelay += (serverTicks[i] - startTicks[i]);
	}
	// Calculate the average delay
	double averageDelay = totalDelay / 100.0;

	// Print the average delay
	std::cout << "Average Client to Server Delay Estimation: " << averageDelay << " ms\n";
}

// Function to handle case 12: GetTimeWithoutDateInCity
void handleGetTimeWithoutDateInCity(char* sendBuff) {
	strcpy(sendBuff, "GetTimeWithoutDateInCity");

	char city[20]; // Adjust size as per the maximum length of city names
	chooseCity(city);

	if (city[0] != '\0') {
		strcat(sendBuff, " ");
		strcat(sendBuff, city);
	}
}

int main() {
	// Initialize Winsock (Windows Sockets)
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		std::cout << "Time Client: Error at WSAStartup()\n";
		return 1;
	}

	// Client side: Create a socket and connect to an internet address
	SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket) {
		std::cout << "Time Client: Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	// Create a sockaddr_in object called server
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);

	// Menu options
	int in = 0;
	while (in != 14) {
		std::cout << "\n";
		std::cout << "Choose which message to send: \n";
		std::cout << "1 - GetTime\n";
		std::cout << "2 - GetTimeWithoutDate\n";
		std::cout << "3 - GetTimeSinceEpoch\n";
		std::cout << "4 - GetClientToServerDelayEstimation\n";
		std::cout << "5 - MeasureRTT\n";
		std::cout << "6 - GetTimeWithoutDateOrSeconds\n";
		std::cout << "7 - GetYear\n";
		std::cout << "8 - GetMonthAndDay\n";
		std::cout << "9 - GetSecondsSinceBeginingOfMonth\n";
		std::cout << "10 - GetWeekOfYear\n";
		std::cout << "11 - GetDaylightSavings\n";
		std::cout << "12 - GetTimeWithoutDateInCity\n";
		std::cout << "13 - MeasureTimeLap\n";
		std::cout << "14 - End the process\n";
		std::cin >> in;
		std::cin.ignore(); // Ignore newline character

		char sendBuff[255];
		switch (in) {
		case 1:
			strcpy(sendBuff, "GetTime");
			break;
		case 2:
			strcpy(sendBuff, "GetTimeWithoutDate");
			break;
		case 3:
			strcpy(sendBuff, "GetTimeSinceEpoch");
			break;
		case 4:
			measureClientToServerDelay(connSocket, server, sendBuff);
			continue;
		case 5:
			measureRTT(connSocket, server);
			 continue;
		case 6:
			strcpy(sendBuff, "GetTimeWithoutDateOrSeconds");
			break;
		case 7:
			strcpy(sendBuff, "GetYear");
			break;
		case 8:
			strcpy(sendBuff, "GetMonthAndDay");
			break;
		case 9:
			strcpy(sendBuff, "GetSecondsSinceBeginingOfMonth");
			break;
		case 10:
			strcpy(sendBuff, "GetWeekOfYear");
			break;
		case 11:
			strcpy(sendBuff, "GetDaylightSavings");
			break;
		case 12:
			// Function to handle case 12: GetTimeWithoutDateInCity
			handleGetTimeWithoutDateInCity(sendBuff);
			break;
		case 13:
			strcpy(sendBuff, "MeasureTimeLap");
			break;
		case 14:
			// Closing connections and Winsock
			std::cout << "Time Client: Closing Connection.\n";
			closesocket(connSocket);
			WSACleanup();
			return 0;
		default:
			std::cout << "Invalid option. Please try again.\n";
			continue;
		}

		send_request(connSocket, server, sendBuff);
	}

	
}