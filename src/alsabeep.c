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
int g_arrNotes[NOTES_LEN]; // globals arrays are initialized with zeros by default  
// macro for structure
// #define NEW_BEEP {440.0f, 1.0f, 48, 0, 1, 1}
struct TPattern {
    unsigned int mode;
    float freq;
    float dur;
    unsigned int note;
    int start;
    int stop;
    float step;
    char* noteString;
};

// struct TBeep Beep1 = NEW_BEEP;
struct TPattern g_arrPattern[PAT_LEN];

//-----------------------------------------

#define HELP_TEXT "AlsaBeep Usage:\n\
  freq dur : without options, play at frequency freq, in duration dur in seconds.\n\
  -d dur : set duration in seconds (default: 1 sec)\n\
  -f freq : set frequency in HZ, and play it. (default: 440 HZ)\n\
  -F freq : set frequency in HZ, and play the sequence freq between start and stop optionss\n\
  -h : print this Help\n\
  -n note : set the note number and play it. (default: 48)\n\
  -N note : set note number and play the notes sequence between start and stop options\n\
  -s start : set start frequency or note for sequence (default: 0)\n\
  -S stop : set stop frequency or note for sequence (default: 1)\n\
  -t step : set step frequency or note for sequence (default: 1)\n\n"
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
    }
  }
//-----------------------------------------

float* genTone(float freq) {
    // must create a pointer to return buffer
    // float *buf = malloc(sizeof(float) * BUF_LEN);
    float vol =1;
    float t = 2*M_PI*freq/(rate*channels);
    // int nbSamples = rate * channels * dur;
    // printf("nbSamples: %d\n", nbSamples);
    int maxSamp = 32767;
    float* buf = g_buffer;
    for (int i=0; i< BUF_LEN; i++) {
        // val = (int)maxSamp*vol*sin(t*i);
        g_buffer[i] = sin(t*i);
    }

    return g_buffer;
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
    float* buf;
    int nbSamples = rate * channels * dur;
    int nbTimes = nbSamples / BUF_LEN;
    int restFrames = nbSamples % BUF_LEN;
    // printf("restFrames: %d\n", restFrames);
    if (nbSamples >0) {
        buf = genTone(freq);
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
    float refNote = 27.5; // note reference A0
    // test whether note between 0 and note_max
    noteNum = (noteNum < 0) ? DEF_NOTE : (noteNum > MAX_NOTE) ? MAX_NOTE : noteNum;
    float freq = refNote*pow(2, noteNum*1/12.0);
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


void playNoteList(char* strg, float dur) {
    // playing notes from string
    int noteCount = stringToIntArray(strg);
    for (int i=0; i < noteCount; i++) {
        printf("%d: ", g_arrNotes[i]);
        playNote(g_arrNotes[i], dur);
    }
    
}
//-----------------------------------------

int runPatterns(unsigned int count) {
    // run all paterns
    struct TPattern* ptrPat = NULL;
    char* noteString = NULL;

    for (int i=0; i < count; i++) {
        ptrPat = &g_arrPattern[i];
        if (ptrPat->mode == 0) { // play frequency
            printf("Playing Freq, Sine tone at %.3fHz during %.3f sec.\n", ptrPat->freq, ptrPat->dur);
            // playFreq(freq, dur);
            playFreq(ptrPat->freq, ptrPat->dur);
        
        } else if (ptrPat->mode == 1) { // -f: play frequency
            printf("Playing Freq, Sine tone at %.3fHz during %.3f sec.\n", ptrPat->freq, ptrPat->dur);
            playFreq(ptrPat->freq, ptrPat->dur);

        } else if (ptrPat->mode == 2) { // -F: play sequence freq
            printf("Playing SeqFreq, Sine tone at %.3fHz, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", ptrPat->freq, ptrPat->dur, ptrPat->start, ptrPat->stop, ptrPat->step);
            playSeq(ptrPat->freq, ptrPat->dur, ptrPat->start, ptrPat->stop, ptrPat->step);

        } else if (ptrPat->mode == 3) { // -n: play note
            printf("Playing Note at %d, during %.3f secs.\n", ptrPat->note, ptrPat->dur);
            playNote(ptrPat->note, ptrPat->dur);

        } else if (ptrPat->mode == 4) { // -N: play sequence note
            printf("Playing sequence Note at note: %d, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", ptrPat->note, ptrPat->dur, 
                ptrPat->start, ptrPat->stop, ptrPat->step);
            playSeqNote(ptrPat->note, ptrPat->dur, 
                ptrPat->start, ptrPat->stop, ptrPat->step);

        } else if (ptrPat->mode == 5) { // -o: play note list
            // TODO: make a better string checking
            noteString = ptrPat->noteString;
            if (noteString == NULL || noteString[0] == '\0' 
                || noteString[0] == ' ' || noteString[0] == ',') {
                fprintf(stderr, "AlsaBeep: Invalid notes list.\n");
                return EXIT_FAILURE;
            }
            printf("Playing Note list: %s, during %.3f secs.\n", ptrPat->noteString, ptrPat->dur);
            playNoteList(ptrPat->noteString, ptrPat->dur);
        }
    }


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
    int mode =0;
    int optIndex =0;
    int patIndex =-1;

    struct TPattern* ptrPat = NULL; 
    ptrPat = g_arrPattern;
    initPatterns();
   
    while (( optIndex = getopt(argc, argv, "d:f:F:hn:N:o:s:S:t:")) != -1 && (patIndex < PAT_LEN)) {
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
            case 'n':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =3;
                ptrPat->mode = mode;
                note = strtol(optarg, NULL, 10); 
                ptrPat->note = note; 
                break;
            case 'N':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =4;
                ptrPat->mode = mode;
                note = strtol(optarg, NULL, 10); break;
            case 'o':
                ptrPat = &g_arrPattern[++patIndex]; 
                mode =5;
                ptrPat->mode = mode;
                noteString = optarg;
                ptrPat->noteString = noteString;
                break;
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
    // if (mode == 0) {
        freq = (argc > 1) ? atof(argv[1]) : DEF_FREQ;
        if (freq == 0) {
            fprintf(stderr, "AlsaBeep: Invalid frequency.\n");
            return EXIT_FAILURE;
        }
        // ptrPat->freq = freq;

        dur = (argc > 2) ? atof(argv[2]) : DEF_DUR;
        if (dur == 0) {
            fprintf(stderr, "AlsaBeep: Invalid duration.\n");
            return EXIT_FAILURE;
        }
        // ptrPat->dur = dur;
        
        start = (argc > 3) ? strtol(argv[3], NULL, 10) : -1;
        // ptrPat->start = start;
        stop = (argc > 4) ? strtol(argv[4], NULL, 10) : 0;
        // ptrPat->stop = stop;
        step = (argc > 5) ? atof(argv[5]) : 1;
        // ptrPat->step = step;
        

        // Playing freq
        if (start == -1) {
            printf("Playing freq, Sine tone at %.3fHz during %.3f secs.\n", freq, dur);
            playFreq(freq, dur);
            // playFreq(ptrPat->freq, ptrPat->dur);
        // Playing sequence freq
        } else {
            printf("Playing SeqFreq, Sine tone at %.3fHz, during %.3f secs, start: %d, stop: %d, step: %.3f.\n", freq, dur, start, stop, step);
            playSeq(freq, dur, start, stop, step);
        }

    } else {
        runPatterns(patIndex+1);
    }
      
    printf("nbFrames played: %d\n", g_frames);

    closeDevice();

    return EXIT_SUCCESS;
}
//-----------------------------------------
