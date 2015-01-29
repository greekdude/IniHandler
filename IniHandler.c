#include "IniHandler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>

_ini* createIni( char *file ){
	_ini *config = calloc( 1, sizeof( _ini ) );

	if( config != NULL ){
		config->groups = NULL;

		if( file != NULL ){
			FILE *ini_file = fopen( file, "r" );

			if( ini_file != NULL ){
				// because fgets appends a \0 at the end, we need to make our buffer size longer for that
				unsigned int buffer_size = BUFFER_SIZE + 1;
				char *buffer = calloc( buffer_size, sizeof( char ) );
				if( buffer == NULL ){
					exit( EXIT_FAILURE );
				}

				char *group = NULL;
				char *extended_buffer = NULL;

				unsigned char no_newline_count = 0;
				while( fgets( buffer, buffer_size, ini_file ) != NULL ){
					if( no_newline_count >= BUFFER_ITERATIVE_THRESHOLD ){
						if( strchr( buffer, '\n' ) != NULL || feof( ini_file ) ){
							if( extended_buffer != buffer ){
								free( extended_buffer );
							}

							extended_buffer = NULL;
							no_newline_count = 0;
						}

						continue;
					}

					// if the buffer contains a newline or is at eof, process the data
					if( strchr( buffer, '\n' ) != NULL || feof( ini_file ) ){
						if( extended_buffer == NULL ){
							extended_buffer = buffer;
						}else{
							// the extended buffer was used, so now we need to add the new buffer to the extended buffer
							unsigned int new_size = buffer_size + ( ( no_newline_count - 1 ) * BUFFER_SIZE ) + strlen( buffer );

							extended_buffer = realloc( extended_buffer, new_size );
							strncat( extended_buffer, buffer, strlen( buffer ) );
						}

						// ignore all lines beginning with #
						if( *extended_buffer != '#' ){
							// save group name
							if( *extended_buffer == STARTGROUP ){
								ptrdiff_t group_size = ( ptrdiff_t )( strrchr( extended_buffer, CLOSEGROUP ) - extended_buffer );
								group = calloc( group_size, sizeof( char ) );
								if( group == NULL ){
									exit( EXIT_FAILURE );
								}

								strncpy( group, (extended_buffer + 1), group_size -1 );
								group[ group_size - 1 ] = '\0';
							}else{
								char *key_val = strchr( extended_buffer, DELIMITER );

								// ignore lines that do not contain the DELIMITER
								if( key_val != NULL ){
									char *key = NULL;
									char *value = NULL;

									ptrdiff_t key_size = (ptrdiff_t)( key_val - extended_buffer ) + 1;
									key = calloc( key_size, sizeof( char ) );
									if( key == NULL ){
										exit( EXIT_FAILURE );
									}

									strncpy( key, extended_buffer, key_size - 1 );

									ptrdiff_t value_size;

									if( ! feof( ini_file ) ){
										// if it is not end of file, then we do not add +1 because there is a newline at the end of buffer
										value_size = strlen( extended_buffer ) - key_size;
									}else{
 										// at the end of file, there is no newline, so we grab all the data
										value_size = strlen( extended_buffer ) - key_size + 1;
									}

									value = calloc( value_size, sizeof( char ) );
									if( value == NULL ){
										exit( EXIT_FAILURE );
									}

									strncpy( value, (key_val + 1), value_size - 1 );

									addElement( config, group, key, value );

									if( key ){
										free( key );
										key = NULL;

										free( value );
										value = NULL;
									}
								}
							}
						}

						if( extended_buffer != buffer ){
							free( extended_buffer );
						}

						extended_buffer = NULL;
						no_newline_count = 0;
					}else{
						// we did not reach a \n so we need to save the current buffer and go again
						no_newline_count++;

						if( extended_buffer == NULL ){
							extended_buffer = calloc( buffer_size , sizeof( char ) );
							if( extended_buffer == NULL ){
								exit( EXIT_FAILURE );
							}

							strncpy( extended_buffer, buffer, buffer_size );
						}else{
							unsigned int new_size = ( ( no_newline_count - 1 ) * BUFFER_SIZE ) + buffer_size;

							extended_buffer = realloc( extended_buffer, new_size );
							strncat( extended_buffer, buffer, strlen( buffer ) );
						}
					}
				}

				if( group ){
					free( group );
					group = NULL;
				}

				fclose( ini_file );
			}else{
				exit( EXIT_FAILURE );
			}
		}
	}

	return config;
}

void destroyIni( _ini *config ){
	while( config->groups != NULL ){
		removeGroup( config, config->groups->group_name );
	}

	free( config );
}

