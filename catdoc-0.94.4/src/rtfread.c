/*****************************************************************/
/* Reading routines for rtf files                                */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 2003, (c) Alex Ott 2003	             */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "catdoc.h"

/********************************************************
 * Datatypes declaration
 *
 */
typedef enum {
	RTF_CODEPAGE,
	RTF_FONT_CHARSET,
	RTF_UC,
	RTF_UNICODE_CHAR,
	RTF_CHAR,
	RTF_PARA,
	RTF_TABLE_START,
	RTF_TABLE_END,
	RTF_ROW,
	RTF_CELL,
	RTF_UNKNOWN,
	RTF_OVERLAY,
	RTF_PICT,
	RTF_F,
	RTF_AUTHOR,
	RTF_FONTTBL,
	RTF_INFO,
	RTF_STYLESHEET,
	RTF_COLORTBL,
	RTF_LISTOVERRIDETABLE,
	RTF_LISTTABLE,
	RTF_RSIDTBL,
	RTF_GENERATOR,
	RTF_DATAFIELD,
	RTF_LANG,
	RTF_PARD,
	RTF_TAB,
	RTF_SPEC_CHAR,
	RTF_EMDASH,
	RTF_ENDASH,
	RTF_EMSPACE,
	RTF_ENSPACE,
	RTF_BULLET,
	RTF_LQUOTE,
	RTF_RQUOTE,
	RTF_LDBLQUOTE,
	RTF_RDBLQUOTE,
	RTF_ZWNONJOINER,
	RTF_PLAIN,
	RTF_LOCH,
	RTF_HICH,
	RTF_DBCH,
	RTF_DEFLANG,
	RTF_DEFLANGFE,
	RTF_FONT,
	RTF_INDEX
} RTFTypes;

typedef struct {
	char *name;
	RTFTypes type;
} RTFTypeMap;

RTFTypeMap rtf_types[]={
	{"uc",RTF_UC},
	{"ansicpg",RTF_CODEPAGE},
	{"pard",RTF_PARD},
	{"par",RTF_PARA},
	{"cell",RTF_CELL},
	{"row",RTF_ROW},
 	{"overlay",RTF_OVERLAY},
 	{"pict",RTF_PICT},
 	{"author",RTF_AUTHOR},
 	{"f",RTF_F},
 	{"fonttbl",RTF_FONTTBL},
 	{"info",RTF_INFO},
 	{"stylesheet",RTF_STYLESHEET},
 	{"colortbl",RTF_COLORTBL},
 	{"listtable",RTF_LISTTABLE},
 	{"listoverridetable",RTF_LISTOVERRIDETABLE},
 	{"rsidtbl",RTF_RSIDTBL},
 	{"generator",RTF_GENERATOR},
 	{"datafield",RTF_DATAFIELD},
 	{"lang",RTF_LANG},
 	{"tab",RTF_TAB},
	{"emdash",RTF_EMDASH},
	{"endash",RTF_ENDASH},
	{"emspace",RTF_EMDASH},
	{"enspace",RTF_ENDASH},
 	{"bullet",RTF_BULLET}, 
 	{"lquote",RTF_LQUOTE},
	{"rquote",RTF_RQUOTE},
	{"ldblquote",RTF_LDBLQUOTE},
	{"rdblquote",RTF_RDBLQUOTE},
	{"zwnj",RTF_ZWNONJOINER},
	{"u",RTF_UNICODE_CHAR},
	{"plain",RTF_PLAIN},
	{"loch",RTF_LOCH},
	{"hich",RTF_HICH},
	{"dbch",RTF_DBCH},
	{"fcharset", RTF_FONT_CHARSET},
	{"deflang",RTF_DEFLANG},
	{"deflangfe",RTF_DEFLANGFE},
	{"xe",RTF_INDEX},
};

#define RTFNAMEMAXLEN 32
#define RTFARGSMAXLEN 64

/**
 * Structure describing rtf command
 *
 */
typedef struct {
	RTFTypes type;
	char name[RTFNAMEMAXLEN+1];
	signed int numarg;
} RTFcommand;


#define MAXFONTNAME 64
/**
 * Structure to describe a font
 *
 */
