#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <raylib.h>

typedef struct AudioEngine {
    Sound compareSound;
    Sound swapSound;
    Sound sortedSound;
    Sound completionSweepSound;
    Sound confirmationSound;
    bool ready;
    int frameSoundCount;
    bool completionSweepActive;
    int completionSweepIndex;
    float completionSweepTimer;
    bool confirmationPending;
} AudioEngine;

void audio_init(AudioEngine *engine, float masterVolume);
void audio_shutdown(AudioEngine *engine);

void audio_begin_frame(AudioEngine *engine);
void audio_set_master_volume(AudioEngine *engine, float volume);

void audio_play_swap(AudioEngine *engine, bool enabled, int arraySize, int firstValue, int secondValue, int leftIndex, int rightIndex);
void audio_play_compare(AudioEngine *engine, bool enabled, int arraySize, int firstValue, int secondValue, int leftIndex, int rightIndex);
void audio_play_sorted(AudioEngine *engine, bool enabled);

void audio_start_completion_sweep(AudioEngine *engine, bool enabled, int arraySize);
void audio_update_completion_sweep(AudioEngine *engine, float dt, bool enabled, int arraySize);

bool audio_completion_sweep_active(const AudioEngine *engine);
int audio_completion_sweep_index(const AudioEngine *engine);
bool audio_is_ready(const AudioEngine *engine);

#endif