unsigned char saveIni( _ini *config, char *file ){
	unsigned char rtv = GENERICERROR;

	if( config != NULL  ){
		int save = open( file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

		if( save != -1 ){
			rtv = GENERICSUCCESS;
			_inigroup *tmp = config->groups;
			while( tmp != NULL ){
				unsigned int grp_tmp_size = strlen( tmp->group_name ) + 4;
				char *grp_tmp = calloc( grp_tmp_size, sizeof(char) );
				snprintf( grp_tmp, grp_tmp_size, "[%s]\n", tmp->group_name );
				write( save, grp_tmp, grp_tmp_size - 1 );

				_inielement *pair = tmp->elements;
				while( pair != NULL ){
					unsigned int pair_tmp_buf_size = strlen( pair->key ) + strlen( pair->value ) + 3;
					char *pair_tmp = calloc( pair_tmp_buf_size, sizeof(char) );
					snprintf( pair_tmp, pair_tmp_buf_size, "%s=%s\n", pair->key, pair->value );

					write( save, pair_tmp, pair_tmp_buf_size-1 );

					pair = pair->next;

					free( pair_tmp );
				}

				write( save, "\n", 1 );
				tmp = tmp->next;
			}
		}else{
			rtv = GENERICERROR;
		}

		close( save );
	}

	return rtv;
}

void setIniFlag( _ini *config, unsigned char flags ){
	config->flags |= flags;
}

void removeIniFlag( _ini *config, unsigned char flags ){
	flags = ~flags;
	config->flags &= flags;
}

// adds element to config
// if OVERWRITE flag is set, then if there already exists a group/key, it will update the value, otherwise, it will not
unsigned char addElement( _ini* config, char* group, char* key, char *value ){
	unsigned char rtv = FAILEDADDELEMENT;

	_inigroup *current_group = NULL;
	char *tmp_group_name = NULL;

	// if a group is not passed, it will default to General
	if( group != NULL ){
		tmp_group_name = calloc( strlen( group ), sizeof( char ) );
		strcpy( tmp_group_name, group );
	}else{
		tmp_group_name = calloc( 7, sizeof( char ) );
		strcpy( tmp_group_name, "General" );
	}

	char *stripped_group = clipWhitespace( tmp_group_name );
	char *stripped_key = clipWhitespace( key );
	char *stripped_value = clipWhitespace( value );

	_inigroup *tmp_group = config->groups;
	_inigroup *prev_group = config->groups;

	char group_exists = 0;
	while( tmp_group != NULL ){
		if( strcmp( tmp_group->group_name, stripped_group ) != 0 ){
			prev_group = tmp_group;
			tmp_group = tmp_group->next;
		}else{
			group_exists = 1;

			_inielement *prev_element = tmp_group->elements;
			_inielement *current_element = tmp_group->elements;

			char element_exists = 0;
			while( current_element != NULL ){
				if( strcmp( stripped_key, current_element->key ) == 0 ){
					element_exists = 1;
					break;
				}

				prev_element = current_element;
				current_element = current_element->next;
			}

			if( !element_exists ){
				prev_element->next = calloc( 1, sizeof( _inielement ) );
				current_element = prev_element->next;
				if( current_element == NULL ){
					exit( EXIT_FAILURE );
				}

				current_element->key = calloc( strlen( stripped_key ), sizeof( char ) );
				if( current_element->key == NULL ){
					exit( EXIT_FAILURE );
				}

				strcpy( current_element->key, stripped_key );

				current_element->value = calloc( strlen( stripped_value ), sizeof( char ) );
				if( current_element->value == NULL ){
					exit( EXIT_FAILURE );
				}

				current_element->next = NULL;

				strcpy( current_element->value, stripped_value );
				rtv = ELEMENTADDED;
			}else if( (config->flags & OVERWRITE) ){
				if( strcmp( stripped_value, current_element->value ) ){
					free( current_element->value );
					current_element->value = calloc( strlen( stripped_value ), sizeof( char ) );
					if( current_element->value == NULL ){
						exit( EXIT_FAILURE );
					}

					strcpy( current_element->value, stripped_value );
					rtv = ELEMENTADDED;
				}
			}else{
				rtv = FOUNDELEMENT;
			}

			break;
		}
	}

	if( !group_exists ){
		if( prev_group != NULL ){
			prev_group->next = calloc( 1, sizeof( _inigroup ) );
			if( prev_group == NULL ){
				exit( EXIT_FAILURE );
			}

			current_group = prev_group->next;
		}else{
			config->groups = calloc( 1, sizeof( _inigroup ) );
			if( config->groups == NULL ){
				exit( EXIT_FAILURE );
			}

			current_group = config->groups;
		}

		current_group->group_name = calloc( strlen(stripped_group), sizeof( char ) );
		if( current_group->group_name == NULL ){
			exit( EXIT_FAILURE );
		}

		strcpy( current_group->group_name, stripped_group );

		current_group->elements = calloc( 1, sizeof( _inielement ) );
		if( current_group->elements == NULL ){
			exit( EXIT_FAILURE );
		}

		current_group->elements->next = NULL;
		current_group->elements->key = calloc( strlen( stripped_key ), sizeof( char ) );
		if( current_group->elements->key == NULL ){
			exit( EXIT_FAILURE );
		}

		strcpy( current_group->elements->key, stripped_key );

		current_group->elements->value = calloc( strlen( stripped_value ), sizeof( char ) );
		if( current_group->elements->value == NULL ){
			exit( EXIT_FAILURE );
		}

		strcpy( current_group->elements->value, stripped_value );
		rtv = ELEMENTADDED;
	}

	return rtv;
}

// finds a group/key and changes the value
unsigned char changeElement( _ini *config, char *group, char *key, char *value ){
	unsigned char rtv = NOELEMENTFOUND;

	_inigroup *tmp = config->groups;

	while( tmp != NULL ){
		if( strcmp( tmp->group_name, group ) == 0 ){
			_inielement *prev_pair = NULL;
			_inielement *pair = tmp->elements;
			while( pair != NULL ){
				if( strcmp( pair->key, key ) == 0 ){
					rtv = FOUNDELEMENT;
					break;
				}

				prev_pair = pair;
				pair = pair->next;
			}

			if( pair != NULL ){
				free( pair->value );
				pair->value = calloc( strlen( value ), sizeof( char ) );
				strcpy( pair->value, value );

				rtv = ELEMENTUPDATED;
			}

			break;
		}

		tmp = tmp->next;
	}

	return rtv;
}

unsigned char removeElement( _ini *config, char* group, char* key){
	unsigned char rtv = NOELEMENTFOUND;
	_inigroup *tmp = config->groups;

	while( tmp != NULL ){
		if( strcmp( tmp->group_name, group ) == 0 ){
			_inielement *prev_pair = NULL;
			_inielement *pair = tmp->elements;
			while( pair != NULL ){
				if( strcmp( pair->key, key ) == 0 ){
					break;
				}

				prev_pair = pair;
				pair = pair->next;
			}

			if( pair != NULL ){
				if( prev_pair == NULL ){
					tmp->elements = pair->next;
				}else{
					prev_pair->next = pair->next;
				}

				free( pair->key );
				free( pair->value );
				free( pair );

				rtv = ELEMENTREMOVED;
			}

			break;
		}

		tmp = tmp->next;
	}

	return rtv;
}

// removes group and all keys associated with it
unsigned char removeGroup( _ini *config, char *group ){
	unsigned char rtv = NOELEMENTFOUND;

	_inigroup *prev_group = NULL;
	_inigroup *current_group = config->groups;
	while( current_group != NULL ){
		if( strcmp( current_group->group_name, group ) == 0 ){
			while( current_group->elements != NULL ){
				removeElement( config, group, current_group->elements->key );
			}

			if( prev_group == NULL ){
				config->groups = current_group->next;
			}else{
				prev_group->next = current_group->next;
			}

			free( current_group->group_name );
			free( current_group );

			rtv = ELEMENTREMOVED;

			break;
		}
	}

	return rtv;
}

// merges two ini files into destination
_ini* mergeIni( _ini *destination, _ini *second_source ){
	if( second_source != NULL  ){
		_inigroup *tmp = second_source->groups;
		while( tmp != NULL ){
			_inielement *pair = tmp->elements;
			while( pair != NULL ){
				addElement( destination, tmp->group_name, pair->key, pair->value );

				pair = pair->next;
			}

			tmp = tmp->next;
		}
	}

	return destination;
}

char* clipWhitespace( char *data ){
	int start = -1;
	int end = -1;

	int i;
	for( i = 0; *(data + i) != '\0'; i++ ){
		if( start == -1 ){
			if( ! isspace( *(data + i) )  ){
				start = i;
			}
		}else{
			if( end == -1 ){
				if( isspace( *(data + i) ) ){
					end = i;
				}
			}else{
				if( ! isspace( *(data + i) ) ){
					end = -1;
				}
			}
		}
	}

	if( end == -1 ){
		end = strlen( data );
	}

	char* stripped = NULL;

	if( start != -1 ){
		int length = end - start;

		stripped = calloc( length, sizeof( char ) );
		memmove( stripped, (data + start), length );
	}

	return stripped;
}

