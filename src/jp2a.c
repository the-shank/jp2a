/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Copyright 2020 Christoph Raitzig
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <locale.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "jp2a.h"
#include "options.h"
#include "image.h"
#include "curl.h"

#ifdef WIN32
#ifdef FEAT_CURL
#include <io.h>
#define close _close
#endif
#include <fcntl.h>
#endif

int main(int argc, char** argv) {
	int store_width, store_height, store_autow, store_autoh;
	FILE *fout = stdout;
#ifdef FEAT_CURL
	FILE *fr;
	int fd;
#endif
	FILE *fp;
	int n;

#if ! ASCII
	setlocale(LC_ALL, "");
#endif

	parse_options(argc, argv);

	store_width = width;
	store_height = height;
	store_autow = auto_width;
	store_autoh = auto_height;

	if ( strcmp(fileout, "-") ) {
		if ( (fout = fopen(fileout, "wb")) == NULL ) {
			fprintf(stderr, "Could not open '%s' for writing.\n", fileout);
			free(html_title);
			return 1;
		}
	}

	if ( html && !html_rawoutput ) print_html_document_start(html_fontsize, fout);
	else if ( xhtml && !html_rawoutput ) print_xhtml_document_start(html_fontsize, fout);
	free(html_title);

	for ( n=1; n<argc; ++n ) {

		width = store_width;
		height = store_height;
		auto_width = store_autow;
		auto_height = store_autoh;

		// skip options
		if ( argv[n][0]=='-' && argv[n][1] )
			continue;

		// read from stdin
		if ( argv[n][0]=='-' && !argv[n][1] ) {
			#ifdef _WIN32
			// Good news, everyone!
			_setmode( _fileno( stdin ), _O_BINARY );
			#endif

			decompress_jpeg(stdin, fout);
			continue;
		}

		#ifdef FEAT_CURL
		if ( is_url(argv[n]) ) {

			if ( verbose )
				fprintf(stderr, "URL: %s\n", argv[n]);

			fd = curl_download(argv[n], debug);

			if ( (fr = fdopen(fd, "rb")) == NULL ) {
				fputs("Could not fdopen read pipe\n", stderr);
				return 1;
			}

			decompress_jpeg(fr, fout);
			fclose(fr);
			close(fd);
			
			continue;
		}
		#endif

		// read files
		if ( (fp = fopen(argv[n], "rb")) != NULL ) {
			if ( verbose )
				fprintf(stderr, "File: %s\n", argv[n]);

			decompress_jpeg(fp, fout);
			fclose(fp);

			continue;

		} else {
			fprintf(stderr, "Can't open %s\n", argv[n]);
			return 1;
		}
	}

	if ( html && !html_rawoutput ) print_html_document_end(fout);
	else if ( xhtml && !html_rawoutput ) print_xhtml_document_end(fout);

	if ( fout != stdout )
		fclose(fout);

	return 0;
}
