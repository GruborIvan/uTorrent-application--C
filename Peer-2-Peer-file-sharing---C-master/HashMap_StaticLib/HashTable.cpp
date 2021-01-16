#include "HashTable.h"
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../ClientList_StaticLib/ClientList.h"

#define FILE_PART_SIZE 512

ClientFile* hashTable[HASH_TABLE_SIZE];

CRITICAL_SECTION csHashTable;

// HASH TABLE Functions..
unsigned int hash(char* fileName) {

    unsigned int hash = 5381;
    int c;

    while (c = *fileName++) {
        hash = ((hash << 5) + hash) + c;
    }

    hash = hash % 15;
    return hash;
}

void init_hash_table() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
}

void init_criticalSectionHashTable()
{
    InitializeCriticalSection(&csHashTable);
}

// Hash table methods ~~~~
void ReadClientFiles(char* fileName, int* ports)
{
    unsigned int id = hash(fileName);

    EnterCriticalSection(&csHashTable);

    ClientFile* pok = hashTable[id];

    for (int i = 0; i < 5; i++)
    {
        if (pok != NULL)
        {
            ports[i] = pok->clientPort;
            pok = pok->next;
        }
    }

    LeaveCriticalSection(&csHashTable);
}

int AddFileToHashTable(char* fileName, int clientPort)
{
    bool alredyStored = false;
    ClientFile* newCf = (ClientFile*)malloc(sizeof(ClientFile));

    Klijent* k = GetClientByPort(clientPort);

    int filePart = 1;

    // Setup..
    //strcpy_s(newCf->fileName, FILE_NAME_SIZE, fileName);
    strcpy_s(newCf->fileName,FILE_NAME_SIZE,fileName);
    newCf->clientPort = k->port;
    newCf->next = NULL;

    unsigned int id = hash(fileName);
    ClientFile* pok = hashTable[id];
    
    if (pok == NULL)
    {
        newCf->filePart = 1;
        hashTable[id] = newCf;
        return 1;
    }

    EnterCriticalSection(&csHashTable);

    if (pok->clientPort == clientPort)
        alredyStored = true;


    while (pok->next != NULL)
    {
        if (pok->clientPort == clientPort)
            alredyStored = true;

        if (pok->clientPort == 0)
        {
            pok->clientPort = newCf->clientPort;
            LeaveCriticalSection(&csHashTable);
            return filePart;
        }

        pok = pok->next;
        filePart++;
    }

    newCf->filePart = ++filePart;
    pok->next = newCf;

    LeaveCriticalSection(&csHashTable);

    if (alredyStored)
        return -1;
    else
        return filePart;
}

void NullateClientFileHashTable(int port)
{
    ClientFile* pok;

    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        pok = hashTable[i];

        if (pok == NULL)
            continue;

        EnterCriticalSection(&csHashTable);

        while (pok != NULL)
        {
            if (pok->clientPort == port)
            {
                pok->clientPort = 0;
                break;
            }
            pok = pok->next;
        }

        LeaveCriticalSection(&csHashTable);
        continue;
    }
}

bool ClientHasFIle(char* fileName, int port)
{
    int id = hash(fileName);

    ClientFile* cf = hashTable[id];

    if (cf == NULL)
        return false;

    ClientFile* pok = cf;

    while (pok)
    {
        if (pok->clientPort == port)
        {
            return true;
        }
        pok = pok->next;
    }

    return false;
}