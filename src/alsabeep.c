/*
 * File: alsabeep.c
 * Play and generate different wave Tone from Alsa Audio Api
 * Derived from AlsaBeep Project.
 * Author: Coolbrother
 * Date: 31/12/2020
 * */

#include <alsa/asoundlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define BUF_LEN 48000
#define DEF_FREQ 440
#define DEF_DUR 1
#define DEF_NOTE 48
#define MAX_NOTE 87
float g_buffer[BUF_LEN];
snd_pcm_t *g_handle;
snd_pcm_sframes_t g_frames;
int channels =1;
snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT;
int rate = 48000;
// notes array
#define NOTES_LEN 1024
#define PAT_LEN 1024
#define TONE_LEN 100
int g_arrNotes[NOTES_LEN]; // globals arrays are initialized with zeros by default  
float g_arrBlank[BUF_LEN]; // globals array for silence
float g_arrBeat1[BUF_LEN]; // globals array for high beat
float g_arrBeat2[BUF_LEN]; // globals array for low beat
// macro for structure
// #define NEW_BEEP {440.0f, 1.0f, 48, 0, 1, 1}
struct TPattern {
    unsigned int mode;
    float freq;
    float dur;
    int note;
    int start;
    int stop;
    float step;
    unsigned int repeat;
    char* noteString;
    unsigned int bpm;
};

struct TNoteDur {
    int note;
    float dur;
};


// struct TBeep Beep1 = NEW_BEEP;
struct TPattern g_arrPattern[PAT_LEN];
struct TNoteDur g_arrNoteDur[PAT_LEN];

// pointer for current pattern
struct TPattern* g_ptrPat = NULL;

//-----------------------------------------

