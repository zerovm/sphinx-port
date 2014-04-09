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
#define SPHINX_CONFIG_FILE "/settings/sphinx.conf"
#define SPHINX_INDEX_NAME "mainindex"
#define INDEX_SAVE_PATH "/dev/output"
#define XML_PATH "xml.dat"

#endif /* TYPES__H_ */
