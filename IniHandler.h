#ifndef INIHANDLER
#define INIHANDLER

// if the buffer max is met, and a newline is not found, the line will not be processed
// the point of having a threshold is so that the heap can't be filled with junk from one line
#define BUFFER_SIZE		75

// because we use fgets to grab the data, the actual amount we will grab is (BUFFER_ITERATIVE_THRESHOLD * BUFFER_SIZE) - 1(newline) - BUFFER_ITERATIVE_THRESHOLD (\0 for each iteration)
#define BUFFER_ITERATIVE_THRESHOLD	4

#define STARTGROUP		'['
#define CLOSEGROUP		']'
#define DELIMITER		'='

/* Return Codes */
#define NOELEMENTFOUND		0
#define FOUNDELEMENT		1
#define ELEMENTADDED		2
#define ELEMENTUPDATED		3
#define ELEMENTREMOVED		4
#define GENERICSUCCESS		5
#define GENERICERROR		6
#define FAILEDADDELEMENT	7

/* Flag(s) */
// if set, key's values can be overwritten in merge and add_element
#define OVERWRITE 1

typedef struct _inielement _inielement;

struct _inielement{
	char *key;
	char *value;
	_inielement *next;
};

typedef struct _inigroup _inigroup;

struct _inigroup{
	char *group_name;
	_inielement *elements;
	_inigroup *next;
};

// add flag structure
// flag: remove group if no more key value pairs exist
typedef struct _ini{
	_inigroup *groups;
	unsigned char flags;
} _ini;

// takes 'file', generates an _ini struct and returns its address
_ini* createIni( char *file );

// frees all memory of _ini from memory
void destroyIni( _ini *config );

// save _ini to 'file'
unsigned char saveIni( _ini *config, char *file );

void setIniFlag( _ini *config, unsigned char flags );
void removeIniFlag( _ini *config, unsigned char flags );

// adds an element to config.  if the group/key already exists, and OVERWRITE is not set, the value will not be overwritten
unsigned char addElement( _ini *config, char *group, char *key, char *value );

// changes a value regardless of OVERWRITE
unsigned char changeElement( _ini *config, char *group, char *key, char *value );

// frees group/key from memory
unsigned char removeElement( _ini *config, char *group, char *key );

// frees a group, including all its keys from memory
unsigned char removeGroup( _ini *config, char *group );

// adds all elements to config, from second_source.  if a group/key already exists, and OVERWRITE is not set, the value will not be overwritten
// mergeIni uses add element to add the data
_ini* mergeIni( _ini *destination, _ini *second_source );

#endif