#define HELP_TEXT "AlsaBeep Usage:\n\
    freq dur : without options, play at frequency freq, in duration dur in seconds.\n\
    -d dur : set duration in seconds (default: 1 sec)\n\
    -f freq : set frequency in HZ, and play it. (default: 440 HZ)\n\
    -F freq : set frequency in HZ, and play the sequence freq between start and stop optionss\n\
    -h : print this Help\n\
    -l loop : set the loop number. (default: 1, [0..32767])\n\
    -n note : set the note number and play it. (default: 48, [-1..87])\n\
    -N note : set note number and play the notes sequence between start and stop options\n\
    -o note : set the note string and play it. (default: \"48\")\n\
    -r repeat : set the repeat number. (default: 1, [0..32767])\n\
    -s start : set start frequency or note for sequence (default: 0)\n\
    -S stop : set stop frequency or note for sequence (default: 1)\n\
    -t step : set step frequency or note for sequence (default: 1)\n\n"
//-----------------------------------------

void initArrBuf() {
    // deprecated
    // initialize buffer array
    memset(g_buffer, 0, BUF_LEN);
}
//-----------------------------------------

void initArrNotes(int stLen) {
    // initialize notes array with stLen of zeros
    for (int i=0; i < stLen; i++) {
        g_arrNotes[i] =0;
    }
    
}
//-----------------------------------------

void initPatterns() {
    struct TPattern* ptrPat;
    for (int i=0; i < PAT_LEN; i++) {
        ptrPat = &g_arrPattern[i];
        ptrPat->mode =0;
        ptrPat->freq = 440.0f;
        ptrPat->dur = 1.0f;
        ptrPat->note = 48;
        ptrPat->start = 0;
        ptrPat->stop = 1;
        ptrPat->step = 1.0f;
        ptrPat->noteString = "48";
        ptrPat->repeat =1;
        ptrPat->bpm =100;

    }
  }
//-----------------------------------------

float* genTone(float* buf, int iLen, float freq) {
    // must create a pointer to return buffer
    // float *buf = malloc(sizeof(float) * BUF_LEN);
    float vol =1;
    float t = 2*M_PI*freq/(rate*channels);
    // int nbSamples = rate * channels * dur;
    // printf("nbSamples: %d\n", nbSamples);
    int maxSamp = 32767;
    // float* buf = g_buffer;
    if (iLen > BUF_LEN) iLen = BUF_LEN;
    for (int i=0; i< iLen; i++) {
        // val = (int)maxSamp*vol*sin(t*i);
        buf[i] = sin(t*i);
    }

    return buf;
}
//-----------------------------------------

float* fillBuf(float freq, float dur) {
  return 0;
}
//-----------------------------------------

int openDevice() {
    // open sound device and set params
 	  const static char *device = "default";
	  snd_output_t *output = NULL;
   int err;

    if ((err = snd_pcm_open(&g_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
      fprintf(stderr, "AlsaBeep: Playback open error: %s\n", snd_strerror(err));
      return EXIT_FAILURE;
    }

    if ((err = snd_pcm_set_params(g_handle,
        format,
        // SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels,
        // BUF_LEN,
        rate,
        1, /* period */
        500000)) < 0) {	 /* latency: 0.5sec */ 
      fprintf(stderr, "AlsaBeep: Playback open error: %s\n", snd_strerror(err));
      return EXIT_FAILURE;
    }

}
//-----------------------------------------

void closeDevice() {
    // closing sound device
    // necessary to flush and send short sample
    snd_pcm_drain(g_handle);
	  snd_pcm_close(g_handle);

}
//-----------------------------------------

void writeBuf(float* buf, int nbFrames, int nbTimes) {
  for (int i=0; i < nbTimes; i++) {
      // Sending the sound
      g_frames += snd_pcm_writei(g_handle, buf, nbFrames);
  }
  // printf("WriteBuf nbFrames: %d\n", g_frames);

}
//-----------------------------------------

void writeAudio(unsigned int nbFrames) {
    /// Not used, just for notes
    // Sending the sound
    int frames = snd_pcm_writei(g_handle, g_buffer, nbFrames);
}
//-----------------------------------------

void playFreq(float freq, float dur) {
    // playing one freq
    float* buf = g_buffer;
    int nbSamples = rate * channels * dur;
    int nbTimes = nbSamples / BUF_LEN;
    int restFrames = nbSamples % BUF_LEN;
    // printf("restFrames: %d\n", restFrames);
    if (nbSamples >0) {
        if (freq <= 0) buf = g_arrBlank;
        else genTone(buf, BUF_LEN, freq);

        if (nbTimes >0) {
            writeBuf(buf, BUF_LEN, nbTimes);	
        }
        if (restFrames > 0) {
            writeBuf(buf, restFrames, 1);
        }
    
    }
    // printf("nbFrames: %d\n", g_frames);

}
//-----------------------------------------

void playSeq(float freq, float dur, int start, int stop, float step) {
    // playing sequence freq
    // step is in float for frequency
    int iStep = (int)(step);
    for (int i=start; i<stop; i += iStep){
        playFreq(freq, dur);	
        freq += step;
    }

}
//-----------------------------------------

void playNote(int noteNum, float dur) {
    // playing note number
    float refNote = 27.5; // base frequency
    float freq = 440; // default freq
    // test whether note between 0 and note_max
    noteNum = (noteNum < -1) ? DEF_NOTE : (noteNum > MAX_NOTE) ? MAX_NOTE : noteNum;
    if (noteNum == -1) freq = 0;
    else freq = refNote*pow(2, noteNum*1/12.0);
    playFreq(freq, dur);	
    printf("Freq: %.3f\n", freq);


}
//-----------------------------------------

void playSeqNote(int noteNum, float dur, int start, int stop, int step) {
    // playing sequence notes
    for (int i=start; i<stop; i += step) {
        playNote(noteNum, dur);	
        noteNum += step;
    }

}
//-----------------------------------------

int stringToIntArray(char* strg) {
    // deprecated function
    // convert string to int array 
    int j=-1;
    int stLen = strlen(strg);
    if (stLen > NOTES_LEN) stLen = NOTES_LEN;
    if (stLen > 0) {
      j =0;
        // initArrNotes(stLen);
        g_arrNotes[j] =0;
    } else { // string is empty
        return 0;
    }

    for (int i=0; strg[i] != '\0'; i++) {
        if (strg[i] == ',') {
            continue;
        } else if (strg[i] == ' ') {
            j++;
            // init array notes field, whether array is not initialized
            if (j < NOTES_LEN) g_arrNotes[j] =0;
        
        } else if (strg[i] == '-') {
            g_arrNotes[j] = -1;
        
        } else if (strg[i] >=48 && strg[i] <=57) { // for numbers in ascii code
            if ( g_arrNotes[j] == -1) { 
                g_arrNotes[j] = g_arrNotes[j] * (strg[i] - 48);
            } else { // not -1
                if ( g_arrNotes[j] < 0) // negative numbers
                    g_arrNotes[j] = g_arrNotes[j] * 10 - (strg[i] - 48);
                else // positive numbers
                    g_arrNotes[j] = g_arrNotes[j] * 10 + (strg[i] - 48);
            }
        }

    }


    return j+1;
}
//-----------------------------------------


int stringToNotes(char* input) {
    // separate input string by tokens
    char* copyInput = strdup(input);
    const char* sep=":, ";
    char* token = NULL;
    int noteIndex =0;
    struct TNoteDur* ptrNote = NULL;
    int note =48;
    float dur =1.0f;

    // Init notes duration array
    for (int i=0; i<NOTES_LEN; i++) {
        ptrNote = &g_arrNoteDur[i];
        ptrNote->note = 48;
        ptrNote->dur =1;
    }
    // init the token
    token = strtok(input, sep);
    // printf("Input: %s\n", input);
    // printf("copyInput: %s\n", copyInput);
    // printf("Tokens\n");
    
    while ((token != NULL) && (noteIndex < PAT_LEN)) {
      ptrNote = &g_arrNoteDur[noteIndex]; 
      // char ch = copyInput[token - input];
      char delim = copyInput[token - input + strlen(token)];
      // printf("First char: %c, delim: %c\n", ch, delim);
      // printf("Token: %s\n", token);
      if ((delim == ',') || (delim == '\0')) {
          ptrNote->note = atoi(token);
          ptrNote->dur = dur;
      } else if (delim == ':') {
          note = ptrNote->note = atoi(token);
          token = strtok(NULL, sep);
          dur = ptrNote->dur = atof(token);
      }
      token = strtok(NULL, sep);
      noteIndex++;
    
    }
    free(copyInput);

    return noteIndex;
}
//----------------------------------------------------------

void playNoteList(char* strg, float dur) {
    // playing notes from string
    struct TNoteDur* ptrNote = NULL;
    int noteCount = stringToNotes(strg);
    int repeat =0;
    if (g_ptrPat != NULL) repeat = g_ptrPat->repeat;
    
    for (int i=0; i < repeat; i++) {
        for (int j=0; j < noteCount; j++) {
            ptrNote = &g_arrNoteDur[j];
            printf("Note %d:%.2f, ", ptrNote->note, ptrNote->dur);
            playNote(ptrNote->note, ptrNote->dur);
        }
    }

   
}
//-----------------------------------------

void playBPM(int bpm, int repeat) {
    int dur = 60000 / bpm; // in millisecs
    float* buf1 = g_arrBeat1;
    float* buf2 = g_arrBeat2;
    // const int iLen = (BUF_LEN / 1000) * TONE_LEN;
    genTone(buf1, TONE_LEN, 880);
    genTone(buf2, TONE_LEN, 440);
    const int nbFrames = (BUF_LEN / 1000) * dur;
    if (repeat ==0) repeat = 32767;
    for (int i=0; i< repeat; i++) {
        writeBuf(buf1, nbFrames, 1);
        writeBuf(buf2, nbFrames, 3);
    }


}
//-----------------------------------------


int playPatterns(unsigned int count, unsigned int loops) {
    // run all paterns
    char* noteString = NULL;
    
    if (loops == 0) loops = 32767; // infinite loop
    if (count > PAT_LEN) count = PAT_LEN;
    for (int j=0; j < loops; j++) {
        for (int i=0; i < count; i++) {
            g_ptrPat = &g_arrPattern[i];

            if (g_ptrPat->mode == 0) { // play frequency
                printf("Playing Freq, Sine tone at %.3fHz during %.3f sec.\n", g_ptrPat->freq, g_ptrPat->dur);
                // playFreq(freq, dur);
                playFreq(g_ptrPat->freq, g_ptrPat->dur);
            
            } else if (g_ptrPat->mode == 1) { // -f: play frequency
                printf("Playing Freq, Sine tone at %.3fHz during %.3f sec.\n", g_ptrPat->freq, g_ptrPat->dur);
                playFreq(g_ptrPat->freq, g_ptrPat->dur);

            } else if (g_ptrPat->mode == 2) { // -F: play sequence freq
                printf("Playing SeqFreq, Sine tone at %.3fHz, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", g_ptrPat->freq, g_ptrPat->dur, g_ptrPat->start, g_ptrPat->stop, g_ptrPat->step);
                playSeq(g_ptrPat->freq, g_ptrPat->dur, g_ptrPat->start, g_ptrPat->stop, g_ptrPat->step);

            } else if (g_ptrPat->mode == 3) { // -m: play metronome
                printf("Playing Metronome at %d BPM, repeat: %d.\n", g_ptrPat->bpm, g_ptrPat->repeat);
                playBPM(g_ptrPat->bpm, g_ptrPat->repeat);
            
            } else if (g_ptrPat->mode == 4) { // -n: play note
                printf("Playing Note at %d, during %.3f secs.\n", g_ptrPat->note, g_ptrPat->dur);
                playNote(g_ptrPat->note, g_ptrPat->dur);

            } else if (g_ptrPat->mode == 5) { // -N: play sequence note
                printf("Playing sequence Note at note: %d, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", g_ptrPat->note, g_ptrPat->dur, 
                    g_ptrPat->start, g_ptrPat->stop, g_ptrPat->step);
                playSeqNote(g_ptrPat->note, g_ptrPat->dur, 
                    g_ptrPat->start, g_ptrPat->stop, g_ptrPat->step);

            } else if (g_ptrPat->mode == 6) { // -o: play note list
                // TODO: make a better string checking
                if (noteString == NULL) noteString = strdup(g_ptrPat->noteString);
                if (noteString == NULL || noteString[0] == '\0' 
                    || noteString[0] == ' ' || noteString[0] == ',') {
                    fprintf(stderr, "AlsaBeep: Invalid notes list.\n");
                    return EXIT_FAILURE;
                }
                printf("Playing Note list: %s, during %.3f secs, repeat: %d times.\n", g_ptrPat->noteString, g_ptrPat->dur, g_ptrPat->repeat);
                playNoteList(noteString, g_ptrPat->dur);
                free(noteString);
                noteString = NULL;
            }
        } // end count
    
    } // end loops

    return 0;
}
//-----------------------------------------

int main(int argc, char *argv[]) {
    int err;
    float freq = DEF_FREQ; // in hertz
    float dur = DEF_DUR; // in seconds
    int note = DEF_NOTE;
    char* noteString = NULL;
    int start =0;
    int stop =1;
    float step =1;
    unsigned int repeat =0;
    unsigned int bpm =100;
    int mode =0;
    int optIndex =0;
    int patIndex =-1;
    unsigned int loops =1;

    struct TPattern* ptrPat = NULL; 
    ptrPat = g_arrPattern;
    initPatterns();
   
    while (( optIndex = getopt(argc, argv, "d:f:F:hl:m:n:N:o:r:s:S:t:")) != -1 && (patIndex < PAT_LEN)) {
        // printf("patIndex: %d\n", patIndex);
        if (patIndex >=0) ptrPat = &g_arrPattern[patIndex]; 
        switch (optIndex) {
            case 'd':
                dur = atof(optarg); 
                ptrPat->dur = dur; break;
            case 'f':
                // patIndex++;
                // increment index, then returns it
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =1;
                ptrPat->mode = mode;
                freq = atof(optarg);
                ptrPat->freq = freq; break;
            case 'F':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =2;
                ptrPat->mode = mode;
                freq = atof(optarg); break;
            case 'h':
                printf(HELP_TEXT);
                return 0;
            case 'l':
                loops = strtol(optarg, NULL, 10); break;
            case 'm':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =3;
                ptrPat->mode = mode;
                bpm = strtol(optarg, NULL, 10); 
                ptrPat->bpm = bpm;
                ptrPat->repeat = repeat;
                break;
            case 'n':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =4;
                ptrPat->mode = mode;
                note = strtol(optarg, NULL, 10); 
                ptrPat->note = note; 
                break;
            case 'N':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =5;
                ptrPat->mode = mode;
                note = strtol(optarg, NULL, 10); break;
            case 'o':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =6;
                ptrPat->mode = mode;
                noteString = optarg;
                ptrPat->noteString = noteString;
                ptrPat->repeat =1;
                break;
        case 'r':
                repeat = strtol(optarg, NULL, 10); 
                ptrPat->repeat = repeat; break;
        case 's':
                start = strtol(optarg, NULL, 10); 
                ptrPat->start = start; break;
            case 'S':
                stop = strtol(optarg, NULL, 10); 
                ptrPat->stop = stop; break;
            case 't':
                step = atof(optarg); 
                ptrPat->step = step; break;

            default:
                printf("Option incorrect\n");
                return 1;
        }
          
    }
    
    // SINE WAVE
    if (err = openDevice()) {
        return EXIT_FAILURE;
    }
    
    // playing mode
    // without options
    if (patIndex == -1) {
        freq = (argc > 1) ? atof(argv[1]) : DEF_FREQ;
        if (freq < 0) {
            fprintf(stderr, "AlsaBeep: Invalid frequency.\n");
            return EXIT_FAILURE;
        }

        dur = (argc > 2) ? atof(argv[2]) : DEF_DUR;
        if (dur == 0) {
            fprintf(stderr, "AlsaBeep: Invalid duration.\n");
            return EXIT_FAILURE;
        }
        
        start = (argc > 3) ? strtol(argv[3], NULL, 10) : -1;
        stop = (argc > 4) ? strtol(argv[4], NULL, 10) : 0;
        step = (argc > 5) ? atof(argv[5]) : 1;

        // Playing freq
        if (start == -1) {
            printf("Playing freq, Sine tone at %.3fHz during %.3f secs.\n", freq, dur);
            playFreq(freq, dur);
            // playFreq(ptrPat->freq, ptrPat->dur);

        } else { // Playing sequence freq
            printf("Playing SeqFreq, Sine tone at %.3fHz, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", freq, dur, start, stop, step);
            playSeq(freq, dur, start, stop, step);
        }

    } else {
        const int count = patIndex+1;
        printf("Playing patterns, count: %d, loops: %d\n", count, loops);
        playPatterns(count, loops);
    }
      
    printf("nbFrames played: %d\n", g_frames);

    closeDevice();

    return EXIT_SUCCESS;
}
//-----------------------------------------
