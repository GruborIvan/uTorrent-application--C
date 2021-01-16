#include <stdlib.h>
#include <stdio.h>
#include "TCP_Methods.h"

#define FILE_SIZE 2008

bool connectClientToServer(SOCKET* connectSocket, char* Port)
{
    ClientRequest* ioc = (ClientRequest*)malloc(sizeof(ClientRequest));
    ioc->Mode = 1;
    ioc->port = atoi(Port);


    // Send an prepared message with null terminator included
    int iResult = send(*connectSocket, (char*)ioc, sizeof(ClientRequest), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(*connectSocket);
        WSACleanup();
        return false;
    }

    free(ioc);
    return true;
}

bool disconnectClientFromServer(ClientRequest* ioc, SOCKET* connectSocket, int port)
{
    ioc->Mode = 2;
    ioc->port = port;
    //memcpy_s(ioc->port,5,Port,5);

    // Send an prepared message with null terminator included
    int iResult = send(*connectSocket, (char*)ioc, sizeof(ClientRequest), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(*connectSocket);
        WSACleanup();
        return false;
    }

    printf_s("Closing client at port %d ...", ioc->port);
    return true;
}

// Dobavljanje liste dostupnih fajlova sa Server-a..
void recieveAllStoredFiles(SOCKET* connectSocket)
{
    char storedFilesOnServer[10][FILE_NAME_SIZE];
    // Retrieving file list to choose files from...
    int iResult = recv(*connectSocket, (char*)storedFilesOnServer, 10 * FILE_NAME_SIZE, 0);

    if (iResult > 0)
    {
        printf_s("Dostupni fajlovi za downloadovanje: \n");

        for (int i = 0; i < 10; i++) {
            printf_s("%s \n", storedFilesOnServer[i]);
        }
    }
}

void sendClientFileRequestToServer(SOCKET* connectSocket, ClientRequest* ioc)
{
    int bytesSent = 0;
    int iResult = 0;

    do
    {
        // Send an prepared message with null terminator included
        iResult = send(*connectSocket, (char*)ioc, sizeof(ClientRequest), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*connectSocket);
            WSACleanup();
            return;
        }

        bytesSent += iResult;
    } 
    while (bytesSent < sizeof(ClientRequest));

    free(ioc);
}

int RecieveResponseFromServer(SOCKET* connectSocket, FileResponse *response)
{
    int bytesRecieved = 0;
    int iResult = 0;

    do
    {
        iResult = recv(*connectSocket, (char*)response, sizeof(FileResponse), 0);

        bytesRecieved += iResult;
    } 
    while (bytesRecieved < sizeof(FileResponse));

    return bytesRecieved;
}

bool AskClientForFilePart(SOCKET* connectSocketToClients, char* fileName)
{
    int bytesSent = 0;
    int iResult = 0;

    do
    {
        // Send file across network & check whether it's sucessfully sent.
        iResult = send(*connectSocketToClients, fileName, FILE_NAME_SIZE, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*connectSocketToClients);
            WSACleanup();
            return false;
        }

        bytesSent += iResult;
    } 
    while (bytesSent < sizeof(FILE_PART_SIZE + 12));

    return true;
}

bool RecieveFilePartFromClient(SOCKET* connectSocketToClients, char* fullFileContent)
{
    char tempp[FILE_PART_SIZE + 10];
    strcpy(tempp,"");
    int bytesRecieved = 0;
    int iResult = 0;

    do
    {
        iResult = recv(*connectSocketToClients, tempp, FILE_PART_SIZE + 10, 0);

        if (iResult > 0)
        {
            strcat(fullFileContent,tempp);
        }
        else
        {
            return false;
        }

        bytesRecieved += iResult;
    } 
    while (bytesRecieved < FILE_PART_SIZE + 10);

    return true;
}

char* RecieveClientP2PRequest(SOCKET* socket)
{
    // Odgovara na serverske zahteve...
    char requestedFileName[FILE_NAME_SIZE];

    int iResult = recv(*socket, (char*)requestedFileName, FILE_NAME_SIZE, 0);

    if (iResult > 0) {
        return requestedFileName;
    }
    else {
        return NULL;
    }
}

void AnswerP2P_Request(char* fileContent, SOCKET* socket)
{
    int bytesSent = 0;
    int iResult = 0;

    do
    {
        // Send an prepared message with null terminator included
        iResult = send(*socket, fileContent, FILE_PART_SIZE + 10, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*socket);
            WSACleanup();
            return;
        }
        bytesSent += iResult;
    } 
    while (bytesSent < (FILE_PART_SIZE + 10));

}

// TCP METHODS CALLED BY SERVER !

int RecieveClientRequest(SOCKET* socket, ClientRequest* req)
{
    int iResult = 0;
    int bytesRecieved = 0;

    do
    {
        // Receive data until the client shuts down the connection
        iResult = recv(*socket, (char*)req, sizeof(ClientRequest), 0);

        if (iResult <= 0) {
            req = NULL;
            closesocket(*socket);
            return 0;
        }

        bytesRecieved += iResult;
    } 
    while (bytesRecieved < sizeof(ClientRequest));

    return bytesRecieved;
}

void SendServerResponseToClient(SOCKET* socket, char* response)
{
    int bytesRead = 0;
    int iResult = 0;

    do
    {
        // Send file across network & check whether it's sucessfully sent.
        iResult = send(*socket, (char*)response, sizeof(FileResponse), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*socket);
            WSACleanup();
            return;
        }

        bytesRead += iResult;
    } while (bytesRead < sizeof(FileResponse));

}

void sendAvailableFilesListToClient(SOCKET* socket, char* availableFiles)
{
    int bytesSent = 0;
    int iResult = 0;

    do
    {
        // Send file across network & check whether it's sucessfully sent.
        iResult = send(*socket, (char*)availableFiles, 10 * FILE_NAME_SIZE, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(*socket);
            WSACleanup();
            return;
        }
        bytesSent += iResult;
    } 
    while (bytesSent < (10 * FILE_NAME_SIZE));

}