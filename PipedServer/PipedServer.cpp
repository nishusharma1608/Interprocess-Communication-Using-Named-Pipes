#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <vector>
#include "Data.hpp"
#define BUFSIZE 512
#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif
DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);
std::vector<Data> itemList;
std::vector<Data> getItemList() { return itemList; };
VOID sendAllData();
VOID insertItemOnList(Data);

bool writePipeData(std::string data);
HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;

int _tmain(VOID)
{
	TCHAR request_buffer[100];
	TCHAR* request_buffer_ptr = request_buffer;
BOOL fConnected = FALSE;
LPTSTR lpszPipename = new TCHAR[100];
std::string pipestr = "\\\\.\\pipe\\mynamedpipe2";
std::copy(pipestr.begin(), pipestr.end(), lpszPipename);
//LPTSTR lpszPipename = TEXT("");

DWORD dwThreadId = 0;

// The main loop creates an instance of the named pipe and 
// then waits for a client to connect to it. When the client 
// connects, a thread is created to handle communications 
// with that client, and this loop is free to wait for the
// next client connect request. It is an infinite loop.

std::cout << "======================== SERVER ============================\n";

for(;;)
{
printf("\nPipe Server :Main Thread is waiting for connection with client ");
hPipe = CreateNamedPipe(
    "\\\\.\\pipe\\mynamedpipe2",
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_MESSAGE|
    PIPE_READMODE_MESSAGE|
    PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES,
    BUFSIZE,
    BUFSIZE,
    0,
    NULL);

if(hPipe == INVALID_HANDLE_VALUE){
  _tprintf(TEXT("Error : Create Named Pipe failed, GLE=%d. \n"), GetLastError());  
    return -1;
}


 fConnected = ConnectNamedPipe(hPipe, NULL) ? 
         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);


if (fConnected)
{
    printf("Client connected, creating a processing thread. \n");

    // Create a thread for this client

    hThread = CreateThread(
        NULL,
        0,
        InstanceThread,
        (LPVOID)hPipe,
        0,
        &dwThreadId);
    
    if (hThread==NULL){
        _tprintf(TEXT("Create thread method failed, GLE=%d"),GetLastError());
        return -1;
    }

    else CloseHandle(hThread);
}

else
// The client could not connect, so close the pipe.
    CloseHandle(hPipe);

}

return 0;




}