typedef struct {
	int name;
	char fontname[MAXFONTNAME+1];
} RTFFont;

/**
 * Structure to describe style
 *
 */
typedef struct {
	int codepage;
} RTFStyle;

/**
 * Structure to store values, local to rtf group
 *
 */
typedef struct {
	int uc;						/**< How much symbols to skip */
	RTFStyle* style;			/**< curren style */
	int mbcs;
	int codepage;
	short int *charset;
} RTFGroupData;

/**
 * Structure to store font table entry
 * 
 */
typedef struct {
	int id;		/* identifier of for entry */
	int charset;	/* charset */
	int codepage;	/* codepage */
} RTFFontTableEntry;

/********************************************************
 * Functions declaration
 *
 */

extern int forced_charset;
static signed long getNumber(FILE *f);

static int getRtfCommand(FILE *f, RTFcommand *command );
static unsigned short int rtf_to_unicode(int code, short int *charset);
static RTFTypes getCommandType(char *name);
static signed int getCharCode(FILE *f);
static void rtfSetCharset(RTFGroupData *group);
static void loadFontTable(FILE *f);
static int loadFontTableEntry(FILE *f, RTFFontTableEntry *entry);
static RTFFontTableEntry *lookupFontTableEntry(int id);

/********************************************************
 * Global data
 * 
 */

#define FONT_TABLE_MAX 50
static RTFFontTableEntry fontTable[FONT_TABLE_MAX];
static int font_table_entries;

/********************************************************
 * Functions implementation
 *
 */

/* load and parse an individual font entry */
static int loadFontTableEntry(FILE *f, RTFFontTableEntry *entry) {
	int level = 0;
	if (!f || !entry)
	    return -1;
	/* label id as invalid */
	entry->id = -1;
	/* check opening brace */
	int c = fgetc(f);
	if (feof(f))
		return -1;
	if (c != '{')
		return -1;
	level = 1;
	do {
	    int c = fgetc(f);
	    if (feof(f))
		return -1;
	    switch (c) {
		case '\\':
		{
			int code;
			RTFcommand com;
			if ((code=getRtfCommand(f, &com)) != 0)
				break;
			switch (com.type) {
				case RTF_F:
					entry->id = com.numarg;
					break;
				case RTF_FONT_CHARSET:
					entry->charset = com.numarg;
					break;
				case RTF_CODEPAGE:
					entry->codepage = com.numarg;
					break;
				default:
				    break;
			}
		}
		break;
		case '\r':
		    break;
		case '\n':
		    break;
		case '{' :
		    level++;
		    break;
		case '}' :
		    level--;
		    break;
		default:
		    break;
	    }
	} while (level > 0);
    return entry->id < 0 ? -1:0;
}

static RTFFontTableEntry * lookupFontTableEntry(int id) {
    int i;
    for (i = 0; i < font_table_entries; i++) {
	if (fontTable[i].id == id)
	    return &fontTable[i];
    }
    return NULL;
}

static void loadFontTable(FILE *f) {
    int c;
    font_table_entries = 0;
    do {
	c = getc(f);
	if (feof(f))
	    return;
	if (c == '{') {
	    ungetc('{',f);
	    /* load entry */
	    if (loadFontTableEntry(f,&fontTable[font_table_entries]) == 0) {
		if (font_table_entries < (FONT_TABLE_MAX - 1))
			font_table_entries++;
	    }
	}
	else if ((c != '\r') && (c != '\n'))
	    break;
    } while(1);
}

extern unsigned short int buffer[];
static void add_to_buffer(int *bufptr,unsigned short int c) {
	buffer[++(*bufptr)]=c;
	if (*bufptr > PARAGRAPH_BUFFER-2) {
		buffer[++(*bufptr)]=0;
		output_paragraph(buffer);
		*bufptr=-1;
	}
}

static void end_paragraph(int *bufptr) {
				   add_to_buffer(bufptr,0x000a);
				   add_to_buffer(bufptr,0);
				   output_paragraph(buffer);
				   *bufptr=-1;
}				   

typedef struct {
	int index;
	int codepage;
} CharsetTypeMap;

