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

#ifndef __GOERTZEL_H__
#define __GOERTZEL_H__

#include <stdint.h>
#include <stdbool.h>

enum dtmf_char_t {
	CHAR_0 = 0,
	CHAR_1 = 1,
	CHAR_2 = 2,
	CHAR_3 = 3,
	CHAR_4 = 4,
	CHAR_5 = 5,
	CHAR_6 = 6,
	CHAR_7 = 7,
	CHAR_8 = 8,
	CHAR_9 = 9,
	CHAR_A = 0xa,
	CHAR_B = 0xb,
	CHAR_C = 0xc,
	CHAR_D = 0xd,
	CHAR_ASTERISK = 0xe,
	CHAR_POUND = 0xf,
	CHAR_NONE,
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
bool goertzel_detect(const int16_t *audio_samples, unsigned int sample_count, float target_frequency, float sample_frequency);
enum dtmf_char_t goertzel_detect_dtmf(const int16_t *audio_samples, unsigned int sample_count, float sample_frequency);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
