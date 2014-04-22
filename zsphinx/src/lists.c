/*
 * lists.c
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lists.h"
#include "types_.h"

int addToList (const char *str, SingleList_t *tList)
{
	if ( tList->max_count == 0)
	{
		initList ( tList );
	}

	int resize = 0;
	int max_count = tList->max_count;
	char *temp_buff = NULL;

	if ( tList->count >= tList->max_count )
	{
		char **temp_buff  = NULL;
		temp_buff = ( char ** ) realloc ( tList->list, sizeof (char *) * ( tList->max_count * 2 ));
		if (temp_buff != NULL)
		{
			tList->list = temp_buff;
			tList->max_count = tList->max_count * 2;
		}
		else
			return -1;
	}

	//
	if (str != NULL)
	{
		tList->list[ tList->count ] = (char *) malloc (sizeof (char) * (strlen (str) + 1));
		strncpy(tList->list[tList->count], str, strlen (str) + 1);
	}
	else
	{
		tList->list[tList->count] = NULL;
	}
	tList->count++;

	return 0;
}

int initList (SingleList_t *list)
{
	int size = INITIAL_LIST_SIZE;
	if (size <= 0)
		return -1;

	list->max_count = size;
	list->list = (char **) malloc (sizeof (char * ) * size);
	list->count = 0;
	if ( list->list == NULL)
		return -1;

	return size;
}

int freeList ( SingleList_t *list )
{
	int i = 0;

	for ( i = 0; i < list->count; i++)
		free (list->list[i]);
	free ( list->list );
	return 0;
}
