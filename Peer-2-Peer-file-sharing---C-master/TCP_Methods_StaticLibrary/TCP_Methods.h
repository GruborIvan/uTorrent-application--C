#pragma once
#include <WinSock2.h>

#define FILE_NAME_SIZE 24
#define FILE_PART_SIZE 512

typedef struct ClientRequest
{
    char fileName[FILE_NAME_SIZE];
    int Mode; // Moze biti 0,1,2  ||  0 - Regularno slanje |  1 - Connect |  2 - Disconnect |
    int port; //Klijent salje svoj Port..
} ClientRequest;

typedef struct FileResponse
{
    char fileName[FILE_NAME_SIZE];
    int keep; //Oznacava deo fajla koji treba sacuvati.
    int ports[5]; // Portovi
    char fileParts[5][FILE_PART_SIZE]; //Delovi fajlova
} FileResponse;

// CLIENT OPERATIONS...

// Returns true if connection is successfull
bool connectClientToServer(SOCKET* connectSocket,char* Port);

bool disconnectClientFromServer(ClientRequest *ioc,SOCKET *connectSocket,int port);

void recieveAllStoredFiles(SOCKET *connectSocket);

void sendClientFileRequestToServer(SOCKET* connectSocket,ClientRequest* ioc);

int RecieveResponseFromServer(SOCKET *connectSocket, FileResponse* response);

bool AskClientForFilePart(SOCKET *connectSocketToClients, char* fileName);

bool RecieveFilePartFromClient(SOCKET* connectSocketToClients, char* fullFileContent);

void RecieveClientP2PRequest(SOCKET *socket, char* buff);

void AnswerP2P_Request(char *fileContent,SOCKET *socket);


// SERVER OPERATIONS...
int RecieveClientRequest(SOCKET *socket, ClientRequest* req);

void SendServerResponseToClient(SOCKET* socket, char* response);

void sendAvailableFilesListToClient(SOCKET* socket,char* availableFiles);