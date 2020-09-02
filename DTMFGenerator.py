#!/usr/bin/env python3
#	dtmfsync - Audio/video synchronization using DTMF timestamps
#	Copyright (C) 2020-2020 Johannes Bauer
#
#	This file is part of dtmfsync.
#
#	dtmfsync is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; this program is ONLY licensed under
#	version 3 of the License, later versions are explicitly excluded.
#
#	dtmfsync is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with dtmfsync; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	Johannes Bauer <JohannesBauer@gmx.de>

import time
import datetime
import collections
import random
import subprocess

class Scrambler():
	def __init__(self, state = 0):
		self._state = state

	@property
	def state(self):
		return self._state

	@state.setter
	def state(self, value):
		self._state = value & 0xff

	def next_value(self, bit_xor = False):
		lsb = self._state & 1
		self._state >>= 1
		if lsb ^ bit_xor:
			self._state ^= 0xaf
		return self._state

	def checksum(self, stream):
		checksum = None
		for byte in stream:
			for bit in range(8):
				checksum = self.next_value((byte >> bit) & 1)
		return checksum

class DTMFGenerator():
	_Timestamp = collections.namedtuple("Timestamp", [ "int_value", "timestamp_utc" ])
	_FREQUENCIES = {
		"0": (941, 1336),
		"1": (697, 1209),
		"2": (697, 1336),
		"3": (697, 1477),
		"4": (770, 1209),
		"5": (770, 1336),
		"6": (770, 1477),
		"7": (852, 1209),
		"8": (852, 1336),
		"9": (852, 1477),
		"a": (697, 1633),
		"b": (770, 1633),
		"c": (852, 1633),
		"d": (941, 1633),
		"*": (941, 1209),
		"#": (941, 1477),
		"e": (941, 1209),
		"f": (941, 1477),
	}

	def __init__(self, duration = 0.1):
		self._duration = duration
		self._fade_time = duration / 10
		if self._fade_time > 0.05:
			self._fade_time = 0.05
		elif self._fade_time < 0.015:
			self._fade_time = 0.015
		self._scrambler = Scrambler()
		self._tones = [ ]

	@property
	def scrambler(self):
		return self._scrambler

	def generate_nibble(self, value):
		frequencies = self._FREQUENCIES["%x" % (value & 0xf)]
		self._tones.append(frequencies)

	def generate_byte(self, value):
		self.generate_nibble((value >> 4) & 0xf)
		self.generate_nibble((value >> 0) & 0xf)

	def generate_scrambled(self, stream):
		for original_byte in stream:
			scrambled_byte = original_byte ^ self.scrambler.next_value()
			self.generate_byte(scrambled_byte)

	def generate_prng_scrambled_crc(self, stream):
		seed = random.randint(1, 255)
		self.scrambler.state = seed
		checksum = Scrambler(seed).checksum(stream)
		self.generate_byte(seed)
		self.generate_scrambled(stream)
		self.generate_byte(checksum)

	def generate_prng_scrambled_crc_timestamp(self):
		ts = round(time.time())
		return self.generate_prng_scrambled_crc(ts.to_bytes(length = 5, byteorder = "little"))

	def _synth_cmd(self):
		first = True
		cmd = [ ]
		for (f1, f2) in self._tones:
			if not first:
				cmd += [ ":" ]
			else:
				first = False
			cmd += [ "synth", "%.3f" % (self._duration), "sin", str(f1), "sin", str(f2), "remix", "-", "gain", "-6", "fade", "%.3f" % (self._fade_time), "-0", "%.3f" % (self._fade_time) ]
		return cmd

	def play(self):
		cmd = [ "play", "-n" ] + self._synth_cmd()
		subprocess.check_call(cmd)

	def write_wav(self, filename):
		cmd = [ "sox", "-n", filename ] + self._synth_cmd()
		subprocess.check_call(cmd)

	@classmethod
	def validate_prng_scrambled_data(cls, data):
		assert(len(data) == 7)
		seed = data[0]
		assert(seed != 0)
		scrambler = Scrambler(state = seed)
		payload = bytes(byte ^ scrambler.next_value() for byte in data[1 : -1])
		calculated_checksum = Scrambler(seed).checksum(payload)
		received_checksum = data[-1]
		if calculated_checksum == received_checksum:
			return payload
		else:
			return None

	@classmethod
	def validate_prng_scrambled_timestamp(cls, data):
		payload = cls.validate_prng_scrambled_data(data)
		if payload is None:
			return None
		int_value = int.from_bytes(payload, byteorder = "little")
		ts = datetime.datetime.utcfromtimestamp(int_value)
		return cls._Timestamp(int_value = int_value, timestamp_utc = ts)

if __name__ == "__main__":
	gen = DTMFGenerator()
	gen.generate_prng_scrambled_crc_timestamp()
	gen.play()

	print(gen.validate_prng_scrambled_timestamp(bytes.fromhex("d0 d0 bf 55 52 a9 57")))	# plain: b88b4f5f00
