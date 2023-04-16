#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "synth.h"
#include "waveforms.h"

#define MAXBUFLEN 1000
#define DURATION 1.0f

int read_notes(char** fnotes){
    FILE *fp = fopen("notes.n", "r");
    char line[MAXBUFLEN + 1];

    if (fp == NULL){
        printf("File not found");
        exit(1);
    }

    int n_notes;
    char space[] = " ";
    while (fgets(line, sizeof(line), fp)) {
        char** tokens = split(line, space, &n_notes);
        for (int i=0; i < n_notes; i++){
            strcpy(&fnotes[i], &tokens[i]);
        }
    }

    fclose(fp);

    return n_notes;
}

void mix(void (*waveforms[])(int, int, struct Note *, int, float *), int n_waveforms, int n, int f, struct Note* note, int pitch, float* buffer){
    //zero buffer
    for (int i=n*f; i < n*(f+1); i++){
        buffer[i] = 0;
    }

    for (int i=0; i < n_waveforms; i++){
        void (*waveform)(int, int, struct Note*, int, float*) = waveforms[i];
        waveform(n, f, note, pitch, buffer);
    }
}

int main(int argc, char *argv[]) {
    void (**waveforms)(int, int, struct Note *, int, float *) = NULL;
    waveforms = (void (**)(int, int, struct Note*, int, float*)) malloc((argc - 1) * sizeof(void (*)()));
    if (waveforms == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }

    int n_waveforms = 0;
    
    for (int i = 1; i < argc; i++){
        char* wavefrm = argv[i];

        if (strcmp(wavefrm, "sine") == 0){
            waveforms[n_waveforms] = sine;
        } else if (strcmp(wavefrm, "tooth") == 0 || strcmp(wavefrm, "sawtooth") == 0){
            waveforms[n_waveforms] = sawtooth;
        } else if (strcmp(wavefrm, "sqr") == 0 || strcmp(wavefrm, "square") == 0){
            waveforms[n_waveforms] = square;
        } else if (strcmp(wavefrm, "tri") == 0 || strcmp(wavefrm, "triangle") == 0){
            waveforms[n_waveforms] = triangle;
        }else{
            printf("Undefined waveform: %s\n", wavefrm);
            exit(1);
        }
        n_waveforms++;
    }
    
    if (n_waveforms == 0){
        waveforms[0] = sine;
        n_waveforms = 1;
    } 

    int n = ceil(SAMPLE_RATE * DURATION);

    struct Note notes[7] = {{'A', 440.0f, 0.01, 0.1, 0.7, 0.1}, 
    {'B', 493.88f, 0.01, 0.1, 0.7, 0.1}, 
    {'C', 523.25f, 0.01, 0.1, 0.7, 0.1}, 
    {'D', 587.33f, 0.01, 0.1, 0.7, 0.1}, 
    {'E', 659.26f, 0.01, 0.1, 0.7, 0.1}, 
    {'F', 698.46f, 0.01, 0.1, 0.7, 0.1}, 
    {'G', 783.99f, 0.01, 0.1, 0.7, 0.1}};

    char* fnotes[MAXBUFLEN + 1];
    int n_notes = read_notes(fnotes);

    float buffer[n*n_notes];
    for (int f=0; f < n_notes; f++){
        char* fnote = fnotes[f];

        int pitch;
        if (fnote[1] != '\0'){
            pitch = fnote[1] - '0';
        } else{
            pitch = 4;
        }

        int id = -1;

        for (int i=0; i < sizeof(notes) / sizeof(struct Note); i++){
            if (notes[i].name == fnote[0]){
                id = i;
                break;
            }
        }

        if (id == -1){
            printf("Note %c not found\n", *fnote);
            exit(1);
        }

        mix(waveforms, n_waveforms, n, f, &notes[id], pitch, buffer);
    }

    write_wav(buffer, n*n_notes);

    return 0;
}



