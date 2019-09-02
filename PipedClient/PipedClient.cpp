#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <vector>
#include <conio.h>
#include "Data.hpp"
#define BUFSIZE 512

unsigned int Menu() {
	std::cout << "\n\n====== CLIENT MENU ======" << std::endl;

	std::cout << "1 - Insert data" << std::endl;
	std::cout << "2 - Read all data from server" << std::endl;
	std::cout << "0 - Close client" << std::endl;

	std::cout << "Choose your option: ";
	int answer = 0;
	std::cin >> answer;
	return answer;
}

int _tmain(int argc, TCHAR * argv[]) {
	HANDLE hPipe;
	//LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
	LPTSTR lpvMessage = new TCHAR[100];
	std::string msgstr = "Default msg from client";
	std::copy(msgstr.begin(), msgstr.end(), lpvMessage);
	TCHAR chBuf[BUFSIZE];
	BOOL fsuccess = false;
	LPTSTR lpszPipename = new TCHAR[100];
	std::string pipestr = "\\\\.\\pipe\\mynamedpipe2";  //local server
	std::copy(pipestr.begin(), pipestr.end(), lpszPipename);

	DWORD  cbRead, cbToWrite, cbWritten, dwMode;
	if (argc > 1)
		lpvMessage = argv[1];

	std::cout << "===================== CLIENT =================================\n";

	while (1)
	{
		hPipe = CreateFile(
			"\\\\.\\pipe\\mynamedpipe2",   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file

	   // Break if the pipe handle is valid

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		//Exit if some error occurs other than ERROR_PIPE_BUSY

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return -1;
		}

	}

	//Now the pipe is connected; 

	std::cout << "Connected to Server";

	//change to message_read_mode from the default byte read mode 

	dwMode = PIPE_READMODE_MESSAGE;

	fsuccess = SetNamedPipeHandleState(
		hPipe,           //pipe handle
		&dwMode,         //new pipe mode
		NULL,            //don't set maximum bytes
		NULL);           //don't set maximum time

	if (!fsuccess)
	{
		_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	int option = 0, totalBytesReceived = 0;
	
	do
	{
		option=Menu();
		
		// write option value to the server
		cbToWrite = (std::size(std::to_string(option)) + 1) * sizeof(TCHAR);
		fsuccess = WriteFile(
			hPipe,
			std::to_string(option).c_str(),
			cbToWrite,
			&cbWritten,
			NULL);

		if (!fsuccess) {
			_tprintf(TEXT("WriteFile to pipe failed Couldn't write option value. GLE=%d\n"), GetLastError());
			return -1;
		}


		switch (option)
		{
		case 1:
			std::cin.ignore(256, '\n');
			std::cout << "\n\"Insert data\" item chosen\n";
			//send a message to pipe server
			std::cout << "Message to be sent : ";
			std::getline(std::cin, msgstr);

			//lpvMessage = new TCHAR[sizeof(msgstr)];
			//std::copy(msgstr.begin(), msgstr.end(), lpvMessage);

			cbToWrite = (std::size(msgstr) + 1) * sizeof(TCHAR);
			std::cout << "\nSending " << cbToWrite << " byte message : " << msgstr;
			std::cout << "\n";
			fsuccess = WriteFile(
				hPipe,
				msgstr.c_str(),
				cbToWrite,
				&cbWritten,
				NULL);

			if (!fsuccess) {
				_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
				return -1;
			}

			printf("\n=> Message sent to server.\n");

			break;

		case 2:
			std::cout << "\n\"Read all data from server\" item chosen";
			printf("\nReceiving data from server..\n");
			do
			{
				// Read from the pipe. 

				fsuccess = ReadFile(
					hPipe,    // pipe handle 
					chBuf,    // buffer to receive reply 
					BUFSIZE * sizeof(TCHAR),  // size of buffer 
					&cbRead,  // number of bytes read 
					NULL);    // not overlapped 

				if (!fsuccess && GetLastError() != ERROR_MORE_DATA)
					break;

				// No data on server
				if (chBuf[0] == '\0' && totalBytesReceived == 0)
				{
					break;
				}

				// Empty message by server indicating all data has been transferred
				if (chBuf[0] != '\0')
				{
					_tprintf(TEXT("\"%s\"\n"), chBuf);
					 totalBytesReceived += cbRead;
				}

			} while (chBuf[0] != '\0');  // repeat loop if ERROR_MORE_DATA 

			if (!fsuccess)
			{
				_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
				return -1;
			}

			else
			{
				if (totalBytesReceived == 0)
					std::cout << "\nNo data on server. Kindly insert data (option 1) and try again.\n";
				else
					std::cout << "\n=> Received all data successfully.\nTotal bytes received=" << totalBytesReceived << std::endl;
			}
			
			break;

		case 0:
			std::cout << "\n\"Close client\" option chosen\n";
			printf("\n<Closing connection, press ENTER to terminate connection and exit>");
			_getch();
			CloseHandle(hPipe);
			break;

		default:
			std::cout << "\nInvalid option. Please try again.\n";
		}

	} while (option);

	
	return 0;

}