static CharsetTypeMap charset_types[]={
	{0, 1252},    // Ansi
	{1, 0},       // Default
	{2, 42},      // Symbol
	{77, 10000},  // Mac Roman
	{78, 10001},  // Mac Shift Jis
	{79, 10002},  // Mac Hangul
	{80, 10008},  // Mac GB2312
	{81, 10002},  // Mac Big5
	{82, 0},      // Mac Johab (old)
	{83, 10005},  // Mac Hebrew
	{84, 10004},  // Mac Arabic
	{85, 10006},  // Mac Greek
	{86, 10081},  // Mac Turkish
	{87, 10021},  // Mac Thai
	{88, 10029},  // Mac East Europe
	{89, 10007},  // Mac Russian
	{128, 932},   // Shift Jis
	{129, 949},   // Hangul
	{130, 1361},  // Johab
	{134, 956},   // GB2312
	{135, 950},   // Big5
	{161, 1253},  // Greek
	{162, 1254},  // Turkish
	{163, 1258},  // Vietnamese
	{177, 1255},  // Hebrew
	{178, 1256},  // Arabic
	{179, 0},     // Arabic Traditional (old)
	{180, 0},     // Arabic User (old)
	{181, 0},     // Hebrew User (old)
	{186, 1257},  // Baltic
	{204, 1251},  // Russian
	{222, 874},   // Thai
	{238, 1250},  // Eastern European
	{254, 437},   // PC 437
	{255, 850},   // OEM
};

static int charsetCodepage(int charset_index) {
	int i, olen=sizeof(charset_types)/sizeof(CharsetTypeMap);
	for (i = 0; i < olen ; i++) {
		if ( charset_index == charset_types[i].index ) {
			return charset_types[i].codepage ? charset_types[i].codepage : 1252;
		}
	}
	return 1252;
}

typedef struct {
	int lcid;
	int codepage;
} LCIDTypeMap;

/* Windows XP Proffessional and Home 
 LCID to ansi codepage table
*/

static LCIDTypeMap lcid_types[]={
	{1025, 1256}, // Arabic
	{1046, 1252}, // Brazilian
	{2052, 936},  // Chinese Simplified
	{1028, 950},  // Chinese Trad
	{3076, 950},  // Chinese HK
	{1029, 1250}, // Czech
	{1030, 1252}, // Danish
	{1043, 1252}, // Dutch
	{1033, 1252}, // English
	{1035, 1252}, // Finnish
	{1036, 1252}, // French
	{1031, 1252}, // Greman
	{1032, 1253}, // Greek
	{1037, 1255}, // Hebrew
	{1038, 1250}, // Hungarian
	{1040, 1252}, // Italian
	{1041, 932},  // Japanese
	{1042, 949},  // Korean
	{1044, 1252}, // Norwegian
	{1045, 1250}, // Polish
	{2070, 1252}, // Portuguese
	{1049, 1251}, // Russian
	{3082, 1252}, // Spanish
	{1053, 1252}, // Swedish
	{1055, 1254}, // Turkish

/* lcid to codepage data for Win2K
 http://www.science.co.il/Language/Locale-Codes.asp?s=codepage
*/

	{1098, 0},    // Telugu
	{1095, 0},    // Gujarati
	{1094, 0},    // Punjabi
	{1103, 0},    // Sanskrit
	{1111, 0},    // Konkani
	{1114, 0},    // Syriac
	{1099, 0},    // Kannada
	{1102, 0},    // Marathi
	{1125, 0},    // Divehi
	{1067, 0},    // Armenian
	{1081, 0},    // Hindi
	{1079, 0},    // Georgian
	{1097, 0},    // Tamil
	{1054, 874},  // Thai
	{1041, 932},  // Japanese
	{2052, 936},  // Chinese (PRC)
	{4100, 936},  // Chinese (Singapore)
	{1042, 949},  // Korean
	{5124, 950},  // Chinese (Macau)
	{3076, 950},  // Chinese (HK)
	{1028, 950},  // Chinese (Taiwan)
	{1048, 1250}, // Romanian
	{1060, 1250}, // Slovenian
	{1038, 1250}, // Hungarian
	{1051, 1250}, // Slovak
	{1045, 1250}, // Polish
	{1052, 1250}, // Albanian
	{2074, 1250}, // Serbian (Latin)
	{1050, 1250}, // Croatian
	{1029, 1250}, // Czech
	{1104, 1251}, // Mongolian (Cyrillic)
	{1071, 1251}, // FYRO Macedonia
	{2115, 1251}, // Uzbek (Cyrillic)
	{1058, 1251}, // Ukranian
	{2092, 1251}, // Azeri (Cyrillic)
	{1092, 1251}, // Tartar
	{1087, 1251}, // Khazakh
	{1059, 1251}, // Belarusian
	{1088, 1251}, // Kyrgyz (Cyrillic)
	{1026, 1251}, // Bulgarian
	{3098, 1251}, // Serbian (Cyrillic)
	{1049, 1251}, // Russian
	{1032, 1253}, // Greek

	// all the missing ones here are 1252
	// and not instantaited
	{1091, 1254}, // Uzbek (Latin)
	{1068, 1254}, // Azeri (Latin)
	{1055, 1254}, // Turkish
	{1037, 1255}, // Hebrew
	{5121, 1256}, // Arabic (Algeria)
	{15361, 1256},// Arabic (Bahrain)
	{9217, 1256}, // Arabic (Yemen)
	{3073, 1256}, // Arabic (Egypt)
	{2049, 1256}, // Arabic (Iraq)
	{11265, 1256},// Arabic (Jordan)
	{13313, 1256},// Arabic (Kuwait)
	{12289, 1256},// Arabic (Lebanon)
	{4097, 1256}, // Arabic (Libya)
	{6145, 1256}, // Arabic (Morocco)
	{8193, 1256}, // Arabic (Oman)
	{16385, 1256},// Arabic (Qatar)
	{14337, 1256},// Arabic (UAE)
	{1065, 1256}, // Farsi
	{1056, 1256}, // Urdu
	{7169, 1256}, // Arabic (Tunisia)
	{1061, 1257}, // Estonian
	{1062, 1257}, // Latvian
	{1063, 1257}, // Lithuanian
	{1066, 1257} // Vietnamese
};

