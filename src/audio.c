#include "audio.h"

#include <math.h>
#include <stdlib.h>

static const int maxSoundsPerFrame = 8;
static const float completionSweepInterval = 0.04f;
static const float completionSweepBasePitch = 0.75f;
static const float completionSweepPitchRange = 1.75f;

static Sound create_tone_sound(float frequency, float durationSeconds)
{
    const unsigned int sampleRate = 44100;
    const unsigned int sampleCount = (unsigned int)(sampleRate * durationSeconds);
    float *samples = (float *)malloc(sizeof(float) * sampleCount);

    if (samples == NULL) {
        return (Sound){ 0 };
    }

    for (unsigned int i = 0; i < sampleCount; i++) {
        float t = (float)i / (float)sampleRate;
        float envelope = 1.0f;
        float fadeTime = durationSeconds * 0.15f;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > durationSeconds - fadeTime) {
            envelope = (durationSeconds - t) / fadeTime;
        }

        samples[i] = 0.25f * envelope * sinf(2.0f * 3.14159265f * frequency * t);
    }

    Wave wave = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32;
    wave.channels = 1;
    wave.data = samples;

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return sound;
}

static float pitch_from_value(int value, int arraySize)
{
    if (arraySize <= 1) {
        return 1.0f;
    }

    float normalized = (float)(value - 1) / (float)(arraySize - 1);
    return 0.7f + normalized * 1.2f;
}

static float pan_from_index(int index, int arraySize)
{
    if (arraySize <= 1) {
        return 0.5f;
    }

    float normalized = (float)index / (float)(arraySize - 1);
    return 0.15f + normalized * 0.7f;
}

static void play_sound(AudioEngine *engine, Sound sound, bool channelEnabled, float pitch, float pan)
{
    if (!channelEnabled || !engine->ready || !IsSoundValid(sound)) {
        return;
    }

    if (engine->frameSoundCount >= maxSoundsPerFrame) {
        return;
    }

    SetSoundPitch(sound, pitch);
    SetSoundPan(sound, pan);
    PlaySound(sound);
    engine->frameSoundCount++;
}

void audio_init(AudioEngine *engine, float masterVolume)
{
    *engine = (AudioEngine){ 0 };

    InitAudioDevice();
    engine->ready = IsAudioDeviceReady();
    if (!engine->ready) {
        return;
    }

    SetMasterVolume(masterVolume);

    engine->compareSound = create_tone_sound(700.0f, 0.012f);
    engine->swapSound = create_tone_sound(660.0f, 0.06f);
    engine->sortedSound = create_tone_sound(880.0f, 0.05f);
    engine->completionSweepSound = create_tone_sound(440.0f, 0.03f);
    engine->confirmationSound = create_tone_sound(523.25f, 0.08f);

    if (IsSoundValid(engine->compareSound)) {
        SetSoundVolume(engine->compareSound, 0.12f);
    }
    if (IsSoundValid(engine->swapSound)) {
        SetSoundVolume(engine->swapSound, 0.4f);
    }
    if (IsSoundValid(engine->sortedSound)) {
        SetSoundVolume(engine->sortedSound, 0.35f);
    }
    if (IsSoundValid(engine->completionSweepSound)) {
        SetSoundVolume(engine->completionSweepSound, 0.45f);
    }
    if (IsSoundValid(engine->confirmationSound)) {
        SetSoundVolume(engine->confirmationSound, 0.5f);
    }
}

void audio_shutdown(AudioEngine *engine)
{
    if (engine->ready && IsSoundValid(engine->compareSound)) {
        UnloadSound(engine->compareSound);
    }
    if (engine->ready && IsSoundValid(engine->swapSound)) {
        UnloadSound(engine->swapSound);
    }
    if (engine->ready && IsSoundValid(engine->sortedSound)) {
        UnloadSound(engine->sortedSound);
    }
    if (engine->ready && IsSoundValid(engine->completionSweepSound)) {
        UnloadSound(engine->completionSweepSound);
    }
    if (engine->ready && IsSoundValid(engine->confirmationSound)) {
        UnloadSound(engine->confirmationSound);
    }
    if (engine->ready) {
        CloseAudioDevice();
    }
}

void audio_begin_frame(AudioEngine *engine)
{
    engine->frameSoundCount = 0;
}

void audio_set_master_volume(AudioEngine *engine, float volume)
{
    if (engine->ready) {
        SetMasterVolume(volume);
    }
}

void audio_play_swap(AudioEngine *engine, bool enabled, int arraySize, int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    float pan = (pan_from_index(leftIndex, arraySize) + pan_from_index(rightIndex, arraySize)) * 0.5f;
    play_sound(engine, engine->swapSound, enabled, pitch_from_value((int)avg, arraySize), pan);
}

void audio_play_compare(AudioEngine *engine, bool enabled, int arraySize, int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    float pan = (pan_from_index(leftIndex, arraySize) + pan_from_index(rightIndex, arraySize)) * 0.5f;
    play_sound(engine, engine->compareSound, enabled, pitch_from_value((int)avg, arraySize), pan);
}

void audio_play_sorted(AudioEngine *engine, bool enabled)
{
    play_sound(engine, engine->sortedSound, enabled, 1.0f, 0.5f);
}

void audio_start_completion_sweep(AudioEngine *engine, bool enabled, int arraySize)
{
    if (engine->completionSweepActive) {
        return;
    }

    engine->completionSweepActive = true;
    engine->confirmationPending = true;
    engine->completionSweepIndex = 0;
    engine->completionSweepTimer = 0.0f;

    play_sound(engine, engine->completionSweepSound, enabled, completionSweepBasePitch, pan_from_index(0, arraySize));
}

void audio_update_completion_sweep(AudioEngine *engine, float dt, bool enabled, int arraySize)
{
    if (!engine->completionSweepActive) {
        return;
    }

    engine->completionSweepTimer += dt;

    while (engine->completionSweepTimer >= completionSweepInterval && engine->completionSweepActive) {
        engine->completionSweepTimer -= completionSweepInterval;
        engine->completionSweepIndex++;

        if (engine->completionSweepIndex >= arraySize) {
            engine->completionSweepActive = false;
            if (engine->confirmationPending) {
                play_sound(engine, engine->confirmationSound, enabled, 1.0f, 0.5f);
                engine->confirmationPending = false;
            }
            break;
        }

        float normalized = (arraySize > 1) ? ((float)engine->completionSweepIndex / (float)(arraySize - 1)) : 1.0f;
        float pitch = completionSweepBasePitch + normalized * completionSweepPitchRange;
        play_sound(engine, engine->completionSweepSound, enabled, pitch, pan_from_index(engine->completionSweepIndex, arraySize));
    }
}

bool audio_completion_sweep_active(const AudioEngine *engine)
{
    return engine->completionSweepActive;
}

int audio_completion_sweep_index(const AudioEngine *engine)
{
    return engine->completionSweepIndex;
}

bool audio_is_ready(const AudioEngine *engine)
{
    return engine->ready;
}
