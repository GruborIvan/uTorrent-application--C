#pragma once
#define FILE_SIZE 2008
#define FILE_NAME_SIZE 24
#define FILE_PART_SIZE 512
#define HASH_TABLE_SIZE 15

typedef struct FileKeep
{
    char fileName[FILE_NAME_SIZE];
    char filePartContent[536];
} FileKeep;

unsigned int hash(char* fileName);

void init_CriticalSection();

void init_fileKeep_table();

void PrintStoredFiles();

FileKeep* GetKeptFileById(int id);

void AddToKeptTable(int id, FileKeep *newKeep);

int TrimContent(char* fileContent);

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();