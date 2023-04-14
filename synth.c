#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "synth.h"

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

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float adsr(float t, float attack, float decay, float sustain, float release) {
    if (t < attack) {
        return lerp(0.0f, 1.0f, t / attack);
    } else if (t < attack + decay) {
        return lerp(1.0f, sustain, (t - attack) / decay);
    } else if (t < attack + decay + sustain) {
        return sustain;
    } else {
        return lerp(sustain, 0.0f, (t - attack - decay - sustain) / release);
    }
}

void sine(int n, int f, struct Note* note, int pitch, float* buffer){
    float freq = (note->freq)/pow(2, (4-pitch));
    //printf("%c : %d : %f\n", note->name, pitch, freq);
    float period = 1.0f / freq;
    float phaseIncrement = 2.0f * M_PI / (SAMPLE_RATE / freq);
    float phase = 0.0f;

    // Generate the sine wave
    for (int i = n*f; i < n*(f+1); i++) {
        buffer[i] = sinf(phase) * adsr(period, note->attack, note->decay, note->sustain, note->release);
        phase += phaseIncrement;
    }
}


int main() {
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

        sine(n, f, &notes[id], pitch, buffer);
    }

    write_wav(buffer, n*n_notes);

    return 0;
}



