#include "ClientFiles.h"
#include <cstddef>
#include <WinSock2.h>
#include <stdio.h>

// Struktura za cuvanje delova fajlova za druge klijente..
FileKeep* fileKeepTable[HASH_TABLE_SIZE];

CRITICAL_SECTION cs;

unsigned int hash(char* fileName)
{
    unsigned int hash = 5381;
    int c;

    while (c = *fileName++) {
        hash = ((hash << 5) + hash) + c;
    }

    hash = hash % 15;
    return hash;
}

void init_CriticalSection()
{
    InitializeCriticalSection(&cs);
}

void AddToKeptTable(int id, FileKeep* newKeep)
{
    if (fileKeepTable[id] == NULL) {
        fileKeepTable[id] = newKeep;
    }
}

FileKeep* GetKeptFileById(int id)
{
    return fileKeepTable[id];
}

void init_fileKeep_table()
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        fileKeepTable[i] = NULL;
    }
}

int TrimContent(char* fileContent)
{
    // Pravljenje stringa, skracivanje buffer-a.
    char ch = '0';
    int cnt = 0;
    for (int i = 0; i < FILE_SIZE; i++)
    {
        ch = fileContent[i];
        if (ch != '0' && ch != '1' && ch != ' ')
        {
            cnt = i;
            fileContent[i] = '\0';
            break;
        }
    }

    return cnt;
}

void CloseClientSession()
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        FileKeep* pok = fileKeepTable[i];
        free(pok);
    }
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }
    return true;
}

void PrintStoredFiles()
{
    printf("\n----------------STORED FILE PARTS------------------------\n");
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        if (GetKeptFileById(i) != NULL)
        {
            printf("File %s stored! \n", GetKeptFileById(i)->fileName);
        }
    }
    printf("------------------------------------------------------\n");
}