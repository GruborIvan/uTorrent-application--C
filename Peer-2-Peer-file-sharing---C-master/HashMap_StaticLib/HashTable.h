#pragma once

#define WIN32_LEAN_AND_MEAN   
#define FILE_SIZE 2008
#define HASH_TABLE_SIZE 15
#define FILE_NAME_SIZE 24

typedef struct ClientFile
{
    char fileName[FILE_NAME_SIZE]; // Naziv fajla
    int clientPort;  // Port klijenta koji èuva ovaj deo fajla
    int filePart;  // Deo fajla (1 - 5)
    struct ClientFile* next;
} ClientFile;

// HASH TABLE Functions..
unsigned int hash(char* fileName);

void init_criticalSectionHashTable();

void init_hash_table();

int AddFileToHashTable(char* fileName, int clientId);

void ReadClientFiles(char *fileName,int *ports);

void NullateClientFileHashTable(int port);

bool ClientHasFIle(char* fileName, int port);

void ClearHashTable();

bool RemoveFromEnd(ClientFile* start);