/*
 * xml_util.h
 *
 *  Created on: Apr 2, 2014
 *      Author: Volodymyr Varchuk
 */

#ifndef XML_UTIL_H_
#define XML_UTIL_H_

int write_XML_Elemet_ (int, char *, char *);
int open_xml_ ( char *);
int close_xml_ (int);
int open_xml_document_ ( int, unsigned long int);
int close_xml_document_ (int);

#endif /* XML_UTIL_H_ */
