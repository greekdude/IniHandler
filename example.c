#include <stdio.h>
#include <stdlib.h>
#include "IniHandler.h"

void print_ini( _ini* );

int main( int argc, char *argv[] ){
	_ini *config = createIni( "settings.ini" );

	if( config == NULL ){
		printf( "Could not allocate the proper amount of memory\n" );
		exit( EXIT_FAILURE );
	}

	print_ini( config );
	printf( "\n\n" );

	// Removed user from group 'General'
	removeElement( config, "General", "user" );

	// add name to group 'user'
	// if the group does not exist, it will be created
	addElement( config, "User", "name", "greekdude" );

	_ini *config2 = createIni( "updated_settings.ini" );

	if( config2 == NULL ){
		printf( "Could not allocate the proper amount of memory\n" );
		exit( EXIT_FAILURE );
	}

	print_ini( config2 );
	printf( "\n\n" );

	// if we merge or add element with config being the destination, then any existing values will be changed
	setIniFlag( config, OVERWRITE );

	// when we merge or add element with config as the destination, if the key already exists, it will not be changed
	removeIniFlag( config, OVERWRITE );

	// move all of config2 into config
	mergeIni( config, config2 );

	print_ini( config );
	printf( "\n\n" );

	saveIni( config, "savetest.ini" );

	// frees all data from memory
	destroyIni( config );
	config = NULL;

	destroyIni( config2 );
	config2 = NULL;

	return 0;
}

void print_ini( _ini* config ){
	if( config != NULL  ){
		_inigroup *tmp = config->groups;
		while( tmp != NULL ){
			printf( "[%s]\n", tmp->group_name );

			_inielement *pair = tmp->elements;
			while( pair != NULL ){
				printf( "%s\t= %s\n", pair->key, pair->value );

				pair = pair->next;
			}

			tmp = tmp->next;
		}
	}
}

