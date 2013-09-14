//
// $Id: search.cpp 3701 2013-02-20 18:10:18Z deogar $
//

//
// Copyright (c) 2001-2013, Andrew Aksyonoff
// Copyright (c) 2008-2013, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "zvmfileutil.h"
#include <time.h>
//#include <stdlib.h>
//#include <string.h>


#define CONF_CHECK(_hash,_key,_msg,_add) \
	if (!( _hash.Exists ( _key ) )) \
	{ \
		fprintf ( stdout, "ERROR: key '%s' not found " _msg, _key, _add ); \
		continue; \
	}


const char * myctime ( DWORD uStamp )
{
	static char sBuf[256];
	time_t tStamp = uStamp; // for 64-bit
	strncpy ( sBuf, ctime ( &tStamp ), sizeof(sBuf) );

	char * p = sBuf;
	while ( (*p) && (*p)!='\n' && (*p)!='\r' ) p++;
	*p = '\0';

	return sBuf;
}

#include <dirent.h>
/*
void mylistdir (char *path)
{
  	DIR *dir;
	struct dirent *entry;
	char newpath[1024];
	dir = opendir(path);
	int len;//, lennew, lennewlast;
	if(dir == 0)
	{
		return;
	}
//	printf ("* %s", path);
	while((entry = readdir(dir)))
	{
		//char *filename = (char *) malloc (strlen (path)+ strlen (entry->d_name) + 2);
		//sprintf (filename, "%s/%s\n",path, entry->d_name);
		//long size = getfilesize_fd (0, filename, 1);

		printf ("%s/%s (%ld)\n",(char *) path, entry->d_name);
		if(entry->d_type == DT_DIR)
		{
			if (strcmp (entry->d_name, ".") != 0 && strcmp (entry->d_name, "..") != 0)
			{
				strcpy (newpath, path);
				len = strlen (newpath);
				if (newpath [len-1] != '/')
					strcat (newpath, "/");
				strcat (newpath, entry->d_name);
				mylistdir (newpath);
			}
		}

	}
	closedir(dir);
}
*/

struct range {
	Hitpos_t tStartPos;
	Hitpos_t tEndPos;
};

typedef range tRange;

void PrintRangeT (range rng)
{
	printf ("start = %d, end=%d\n", rng.tStartPos, rng.tEndPos);
}

void PrintRangeArray ( Hitpos_t *pHitsArray, int iHitsCount )
{
	int i;
	for (i = 0; i < iHitsCount; i++)
		printf ("pHitsArray[%d]=%d\n", i, pHitsArray[i]);
}

void PrintHitsArray (Hitpos_t **pHits, int *pHitCount, int iWordCount)
{
	int i, j;
	for ( i = 0; i < iWordCount; i++)
	{
		for (j = 0; j < pHitCount[i]; j++)
			printf("pHits[%d][%d]=%d\n", i,j,pHits[i][j]);
	}
}

int GetDistanceFrom2Range (range tRange)
{
	int iDist =0;

	iDist = abs(tRange.tEndPos - tRange.tStartPos);

	return iDist;
}

int GetDistanceFrom2Hits (Hitpos_t iStartPos, Hitpos_t iEndPos)
{
	int iDist =0;

	iDist = abs(iEndPos - iStartPos);

	return iDist;
}

Hitpos_t GetMinPosFromArray (Hitpos_t *pHitsArray, int iHitsCount)
{
	Hitpos_t tMinHitPos =0;
	int i = 0;
	if (iHitsCount > 0)
		tMinHitPos = pHitsArray [0];
	for  (i = 0; i < iHitsCount; i++)
	{
		if (tMinHitPos > pHitsArray [i])
			tMinHitPos = pHitsArray [i];
	}
	return tMinHitPos;
}

Hitpos_t GetMaxPosFromArray (Hitpos_t *pHitsArray, int iHitsCount)
{
	Hitpos_t tMaxHitPos = 0;
	int i = 0;
	if (iHitsCount > 0)
		tMaxHitPos = pHitsArray [0];
	for  (i = 0; i < iHitsCount; i++)
	{
		if (tMaxHitPos < pHitsArray [i])
			tMaxHitPos = pHitsArray [i];
	}
	return tMaxHitPos;
}


