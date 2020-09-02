/*
 *	dtmfsync - Audio/video synchronization using DTMF timestamps
 *	Copyright (C) 2020-2020 Johannes Bauer
 *
 *	This file is part of dtmfsync.
 *
 *	dtmfsync is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; this program is ONLY licensed under
 *	version 3 of the License, later versions are explicitly excluded.
 *
 *	dtmfsync is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with dtmfsync; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Johannes Bauer <JohannesBauer@gmx.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pgmopts.h"
#include "audio_extract.h"

int main(int argc, char **argv) {
	pgmopts_parse(argc, argv);

	struct audio_stream_t *stream = extract_audio(pgmopts->filename);
	if (!stream) {
		fprintf(stderr, "Error opening stream: %s\n", pgmopts->filename);
		exit(EXIT_FAILURE);
	}

	int16_t data_buffer[1024];
	int samples;
	while ((samples = grab_audio_chunk(stream, data_buffer, sizeof(data_buffer))) > 0) {
		printf("got %d\n", samples);
		for (int i = 0; i < samples; i++) {
			printf("%d\n", data_buffer[i]);
		}
	}

	close_audio(stream);
	return 0;
}
