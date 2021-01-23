#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "../ClientList_StaticLib/ClientList.h"
#include "../HashMap_StaticLib/HashTable.h"
#include "../TCP_Methods_StaticLibrary/TCP_Methods.h"
#include "../TCP_Methods_StaticLibrary/TCP_Methods.h"

#define RED "\033[30m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[94m"
#define RESET "\033[0m"
#define MAGNETA "\033[95m"
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define FILE_PART_SIZE 512
#define RESPONSE_SIZE 4096

// Parametri koji se salju u ClientThread iz AcceptClients, kad se poveze novi klijent..
typedef struct Parameters
{
    SOCKET acceptedSocket;
    SOCKET listenSocket;
} Parameters;

bool InitializeWindowsSockets();

bool ReadFilePart(char* fileName, int part, char* fileInput);

DWORD WINAPI ClientThread(LPVOID lpParam)
{
    Parameters *parameters = (Parameters*)lpParam;

    int iResult;
    fd_set readfds;
    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    SOCKET acceptedSocket;
    acceptedSocket = parameters->acceptedSocket;

    //free(parameters); // Oslobadjanje memorije za parametre

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(acceptedSocket,&readfds);

        iResult = select(0, &readfds, NULL, NULL, &timeVal);
        if (iResult == 0) 
        {
            continue;
        }
        else if (iResult == SOCKET_ERROR)
        {
            // Desila se greska prilikom poziva funkcije..
            printf("Greska select ClientThread..\n");
        }
        else
        {
            ClientRequest* pok = (ClientRequest*)malloc(sizeof(ClientRequest));
            int byts = RecieveClientRequest(&acceptedSocket,pok);

            if (byts > 0)
            {
                if (pok->Mode == 1)
                {
                    // Client connect!
                    int clientId = DodavanjeKlijenataUListu(acceptedSocket, pok->port);

                    char availableFiles[10][FILE_NAME_SIZE];
                    strcpy_s(availableFiles[0], "thisfile.bin");
                    strcpy_s(availableFiles[1], "novisa.bin");
                    strcpy_s(availableFiles[2], "movie.bin");
                    strcpy_s(availableFiles[3], "randomFile.bin");
                    strcpy_s(availableFiles[4], "extra.bin");
                    strcpy_s(availableFiles[5], "chromePlug.bin");
                    strcpy_s(availableFiles[6], "svasta.bin");
                    strcpy_s(availableFiles[7], "tamonekifajl.bin");
                    strcpy_s(availableFiles[8], "content.bin");
                    strcpy_s(availableFiles[9], "everything.bin");

                    sendAvailableFilesListToClient(&acceptedSocket,(char*)availableFiles);
                    free(pok);
                }
                else if (pok->Mode == 2)
                {
                    // Client Disconnect !
                    NullateClientFileHashTable(pok->port);
                    RemoveClientFromList(pok->port);
                    free(pok);
                    break;
                }
                else
                {
                    // Request for file from Client!
                    FileResponse* response = (FileResponse*)malloc(sizeof(FileResponse));

                    FILE* check;
                    fopen_s(&check, pok->fileName, "rb");

                    if (check == NULL)
                    {
                        // Ukoliko trazeni fajl nije pronadjen...
                        response->ports[0] = 404;
                        printf("Unable to find requested file.!");
                    }
                    else if (ClientHasFIle(pok->fileName, pok->port))
                    {
                        response->keep = -1;
                        fclose(check);
                    }
                    else
                    {
                        // Zatvaranje file pointer-a..
                        fclose(check);

                        // Settovanje imena fajla u response..
                        strcpy_s(response->fileName, FILE_NAME_SIZE, pok->fileName);

                        printf_s("\n*Trazeni fajl " BLUE "%s" RESET " od strane klijenta %d\n", pok->fileName, pok->port);

                        // Anuliranje svih portova..
                        for (int i = 0; i < 5; i++) {
                            response->ports[i] = 0;
                        }

                        // Read ports of clients that contain this file..
                        ReadClientFiles(response->fileName, response->ports);

                        // Ucitavanje portova klienata koji sadrze trazeni fajl!
                        for (int i = 0; i < 5; i++)
                        {
                            if (response->ports[i] == 0)
                            {
                                // Read part of the file specified with i.   File parts: 1-5  //
                                ReadFilePart(response->fileName, i + 1, response->fileParts[i]);
                            }
                        }

                        // Pronadji onaj deo fajla koji treba klijent da sacuva kod sebe...
                        response->keep = 0;
                        response->keep = AddFileToHashTable(response->fileName, pok->port);

                    }

                    // Slanje Serverskog odgovora Klijentu...
                    SendServerResponseToClient(&acceptedSocket, (char*)response);
                    free(response);
                    free(pok);
                }

            }
            else if (byts == 0)
            {
                // OVDE SE GASI KLIJENT...
                // connection was closed gracefully
                printf("Connection with client closed.\n");
                closesocket(acceptedSocket);
                return 1;
            }
            else
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(acceptedSocket);
                return 0;
            }
        }
        Sleep(500);
    }

    // Ovde dodje kada se klijent disconnectuje.

    // Odraditi gracefully exit

    return 0;
}

