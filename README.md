# Time Server/Client Project

This project implements a Time Server and Client application using C++ and Windows Sockets (Winsock). The server provides various time-related services, and the client can request these services.

## Features

### Server
- Handles multiple time-related requests
- Supports custom time formats
- Provides time information for different cities
- Measures time lapses
- Estimates client-to-server delay

### Client
- Offers a menu-driven interface for different time requests
- Supports measuring Round Trip Time (RTT)
- Allows choosing specific cities for time requests

## Requirements

- Windows operating system
- C++ compiler (compatible with C++11 or later)
- Winsock library (usually included with Windows)

## Compilation

Compile both the server and client programs separately:

```
g++ server.cpp -o server.exe -lws2_32
g++ client.cpp -o client.exe -lws2_32
```

## Usage

1. Start the server:
   ```
   ./server.exe
   ```

2. In a separate terminal, start the client:
   ```
   ./client.exe
   ```

3. Follow the on-screen menu in the client to send various time-related requests to the server.

## Available Commands

The client supports the following commands:

1. GetTime
2. GetTimeWithoutDate
3. GetTimeSinceEpoch
4. GetClientToServerDelayEstimation
5. MeasureRTT
6. GetTimeWithoutDateOrSeconds
7. GetYear
8. GetMonthAndDay
9. GetSecondsSinceBeginingOfMonth
10. GetWeekOfYear
11. GetDaylightSavings
12. GetTimeWithoutDateInCity
13. MeasureTimeLap
14. End the process

## Notes

- The server runs on localhost (127.0.0.1) and uses port 27015.
- The client allows selecting from predefined cities (Doha, Prague, New York, Berlin) for city-specific time requests.
- The MeasureTimeLap feature starts a measurement and reports the time lapse on subsequent calls.