DWORD WINAPI InstanceThread(LPVOID lpvParam)
//This is a thread processing function to read from and reply
// to a client via the open pipe connection passed from the main loop.
{
    HANDLE hHeap = GetProcessHeap();
    /*TCHAR* pchRequest = (TCHAR*) HeapAlloc(hHeap,0,BUFSIZE*sizeof(TCHAR));
    TCHAR* pchReply = (TCHAR*) HeapAlloc(hHeap,0,BUFSIZE*sizeof(TCHAR));
*/
	TCHAR request_buffer[1000];
	TCHAR* request_buffer_ptr = request_buffer;
	TCHAR response_buffer[1000];
	TCHAR* response_buffer_ptr = response_buffer;


   DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0; 
   BOOL fSuccess = FALSE;
   HANDLE hPipe  = NULL;


   if (lpvParam == NULL)
   {
       printf( "\nERROR - Pipe Server Failure:\n");
       printf( "   InstanceThread got an unexpected NULL value in lpvParam.\n");
       printf( "   InstanceThread exitting.\n");
       return (DWORD)-1;
   }

   if (request_buffer_ptr == NULL)
   {
       printf( "\nERROR - Pipe Server Failure:\n");
       printf( "   InstanceThread got an unexpected NULL heap allocation.\n");
       printf( "   InstanceThread exitting.\n");
       return (DWORD)-1;
   }

   if (response_buffer_ptr == NULL)
   {
       printf( "\nERROR - Pipe Server Failure:\n");
       printf( "   InstanceThread got an unexpected NULL heap allocation.\n");
       printf( "   InstanceThread exitting.\n");
       return (DWORD)-1;
   }

   // Print verbose messages. In production code, this should be for debugging only.
   printf("InstanceThread created, receiving and processing messages.\n");


   
   hPipe = (HANDLE) lpvParam; 
   int option = 99;
// Loop until done reading
   while (option) 
   { 
   // Read client requests from the pipe. This simplistic code only allows messages
   // up to BUFSIZE characters in length.
	   
	   fSuccess = ReadFile(
		   hPipe,        // handle to pipe 
		   request_buffer_ptr,    // buffer to receive data 
		   BUFSIZE * sizeof(TCHAR), // size of buffer 
		   &cbBytesRead, // number of bytes read 
		   NULL);        // not overlapped I/O 

	   if (!fSuccess || cbBytesRead == 0)
	   {
		   if (GetLastError() == ERROR_BROKEN_PIPE)
		   {
			   _tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
		   }
		   else
		   {
			   _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
		   }
		   break;
	   }

	   bool val, result;
	   DWORD cbToWrite = 0, numBytesWritten = 0;
	   int totalBytesSent = 0;
	   std::vector<Data> serverData;
	   std::string msgstr;

	   
	   option = request_buffer[0] - '0';
	   switch (option)
	   {
	   case 1:
		   std::cout << "\n=> Client wants to insert data\n";
		   fSuccess = ReadFile(
			   hPipe,        // handle to pipe 
			   request_buffer_ptr,    // buffer to receive data 
			   BUFSIZE * sizeof(TCHAR), // size of buffer 
			   &cbBytesRead, // number of bytes read 
			   NULL);        // not overlapped I/O 

		   if (!fSuccess || cbBytesRead == 0)
		   {
			   if (GetLastError() == ERROR_BROKEN_PIPE)
			   {
				   _tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
			   }
			   else
			   {
				   _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			   }
			   break;
		   }

		   insertItemOnList(Data(std::string(request_buffer)));
		   std::cout << "Message received : " << std::string(request_buffer) << std::endl;

		   break;

	   case 2:
		   std::cout << "\n=> Client requested for all data to be sent\n";
		   if (getItemList().empty())
		   {
			   result = ConnectNamedPipe(hPipe, NULL);
			   std::cout << "Can't complete operation. No data available.\n";
			   msgstr = "";
			   cbToWrite = 1;
			   result = WriteFile(
				   hPipe,
				   msgstr.c_str(),
				   cbToWrite,
				   &numBytesWritten,
				   NULL
			   );
			   
		   }

		   else
		   {
			   // Process the incoming message.
			   GetAnswerToRequest(request_buffer_ptr, response_buffer_ptr, &cbReplyBytes);

			   // Write the reply to the pipe. 
				  //sendAllData();
			   std::cout << "\nSending data to the client..\n";
			   //itemList.push_back(Data("That's all folks!"));

			   serverData = getItemList();

			   result = ConnectNamedPipe(hPipe, NULL);

			   while (!serverData.empty()) {
				   std::cout << serverData.back() << std::endl;
				   msgstr = serverData.back().toString();
				   cbToWrite = (std::size(msgstr) + 1) * sizeof(TCHAR);

				   result = WriteFile(
					   hPipe,
					   msgstr.c_str(),
					   cbToWrite,
					   &numBytesWritten,
					   NULL
				   );

				   if (result) {
					   std::cout << "Bytes sent: " << numBytesWritten << std::endl;
					   val = true;
					   totalBytesSent += numBytesWritten;
				   }
				   else {
					   std::cout << "Failed to send data." << std::endl;
					   val = false;
					   break;
				   }
				   serverData.pop_back();

			   }

			   if (val == true)
			   {
				   // send an empty string to indicate successfull data transfer
				   msgstr = "";
				   result = WriteFile(
					   hPipe,
					   msgstr.c_str(),
					   cbToWrite,
					   &numBytesWritten,
					   NULL
				   );

				   std::cout << "=>Sent all data successfully.\nTotal bytes sent=" << totalBytesSent << std::endl;
			   }
		   }

		   break;

	   case 0:
			// Flush the pipe to allow the client to read the pipe's contents 
			// before disconnecting. Then disconnect the pipe, and close the 
			// handle to this pipe instance. 

		   FlushFileBuffers(hPipe);
		   DisconnectNamedPipe(hPipe);
		   CloseHandle(hPipe);

		   printf("InstanceThread exitting.\n");
		   break;

	   default:
		   break;
	   }

      
	  
   

      //fSuccess = WriteFile( 
      //   hPipe,        // handle to pipe 
      //   pchReply,     // buffer to write from 
      //   cbReplyBytes, // number of bytes to write 
      //   &cbWritten,   // number of bytes written 
      //   NULL);        // not overlapped I/O 

      //if (!fSuccess || cbReplyBytes != cbWritten)
      //{   
      //    _tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError()); 
      //    break;
      //}
  }

 
   return 1;
}

VOID GetAnswerToRequest( LPTSTR pchRequest, 
                         LPTSTR pchReply, 
                         LPDWORD pchBytes )
// This routine is a simple function to print the client request to the console
// and populate the reply buffer with a default data string. This is where you
// would put the actual client request processing code that runs in the context
// of an instance thread. Keep in mind the main thread will continue to wait for
// and receive other client connections while the instance thread is working.
{
    
    // Check the outgoing message to make sure it's not too long for the buffer.
    if (FAILED(StringCchCopy( pchReply, BUFSIZE, TEXT("default answer from server") )))
    {
        *pchBytes = 0;
        pchReply[0] = 0;
        printf("StringCchCopy failed, no outgoing message.\n");
        return;
    }
    *pchBytes = (lstrlen(pchReply)+1)*sizeof(TCHAR);
}

void insertItemOnList(Data item) {
	itemList.push_back(item);
}


//void sendAllData() {
//	//CloseHandle(hPipe);
//	std::vector<Data> serverData = getItemList();
//	bool val=false;
//	
//	while (!serverData.empty()) {
//		std::cout << "\npain :/ " << serverData.back() << std::endl;
//		val = writePipeData(serverData.back().toString());
//		serverData.pop_back();
//	}
//	writePipeData("");
//}
//
//bool writePipeData(std::string data) {
//
//	try
//	{
//		LPTSTR lpvMessage = new TCHAR[100];
//		std::string msgstr = data;
//		std::copy(msgstr.begin(), msgstr.end(), lpvMessage);
//		//sizeof(data) * sizeof(char)
//		DWORD cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
//		DWORD numBytesWritten = 0;
//
//		BOOL result = ConnectNamedPipe(hPipe, NULL);
//		result = WriteFile(
//			hPipe,
//			data.c_str(),
//			cbToWrite,
//			&numBytesWritten,
//			NULL
//		);
//
//
//
//		if (result) {
//			std::cout << "Bytes sent: " << numBytesWritten << std::endl;
//			return true;
//		}
//		else {
//			std::cout << "Failed to send data." << std::endl;
//			return false;
//		}
//	}
//	catch (const std::exception& exec)
//	{
//		std::cout << exec.what();
//	}
//
//}
//