DWORD WINAPI AcceptClients(LPVOID lpParam)
{
    int iResult;
    int socketCount = 0;
    unsigned long mode = 1;

    fd_set readfds;
    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    DWORD dwordList[20];
    HANDLE handleList[20];

    Parameters* params = (Parameters*)lpParam;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(params->listenSocket,&readfds);

        iResult = select(0, &readfds, NULL, NULL, &timeVal);
        if (iResult == 0){
            continue;
        }
        else if (iResult == SOCKET_ERROR) {
            printf("Desila se greska prilikom poziva Thread-a..\n");
        }
        else 
        {
            params->acceptedSocket = accept(params->listenSocket, NULL, NULL);

            if (params->acceptedSocket == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(params->listenSocket);
                WSACleanup();
                break;
            }
            else
            {
                iResult = ioctlsocket(params->acceptedSocket, FIONBIO, &mode);

                // Salji ovo kao pokazivac...
                Parameters parameters;
                parameters.acceptedSocket = params->acceptedSocket;

                handleList[socketCount] = CreateThread(NULL, 0, &ClientThread, &parameters, 0, &dwordList[socketCount]);
                socketCount++;
            }

        }

        Sleep(500);
    }

    free(params);
    return 0;
}

int  main(void) 
{
    DWORD dword;
    HANDLE handle;

    // variable used to store function return value
    int iResult;

    unsigned long mode = 1;

    // Socket used for listening for clients
    SOCKET listenSocket = INVALID_SOCKET;

    // Accepted client socket..
    SOCKET acceptedSocket = INVALID_SOCKET;

    // Initializing all hash table values to NULL..
    init_hash_table();

    init_client_list();

    init_criticalSectionClientList();
    init_criticalSectionHashTable();

    // TESTING HASH FUNCTION...
    //system("COLOR B");
    printf(YELLOW " Dostupni fajlovi:   | \n" RESET);
    printf(MAGNETA " thisfile.bin" YELLOW "        | \n" RESET);
    printf(MAGNETA " novisa.bin" YELLOW "          | \n" RESET);
    printf(MAGNETA " movie.bin" YELLOW"           | \n" RESET);
    printf(MAGNETA " randomFile.bin" YELLOW"      | \n" RESET);
    printf(MAGNETA " extra.bin" YELLOW"           | \n" RESET);
    printf(MAGNETA " chromePlug.bin" YELLOW"      | \n" RESET);
    printf(MAGNETA " svasta.bin" YELLOW"          | \n" RESET);
    printf(MAGNETA " tamonekifajl.bin" YELLOW"    | \n" RESET);
    printf(MAGNETA " content.bin" YELLOW"         | \n" RESET);
    printf(MAGNETA " everything.bin" YELLOW"      | \n" RESET);
    printf(YELLOW " ---------------------\n" RESET);
    
    if(InitializeWindowsSockets() == false)
    {
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
    }
    
    // Prepare address information structures
    addrinfo *resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
                          SOCK_STREAM,  // stream socket
                          IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind( listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    /////////////////// Postavljanje u neblokirajuci rezim ////////////////////////////////
    iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
    //////////////////////////////////////////////////////////////////////////////////////

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

	printf(YELLOW "Server aktivan, ceka klijentske zahteve!.\n" RESET);

    Parameters* params = (Parameters*)malloc(sizeof(Parameters));
    params->acceptedSocket = acceptedSocket;
    params->listenSocket = listenSocket;

    //////////////////////////////////////////////////////////////////
    handle = CreateThread(NULL, 0, &AcceptClients, params, 0, &dword);
    //////////////////////////////////////////////////////////////////

    int end;

    while (1)
    {
        printf_s("Unesite 0 kako biste ugasili server: ");
        scanf_s("%d",&end);
        if (end == 0) {
            printf_s("Kraj! \n");
            break;
        }
    }

    free(params);
    ClearHashTable();
    EmptyClientList();

    // Pokretanje gasenja ostalih handlova na niti 2..

    CloseHandle(handle);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();
    return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
	return true;
}

bool ReadFilePart(char* fileName, int part,char* fileInput)
{
    FILE* fptr;
    char fullUrl[200] = "D:\\Industrijski Komunikacioni Protokoli\\Mrezno\\UDP_TCP\\WinSock_TCP_Blocking\\WinSockServer\\";
    strcat_s(fullUrl, fileName);

    // ----------------------------------------------------------------------------------
               // CITANJE SADRZAJA FAJLA..
    if ((fptr = fopen(fileName, "rb")) == NULL)
    {
        printf_s("Unable to open file.\n");
        printf_s("Cannot locate file.\n");
        return false;
    }

    char readBuffer[FILE_SIZE];

    // Citanje celokupnog fajla..
    const size_t fileSize = fread(readBuffer, FILE_SIZE, 1, fptr);
    fclose(fptr);

    // Pravljenje stringa, skracivanje buffer-a.
    char ch = '0';
    int cnt = 0;
    for (int i = 0; i < FILE_SIZE; i++)
    {
        ch = readBuffer[i];
        if (ch != '0' && ch != '1' && ch != ' ')
        {
            cnt = i;
            readBuffer[i] = '\0';
            break;
        }
    }

    if (part == 5)
    {
        int skip = 4 * (cnt / 5);
        int upis = (cnt / 5);
        memcpy_s(fileInput,upis,readBuffer + skip,upis);
    }
    else
    {
        int skip = (part - 1) * (cnt / 5);
        int upis = cnt / 5;
        memcpy_s(fileInput, upis, readBuffer + skip, upis);
    }

    return true;
}