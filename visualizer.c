#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <raylib.h>

#define SIZE 100

typedef enum SortMode {
    SORT_BUBBLE,
    SORT_INSERTION,
    SORT_SELECTION
} SortMode;

static const int WIDTH = 1800;
static const int HEIGHT = 1000;
static const char* TITLE = "Sorting Visualizer";
static const Color DEFAULT_COLOR = LIGHTGRAY;

int numbers[SIZE];

static SortMode currentSort = SORT_BUBBLE;
static int bubbleIndex = 0;
static int bubblePass = 0;
static int insertionIndex = 1;
static int insertionPos = 0;
static int insertionKey = 0;
static bool insertionHoldingKey = false;
static int selectionI = 0;
static int selectionJ = 1;
static int selectionMin = 0;
static bool sortingDone = false;
static bool paused = false;
static Sound compareSound = { 0 };
static Sound swapSound = { 0 };
static Sound sortedSound = { 0 };
static Sound completionSweepSound = { 0 };
static Sound confirmationSound = { 0 };
static bool completionSweepActive = false;
static int completionSweepIndex = 0;
static float completionSweepTimer = 0.0f;
static bool confirmationPending = false;
static bool audioReady = false;
static bool compareAudioEnabled = true;
static bool swapAudioEnabled = true;
static bool progressAudioEnabled = true;
static bool finishAudioEnabled = true;
static int frameSoundCount = 0;

static const float completionSweepInterval = 0.04f;
static const float completionSweepBasePitch = 0.75f;
static const float completionSweepPitchRange = 1.75f;
static const int maxSoundsPerFrame = 8;

static const float padding = 20.0f;
static const float gap = 4.0f;

static const float availableWidth = WIDTH - 2.0f * padding;
static const float availableHeight = HEIGHT - 2.0f * padding;

static const float barWidth = (availableWidth - gap * (SIZE - 1)) / SIZE;

static void shuffle_numbers(void)
{
    for (int i = SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }
}

static void reset_completion_effects(void)
{
    completionSweepActive = false;
    completionSweepIndex = 0;
    completionSweepTimer = 0.0f;
    confirmationPending = false;
}

static void reset_bubble_state(void)
{
    bubbleIndex = 0;
    bubblePass = 0;
}

static void reset_insertion_state(void)
{
    insertionIndex = 1;
    insertionPos = 0;
    insertionKey = 0;
    insertionHoldingKey = false;
}

static void reset_selection_state(void)
{
    selectionI = 0;
    selectionJ = 1;
    selectionMin = 0;
}

static void reset_sort_state(bool reshuffle)
{
    sortingDone = false;
    paused = false;
    reset_completion_effects();
    reset_bubble_state();
    reset_insertion_state();
    reset_selection_state();

    if (reshuffle) {
        shuffle_numbers();
    }
}

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

static float pitch_from_value(int value)
{
    float normalized = (float)(value - 1) / (float)(SIZE - 1);
    return 0.7f + normalized * 1.2f;
}

static void play_sound(Sound sound, bool channelEnabled, float pitch)
{
    if (!channelEnabled || !audioReady || !IsSoundValid(sound)) {
        return;
    }

    if (frameSoundCount >= maxSoundsPerFrame) {
        return;
    }

    SetSoundPitch(sound, pitch);
    PlaySound(sound);
    frameSoundCount++;
}

static void play_swap_sound(int firstValue, int secondValue)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    play_sound(swapSound, swapAudioEnabled, pitch_from_value((int)avg));
}

static void play_compare_sound(int firstValue, int secondValue)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    play_sound(compareSound, compareAudioEnabled, pitch_from_value((int)avg));
}

static void play_sorted_sound(void)
{
    play_sound(sortedSound, progressAudioEnabled, 1.0f);
}

static const char *get_sort_name(void)
{
    if (currentSort == SORT_BUBBLE) {
        return "Bubble";
    }

    if (currentSort == SORT_INSERTION) {
        return "Insertion";
    }

    return "Selection";
}

static void start_completion_sweep(void)
{
    if (completionSweepActive) {
        return;
    }

    completionSweepActive = true;
    confirmationPending = true;
    completionSweepIndex = 0;
    completionSweepTimer = 0.0f;

    play_sound(completionSweepSound, finishAudioEnabled, completionSweepBasePitch);
}

static void update_completion_sweep(float dt)
{
    if (!completionSweepActive) {
        return;
    }

    completionSweepTimer += dt;

    while (completionSweepTimer >= completionSweepInterval && completionSweepActive) {
        completionSweepTimer -= completionSweepInterval;
        completionSweepIndex++;

        if (completionSweepIndex >= SIZE) {
            completionSweepActive = false;
            if (confirmationPending) {
                play_sound(confirmationSound, finishAudioEnabled, 1.0f);
                confirmationPending = false;
            }
            break;
        }

        float normalized = (float)completionSweepIndex / (float)(SIZE - 1);
        float pitch = completionSweepBasePitch + normalized * completionSweepPitchRange;
        play_sound(completionSweepSound, finishAudioEnabled, pitch);
    }
}

