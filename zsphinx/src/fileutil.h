/*
 * fileutil.h
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */

#ifndef FILEUTIL_H_
#define FILEUTIL_H_

int get_file_list (char *path, SingleList_t *, SingleList_t *);
int print_dir_tree (char *);
void print_file (char *);
int save_from_stdin ( char *);

#endif /* FILEUTIL_H_ */
