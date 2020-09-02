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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "audio_extract.h"

static void child_execute(const char *input_filename) {
	char *input_filename_dup = strdup(input_filename);
	if (!input_filename_dup) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	char* const command[] = {
		"ffmpeg",
		"-i",
		input_filename_dup,
		"-vn",
		"-ar",
		"11000",
		"-ac",
		"1",
		"-f",
		"wav",
		"-",
		NULL
	};
	execvp(command[0], command);
	free(input_filename_dup);
	perror("execvp");
	exit(EXIT_FAILURE);
}

struct audio_stream_t *extract_audio(const char *input_filename) {
	struct audio_stream_t *stream = calloc(1, sizeof(*stream));
	if (!stream) {
		perror("calloc");
		return NULL;
	}

	int pipefd[2];
	if (pipe(pipefd) == -1) {
		perror("pipe");
		close_audio(stream);
		return NULL;
	}

//	set_fd_nonblock(pipefd[0]);
//	stream->child_f = fdopen(pipefd[0], "r");
	stream->child_fd = pipefd[0];
	stream->child_pid = fork();
	if (stream->child_pid == -1) {
		perror("fork");
		close_audio(stream);
		return NULL;
	}

	if (stream->child_pid == 0) {
		/* Child process, exec. */
		int write_fd = pipefd[1];
		close(pipefd[0]);
		dup2(write_fd, STDOUT_FILENO);
		child_execute(input_filename);
		exit(EXIT_FAILURE);
	} else {
		/* Parent process, close child end of pipe */
		close(pipefd[1]);
	}

	return stream;
}

int grab_audio_chunk(struct audio_stream_t *stream, uint8_t *buffer, unsigned int max_length) {
	ssize_t bytes_read = read(stream->child_fd, buffer, max_length);
	return bytes_read;
}

void close_audio(struct audio_stream_t *stream) {
	if (stream->child_fd) {
		close(stream->child_fd);
	}
	free(stream);
}
