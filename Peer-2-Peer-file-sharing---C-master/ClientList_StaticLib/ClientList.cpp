// ClientList_StaticLib.cpp : Defines the functions for the static library.
//
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ClientList.h"

#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

struct Klijent* clientLista;

CRITICAL_SECTION csList;

void init_criticalSectionClientList()
{
    InitializeCriticalSection(&csList);
}

void init_client_list()
{
    clientLista = NULL;
}

int DodavanjeKlijenataUListu(SOCKET socket, int port)
{
    // Iterator..
    Klijent* pok = clientLista;

    // Inicijalizacija novog Node-a.
    Klijent* newClient = (Klijent*)malloc(sizeof(Klijent));
    newClient->socket = socket;
    newClient->port = port;
    newClient->next = NULL;

    int num = 1;

    EnterCriticalSection(&csList);

    printf(YELLOW "\n-----------------------------------------------------------\n" RESET);
    if (clientLista == NULL)
    {
        clientLista = newClient;
        printf("%d . Cliend Id: %d  Port: %d    Status: " GREEN "CONNECTED \n" RESET, clientLista->port, clientLista->port, clientLista->port);
    }
    else
    {
        while (pok->next != NULL)
        {
            printf("%d . Cliend Id: %d  Port: %d    Status: " GREEN "CONNECTED \n" RESET, pok->port, pok->port, pok->port);
            pok = pok->next;
            num++;
        }

        pok->next = newClient;
        printf("%d . Cliend Id: %d  Port: %d    Status: " GREEN "CONNECTED \n" RESET, pok->port,pok->port, pok->port);
        printf("%d . Cliend Id: %d  Port: %d    Status: " GREEN "CONNECTED \n" RESET, pok->port,newClient->port, newClient->port);
        num++;
    }
    printf(YELLOW "-----------------------------------------------------------\n" RESET);

    LeaveCriticalSection(&csList);

    return newClient->port;
}

Klijent* GetClientByPort(int clientPort)
{
    Klijent* pok = clientLista;

    while (pok != NULL)
    {
        if (pok->port == clientPort)
        {
            return pok;
        }
        pok = pok->next;
    }
    return pok;
}

void RemoveClientFromList(int clientPort)
{
    if (clientLista == NULL) {
        return;
    }

    Klijent* itr = clientLista;
    Klijent* pok = clientLista->next;

    EnterCriticalSection(&csList);

    if (clientLista->port == clientPort)
    {
        clientLista = pok;
        free(itr);
    }
    else
    {
        while (itr->next != NULL)
        {
            if (itr->next->port == clientPort)
            {
                itr->next = pok->next;
                free(pok);
                break;
            }
            else
            {
                itr = pok;
                pok = pok->next;
            }
        }
    }

    LeaveCriticalSection(&csList);
}

int GetLastClientPort()
{
    Klijent* pok = clientLista;

    if (pok == NULL)
        return 0;

    while (pok->next != NULL)
    {
        pok = pok->next;
    }

    return pok->port;
}

void EmptyClientList()
{
    int curr_port;
    while ((curr_port = GetLastClientPort()) != 0)
    {
        RemoveClientFromList(curr_port);
    }
}