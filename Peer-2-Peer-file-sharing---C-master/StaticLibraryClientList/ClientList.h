#pragma once

typedef struct Klijent
{
    int clientId;
    SOCKET socket;
    int port;
    struct Klijent* next;
} Klijent;

struct Klijent* clientList = NULL;


int DodavanjeKlijenataUListu(SOCKET socket, int port);

Klijent* GetClientById(int clientId);

void RemoveClientFromList(int clientId);