range GetStartEndRange (Hitpos_t *pHitsArray, int iHitsCount)
{
	range tRange;
	Hitpos_t tMaxHitPos = GetMaxPosFromArray (pHitsArray, iHitsCount);
	Hitpos_t tMinHitPos = GetMinPosFromArray (pHitsArray, iHitsCount);

	tRange.tStartPos = tMinHitPos;
	tRange.tEndPos = tMaxHitPos;

	return (tRange);
}

int GetDistanceFromArrayHits (Hitpos_t *pHitsArray, int iHitsCount)
{
	int iDist = 0;

	range tRange;

	tRange = GetStartEndRange (pHitsArray, iHitsCount);

	iDist = abs(tRange.tEndPos - tRange.tStartPos);

	return iDist;
}

range GetSnippetRange (Hitpos_t **pHits, int *pHitCount, int iWordCount)//, int *pStartPos, int *pEndPos)
{
	Hitpos_t tMaxHitPos = 0;
	Hitpos_t tMinHitPos = 1;

	int i = 0, j =0, k =0; // counters
	int iMinHits; // the minimum number of hits
	int iIndexMinHits = 0; // index in array of hits of the minimum number of hits
	int iMinDif;
	range tMinDif;





	Hitpos_t *pHitpos = (Hitpos_t *) malloc (sizeof (Hitpos_t) * iWordCount) ;// array of poses


	// find min and max hits poses in the text
	for (i =0; i < iWordCount; i++)
	{
		for (j = 0; j < pHitCount[i]; j++)
		{
			if (tMaxHitPos < pHits[i][j])
				tMaxHitPos = pHits[i][j];
			if (tMinHitPos > pHits[i][j])
				tMinHitPos = pHits[i][j];
		}
	}

	//finding words with the fewest hits
	iMinHits = pHitCount[0];
	for (i = 0; i < iWordCount; i++)
	{
		if (pHitCount[i] < iMinHits)
		{
			iMinHits = pHitCount[i];
			iIndexMinHits = i;
		}
	}

	iMinDif = GetMaxPosFromArray(pHits[iIndexMinHits], pHitCount[iIndexMinHits]);

	for (i = 0; i < pHitCount[iIndexMinHits]; i++)// array hits of word with minimum number of hits
	{
		for (j = 0; j < iWordCount; j++)// array of number of hits
		{
			if ( j == iIndexMinHits)
			{
				pHitpos[j] = pHits[j][i];
				continue;
			}
			int iMinDist;
			int iIndexMinDist = 0;
			iMinDist = tMaxHitPos; // initial distance - max hit pos
			for (k = 0; k < pHitCount[j]; k++)
			{
				int iDist;
				iDist = GetDistanceFrom2Hits(pHits[iIndexMinHits][i], pHits[j][k]);
				if (iDist < iMinDist)
				{
					iMinDist = iDist;
					iIndexMinDist = k;
				}
			}
			pHitpos[j] = pHits[j][iIndexMinDist];
		}
		int iDistRange;
		iDistRange = GetDistanceFromArrayHits (pHitpos, iWordCount);

		if (iDistRange < iMinDif)
		{
			iMinDif = iDistRange;
			tMinDif = GetStartEndRange (pHitpos, iWordCount);
		}
	}

	return (range)(tMinDif);
}

