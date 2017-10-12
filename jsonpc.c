#include <jak_dbg.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>




struct beautify_arg {
	int c;
	int c_p;
	int c_l;
	int c_t;
	int c_b;
	int c_r;
	int c_n;
	int c_e;
	int indent;
	int in_dd;
	int is_txt;
};
static int beautify( struct beautify_arg * b, char * json, FILE * file, int (*fprintf_func)( FILE *, const char *, ... ) ) {

	int i;
	int j;
#define P_BUF_SIZE 128
	char buf[ P_BUF_SIZE ];


#define p( some_char ) \
	{ \
		buf[ i++ ] = some_char; \
		if( i == P_BUF_SIZE ) { \
			fprintf_func( file, "%.*s", i, buf ); \
			i = 0; \
		} \
	}

#define p_new_line() \
	{ \
		p( '\n' ); \
		for( j = 0; j < b->indent; ++j ) p( ' ' ); \
	}

	i = 0;
	while( *json ) {
		if( b->is_txt ) {
			if( *json == '"' ) b->is_txt = 0;
			p( *json );
		} else {
			if( ! isspace( *json ) ) {
				if( *json == ']' || *json == '[' || *json == '{' || *json == '}' || *json == ',' ) {
					if( *json == ']' || *json == '}' ) {
						b->indent -= b->in_dd;
						check_msg( b->indent >= 0, final_cleanup, "Malformed json." );
					}
					if( *json != ',' ) p_new_line();
					if( *json == '[' || *json == '{' ) b->indent += b->in_dd;
					p( *json );
					if(
							*( json + 1 ) != ']'
							&& *( json + 1 ) != '}'
							&& *( json + 1 ) != ','
							&& ( *json != ',' || (
									*( json + 1 ) != '{'
									&& *( json + 1 ) != '['
									)
							   )
					  ) p_new_line();
				} else {
					p( *json );
				}
			}
		}
		json++;
	}
	fprintf_func( file, "%.*s\n", i, buf );

	return 0;

final_cleanup:
	return -1;

}

static int hex_to_term256( int * res, char * hex ) {

	char * end_ptr;

	float rgb[3];
	char hex_part[3];
	int i;

#define P_ERR_MSG "Wrong color value: \"%s\". Must be a '\\0' terminated hex color string."

	check_msg_v( hex[0] == '#' && hex[7] == '\0', final_cleanup, P_ERR_MSG, hex );

	end_ptr = NULL;
	hex_part[2] = '\0';

	for( i = 0; i < 3; ++i ) {
		errno = 0;
		hex_part[0] = hex[ i * 2 + 1 ];
		hex_part[1] = hex[ i * 2 + 2 ];
		rgb[i] = strtol( hex_part, &end_ptr, 16 ) / 255.0f;
		check_msg_v( errno == 0 && end_ptr == hex_part + 3, final_cleanup, P_ERR_MSG, hex );
		check( rgb[i] <= 1.0f, final_cleanup );
	}

	//TODO add an if for the base 16 colours and the grayscale strip
	*res = round( 36 * ( rgb[0] * 5) + 6 * ( rgb[1] * 5 ) + ( rgb[2] * 5 ) + 16 );

	return 0;

final_cleanup:
	*res = -1;
	return -1;

}

int main( int argc, char * * argv ) {

	int rc;

	int op;
	struct beautify_arg b;
	char * file_path = NULL; //config file path
	static struct option long_options[] = {
		{ "colors", no_argument, 0, 'c' },
		{ "punctuation-color", required_argument, 0, 'p' },
		{ "labels-color", required_argument, 0, 'l' },
		{ "text-values-color", required_argument, 0, 't' },
		{ "boolean-values-color", required_argument, 0, 'b' },
		{ "number-values-color", required_argument, 0, 'r' },
		{ "null-values-color", required_argument, 0, 'n' },
		{ "syntax-errors-color", required_argument, 0, 'e' },
		{ "indent", required_argument, 0, 'i' },
		{ "config-file", required_argument, 0, 'f' }
	};
	int option_index = 0;
	char * json;


	// Options defaults:
	b.c = 0; //display colored output
	b.c_p = 0; //punctuations colors
	b.c_l = 88; //labels colors
	b.c_t = 88; //text values colors
	b.c_b = 20; //boolean values colors
	b.c_r = 34; //number values colors
	b.c_n = 20; //null values colors
	b.c_e = 196; //syntax errors colors
	b.in_dd = 2; //indent for n spaces

	b.indent = 0;
	b.is_txt = 0;

#undef p
#define p( cc, some_var ) case cc: \
	rc = hex_to_term256( &b.some_var, optarg ); \
	check( rc == 0, final_cleanup ); \
	break

	while( 1 ) {
		op = getopt_long( argc, argv, "cp:l:t:b:r:n:e:i:f:", long_options, &option_index );
		if( op == -1 ) break;
		check( op == 'c' || optarg, final_cleanup );
		switch( op ) {
			case 'c':
				b.c = 1;
				break;
				p( 'p', c_p );
				p( 'l', c_l );
				p( 't', c_t );
				p( 'b', c_b );
				p( 'r', c_r );
				p( 'n', c_n );
				p( 'e', c_e );
			case 'i':
				b.in_dd = atoi( optarg );
				break;
			case 'f':
				file_path = optarg;
				break;
		}
	}

	if( optind < argc ) {
		check_msg( optind == argc - 1, final_cleanup, "Too many command line arguments." );
		json = argv[ optind ];
		printf( "\n" );
		rc = beautify( &b, json, stdout, fprintf );
		check( rc == 0, final_cleanup );
	} else {
		printf(
				"We have no json to beautify :(.\n"
				"Unix pipes are not supported for now.\n"
				"Please use command line arguments or the xargs program for now to pass your json to the program.\n"
				"Thanks :)\n"
		      );
		return 2;
	}

	return 0;

final_cleanup:
	return 1;

}
