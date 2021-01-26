#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <cstdio>

#include "../ClientOperations_StaticLib/ClientFiles.h"
#include "../TCP_Methods_StaticLibrary/TCP_Methods.h"

#define BUFFER_SIZE 128
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

#define FILE_SIZE 2008
#define FILE_NAME_SIZE 24
#define FILE_PART_SIZE 512
#define HASH_TABLE_SIZE 15
#define RESPONSE_SIZE 4096

typedef struct ParametersListen
{
    SOCKET listenSocketShare;
    SOCKET acceptedSocketShare;
    int port;
    bool IsAlive;
} ParametersListen;

DWORD WINAPI ClientThread(LPVOID lpParam) 
{
    int iResult;
    bool* IsAlive = (bool*)&((ParametersListen*)lpParam)->IsAlive;

    ParametersListen parameters = *(ParametersListen*)lpParam;

    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    char port[6];
    _itoa_s(parameters.port,port,10);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    parameters.listenSocketShare = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (parameters.listenSocketShare == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    iResult = bind(parameters.listenSocketShare, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(parameters.listenSocketShare);
        WSACleanup();
        return 1;
    }

    unsigned long mode = 1;
    freeaddrinfo(resultingAddress);

    // Postavljanje u neblokirajuci rezim..
    iResult = ioctlsocket(parameters.listenSocketShare, FIONBIO, &mode);

    iResult = listen(parameters.listenSocketShare, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(parameters.listenSocketShare);
        WSACleanup();
        return 1;
    }

    fd_set readfds;
    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    while (*IsAlive)
    {
        FD_ZERO(&readfds);
        FD_SET(parameters.listenSocketShare, &readfds);

        iResult = select(0, &readfds, NULL, NULL, &timeVal);
        if (iResult == 0) {
            continue;
        }
        else if (iResult == SOCKET_ERROR) {
            printf("Desila se greska prilikom poziva Thread-a..\n");
        }
        else
        {
            parameters.acceptedSocketShare = accept(parameters.listenSocketShare, NULL, NULL);

            if (parameters.acceptedSocketShare == INVALID_SOCKET)
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(parameters.listenSocketShare);
                WSACleanup();
                return 1;
            }
            else
            {
                char reqFile[FILE_NAME_SIZE];
                strcpy(reqFile, RecieveClientP2PRequest(&parameters.acceptedSocketShare)); // Primanje P2P zahteva!

                if (reqFile == nullptr) {
                    printf_s("Neuspesno preuzimanje P2P zahteva!");
                    continue;
                }

                int id = hash(reqFile);
                FileKeep fkp = *GetKeptFileById(id); // Dobavljanje dela fajla koji se cuva na drugom klijentu!

                AnswerP2P_Request(fkp.filePartContent,&parameters.acceptedSocketShare); // Odgovor na P2P zahtev!
            }
        }
        //Sleep(1000);
    }

    return 0;
}

