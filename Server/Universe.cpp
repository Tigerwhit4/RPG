#include "StdAfx.h"
#include "Universe.h"

Universe* Universe::instance;

Universe::Universe(void)
{
	instance = this;
}

Universe::~Universe(void)
{
	instance = NULL;
}

void Universe::Run()
{
	bool continueFlag; //Continue game or not
	char inPacket[256]; //Holds the input packet
	char outPacket[256]; //Holds the output packet
	int iResult; //Input/output packet length
	ServerClientSocket** clients; //Connection sockets to the clients
	int clientsCount; //The size of 'clients'
	SOCKET tmp; //Socket for accepting clients. Used to initialize the element of 'clients'

	bool isOnline;
	int ci, i, j, result;
	char query[256];
	std::map<std::string, std::string> strings;
	std::map<std::string, int> integers;

	continueFlag = true;
	clientsCount = 0;
	clients = NULL;

	game = new Game("testgame");
	printf("Game %s initialized\n", game->name);
	serverSocket = new ServerSocket("3127");
	
	while (continueFlag)
	{
		//Accepting connections from clients
		tmp = accept(serverSocket->connectSocket, NULL, NULL);
		if (tmp != INVALID_SOCKET)
		{//Client connected
			clientsCount++;
			clients = (ServerClientSocket**)realloc(clients, clientsCount * sizeof(ServerClientSocket*));
			clients[clientsCount - 1] = new ServerClientSocket(tmp);
			printf("Client %d: Connected\n", clientsCount - 1);
		}

		//Receiving packets from connected clients
		for (ci = 0; ci < clientsCount; ci++)
		{
			iResult = clients[ci]->Receive(inPacket);
			if (iResult)
			{
				if (iResult > 0)
				{//Packet received
					printf("Packet received\n");
					switch (inPacket[2])
					{
						case LogIn:
							printf("Client %d trying to log in:\nLogin: %s\nPassword: %s\n", ci, inPacket + 3, inPacket + 3 + strlen(inPacket + 3) + 1);
							
							if (clients[ci]->character)
							{
								printf("Client is already logged in\n");
								break;
							}

							isOnline = false;
							for (i = 0; i < game->data->locationsCount; i++)
								for (j = 0; j < game->data->locations[i]->currentCharactersCount; j++)
									if (!strcmp(game->data->locations[i]->currentCharacters[j]->login, inPacket + 3))
									{
										isOnline = true;
										i = game->data->locationsCount;
										break;
									}
							if (isOnline)
							{
								printf("Character is currently in use\n");
								break;
							}
						
							sprintf(query, "SELECT * FROM CurrentCharacter WHERE login='%s';", inPacket + 3);
							result = SqliteGetRow(game->db, query, strings, integers);
							if (result < 0)
							{
								strings.clear();
								integers.clear();
								break;
							}
							else if (result == 0)
							{
								printf("Character does not exist\n");
								strings.clear();
								integers.clear();
								break;
							}
							else
							{
								if (strcmp(inPacket + 3 + strlen(inPacket + 3) + 1, strings["password"].c_str()))
								{
									printf("Password does not match this account\n");
									strings.clear();
									integers.clear();
									break;
								}
								for (i = 0; i < game->data->locationsCount; i++)
									if (game->data->locations[i]->id == integers["locationId"])
									{
										CurrentCharacter* newCurrentCharacter = new CurrentCharacter(strings, integers);
										if (game->data->locations[i]->AddCurrentCharacter(newCurrentCharacter))
											delete newCurrentCharacter;
										else
										{
											clients[ci]->character = newCurrentCharacter;
											printf("Character %s logged in\n", inPacket + 3);
										}
										break;
									}
							}
							strings.clear();
							integers.clear();
							break;
						case LogOut:
							break;
					}
				}
				else if (iResult == -1)
				{//Client disconnected
					//TODO: data->UnspawnCharacter(clients[ci]->character)
					delete clients[ci]->character;

					delete clients[ci];
					clientsCount--;
					
					//Moving the last client to disconnected client position
					clients[ci] = clients[clientsCount];
					clients = (ServerClientSocket**)realloc(clients, clientsCount * sizeof(ServerClientSocket*));
					
					printf("Client %d: Disconnected\n", ci);
				}
				else
				{//Wrong packet from the server
					printf("Warning! Wrong packet from client %d\n", ci);
				}
			}
		}

		//Simulating world
		/*
		printf("\nCharacters online:\n");
		int i, j;
		for (i = 0; i < game->data->locationsCount; i++)
		{
			for (j = 0; j < game->data->locations[i]->currentCharactersCount; j++)
			{
				printf("\t%s\n", game->data->locations[i]->currentCharacters[j]->login);
			}
		}
		*/
		//TEST
		for (ci = 0; ci < clientsCount; ci++)
		{
			//outPacket[1] = '\0';
			strcpy(outPacket + 2, "test packet");
			SetPacketLength(outPacket, strlen(outPacket + 2) + 1);
			//clients[cClient]->Send(outPacket);
		}
		//Sleep(200);
	}
	
	delete serverSocket;
	delete game;

	system("pause");
}
