/*
 * types_.h
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */

#ifndef TYPES__H_
#define TYPES__H_

typedef struct {
	char **list;
	int count;
	int max_count;
} SingleList_t;

#define INITIAL_LIST_SIZE 10

#endif /* TYPES__H_ */