int __cdecl main(int argc, char **argv) 
{
    char Port[6];
    int port;
    int fileSize = 0;
    DWORD dword;
    HANDLE handle;

    // variable used to store function return value
    int iResult = 0;

    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    SOCKET connectSocketToClients = INVALID_SOCKET; // Socket za iniciranje P2P konekcije!

    SOCKET listenSocketShare = INVALID_SOCKET;     // Socket za slusanje P2P zahteva!
    SOCKET acceptedSocketShare = INVALID_SOCKET;     // Accepted socket za P2P!

    // Inicijalizacija Hash tabele za cuvanje delova podataka & inic kriticne sekcije!
    init_fileKeep_table();
    init_CriticalSection(); 
    
    do {
        printf_s("Unesite Port klijenta: ");
        scanf("%s", Port);                   // Unos Port-a dok ne bude 5-cifreni broj!
    } 
    while (atoi(Port) < 10000 || atoi(Port) > 99999);

    if(InitializeWindowsSockets() == false) { return 1;}

    connectSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); // Kreiranje Socket-a za povezivanje na Server!

    if (connectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(DEFAULT_PORT); // Kreiranje i inicijalizacija adresne strukture!

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server. Reason: %ld \n",WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
    }


    // -----------------------------------------------------------------
    // ------------------CONNECTING CLIENT TO SERVER--------------------
    port = atoi(Port);

    bool success = connectClientToServer(&connectSocket, Port);
    if (!success) {
        printf_s("Neuspesno konektovanje na server!");
    }
  
    recieveAllStoredFiles(&connectSocket);
    // ========================================


    // OPERNING CLIENT LISTENING THREAD...
    ParametersListen* parameters = (ParametersListen*)malloc(sizeof(ParametersListen));
    parameters->acceptedSocketShare = acceptedSocketShare;
    parameters->listenSocketShare = listenSocketShare;
    parameters->port = port;
    parameters->IsAlive = true;

    // Pokretanje Thread-a za osluskivanje pristiglih zahteva za delove fajlova... (Podizanje klijent-serverskog Socket-a).
    handle = CreateThread(NULL, 0, &ClientThread, parameters, 0, &dword);

    // --------------------------------------------

    while (1)
    {
        ClientRequest* ioc = (ClientRequest*)malloc(sizeof(ClientRequest));

        printf_s("\nUnesite naziv trazenog fajla: [type exit to close]:  ");  // Pitanje za FileName koji se trazi.
        scanf("%s",ioc->fileName);

        if (strcmp(ioc->fileName, "exit") == 0)
        {
            // ------------- DISCONNECTING CLIENT-----------------
            disconnectClientFromServer(ioc,&connectSocket,port);
            CloseClientSession();
            parameters->IsAlive = false;
            free(ioc);
            break;
        }
        else
        {
            ioc->Mode = 0;
            ioc->port = port;
        }

        // Slanje zahteva serveru za fajl...
        sendClientFileRequestToServer(&connectSocket,ioc);
        free(ioc);

        // Dobijanje odgovora od servera...
        FileResponse* srvResponse = (FileResponse*)malloc(sizeof(FileResponse));
        iResult = RecieveResponseFromServer(&connectSocket,srvResponse);
        
        if (iResult > 0)
        {
            char fullFileContent[FILE_SIZE];
            fullFileContent[0] = '\0';

            if (srvResponse->ports[0] == 404) {
                // Desava se ukoliko trazeni fajl ne postoji..
                printf_s("File not found or doesn't exist! \n");
                continue;
            }
            
            if (srvResponse->keep == -1) {
                printf_s("Client has alredy downloaded this file. \n");
                continue;
            }

            printf("\n------------------------------------------\n");

            for (int i = 0; i < 5; i++)
            {
                if (srvResponse->ports[i] == 0)
                {
                    TrimContent(srvResponse->fileParts[i]);

                    strcat_s(fullFileContent,srvResponse->fileParts[i]);
                    fileSize = TrimContent(fullFileContent);

                    // Ako je srvResponse.keep == -1, znaci da ne cuvamo nijedan deo fajla..
                    if (srvResponse->keep == (i + 1))
                    {
                        int id = hash(srvResponse->fileName);
                        
                        FileKeep* keepf = (FileKeep*)malloc(sizeof(FileKeep));
                        strcpy_s(keepf->fileName,FILE_NAME_SIZE,srvResponse->fileName);
                        strcpy_s(keepf->filePartContent,FILE_PART_SIZE + 10,srvResponse->fileParts[i]);
                        
                        AddToKeptTable(id,keepf);
                        printf("Deo fajla %d je poslat sa servera! - Keep this one \n", i + 1);
                    }
                    else
                    {
                        printf("Deo fajla %d je poslat sa servera! \n", i + 1);
                    }
                }
                else
                {   
                    connectSocketToClients = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //Socket za trazenje dela fajla..

                    if (connectSocketToClients == INVALID_SOCKET)
                    {
                        printf("socket failed with error: %ld\n", WSAGetLastError());
                        WSACleanup();
                        return 1;
                    }

                    sockaddr_in serverAddress;   //Kreiranje i inicijalizacija adresne strukture!
                    serverAddress.sin_family = AF_INET;
                    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
                    serverAddress.sin_port = htons((unsigned short)srvResponse->ports[i]);

                    if (connect(connectSocketToClients, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
                    {
                        printf("Unable to connect to server. Reason: %ld \n", WSAGetLastError());
                        closesocket(connectSocketToClients);
                        WSACleanup();
                    }

                    bool success = AskClientForFilePart(&connectSocketToClients,srvResponse->fileName);
                    if (!success) {
                        return 1;
                    }

                    success = RecieveFilePartFromClient(&connectSocketToClients,fullFileContent);
                    if (!success) {
                        return 1;
                    }

                    printf_s("Deo fajla %d je downloadovan sa klijenta sa portom %d \n", i + 1, srvResponse->ports[i]);


                    iResult = shutdown(connectSocketToClients, 1); // Gasenje konekcije kad je deo fajla isporucen!
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("Shutdown failed with error: %d\n", WSAGetLastError());
                        closesocket(connectSocketToClients);
                        WSACleanup();
                        return 1;
                    }

                    closesocket(connectSocketToClients);
                    continue; // Od for-a..
                }

            }

            PrintStoredFiles();  //Ispis svih fajlova ciji se deo cuva!

            FILE* fp;
            char FileInFolderName[2 * FILE_NAME_SIZE] = "0_RecievedFiles\\Port ";
            strcat(FileInFolderName,Port);
            strcat(FileInFolderName, "__");
            strcat(FileInFolderName, srvResponse->fileName);

            fopen_s(&fp,FileInFolderName,"wb");

            if (fp == NULL) {
                printf("Unable to upen file pointer to write recieved file!");
                return 1;
            }

            size_t elementsWritten = fwrite(fullFileContent,fileSize + 1, 1, fp);
            fclose(fp);

            free(srvResponse);

            if (elementsWritten == 0) {
                printf("Nisam uspeo da upisem element!");
                return 2;
            }

        }
        else
        {
            printf("Unable to recieve or parse Server response!");
            return 0;
        }

    }
 
    // cleanup
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}