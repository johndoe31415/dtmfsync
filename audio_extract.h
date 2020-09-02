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

#ifndef __AUDIO_EXTRACT_H__
#define __AUDIO_EXTRACT_H__

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

struct audio_stream_t {
	int child_fd;
	pid_t child_pid;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
struct audio_stream_t *extract_audio(const char *input_filename);
int grab_audio_chunk(struct audio_stream_t *stream, uint8_t *buffer, unsigned int max_length);
void close_audio(struct audio_stream_t *stream);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
