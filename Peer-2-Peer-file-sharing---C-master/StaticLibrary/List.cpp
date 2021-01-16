#include "list.h"
#include <WinSock2.h>

void AddClientToList(Klijent* list, Klijent* newNode)
{
	while (list->sled != NULL)
	{
		list = list->sled;
	}

	list->sled = newNode;
}


void RemoveClientFromList(Klijent* list, int removeId)
{

}
