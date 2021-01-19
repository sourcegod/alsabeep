# AlsaBeep

AlsaBeep is a player and sound generator for different wave tone, from Alsa Audio Api.
Derived from AlsaTonic project.

[alsabeep](https://github.com/sourcegod/alsabeep.git "AlsaBeep Project")

### How to compile it ?

Just write in a console :  
`sudo apt-get install libasound2-dev`  
`git clone https://github.com/sourcegod/alsabeep.git`  
`make`

### How to run it ?

Examples

`./alsabeep [frequency, default 440Hz] [duration, default 1 seconds]`  
Play default frequency with default duration.


`./alsabeep freq dur start [stop, default 1Hz] [step, default 1Hz]` 
Play frequency freq, with duration dur, starting frequency at start, with default
stop frequency, with default step frequency.


`./alsabeep -h`
display the help.


`./alsabeep -F 440 -d 0.2 -s 0 -S 400 -t 20`
Play sequence frequencies from 440 HZ, with duration 0.2 seconds, starting at
440+0 HZ, stopping at 440+400 HZ,  by step 20 HZ.


`./alsabeep -n 48 -d 3`
Play note number 48 (A4), with duration 3 seconds.


`./alsabeep -N 51 -d 0.2 -s 0 -S 13 -t 1`
Play sequence notes from 51 note number, with duration 0.5 seconds, starting at
51 note number (C5), stopping at 63 note number (C6),  by step 1 note.

`./alsabeep -o "51:0.1, 53, 55, 56:0.5, 58" -r 3`
Repeat 3 times Play Note String with notes numbers:
51 with duration 0.1 secs, 53 with previous duration, 55, 58 with duration 0.5
secs, 58 with previous duration.

`./alsabeep -m 120`
Play the metronome at 120 BPM.


### See also:
[alsatonic](https://github.com/sourcegod/alsatonic.git "AlsaTonic Project")