static int lcidCodepage(int localeid) {
	int i, olen=sizeof(lcid_types)/sizeof(LCIDTypeMap);
	for (i = 0; i < olen ; i++) {
		if ( localeid == lcid_types[i].lcid ) {
			return lcid_types[i].codepage ? lcid_types[i].codepage : 1252;
		}
	}
	return 1252;
}

typedef struct {
	short int *charset;
	int codepage;
} CharsetMap;

#define CHARSET_TABLE_MAX 20
CharsetMap charsets[CHARSET_TABLE_MAX];
int charset_table_count;
short int *getCharset(int codepage) {
	int i;
	for (i = 0; i < charset_table_count; i++)
	    if (charsets[i].codepage == codepage)
			return charsets[i].charset;
	return NULL;
}

short int *getDefaultCharset(void) {
	return charset_table_count ? charsets[0].charset : NULL;
}

void addCharset(short int *charset, int codepage) {
	if ((getCharset(codepage) == NULL) && (charset_table_count < CHARSET_TABLE_MAX)) {
		charsets[charset_table_count].codepage = codepage;
		charsets[charset_table_count].charset = charset;
		charset_table_count++;
	}
}

/** 
 * Parses RTF file from file stream
 * 
 * @param f - file stream descriptor
 */

int parse_rtf(FILE *f) {
	int para_mode=0, data_skip_mode=0,i;
	RTFGroupData *groups=NULL;
	int group_count=0, group_store=20;
	// this makes sure we have an mbcs lookup table available
	(void)read_charset("shiftjis");
	// and now set it back to the original
	(void)read_charset(source_csname);
	int bufptr=-1;
	fseek(f,0,SEEK_SET);
	if((groups=(RTFGroupData*)calloc(group_store,sizeof(RTFGroupData))) == NULL ) {
		perror("Can\'t allocate memory: ");
		return 1;
	}
	groups[0].uc = 1; /* RTF spec says DEfault uc = 1 */
	groups[0].mbcs = 0; /* assume not using multibyte characters */
	groups[0].codepage = 1252;
	groups[0].charset = source_charset;
	while ( !feof(f) ) {
		int c = fgetc(f);
		if ( feof( f ) )
			break;
		switch (c) {
		case '\\': {
			int code;
			RTFcommand com;
			if ((code=getRtfCommand(f, &com)) != 0)
				break;
			switch (com.type) {
			case RTF_SPEC_CHAR:
				if (com.numarg == '*' && data_skip_mode == 0) {
					data_skip_mode=group_count;
				} else if (com.numarg == '\r') {
					end_paragraph(&bufptr);
				} else if (com.numarg == '~') {
					add_to_buffer(&bufptr,0xA0);/* NO-BREAK SPACE */
				} else if (com.numarg == '-') {
					add_to_buffer(&bufptr,0xAD);/* Optional hyphen */
				}
				   break;
			case RTF_EMDASH:
				   add_to_buffer(&bufptr,0x2014);/* EM DASH*/
				   break;
			case RTF_ENDASH: 
				   add_to_buffer(&bufptr,0x2013);break;
			case RTF_BULLET: 
				   add_to_buffer(&bufptr,0x2022);break;
			case RTF_LQUOTE: add_to_buffer(&bufptr,0x2018);break;
			case RTF_RQUOTE: add_to_buffer(&bufptr,0x2019);break;
			case RTF_LDBLQUOTE: add_to_buffer(&bufptr,0x201C);break;
			case RTF_RDBLQUOTE: add_to_buffer(&bufptr,0x201D);break;
			case RTF_ZWNONJOINER: add_to_buffer(&bufptr,0xfeff);break;
			case RTF_EMSPACE:
			case RTF_ENSPACE:
					add_to_buffer(&bufptr,' ');break;
			case RTF_CHAR:
				if (data_skip_mode == 0) {
					short int *charset = groups[group_count].charset;
					// check for multibyte characters - filter check on DBCS lead bytes as unicode charset cp932
					if (groups[group_count].mbcs &&
						(
						((com.numarg >= 0x81) && (com.numarg <= 0x9f)) ||
						((com.numarg >= 0xe0) && (com.numarg <= 0xfc))
						)
					) {
						// is next char a command lead-in
						int next_char = fgetc(f);
						if (next_char == '\\') {
							next_char = fgetc(f);
							// is it an escaped character?
							if (next_char == '\'') {
								RTFcommand com2;
								ungetc(next_char,f);
								next_char=getRtfCommand(f, &com2);
								if ((next_char != -1) && (com2.type == RTF_CHAR)) {
									// if a trailing byte in mcbs 2nd byte range
									if ((com2.numarg >= 0x40) && (com2.numarg <= 0xfc))
										// add mbcs char
										add_to_buffer(&bufptr, rtf_to_unicode((int) (((unsigned char)com.numarg) << 8) | (unsigned char)(com2.numarg), charset));
									else {
										// else add as 2 hich bytes
										add_to_buffer(&bufptr,rtf_to_unicode(com.numarg, charset));
										add_to_buffer(&bufptr,rtf_to_unicode(com2.numarg, charset));
									}
								}
								// screwup in 2nd byte. Add hich char
								else
									add_to_buffer(&bufptr,rtf_to_unicode(com.numarg,charset));
							}
							// not a escaped character
							else {
								// push back values
								ungetc(next_char,f);
								ungetc('\\',f);
								// add hich char
								add_to_buffer(&bufptr,rtf_to_unicode(com.numarg, charset));
							}
						}
						// not a command following
						else {
							// push back values
							ungetc(next_char,f);
							// add hich char
							add_to_buffer(&bufptr,rtf_to_unicode(com.numarg, charset));
						}
					}
					else
						add_to_buffer(&bufptr,rtf_to_unicode(com.numarg, charset));
				}
				break;
			case RTF_UC:
				groups[group_count].uc=com.numarg;
				break;
			case RTF_TAB:
				add_to_buffer(&bufptr,0x0009);
				break;
			case RTF_UNICODE_CHAR:
				if (com.numarg < 0)
					break;
				if (data_skip_mode == 0)
					add_to_buffer(&bufptr,com.numarg);
				i=groups[group_count].uc;
				if (i > 0) {
					while (i--) {
						c = fgetc(f);
						// are we reading and skipping a control sequence?
						if (c == '\\') {
							// bin it (likely a \'xx value)
							getRtfCommand(f, &com);
						}
					}
				}
				break;
			case RTF_PARA:
				/* *** CHECKME *** if (para_mode > 0) {*/
					end_paragraph(&bufptr);	
				/*}*/	
				para_mode=group_count;
				break;
			case RTF_PICT:
			case RTF_FONTTBL:
				loadFontTable(f);
				break;
			case RTF_F:
				{
					RTFFontTableEntry *entry = lookupFontTableEntry(com.numarg);
					if (entry) {
						if (!entry->codepage)
							entry->codepage = charsetCodepage(entry->charset);
						if (entry->codepage != groups[group_count].codepage) {
							groups[group_count].codepage = entry->codepage;
							rtfSetCharset(&groups[group_count]);
						}
					}
				}
				break;
			case RTF_INFO:
			case RTF_COLORTBL:
			case RTF_STYLESHEET:
			case RTF_LISTTABLE:
			case RTF_LISTOVERRIDETABLE:
			case RTF_RSIDTBL:
			case RTF_GENERATOR:
			case RTF_DATAFIELD:
				if (data_skip_mode == 0){
					data_skip_mode=group_count;
				}
				break;
			case RTF_LANG:
/* 				fprintf(stderr, "Selected lang = %d\n",com.numarg); */ 
				groups[group_count].codepage = lcidCodepage(com.numarg);
				rtfSetCharset(&groups[group_count]);
				break;
			case RTF_DEFLANG:
			case RTF_DEFLANGFE:
				groups[group_count].codepage = lcidCodepage(com.numarg);
				rtfSetCharset(&groups[group_count]);
				break;
			case RTF_FONT_CHARSET:
				groups[group_count].codepage = charsetCodepage(com.numarg);
				rtfSetCharset(&groups[group_count]);
				break;
			case RTF_CODEPAGE:
				groups[group_count].codepage = com.numarg;
				rtfSetCharset(&groups[group_count]);
				break;
			case RTF_PLAIN:
				groups[group_count].mbcs = 0;
				groups[group_count].codepage = groups[0].codepage;
				rtfSetCharset(&groups[group_count]);
				break;
			case RTF_LOCH:
				groups[group_count].mbcs = 1;
				break;
			case RTF_HICH:
				groups[group_count].mbcs = 1;
				break;
			case RTF_DBCH:
				groups[group_count].mbcs = 1;
				break;
			case RTF_INDEX:
				{
					int current_group = group_count;
					/* skip all of current group */
					do {
						c = fgetc(f);
						if (c == '{')
							group_count++;
						else if (c == '}')
							group_count--;
					} while (group_count >= current_group);
					ungetc('}',f);
				}
				break;
		default:
/*  				fprintf(stderr, "Unknown command with name %s and arg=%d\n",  */
/*  						com.name, com.numarg);  */
			;
			}
			break;
		}
		case '{':
			group_count++;
			if (group_count >= group_store ) {
				group_store+=10;
				if((groups=(RTFGroupData*)realloc(groups,
					group_store*sizeof(RTFGroupData)))
					== NULL ) {
					perror("Can\'t allocate memory: ");
					return 1;
				}
			}
// this looks wrong - removed pending review FIXME
//			if (para_mode)
//				add_to_buffer(&bufptr,0x20);
			groups[group_count]=groups[group_count-1];
			break;
		case '}':
			group_count--;
			if(group_count < 0)
				group_count=0;
			if(para_mode > 0 && para_mode > group_count) {
				/*add_to_buffer(&bufptr,0);
				output_paragraph(buffer);
				fprintf(stderr,"\nGROUP_END para_mode=%d group_count=%d bufptr=%d\n", para_mode,group_count,bufptr);
				bufptr=-1;*/
				para_mode=0;
			}
			if(data_skip_mode > group_count) {
				data_skip_mode=0;
			}
			break;
		default:
			if (data_skip_mode == 0)
				if (c != '\n' && c != '\r')
					add_to_buffer(&bufptr,rtf_to_unicode(c, groups[group_count].charset));
		}
	}
	if (bufptr>=0) {
		add_to_buffer(&bufptr,'\n');
		add_to_buffer(&bufptr,0);
		output_paragraph(buffer);
	}	
	free(groups);
	return 0;
}

