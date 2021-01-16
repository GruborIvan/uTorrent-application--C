#pragma once
#include <WinSock2.h>
#include "List.cpp"

typedef struct Klijent
{
    SOCKET socket;
    int port;
    int clientId;
    struct Klijent* sled;
} Klijent;

void AddClientToList(Klijent* list, Klijent* newNode);

void RemoveClientFromList(Klijent* list, int removeId);

void PrintOutClientList(Klijent* list);