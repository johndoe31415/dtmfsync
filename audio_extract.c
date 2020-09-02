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

struct riff_header_t {
	uint8_t identifier[4];
	uint32_t chunk_size;
	uint8_t format[4];
} __attribute__ ((packed));

struct format_header_t {
	uint32_t chunk_size;
	uint16_t data_format;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t bytes_per_second;
	uint16_t block_alignment;
	uint16_t bits_per_sample;
} __attribute__ ((packed));

struct list_header_t {
	uint32_t chunk_size;
} __attribute__ ((packed));

static void child_execute(const char *input_filename) {
	char *input_filename_dup = strdup(input_filename);
	if (!input_filename_dup) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	// ffmpeg -i example.wav -vn -ar 11000 -ac 1 -f wav -
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

	stream->child_f = fdopen(pipefd[0], "r");
	if (!stream->child_f) {
		perror("fdopen");
		close(pipefd[0]);
		close(pipefd[1]);
		close_audio(stream);
		return NULL;
	}
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

enum wav_read_t {
	WAV_SUCCESS,
	WAV_FINISHED,
	WAV_ERROR
};

static bool discard_bytes(FILE *f, unsigned int l) {
	uint8_t buffer[1024];
	while (l > 0) {
		ssize_t bytes_discarded = fread(buffer, 1, (l > sizeof(buffer)) ? sizeof(buffer) : l, f);
		if (bytes_discarded <= 0) {
			break;
		}
		l -= bytes_discarded;
	}
	return l == 0;
}

static enum wav_read_t read_next_wav_header(struct audio_stream_t *stream) {
	uint8_t identifier[5];
	ssize_t blocks_read = fread(identifier, 4, 1, stream->child_f);
	if (blocks_read != 1) {
		fprintf(stderr, "Short read when trying to parse next RIFF header identifier.\n");
		return WAV_ERROR;
	}

	if (!memcmp(identifier, "data", 4)) {
		if (!discard_bytes(stream->child_f, 4)) {
			fprintf(stderr, "Short read when trying to parse data length.\n");
			return WAV_ERROR;
		}
		return WAV_FINISHED;
	} else if (!memcmp(identifier, "fmt ", 4)) {
		struct format_header_t fmt;
		blocks_read = fread(&fmt, sizeof(fmt), 1, stream->child_f);
		if (blocks_read != 1) {
			fprintf(stderr, "Short read when trying to parse format header identifier.\n");
			return WAV_ERROR;
		}
		stream->sample_rate = fmt.sample_rate;
		if (fmt.data_format != 0x1) {
			fprintf(stderr, "Unexpected WAV format: 0x%x (expected PCM/0x01).\n", fmt.data_format);
			return WAV_ERROR;

		}
		if (fmt.channels != 1) {
			fprintf(stderr, "Unexpected channel count: %d (expected 1).\n", fmt.channels);
			return WAV_ERROR;
		}
		if (fmt.bits_per_sample != 16) {
			fprintf(stderr, "Unexpected bits per sample: %d (expected 16).\n", fmt.bits_per_sample);
			return WAV_ERROR;
		}
		return WAV_SUCCESS;
	} else if (!memcmp(identifier, "LIST", 4)) {
		struct list_header_t list;
		blocks_read = fread(&list, sizeof(list), 1, stream->child_f);
		if (blocks_read != 1) {
			fprintf(stderr, "Short read when trying to parse list header identifier.\n");
			return WAV_ERROR;
		}
		if (!discard_bytes(stream->child_f, list.chunk_size)) {
			return WAV_ERROR;
		}
		return WAV_SUCCESS;

	} else {
		identifier[4] = 0;
		fprintf(stderr, "Unknown next header: %s\n", identifier);
	}
	return WAV_ERROR;
}

static bool read_header(struct audio_stream_t *stream) {
	struct riff_header_t riff_header;
	ssize_t blocks_read = fread(&riff_header, sizeof(riff_header), 1, stream->child_f);
	if (blocks_read != 1) {
		fprintf(stderr, "Short read when trying to parse RIFF header.\n");
		return false;
	}

	if (memcmp(riff_header.identifier, "RIFF", 4)) {
		fprintf(stderr, "Error of WAV magic1 trying to parse WAV header.\n");
		return false;
	}
	if (memcmp(riff_header.format, "WAVE", 4)) {
		fprintf(stderr, "Error of WAV magic2 trying to parse WAV header.\n");
		return false;
	}

	enum wav_read_t result;
	while ((result = read_next_wav_header(stream)) == WAV_SUCCESS);
	if (result == WAV_FINISHED) {
		stream->header_read = true;
	}
	return result == WAV_FINISHED;
}

int grab_audio_chunk(struct audio_stream_t *stream, void *buffer, unsigned int max_length) {
	if (!stream->header_read) {
		if (!read_header(stream)) {
			fprintf(stderr, "Error parsing WAV header.\n");
			return 0;
		}
	}
	ssize_t bytes_read = fread(buffer, 1, max_length, stream->child_f);
	if (bytes_read > 0) {
		return bytes_read / 2;
	} else {
		return bytes_read;
	}
}

void close_audio(struct audio_stream_t *stream) {
	if (stream->child_f) {
		fclose(stream->child_f);
	}
	free(stream);
}