char** getwordsformstr (const char *s, int *wordcount)// char **res)
{

	char *str = (char *) malloc (sizeof (char) * (strlen (s) + 1));
	strncpy (str, s, strlen (s));
	int len = strlen (str);
	int i = 0;
	int j = 0;
	int wc = 0;


	//remobe spaces from start of string
	while (!isalnum(*str) )
		str++;
	len = strlen(str);

	// remove spaces on ends of string
	while (!isalnum (str[len -1]))
		len--;
	str[len] = '\0';

	//counting the number of words in query
	while (i < len)
	{
		if (!isalnum (str[i]))
		{
			(*wordcount)++;
			while (!isalnum(str[i+1]))
				i++;
		}
		if (str [i+1] == '\0')
			(*wordcount)++;
		i++;
	}
	//
	char **res = (char **) malloc (sizeof (char *) * *wordcount);
	char *temp = NULL;
	temp = (char *) malloc (sizeof (char) * len);
	for (i = 0; i <= len; i++)
	{
		if (temp == NULL)
			return 0;
		if (!isalnum(str[i]) || str [i] == '\0')
		{
			temp[j] = '\0';
			res[wc] = (char *) malloc (sizeof (char) * (strlen(temp) + 1));
			strncpy (res[wc], temp, strlen (temp) + 1);
			j =0;
			wc++;
			while (!isalnum(str[i+1]))
				i++;
		}
		else
		{
			temp[j++] = str[i];
		}
	}
#ifdef TEST
	printf ("OK");
#endif
	return res;
}

int mystrindex (const char *s, const char *t)
{
	int i,j,k;
	printf ("%s, %s\n", t, s);
	for (i  =0; (unsigned char) s[i] != '\0'; i++)
	{
		printf ("%c\n", s[i]);
		for (j = i, k = 0; t[k] != '\0' && s[j] == t[k]; j++, k++)
		{
			printf ("s[%d]=%c,  t[%d]=%c\n", j, s[j], k, t[k]);
		}
		if ( k > 0 && t[k] == '\0')
			return i;
	}
	return -1;
}