/**
 * Convert text string to number
 * 
 * @param f stream to read data from
 *
 * @return converted number
 */

static signed long getNumber(FILE *f) {
	int c,count=0;
	char buf[RTFARGSMAXLEN];
	
	while(isdigit(c=fgetc(f)) || c=='-') {
		if(feof(f))
			return -1;
		buf[count++]=(char)c;
	}
	ungetc(c,f);
	buf[count]='\0';
	return strtol(buf, (char **)NULL, 10);
}

/**
 * Parse command stream from rtf file and fill command structure
 *
 * @param f - rtf file stream
 * @param command - pointer to RTFcommand structure to fill
 *
 * @return parse code not 0 - error, 0 - success
 */

static int getRtfCommand(FILE *f, RTFcommand *command ) {
	int c=fgetc(f);
	if (isalpha(c)) {
		int name_count=1;
		command->name[0]=(char)c;
		while(isalpha(c=fgetc(f)) && name_count < RTFNAMEMAXLEN) {
			if(feof(f))
				return 1;
			command->name[name_count++]=(char)c;
		}
		command->name[name_count]='\0';
		command->type=getCommandType(command->name);
/* 		command->args=NULL; */
		ungetc(c,f);
		if (isdigit(c) || c == '-' )
			command->numarg=getNumber(f);
		else
			command->numarg=0;
		c=fgetc(f);
		if(!(c==' ' || c=='\t'))
			ungetc(c,f);
	} else {
		command->name[0]=(char)c;
		command->name[1]='\0';
/* 		command->args=NULL; */
		if (c == '\'') {
			command->type=RTF_CHAR;
			command->numarg=getCharCode(f);
			if(feof(f))
				return -1;
		} else {
			command->type=RTF_SPEC_CHAR;
			command->numarg=c;
		}
	}
	
	return 0;
}

