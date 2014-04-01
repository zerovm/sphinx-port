/*
 * opt.c
 *
 *  Created on: Dec 16, 2013
 *      Author: Volodymyr Varchuk
 */

int get_Act (int argc, char ** argv)
{
	int i = 0;
	int do_Act = 0;
	for ( i = 0; i < argc; i++)
	{
		if (strcasecmp (  argv[i], "-a" ) == 0)
			do_Act = 1;
		else if (strcasecmp (  argv[i], "-d" ) == 0)
			do_Act = 2;
		else if (strcasecmp (  argv[i], "-p" ) == 0)
			do_Act = 3;
		else if (strcasecmp (  argv[i], "-t" ) == 0)
			do_Act = 4;
	}
	return do_Act;
}