int main ( int argc, char ** argv )
{
//	mylistdir("/");
	unpackindex_fd( (char *)S_DEVINPUTDATA);
//	mylistdir("/");
#ifdef TEST
	fprintf ( stdout, SPHINX_BANNER );
#endif
	if ( argc<=1 )
	{
		fprintf ( stdout,
			"Usage: search [OPTIONS] <word1 [word2 [word3 [...]]]>\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\tuse given config file instead of defaults\n"
			"-i, --index <index>\tsearch given index only (default: all indexes)\n"
			"-a, --any\t\tmatch any query word (default: match all words)\n"
			"-b, --boolean\t\tmatch in boolean mode\n"
			"-p, --phrase\t\tmatch exact phrase\n"
			"-e, --extended\t\tmatch in extended mode\n"
			"-j, --json <key> <v>\tonly match if key in JSON attr value is v\n"
			"-f, --filter <attr> <v>\tonly match if attribute attr value is v\n"
			"-s, --sortby <CLAUSE>\tsort matches by 'CLAUSE' in sort_extended mode\n"
			"-S, --sortexpr <EXPR>\tsort matches by 'EXPR' DESC in sort_expr mode\n"
			"-o, --offset <offset>\tprint matches starting from this offset (default: 0)\n"
			"-l, --limit <count>\tprint this many matches (default: 20)\n"
			"-q, --noinfo\t\tdon't print document info from SQL database\n"
			"-g, --group <attr>\tgroup by attribute named attr\n"
			"-gs,--groupsort <expr>\tsort groups by <expr>\n"
			"--sort=date\t\tsort by date, descending\n"
			"--rsort=date\t\tsort by date, ascending\n"
			"--sort=ts\t\tsort by time segments\n"
			"--stdin\t\t\tread query from stdin\n"
			"\n"
			"This program (CLI search) is for testing and debugging purposes only;\n"
			"it is NOT intended for production use.\n"
		);
		exit ( 0 );
	}

	///////////////////////////////////////////
	// get query and other commandline options
	///////////////////////////////////////////

	CSphQuery tQuery;
	char sQuery [ 1024 ];
	sQuery[0] = '\0';

	const char * sOptConfig = NULL;
	const char * sIndex = NULL;
	bool bNoInfo = false;
	bool bStdin = false;
	int iStart = 0;
	int iLimit = 20;

	/////
	//max param length
	////
	int MaxParamLen = 0; //max command line parameter length
	for (int i =0; i < argc; i++)
	{
		if ( MaxParamLen < strlen (argv[i]))
			MaxParamLen = strlen (argv[i]);
	}

	char *pJSONFilterKey [MaxParamLen];
	char *pJSONFilterVal [MaxParamLen];

	int iJSONFilterCount = 0;

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( argv[i][0]=='-' )
		{
			// this is an option
			if ( i==0 );
			OPT ( "-a", "--any" )		tQuery.m_eMode = SPH_MATCH_ANY;
			OPT ( "-b", "--boolean" )	tQuery.m_eMode = SPH_MATCH_BOOLEAN;
			OPT ( "-p", "--phrase" )	tQuery.m_eMode = SPH_MATCH_PHRASE;
			OPT ( "-e", "--ext" )		tQuery.m_eMode = SPH_MATCH_EXTENDED;
			OPT ( "-e2", "--ext2" )		tQuery.m_eMode = SPH_MATCH_EXTENDED2;
			OPT ( "-q", "--noinfo" )	bNoInfo = true;
			OPT1 ( "--sort=date" )		tQuery.m_eSort = SPH_SORT_ATTR_DESC;
			OPT1 ( "--rsort=date" )		tQuery.m_eSort = SPH_SORT_ATTR_ASC;
			OPT1 ( "--sort=ts" )		tQuery.m_eSort = SPH_SORT_TIME_SEGMENTS;
			OPT1 ( "--stdin" )			bStdin = true;

			else if ( (i+1)>=argc )		break;
			OPT ( "-o", "--offset" )	iStart = atoi ( argv[++i] );
			OPT ( "-l", "--limit" )		iLimit = atoi ( argv[++i] );
			OPT ( "-c", "--config" )	sOptConfig = argv[++i];
			OPT ( "-i", "--index" )		sIndex = argv[++i];
			OPT ( "-g", "--group" )		{ tQuery.m_eGroupFunc = SPH_GROUPBY_ATTR; tQuery.m_sGroupBy = argv[++i]; }
			OPT ( "-gs","--groupsort" )	{ tQuery.m_sGroupSortBy = argv[++i]; } // NOLINT
			OPT ( "-s", "--sortby" )	{ tQuery.m_eSort = SPH_SORT_EXTENDED; tQuery.m_sSortBy = argv[++i]; }
			OPT ( "-S", "--sortexpr" )	{ tQuery.m_eSort = SPH_SORT_EXPR; tQuery.m_sSortBy = argv[++i]; }

			else if ( (i+2)>=argc )		break;
			OPT ( "-f", "--filter" )
			{
				DWORD uVal = strtoul ( argv[i+2], NULL, 10 );
				CSphFilterSettings * pFilter = NULL;
				// do we already have a filter for that attribute?
				ARRAY_FOREACH ( j, tQuery.m_dFilters )
				{
					if ( tQuery.m_dFilters[j].m_sAttrName==argv[i+1] )
					{
						pFilter = &tQuery.m_dFilters[j];
						break;
					}
				}
				if ( !pFilter )
				{
					pFilter = &tQuery.m_dFilters.Add ();
					pFilter->m_eType = SPH_FILTER_VALUES;
					pFilter->m_sAttrName = argv[i+1];
				}
				pFilter->m_dValues.Add ( uVal );
				pFilter->m_dValues.Uniq ();
				i += 2;
			}
			/*this snippet must be copied in to new version*/
			OPT ("-j", "--json")
			{
				// json filter settings
				iJSONFilterCount++;
				//copy keys
				pJSONFilterKey [iJSONFilterCount - 1] = (char *) malloc (sizeof (char) * strlen (argv[i+1]) + 1);
				strncpy (pJSONFilterKey [iJSONFilterCount - 1], argv[i+1], strlen (argv[i+1]) + 1);
				//copy value
				pJSONFilterVal [iJSONFilterCount - 1] = (char *) malloc (sizeof (char) * strlen (argv[i+2]) + 1);
				strncpy (pJSONFilterVal [iJSONFilterCount - 1], argv[i+2], strlen (argv[i+2]) + 1);
				i += 2;
			}
			/*this snippet must be copied in to new version*/
			else
				break; // unknown option

		} else if ( strlen(sQuery) + strlen(argv[i]) + 1 < sizeof(sQuery) )
		{
			// this is a search term
			strcat ( sQuery, argv[i] ); // NOLINT
			strcat ( sQuery, " " ); // NOLINT
		}
	}

	iStart = Max ( iStart, 0 );
	iLimit = Max ( iLimit, 0 );

	// test input parameters
