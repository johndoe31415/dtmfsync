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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "goertzel.h"

#define DTMF_COL_1		1209
#define DTMF_COL_2		1336
#define DTMF_COL_3		1477
#define DTMF_COL_4		1633
#define DTMF_ROW_1		697
#define DTMF_ROW_2		770
#define DTMF_ROW_3		852
#define DTMF_ROW_4		941

bool goertzel_detect(const int16_t *audio_samples, unsigned int sample_count, float target_frequency, float sample_frequency) {
	float omega = 2 * M_PI * target_frequency / sample_frequency;
	float coeff = 2 * cos(omega);
	float q0, q1, q2;
	q2 = 0;
	q1 = 0;
	for (unsigned int i = 0; i < sample_count; i++) {
		q0 = (coeff * q1) - q2 + audio_samples[i];
		q2 = q1;
		q1 = q0;
	}
	float magnitude = sqrt((q1 * q1) + (q2 * q2) - (coeff * q1 * q2));
//	printf("%f\n", magnitude);
	return magnitude > 250000;
}

enum dtmf_char_t goertzel_detect_dtmf(const int16_t *audio_samples, unsigned int sample_count, float sample_frequency) {
	//goertzel_detect(audio_samples, sample_count, DTMF_ROW_1, sample_frequency);
	if (goertzel_detect(audio_samples, sample_count, DTMF_ROW_1, sample_frequency)) {
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_1, sample_frequency)) return CHAR_1;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_2, sample_frequency)) return CHAR_2;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_3, sample_frequency)) return CHAR_3;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_4, sample_frequency)) return CHAR_A;
	} else if (goertzel_detect(audio_samples, sample_count, DTMF_ROW_2, sample_frequency)) {
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_1, sample_frequency)) return CHAR_4;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_2, sample_frequency)) return CHAR_5;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_3, sample_frequency)) return CHAR_6;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_4, sample_frequency)) return CHAR_B;
	} else if (goertzel_detect(audio_samples, sample_count, DTMF_ROW_3, sample_frequency)) {
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_1, sample_frequency)) return CHAR_7;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_2, sample_frequency)) return CHAR_8;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_3, sample_frequency)) return CHAR_9;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_4, sample_frequency)) return CHAR_C;
	} else if (goertzel_detect(audio_samples, sample_count, DTMF_ROW_4, sample_frequency)) {
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_1, sample_frequency)) return CHAR_ASTERISK;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_2, sample_frequency)) return CHAR_0;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_3, sample_frequency)) return CHAR_POUND;
		if (goertzel_detect(audio_samples, sample_count, DTMF_COL_4, sample_frequency)) return CHAR_D;
	}
	return CHAR_NONE;
}