void draw_elements(float speedMultiplier, bool showInfo){
    for(int i = 0; i < SIZE; i++){
        int displayValue = numbers[i];
        if (currentSort == SORT_INSERTION && insertionHoldingKey && i == insertionIndex) {
            displayValue = insertionKey;
        }

        float barHeight = ((float)displayValue / (float)SIZE) * availableHeight;
        float x = padding + i * (barWidth + gap);
        float y = HEIGHT - padding - barHeight;

        Color barColor = WHITE;
        if (completionSweepActive && i == completionSweepIndex) {
            barColor = YELLOW;
        } else if (currentSort == SORT_BUBBLE && !sortingDone && (i == bubbleIndex || i == bubbleIndex + 1)) {
            barColor = RED;
        } else if (currentSort == SORT_INSERTION && !sortingDone && insertionHoldingKey && i == insertionPos) {
            barColor = ORANGE;
        } else if (currentSort == SORT_SELECTION && !sortingDone && i == selectionMin) {
            barColor = ORANGE;
        } else if (currentSort == SORT_SELECTION && !sortingDone && i == selectionJ && selectionJ < SIZE) {
            barColor = RED;
        } else if (sortingDone || (currentSort == SORT_BUBBLE && i >= SIZE - bubblePass) || (currentSort == SORT_INSERTION && i < insertionIndex) || (currentSort == SORT_SELECTION && i < selectionI)) {
            barColor = GREEN;
        }

        Rectangle bar = {x, y, barWidth, barHeight};
        DrawRectangleRec(bar, barColor);
    }

    DrawText(TextFormat("Sort: %s (TAB)", get_sort_name()), 20, 20, 30, YELLOW);
    DrawText(TextFormat("Speed: %.1fx", speedMultiplier), 20, 60, 30, YELLOW);

    if (paused) {
        DrawText("PAUSED", 20, 100, 30, YELLOW);
    }

    if (showInfo) {
        DrawText("Hotkeys (hold I)", 20, 150, 24, SKYBLUE);
        DrawText("SPACE: Pause/Resume", 20, 180, 20, LIGHTGRAY);
        DrawText("UP/DOWN: Speed +/-", 20, 205, 20, LIGHTGRAY);
        DrawText("TAB: Cycle Sort", 20, 230, 20, LIGHTGRAY);
        DrawText(TextFormat("C: Compare [%s]", compareAudioEnabled ? "ON" : "OFF"), 20, 255, 20, LIGHTGRAY);
        DrawText(TextFormat("S: Swap [%s]", swapAudioEnabled ? "ON" : "OFF"), 20, 280, 20, LIGHTGRAY);
        DrawText(TextFormat("P: Progress [%s]", progressAudioEnabled ? "ON" : "OFF"), 20, 305, 20, LIGHTGRAY);
        DrawText(TextFormat("F: Finish [%s]", finishAudioEnabled ? "ON" : "OFF"), 20, 330, 20, LIGHTGRAY);
    }
}

void bubble_sort_step(void){
    if (sortingDone) {
        return;
    }

    if (bubblePass >= SIZE - 1) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
        return;
    }

    if (bubbleIndex < SIZE - bubblePass - 1) {
        int left = numbers[bubbleIndex];
        int right = numbers[bubbleIndex + 1];
        play_compare_sound(left, right);
        if (numbers[bubbleIndex] > numbers[bubbleIndex + 1]) {
            int temp = numbers[bubbleIndex];
            numbers[bubbleIndex] = numbers[bubbleIndex + 1];
            numbers[bubbleIndex + 1] = temp;
            play_swap_sound(left, right);
        }
        bubbleIndex++;
    } else {
        bubbleIndex = 0;
        bubblePass++;

        if (bubblePass < SIZE - 1) {
            play_sorted_sound();
        }
    }

    if (bubblePass >= SIZE - 1) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
    }
}

void insertion_sort_step(void){
    if (sortingDone) {
        return;
    }

    if (insertionIndex >= SIZE) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
        return;
    }

    if (!insertionHoldingKey) {
        insertionKey = numbers[insertionIndex];
        insertionPos = insertionIndex - 1;
        insertionHoldingKey = true;
        return;
    }

    if (insertionPos >= 0) {
        play_compare_sound(numbers[insertionPos], insertionKey);
    }

    if (insertionPos >= 0 && numbers[insertionPos] > insertionKey) {
        int movedValue = numbers[insertionPos];
        numbers[insertionPos + 1] = numbers[insertionPos];
        insertionPos--;
        play_swap_sound(movedValue, insertionKey);
    } else {
        numbers[insertionPos + 1] = insertionKey;
        insertionHoldingKey = false;
        insertionIndex++;

        if (insertionIndex < SIZE) {
            play_sorted_sound();
        }
    }

    if (insertionIndex >= SIZE) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
    }
}

