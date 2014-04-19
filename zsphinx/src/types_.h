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

//typedef enum { zip, mail, error_type } obj_type;

#define INITIAL_LIST_SIZE 10
#define SPHINX_CONFIG_FILE "/settings/sphinx.conf"
#define SPHINX_INDEX_NAME "mainindex"
#define INDEX_SAVE_PATH "/dev/output"
#define OBJECT_DEVICE_NAME "/dev/input"
#define XML_PATH "xml.dat"
#define TEMP_DIR "temp_dir"
#define PATH_INFO_NAME "PATH_INFO"
#define OLD_ZRT


#endif /* TYPES__H_ */