//	printf ("test input parameters\n");
//	for ( int a = 0; a < iJSONFilterCount; a++)
//	{
//		printf ("%d. %s = %s\n", a, pJSONFilterKey[a], pJSONFilterVal[a]);
//	}

	if ( i!=argc )
	{
		fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		return 1;
	}

	#undef OPT

	tzset();

	if ( bStdin )
	{
		int iPos = 0, iLeft = sizeof(sQuery)-1;
		char sThrowaway [ 256 ];

		while ( !feof(stdin) )
		{
			if ( iLeft>0 )
			{
				int iLen = fread ( sQuery, 1, iLeft, stdin );
				iPos += iLen;
				iLeft -= iLen;
			} else
			{
				int iDummy; // to avoid gcc unused result warning
				iDummy = fread ( sThrowaway, 1, sizeof(sThrowaway), stdin );
			}
		}

		assert ( iPos<(int)sizeof(sQuery) );
		sQuery[iPos] = '\0';
	}

	/////////////
	// configure
	/////////////

	tQuery.m_iMaxMatches = Max ( 1000, iStart + iLimit );

	CSphConfigParser cp;
	CSphConfig & hConf = cp.m_tConf;
	sphLoadConfig ( sOptConfig, false, cp );

	/////////////////////
	// get word list from string query
	/////////////////////
	int wordcount = 0;
	char **res = NULL;
	res = getwordsformstr (sQuery, &wordcount);


	/////////////////////
	// search each index
	/////////////////////

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext () )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet ();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( sIndex && strcmp ( sIndex, sIndexName ) )
			continue;

		if ( hIndex("type") && hIndex["type"]=="distributed" )
			continue;

		if ( !hIndex.Exists ( "path" ) )
			sphDie ( "key 'path' not found in index '%s'", sIndexName );

		CSphString sError;

		// do we want to show document info from database?
		#if USE_MYSQL
		MYSQL tSqlDriver;
		const char * sQueryInfo = NULL;

		while ( !bNoInfo )
		{
			if ( !hIndex("source") || !hConf("source") || !hConf["source"]( hIndex["source"] ) )
				break;

			const CSphConfigSection & hSource = hConf["source"][ hIndex["source"] ];
			if ( !hSource("type") || hSource["type"]!="mysql"
				|| !hSource("sql_host") || !hSource("sql_user") || !hSource("sql_db") || !hSource("sql_pass") || !hSource("sql_query_info") )
			{
				break;
			}

			sQueryInfo = hSource["sql_query_info"].cstr();
			if ( !strstr ( sQueryInfo, "$id" ) )
				sphDie ( "'sql_query_info' value must contain '$id'" );

			int iPort = 3306;
			if ( hSource.Exists ( "sql_port" ) && hSource["sql_port"].intval() )
				iPort = hSource["sql_port"].intval();

			mysql_init ( &tSqlDriver );
			if ( !mysql_real_connect ( &tSqlDriver,
				hSource["sql_host"].cstr(),
				hSource["sql_user"].cstr(),
				hSource["sql_pass"].cstr(),
				hSource["sql_db"].cstr(),
				iPort,
				hSource.Exists ( "sql_sock" ) ? hSource["sql_sock"].cstr() : NULL,
				0 ) )
			{
				sphDie ( "failed to connect to MySQL (error=%s)", mysql_error ( &tSqlDriver ) );
			}

			// all good
			break;
		}
		#endif

		//////////
		// search
		//////////

		tQuery.m_sQuery = sQuery;
		CSphQueryResult * pResult = NULL;

		CSphIndex * pIndex = sphCreateIndexPhrase ( sIndexName, hIndex["path"].cstr() );
		pIndex->SetEnableStar ( hIndex.GetInt("enable_star")!=0 );
		pIndex->SetWordlistPreload ( hIndex.GetInt("ondisk_dict")==0 );
		pIndex->SetGlobalIDFPath ( hIndex.GetStr ( "global_idf" ) );

		CSphString sWarning;

		int iSkipedMatches = 0;

		sError = "could not create index (check that files exist)";
		for ( ; pIndex; )
		{
			if ( !pIndex->Prealloc ( false, false, sWarning ) || !pIndex->Preread() )
			{
				sError = pIndex->GetLastError ();
				break;
			}
			const CSphSchema * pSchema = &pIndex->GetMatchSchema();

			if ( !sWarning.IsEmpty () )
				fprintf ( stdout, "WARNING: index '%s': %s\n", sIndexName, sWarning.cstr () );

			// handle older index versions (<9)
			if ( !sphFixupIndexSettings ( pIndex, hIndex, sError ) )
				sphDie ( "index '%s': %s", sIndexName, sError.cstr() );

			if ( hIndex ( "global_idf" ) && !sphPrereadGlobalIDF ( hIndex.GetStr ( "global_idf" ), sError ) )
				sphDie ( "index '%s': %s", sIndexName, sError.cstr() );

/*
			 for (int i = 0; i < pSchema->GetAttrsCount(); i++)
			 {
				printf("attr type = %d, ", pSchema->GetAttr(i).m_eAttrType);
				printf("aggr func = %d, ", pSchema->GetAttr(i).m_eAggrFunc);
				printf("src = %d, ", pSchema->GetAttr(i).m_eSrc);
				printf("attr[%d]=%s\n", i, pSchema->GetAttr(i).m_sName.cstr());
			}
*/

			// lookup first timestamp if needed
			// FIXME! remove this?
			if ( tQuery.m_eSort!=SPH_SORT_RELEVANCE && tQuery.m_eSort!=SPH_SORT_EXTENDED && tQuery.m_eSort!=SPH_SORT_EXPR )
			{
				int iTS = -1;
				for ( int i=0; i<pSchema->GetAttrsCount(); i++ )
					if ( pSchema->GetAttr(i).m_eAttrType==SPH_ATTR_TIMESTAMP )
				{
					tQuery.m_sSortBy = pSchema->GetAttr(i).m_sName;
					iTS = i;
					break;
				}
				if ( iTS<0 )
				{
					fprintf ( stdout, "index '%s': no timestamp attributes found, sorting by relevance.\n", sIndexName );
					tQuery.m_eSort = SPH_SORT_RELEVANCE;
				}
			}

			// do querying
			ISphMatchSorter * pTop = sphCreateQueue ( &tQuery, pIndex->GetMatchSchema(), sError, NULL );
			if ( !pTop )
			{
				sError.SetSprintf ( "failed to create sorting queue: %s", sError.cstr() );
				break;
			}

			pResult = new CSphQueryResult();
			if ( !pIndex->MultiQuery ( &tQuery, pResult, 1, &pTop, NULL ) )
			{
				// failure; pull that error message
				sError = pResult->m_sError;
				SafeDelete ( pResult );
			} else
			{
				// success; fold them matches
				pResult->m_dMatches.Reset ();
				pResult->m_iTotalMatches += pTop->GetTotalCount();
				pResult->m_tSchema = pTop->GetSchema();
				sphFlattenQueue ( pTop, pResult, 0 );
			}

			SafeDelete ( pTop );
			break;
		}

		/////////
		// print
		/////////

		if ( !pResult )
		{
			fprintf ( stdout, "index '%s': search error: %s.\n", sIndexName, sError.cstr() );
			return 1;
		}