void selection_sort_step(void){
    if (sortingDone) {
        return;
    }

    if (selectionI >= SIZE - 1) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
        return;
    }

    if (selectionJ < SIZE) {
        play_compare_sound(numbers[selectionJ], numbers[selectionMin]);
        if (numbers[selectionJ] < numbers[selectionMin]) {
            selectionMin = selectionJ;
        }
        selectionJ++;
        return;
    }

    if (selectionMin != selectionI) {
        int left = numbers[selectionI];
        int right = numbers[selectionMin];
        int temp = numbers[selectionI];
        numbers[selectionI] = numbers[selectionMin];
        numbers[selectionMin] = temp;
        play_swap_sound(left, right);
    }

    selectionI++;
    if (selectionI < SIZE - 1) {
        play_sorted_sound();
    }

    selectionMin = selectionI;
    selectionJ = selectionI + 1;

    if (selectionI >= SIZE - 1) {
        if (!sortingDone) {
            sortingDone = true;
            start_completion_sweep();
        }
    }
}

int main(){
    srand((unsigned int)time(NULL));

    for(int i = 0; i < SIZE; i++){
        numbers[i] = i + 1;
        //printf("numbers[%d] = %d\n", i, numbers[i]);
    }

    shuffle_numbers();

    InitWindow(WIDTH, HEIGHT, TITLE);
    InitAudioDevice();
    audioReady = IsAudioDeviceReady();
    if (audioReady) {
        compareSound = create_tone_sound(760.0f, 0.02f);
        swapSound = create_tone_sound(660.0f, 0.06f);
        sortedSound = create_tone_sound(880.0f, 0.05f);
        completionSweepSound = create_tone_sound(440.0f, 0.03f);
        confirmationSound = create_tone_sound(523.25f, 0.08f);
        if (IsSoundValid(compareSound)) {
            SetSoundVolume(compareSound, 0.2f);
        }
        if (IsSoundValid(swapSound)) {
            SetSoundVolume(swapSound, 0.4f);
        }
        if (IsSoundValid(sortedSound)) {
            SetSoundVolume(sortedSound, 0.35f);
        }
        if (IsSoundValid(completionSweepSound)) {
            SetSoundVolume(completionSweepSound, 0.45f);
        }
        if (IsSoundValid(confirmationSound)) {
            SetSoundVolume(confirmationSound, 0.5f);
        }
    }

    SetTargetFPS(60);
    float speedMultiplier = 1.0f;
    float speedStep = 0.1f;
    float baseDelay = 0.05f;
    float stepTimer = 0.0f;

    reset_sort_state(false);
    
    while(!WindowShouldClose()){
        float dt = GetFrameTime();
        frameSoundCount = 0;

        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }

        if (IsKeyPressed(KEY_C)) {
            compareAudioEnabled = !compareAudioEnabled;
        }

        if (IsKeyPressed(KEY_S)) {
            swapAudioEnabled = !swapAudioEnabled;
        }

        if (IsKeyPressed(KEY_P)) {
            progressAudioEnabled = !progressAudioEnabled;
        }

        if (IsKeyPressed(KEY_F)) {
            finishAudioEnabled = !finishAudioEnabled;
        }

        if (IsKeyPressed(KEY_TAB)) {
            if (currentSort == SORT_BUBBLE) {
                currentSort = SORT_INSERTION;
            } else if (currentSort == SORT_INSERTION) {
                currentSort = SORT_SELECTION;
            } else {
                currentSort = SORT_BUBBLE;
            }
            reset_sort_state(true);
            stepTimer = 0.0f;
        }

        if(IsKeyDown(KEY_UP)) speedMultiplier += speedStep;
        if(IsKeyDown(KEY_DOWN)) speedMultiplier -= speedStep;

        if(speedMultiplier < 0.1f) speedMultiplier = 0.1f;
        if(speedMultiplier > 10.0f) speedMultiplier = 10.0f;

        if (!paused && !sortingDone) {
            stepTimer += dt;
            float animationDelay = baseDelay / speedMultiplier;
            while(stepTimer >= animationDelay && !sortingDone){
                if (currentSort == SORT_BUBBLE) {
                    bubble_sort_step();
                } else if (currentSort == SORT_INSERTION) {
                    insertion_sort_step();
                } else {
                    selection_sort_step();
                }
                stepTimer -= animationDelay;
            }
        }

        update_completion_sweep(dt);

        BeginDrawing();
        ClearBackground(BLACK);

        //Draw shit
        draw_elements(speedMultiplier, IsKeyDown(KEY_I));

        EndDrawing();
    }

    CloseWindow();

    if (audioReady && IsSoundValid(compareSound)) {
        UnloadSound(compareSound);
    }
    if (audioReady && IsSoundValid(swapSound)) {
        UnloadSound(swapSound);
    }
    if (audioReady && IsSoundValid(sortedSound)) {
        UnloadSound(sortedSound);
    }
    if (audioReady && IsSoundValid(completionSweepSound)) {
        UnloadSound(completionSweepSound);
    }
    if (audioReady && IsSoundValid(confirmationSound)) {
        UnloadSound(confirmationSound);
    }
    if (audioReady) {
        CloseAudioDevice();
    }

    return 0;
}