/**
 * Converts char to unicode.
 *
 * @param code - integer code of char
 *
 * @return converted char
 */
static unsigned short int rtf_to_unicode(int code, short int *charset) {
	int cc=code;
	if (code < 0 || (cc=to_unicode(charset, code)) < 0 ) return 0xFEFF;
	return cc;
}

/**
 * Convert name of RTF command to RTFType
 *
 * @param name name to convert
 *
 * @return RTFType, if unknown command, then return RTF_UNKNOWN
 */

static RTFTypes getCommandType(char *name) {
	int i, olen=sizeof(rtf_types)/sizeof(RTFTypeMap);
	for (i = 0; i < olen ; i++) {
		if ( strcmp(name,rtf_types[i].name) == 0 ) {
			return rtf_types[i].type;
		}
	}
	return RTF_UNKNOWN;
}

/**
 * Return number representing char code in Hex
 *
 * @param f stream to read data from
 *
 * @return converted number
 */

static signed int getCharCode(FILE *f) {
	int c,count=0,i;
	char buf[RTFARGSMAXLEN];
	for(i=0;i<2; i++) {
		if (isdigit(c=fgetc(f))||(c>='a' && c<='f')) {
			if(feof(f))
				return -1;
			buf[count++]=(char)c;
		} else 
			ungetc(c,f);
	}

	buf[count]='\0';
	return strtol(buf, (char **)NULL, 16);
}

static void rtfSetCharset(RTFGroupData *group)
{
	const char *charset_name;
	char *save_buf = input_buffer;
	if (forced_charset) return;
	if (getCharset(group->codepage)) return;
	charset_name = charset_from_codepage(group->codepage);
	check_charset(&source_csname,charset_name);
	input_buffer=NULL;
//	if (group->charset && *group->charset) {
//		free(group->charset);
//		group->charset = NULL;
//	}
	addCharset(read_charset(source_csname), group->codepage);
	group->charset = getCharset(group->codepage);
	if (!group->charset)
		group->charset = getDefaultCharset();
	input_buffer = save_buf;
}