#ifdef TEST
		fprintf ( stdout, "index '%s': query '%s': returned %d matches without filtering by meta tgs of "INT64_FMT" total in %d.%03d sec\n",
			sIndexName, sQuery, pResult->m_dMatches.GetLength(), pResult->m_iTotalMatches,
			pResult->m_iQueryTime/1000, pResult->m_iQueryTime%1000 );
#endif
		if ( !pResult->m_sWarning.IsEmpty() )
			fprintf ( stdout, "WARNING: %s\n", pResult->m_sWarning.cstr() );

		int iFilteredCount = 0;

		if ( pResult->m_dMatches.GetLength() )
		{
#ifdef TEST
			fprintf ( stdout, "\ndisplaying matches:\n" );
#endif

			int iMaxIndex = Min ( iStart+iLimit, pResult->m_dMatches.GetLength() );
			for ( int i=iStart; i<iMaxIndex; i++ )
			{
				CSphMatch & tMatch = pResult->m_dMatches[i];

				//if ( tMatch. )
				const CSphColumnInfo & _tAttr = pResult->m_tSchema.GetAttr(1);

				const BYTE *_pStr;
				int iLen = sphUnpackStr ( pResult->m_pStrings + tMatch.GetAttr ( _tAttr.m_tLocator ), &_pStr );
				char *cStr = (char *) malloc (sizeof (char) * iLen + 1);
				char *pKey;
				char *pVal;
				//find all keys
				int iMatchKeys = 0;
				int a= 0, b =0;
				for (a=0; a < iLen; a++)
				{
					if (isprint(_pStr[a]))
						cStr [b++] = _pStr[a];
				}

				cStr[b] = '\0';

				for ( int l = 0; l < iJSONFilterCount; l++)
				{
					//printf ("key=%s val=%s, _pStr=%s \n", pJSONFilterKey [l], pJSONFilterVal [l], cStr);
					if ( (pKey = strstr ((char *) cStr, pJSONFilterKey [l] )) == NULL)
					{
						//printf ("*1\n");
						continue;
					}
					else
					{
						if ( (pVal = strstr ((char*) cStr, pJSONFilterVal[l] )) == NULL)
						{
							//printf ("*2\n");
							continue;
						}
						else
						{
							//printf ("*3\n");
							if ((strlen (pJSONFilterKey [l]) - (pVal - pKey) + 1) > 2)
							{
								//printf("*4 %d,  %d\n",strlen (pJSONFilterKey [l]), pVal - pKey);
								continue;
							}
							else
							{
								//printf ("find match key + value\n");
								iMatchKeys++;
							}
						}
					}
				}
				//printf ("iMatchKeys=%d, iJSONFilterCount=%d\n", iMatchKeys, iJSONFilterCount);
				if (iMatchKeys != iJSONFilterCount)
					continue;
				else
					iFilteredCount++;
				//printf ("start print result\n");

				//printf ("iFilteredCount = %d\n", iFilteredCount);
				fprintf ( stdout, "%d. document=" DOCID_FMT ", weight=%d", 1+i, tMatch.m_iDocID, tMatch.m_iWeight );

				for ( int j=0; j<pResult->m_tSchema.GetAttrsCount(); j++ )
				{
					const CSphColumnInfo & tAttr = pResult->m_tSchema.GetAttr(j);
					fprintf ( stdout, ", %s=", tAttr.m_sName.cstr() );

					if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET )
					{
						fprintf ( stdout, "(" );
						SphAttr_t iIndex = tMatch.GetAttr ( tAttr.m_tLocator );
						if ( iIndex )
						{
							const DWORD * pValues = pResult->m_pMva + iIndex;
							int iValues = *pValues++;
							if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
							{
								assert ( ( iValues%2 )==0 );
								for ( int k=0; k<iValues; k+=2, pValues+=2 )
								{
									uint64_t uMva = MVA_UPSIZE ( pValues );
									fprintf ( stdout, k ? ","UINT64_FMT : UINT64_FMT, uMva );
								}
							} else
							{
								for ( int k=0; k<iValues; k++ )
									fprintf ( stdout, k ? ",%u" : "%u", *pValues++ );
							}
						}
						fprintf ( stdout, ")" );

					} else switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:
						case SPH_ATTR_ORDINAL:
						case SPH_ATTR_BOOL:			fprintf ( stdout, "%u", (DWORD)tMatch.GetAttr ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_TIMESTAMP:	fprintf ( stdout, "%s", myctime ( (DWORD)tMatch.GetAttr ( tAttr.m_tLocator ) ) ); break;
						case SPH_ATTR_FLOAT:		fprintf ( stdout, "%f", tMatch.GetAttrFloat ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_BIGINT:		fprintf ( stdout, INT64_FMT, tMatch.GetAttr ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_STRING:
							{
								const BYTE * pStr;
								int iLen = sphUnpackStr ( pResult->m_pStrings + tMatch.GetAttr ( tAttr.m_tLocator ), &pStr );
								fwrite ( pStr, 1, iLen, stdout );
								break;
							}
						case SPH_ATTR_JSON:
							{
								printf ("JSON field");
								break;
							}
						default:
							fprintf ( stdout, "(unknown-type-%d)", tAttr.m_eAttrType );
					}
				}

				int *hitcount = (int *) malloc (sizeof (int) * wordcount);

				Hitpos_t **pHits = (Hitpos_t **) malloc (sizeof (Hitpos_t *) * wordcount);
				int ii = 0;
				for (ii =0; ii < wordcount; ii++)
				{
					hitcount[ii] = 0;
					pHits[ii] = pIndex->ZGetHitlist(stdout, res[ii] , false, &hitcount[ii], tMatch.m_iDocID);

				}
				range tSnippetRange = GetSnippetRange(pHits, hitcount, wordcount);
				printf ("; start=%d; end=%d\n", tSnippetRange.tStartPos, tSnippetRange.tEndPos);

				//fprintf ( stdout, "\n" );

				#if USE_MYSQL
				if ( sQueryInfo )
				{
					char * sQuery = sphStrMacro ( sQueryInfo, "$id", tMatch.m_iDocID );
					const char * sError = NULL;

					#define LOC_MYSQL_ERROR(_arg) { sError = _arg; break; }
					for ( ;; )
					{
						if ( mysql_query ( &tSqlDriver, sQuery ) )
							LOC_MYSQL_ERROR ( "mysql_query" );

						MYSQL_RES * pSqlResult = mysql_use_result ( &tSqlDriver );
						if ( !pSqlResult )
							LOC_MYSQL_ERROR ( "mysql_use_result" );

						MYSQL_ROW tRow = mysql_fetch_row ( pSqlResult );
						if ( !tRow )
						{
							fprintf ( stdout, "\t(document not found in db)\n" );
							break;
						}

						for ( int iField=0; iField<(int)pSqlResult->field_count; iField++ )
							fprintf ( stdout, "\t%s=%s\n",
								( pSqlResult->fields && pSqlResult->fields[iField].name ) ? pSqlResult->fields[iField].name : "(NULL)",
								tRow[iField] ? tRow[iField] : "(NULL)" );

						mysql_free_result ( pSqlResult );
						break;
					}

					if ( sError )
						sphDie ( "sql_query_info: %s: %s", sError, mysql_error ( &tSqlDriver ) );

					delete [] sQuery;
				}
				#endif
			}
		}

#ifdef TEST
		fprintf ( stdout, "index '%s': query '%s': returned %d matches with filtering by META tags (total returned %d) of "INT64_FMT" total in %d.%03d sec\n",
			sIndexName, sQuery, iFilteredCount, pResult->m_dMatches.GetLength(), pResult->m_iTotalMatches,
			pResult->m_iQueryTime/1000, pResult->m_iQueryTime%1000 );
#endif

		fprintf ( stdout, "\nwords:\n" );
		pResult->m_hWordStats.IterateStart();
		int iWord = 1;
		while ( pResult->m_hWordStats.IterateNext() )
		{
			const CSphQueryResultMeta::WordStat_t & tStat = pResult->m_hWordStats.IterateGet();
			fprintf ( stdout, "%d. '%s': "INT64_FMT" documents, "INT64_FMT" hits\n",
				iWord,
				pResult->m_hWordStats.IterateGetKey().cstr(),
				tStat.m_iDocs,
				tStat.m_iHits );
			iWord++;
		}
		fprintf ( stdout, "\n" );

		///////////
		// cleanup
		///////////

		SafeDelete ( pIndex );
	}

	sphShutdownWordforms ();
}

//
// $Id: search.cpp 3701 2013-02-20 18:10:18Z deogar $
//
