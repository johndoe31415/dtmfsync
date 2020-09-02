# dtmfsync
The idea of this tool is to synchronize timestamps when recoding video and
audio separately. When you have many video files and many audio files (not
necessarily the same count either), it can be cumbersome to find the video
files that correspond to audio files and to manually aligning them.  Typically
this is done with a clapper board, but it can't be easily automated.

So my idea was to use a PC (which I always have nearby when recoding anyways)
to play a DTMF-encoded and PRNG-scrambled timestamp value. Both the internal
microphone of the camera and the external audio sink pick that up. Then, later
on, we could go through all files and identify sample-accurately the exact
files that correspond to each other.  At least that's the theory; I've not
written the decoder yet.

## License
GNU GPL-3.
