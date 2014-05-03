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
int copy_to_file_from_fd (int, char *);
void newbufferedpack_ (char *, char *);
int prepare_temp_dir (char *);
void get_list_from_file ( SingleList_t* );
int check_dir_exist (char *);
char * get_file_name_without_ext ( char *);
int get_single_message_from_offset ( char *, char *, long int );

#endif /* FILEUTIL_H_ */
