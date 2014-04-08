/*
 * opt.h
 *
 *  Created on: Dec 16, 2013
 *      Author: Volodymyr Varchuk
 */

#ifndef OPT_H_
#define OPT_H_

struct programm_options {
	int action; // 0 - (-a) add file to index;
				// 1 - (-d) delete from index;
				// 2 - (-p) preview text from file;
				// 3 - (-t) indexing from xml file;
				// 4 - ()

};

#endif /* OPT_H_ */
