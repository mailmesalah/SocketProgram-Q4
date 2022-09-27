#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define MY_PORT		8888
#define MAXBUF		256

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET serverSocket, clientSocket, clientSocketList[30],s;
	struct sockaddr_in serverAddress, clientAddress;
	int sizeOfAddress, incomingMsgLen,i, maxClients = 30, activity;
	char message[MAXBUF],reverseMsg[MAXBUF];

	//set of socket descriptors
	fd_set readfds;
	//1 extra for null character, string termination
	char *buffer;
	buffer = (char*)malloc((MAXBUF + 1) * sizeof(char));


	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		WSACleanup();
		exit(errno);
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(MY_PORT);

	//Bind
	if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		WSACleanup();
		exit(errno);
	}

	puts("Bind done");


	//Listen to incoming connections
	listen(serverSocket, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	sizeOfAddress = sizeof(struct sockaddr_in);

	//Handling multiple connection
	//Initialising clients to 0
	for (i = 0; i < 30; i++)
	{
		clientSocketList[i] = 0;
	}

	while (TRUE)
	{
		//clear the socket fd set
		FD_ZERO(&readfds);
		//add master socket to fd set
		FD_SET(serverSocket, &readfds);

		//add child sockets to fd set
		for (i = 0; i < maxClients; i++)
		{
			s = clientSocketList[i];
			if (s > 0)
			{
				FD_SET(s, &readfds);
			}
		}

		//wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
		activity = select(0, &readfds, NULL, NULL, NULL);
		if (activity == SOCKET_ERROR)
		{
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(serverSocket, &readfds))
		{
			if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (int *)&sizeOfAddress))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//Display client IP Address and Port No			
			puts("New Client is Added\n");
			//, inet_ntop(client.sin_addr)
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n", clientSocket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

			//add new socket to array of sockets
			for (i = 0; i < maxClients; i++)
			{
				if (clientSocketList[i] == 0)
				{
					clientSocketList[i] = clientSocket;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}
		}


		//else its some IO operation on some other socket :)
		for (i = 0; i < maxClients; i++)
		{
			s = clientSocketList[i];
			//if client presend in read sockets             
			if (FD_ISSET(s, &readfds))
			{
				//get details of the client
				getpeername(s, (struct sockaddr*)&clientAddress, (int*)&sizeOfAddress);

				//Check if it was for closing , and also read the incoming message
				//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
				incomingMsgLen = recv(s, message, MAXBUF, 0);

				if (incomingMsgLen == SOCKET_ERROR)
				{
					int error_code = WSAGetLastError();
					if (error_code == WSAECONNRESET)
					{
						//Somebody disconnected , get his details and print
						//printf("Host disconnected unexpectedly , ip %s , port %d \n", inet_ntop(AF_INET, &client.sin_addr, buffer, sizeof(buffer)), ntohs(client.sin_port));

						//Close the socket and mark as 0 in list for reuse
						closesocket(s);
						clientSocketList[i] = 0;
					}
					else
					{
						printf("recv failed with error code : %d", error_code);
					}
				}
				if (incomingMsgLen == 0)
				{
					//Somebody disconnected , get his details and print
					//printf("Host disconnected , ip %s , port %d \n", inet_ntop(AF_INET, &client.sin_addr, buffer, sizeof(buffer)), ntohs(client.sin_port));

					//Close the socket and mark as 0 in list for reuse
					closesocket(s);
					clientSocketList[i] = 0;
				}

				//Echo back the message that came in
				else
				{
					//Reply to the client
					message[incomingMsgLen] = '\0';
					//Print length of the message
					printf("Length of the incoming message is %d\n",incomingMsgLen);
					
					for (i = 0; i<incomingMsgLen; i++)
					{
						if (isalpha(message[i]))
						{
							if (IsCharLower(message[i])) {
								message[i] = _toupper(message[i]);
							}
						}
					}

					send(s, _strrev(message), incomingMsgLen, 0);
				}
			}
		}


	}

	
	closesocket(s);
	WSACleanup();

	return 0;
}
