#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <raylib.h>

#define SIZE 50

typedef enum SortMode {
    SORT_BUBBLE,
    SORT_INSERTION,
    SORT_SELECTION,
    SORT_HEAP,
    SORT_QUICK
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
static int heapBuildIndex = 0;
static int heapSortEnd = 0;
static int heapSiftRoot = 0;
static int heapSiftEnd = 0;
static int heapFocusIndex = -1;
static int heapCandidateIndex = -1;
static bool heapBuilding = true;
static bool heapSiftActive = false;
static int quickStackLow[SIZE];
static int quickStackHigh[SIZE];
static int quickTop = -1;
static int quickLow = 0;
static int quickHigh = 0;
static int quickPivotValue = 0;
static int quickPivotIndex = -1;
static int quickI = -1;
static int quickJ = -1;
static bool quickPartitionActive = false;
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
static bool showValues = false;
static bool showLegend = true;
static bool showHud = true;
static int frameSoundCount = 0;
static float masterVolume = 0.8f;
static float autoNextSortTimer = 0.0f;
static unsigned long long statComparisons = 0;
static unsigned long long statSwaps = 0;
static unsigned long long statSteps = 0;
static float statElapsed = 0.0f;
static char statusText[128] = "";
static float statusTimer = 0.0f;

static const float completionSweepInterval = 0.04f;
static const float completionSweepBasePitch = 0.75f;
static const float completionSweepPitchRange = 1.75f;
static const int maxSoundsPerFrame = 8;
static const float masterVolumeStep = 0.05f;
static const float autoNextSortDelay = 3.0f;
static const char *presetPath = "visualizer_preset.cfg";

static const float padding = 20.0f;
static const float gap = 4.0f;

static const float availableWidth = WIDTH - 2.0f * padding;
static const float availableHeight = HEIGHT - 2.0f * padding;

static const float barWidth = (availableWidth - gap * (SIZE - 1)) / SIZE;

static void set_status_text(const char *text)
{
    snprintf(statusText, sizeof(statusText), "%s", text);
    statusTimer = 2.0f;
}

static void reset_stats(void)
{
    statComparisons = 0;
    statSwaps = 0;
    statSteps = 0;
    statElapsed = 0.0f;
}

static void save_preset(float speedMultiplier)
{
    FILE *f = fopen(presetPath, "w");
    if (!f) {
        set_status_text("Preset save failed");
        return;
    }

    fprintf(f, "sort %d\n", (int)currentSort);
    fprintf(f, "speed %.4f\n", speedMultiplier);
    fprintf(f, "compare %d\n", compareAudioEnabled ? 1 : 0);
    fprintf(f, "swap %d\n", swapAudioEnabled ? 1 : 0);
    fprintf(f, "progress %d\n", progressAudioEnabled ? 1 : 0);
    fprintf(f, "finish %d\n", finishAudioEnabled ? 1 : 0);
    fprintf(f, "values %d\n", showValues ? 1 : 0);
    fprintf(f, "legend %d\n", showLegend ? 1 : 0);
    fprintf(f, "hud %d\n", showHud ? 1 : 0);
    fprintf(f, "volume %.4f\n", masterVolume);
    fclose(f);
    set_status_text("Preset saved");
}

static bool load_preset(float *speedMultiplier)
{
    FILE *f = fopen(presetPath, "r");
    if (!f) {
        set_status_text("Preset file missing");
        return false;
    }

    char key[32];
    while (fscanf(f, "%31s", key) == 1) {
        if (strcmp(key, "sort") == 0) {
            int mode = 0;
            if (fscanf(f, "%d", &mode) == 1 && mode >= SORT_BUBBLE && mode <= SORT_QUICK) {
                currentSort = (SortMode)mode;
            }
        } else if (strcmp(key, "speed") == 0) {
            float value = 1.0f;
            if (fscanf(f, "%f", &value) == 1) {
                *speedMultiplier = value;
            }
        } else if (strcmp(key, "compare") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) compareAudioEnabled = (v != 0);
        } else if (strcmp(key, "swap") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) swapAudioEnabled = (v != 0);
        } else if (strcmp(key, "progress") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) progressAudioEnabled = (v != 0);
        } else if (strcmp(key, "finish") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) finishAudioEnabled = (v != 0);
        } else if (strcmp(key, "values") == 0) {
            int v = 0;
            if (fscanf(f, "%d", &v) == 1) showValues = (v != 0);
        } else if (strcmp(key, "legend") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) showLegend = (v != 0);
        } else if (strcmp(key, "hud") == 0) {
            int v = 1;
            if (fscanf(f, "%d", &v) == 1) showHud = (v != 0);
        } else if (strcmp(key, "volume") == 0) {
            float v = 0.8f;
            if (fscanf(f, "%f", &v) == 1) masterVolume = v;
        }
    }

    fclose(f);

    if (*speedMultiplier < 0.1f) *speedMultiplier = 0.1f;
    if (*speedMultiplier > 10.0f) *speedMultiplier = 10.0f;
    if (masterVolume < 0.0f) masterVolume = 0.0f;
    if (masterVolume > 1.0f) masterVolume = 1.0f;
    if (audioReady) SetMasterVolume(masterVolume);

    set_status_text("Preset loaded");
    return true;
}

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

static void reset_heap_state(void)
{
    heapBuildIndex = (SIZE / 2) - 1;
    heapSortEnd = SIZE - 1;
    heapSiftRoot = 0;
    heapSiftEnd = 0;
    heapFocusIndex = -1;
    heapCandidateIndex = -1;
    heapBuilding = true;
    heapSiftActive = false;
}

static void quick_push_range(int low, int high)
{
    if (quickTop >= SIZE - 1) {
        return;
    }

    quickTop++;
    quickStackLow[quickTop] = low;
    quickStackHigh[quickTop] = high;
}

static void reset_quick_state(void)
{
    quickTop = -1;
    quickLow = 0;
    quickHigh = 0;
    quickPivotValue = 0;
    quickPivotIndex = -1;
    quickI = -1;
    quickJ = -1;
    quickPartitionActive = false;

    if (SIZE > 1) {
        quick_push_range(0, SIZE - 1);
    }
}

static void reset_sort_state(bool reshuffle)
{
    sortingDone = false;
    paused = false;
    autoNextSortTimer = 0.0f;
    reset_stats();
    reset_completion_effects();
    reset_bubble_state();
    reset_insertion_state();
    reset_selection_state();
    reset_heap_state();
    reset_quick_state();

    if (reshuffle) {
        shuffle_numbers();
    }
}

static void cycle_sort_mode(void)
{
    if (currentSort == SORT_BUBBLE) {
        currentSort = SORT_INSERTION;
    } else if (currentSort == SORT_INSERTION) {
        currentSort = SORT_SELECTION;
    } else if (currentSort == SORT_SELECTION) {
        currentSort = SORT_HEAP;
    } else if (currentSort == SORT_HEAP) {
        currentSort = SORT_QUICK;
    } else {
        currentSort = SORT_BUBBLE;
    }

    reset_sort_state(true);
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

static float pan_from_index(int index)
{
    float normalized = (float)index / (float)(SIZE - 1);
    return 0.15f + normalized * 0.7f;
}

static void play_sound(Sound sound, bool channelEnabled, float pitch, float pan)
{
    if (!channelEnabled || !audioReady || !IsSoundValid(sound)) {
        return;
    }

    if (frameSoundCount >= maxSoundsPerFrame) {
        return;
    }

    SetSoundPitch(sound, pitch);
    SetSoundPan(sound, pan);
    PlaySound(sound);
    frameSoundCount++;
}

static void play_swap_sound(int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    float pan = (pan_from_index(leftIndex) + pan_from_index(rightIndex)) * 0.5f;
    play_sound(swapSound, swapAudioEnabled, pitch_from_value((int)avg), pan);
}

static void play_compare_sound(int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    float avg = ((float)firstValue + (float)secondValue) * 0.5f;
    float pan = (pan_from_index(leftIndex) + pan_from_index(rightIndex)) * 0.5f;
    play_sound(compareSound, compareAudioEnabled, pitch_from_value((int)avg), pan);
}

static void play_sorted_sound(void)
{
    play_sound(sortedSound, progressAudioEnabled, 1.0f, 0.5f);
}

static const char *get_sort_name(void)
{
    if (currentSort == SORT_BUBBLE) {
        return "Bubble";
    }

    if (currentSort == SORT_INSERTION) {
        return "Insertion";
    }

    if (currentSort == SORT_SELECTION) {
        return "Selection";
    }

    if (currentSort == SORT_HEAP) {
        return "Heap";
    }

    return "Quick";
}

static void heap_start_sift(int root, int end)
{
    heapSiftRoot = root;
    heapSiftEnd = end;
    heapFocusIndex = root;
    heapCandidateIndex = -1;
    heapSiftActive = true;
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

    play_sound(completionSweepSound, finishAudioEnabled, completionSweepBasePitch, pan_from_index(0));
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
                play_sound(confirmationSound, finishAudioEnabled, 1.0f, 0.5f);
                confirmationPending = false;
            }
            break;
        }

        float normalized = (float)completionSweepIndex / (float)(SIZE - 1);
        float pitch = completionSweepBasePitch + normalized * completionSweepPitchRange;
        play_sound(completionSweepSound, finishAudioEnabled, pitch, pan_from_index(completionSweepIndex));
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
        } else if (currentSort == SORT_QUICK && !sortingDone && i == quickPivotIndex) {
            barColor = ORANGE;
        } else if (currentSort == SORT_QUICK && !sortingDone && i == quickJ) {
            barColor = RED;
        } else if (currentSort == SORT_QUICK && !sortingDone && i == quickI) {
            barColor = PINK;
        } else if (currentSort == SORT_HEAP && !sortingDone && i == heapCandidateIndex) {
            barColor = ORANGE;
        } else if (currentSort == SORT_HEAP && !sortingDone && i == heapFocusIndex) {
            barColor = RED;
        } else if (currentSort == SORT_BUBBLE && !sortingDone && (i == bubbleIndex || i == bubbleIndex + 1)) {
            barColor = RED;
        } else if (currentSort == SORT_INSERTION && !sortingDone && insertionHoldingKey && i == insertionPos) {
            barColor = ORANGE;
        } else if (currentSort == SORT_SELECTION && !sortingDone && i == selectionMin) {
            barColor = ORANGE;
        } else if (currentSort == SORT_SELECTION && !sortingDone && i == selectionJ && selectionJ < SIZE) {
            barColor = RED;
        } else if (sortingDone || (currentSort == SORT_BUBBLE && i >= SIZE - bubblePass) || (currentSort == SORT_INSERTION && i < insertionIndex) || (currentSort == SORT_SELECTION && i < selectionI) || (currentSort == SORT_HEAP && !heapBuilding && i > heapSortEnd)) {
            barColor = GREEN;
        }

        Rectangle bar = {x, y, barWidth, barHeight};
        DrawRectangleRec(bar, barColor);

        if (showValues && showHud) {
            int fontSize = 20;
            const char *valueText = TextFormat("%d", displayValue);
            int textWidth = MeasureText(valueText, fontSize);
            int textX = (int)(x + (barWidth - (float)textWidth) * 0.5f);
            int textY = (int)y - fontSize - 4;
            if (textY < 4) textY = 4;
            DrawText(valueText, textX, textY, fontSize, RAYWHITE);
        }
    }

    if (!showHud) {
        return;
    }

    DrawText(TextFormat("Sort: %s (TAB)", get_sort_name()), 20, 20, 30, YELLOW);
    DrawText(TextFormat("Speed: %.1fx", speedMultiplier), 20, 60, 30, YELLOW);
    DrawText(TextFormat("Vol: %.2f [ / ]", masterVolume), 20, 100, 24, SKYBLUE);
    DrawText(TextFormat("Audio C:%s S:%s P:%s F:%s", compareAudioEnabled ? "ON" : "OFF", swapAudioEnabled ? "ON" : "OFF", progressAudioEnabled ? "ON" : "OFF", finishAudioEnabled ? "ON" : "OFF"), 20, 130, 20, LIGHTGRAY);
    DrawText(TextFormat("Values [V]: %s", showValues ? "ON" : "OFF"), 20, 155, 20, LIGHTGRAY);
    DrawText(TextFormat("Legend [L]: %s", showLegend ? "ON" : "OFF"), 20, 180, 20, LIGHTGRAY);
    DrawText(TextFormat("HUD [H]: %s", showHud ? "ON" : "OFF"), 20, 205, 20, LIGHTGRAY);
    DrawText(TextFormat("Stats  cmp:%llu  swp:%llu  steps:%llu", statComparisons, statSwaps, statSteps), 360, 20, 20, LIGHTGRAY);
    DrawText(TextFormat("Time: %.2fs  Steps/s: %.1f", statElapsed, (statElapsed > 0.0f) ? ((float)statSteps / statElapsed) : 0.0f), 360, 40, 20, LIGHTGRAY);

    if (statusTimer > 0.0f) {
        DrawText(statusText, 20, 285, 22, SKYBLUE);
    }

    if (showLegend) {
        int legendX = WIDTH - 280;
        int legendY = 20;
        DrawText("Legend", legendX, legendY, 24, SKYBLUE);

        if (currentSort == SORT_BUBBLE) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Compare Pair", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (currentSort == SORT_INSERTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ORANGE);
            DrawText("Insert Compare", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Prefix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (currentSort == SORT_SELECTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Scan Index", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ORANGE);
            DrawText("Current Min", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Finalized Prefix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (currentSort == SORT_HEAP) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Sift Root", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ORANGE);
            DrawText("Largest Candidate", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Scan j", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, PINK);
            DrawText("Partition i", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, ORANGE);
            DrawText("Pivot", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        }
    }

    if (paused) {
        DrawText("PAUSED", 20, 235, 30, YELLOW);
    }

    if (sortingDone && !completionSweepActive) {
        float remaining = autoNextSortDelay - autoNextSortTimer;
        if (remaining < 0.0f) remaining = 0.0f;
        DrawText(TextFormat("Next Sort In: %.1fs", remaining), 20, 265, 24, SKYBLUE);
    }

    if (showInfo) {
        DrawText("Hotkeys (hold I)", 20, 300, 24, SKYBLUE);
        DrawText("SPACE: Pause/Resume", 20, 330, 20, LIGHTGRAY);
        DrawText("UP/DOWN: Speed +/-", 20, 355, 20, LIGHTGRAY);
        DrawText("TAB: Cycle Sort", 20, 380, 20, LIGHTGRAY);
        DrawText("R: Reshuffle/Restart", 20, 405, 20, LIGHTGRAY);
        DrawText("K: Save Preset", 20, 430, 20, LIGHTGRAY);
        DrawText("O: Load Preset", 20, 455, 20, LIGHTGRAY);
        DrawText(TextFormat("V: Values [%s]", showValues ? "ON" : "OFF"), 20, 480, 20, LIGHTGRAY);
        DrawText(TextFormat("L: Legend [%s]", showLegend ? "ON" : "OFF"), 20, 505, 20, LIGHTGRAY);
        DrawText(TextFormat("H: HUD [%s]", showHud ? "ON" : "OFF"), 20, 530, 20, LIGHTGRAY);
        DrawText(TextFormat("C: Compare [%s]", compareAudioEnabled ? "ON" : "OFF"), 20, 555, 20, LIGHTGRAY);
        DrawText(TextFormat("S: Swap [%s]", swapAudioEnabled ? "ON" : "OFF"), 20, 580, 20, LIGHTGRAY);
        DrawText(TextFormat("P: Progress [%s]", progressAudioEnabled ? "ON" : "OFF"), 20, 605, 20, LIGHTGRAY);
        DrawText(TextFormat("F: Finish [%s]", finishAudioEnabled ? "ON" : "OFF"), 20, 630, 20, LIGHTGRAY);
        DrawText("[ / ]: Master Volume", 20, 655, 20, LIGHTGRAY);
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
        statComparisons++;
        play_compare_sound(left, right, bubbleIndex, bubbleIndex + 1);
        if (numbers[bubbleIndex] > numbers[bubbleIndex + 1]) {
            int temp = numbers[bubbleIndex];
            numbers[bubbleIndex] = numbers[bubbleIndex + 1];
            numbers[bubbleIndex + 1] = temp;
            statSwaps++;
            play_swap_sound(left, right, bubbleIndex, bubbleIndex + 1);
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
        statComparisons++;
        play_compare_sound(numbers[insertionPos], insertionKey, insertionPos, insertionIndex);
    }

    if (insertionPos >= 0 && numbers[insertionPos] > insertionKey) {
        int movedValue = numbers[insertionPos];
        numbers[insertionPos + 1] = numbers[insertionPos];
        insertionPos--;
        statSwaps++;
        play_swap_sound(movedValue, insertionKey, insertionPos + 1, insertionPos + 2);
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
        statComparisons++;
        play_compare_sound(numbers[selectionJ], numbers[selectionMin], selectionJ, selectionMin);
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
        statSwaps++;
        play_swap_sound(left, right, selectionI, selectionMin);
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

void heap_sort_step(void){
    if (sortingDone) {
        return;
    }

    if (heapBuilding) {
        if (!heapSiftActive) {
            if (heapBuildIndex < 0) {
                heapBuilding = false;
                heapSortEnd = SIZE - 1;
                heapFocusIndex = -1;
                heapCandidateIndex = -1;
                return;
            }

            heap_start_sift(heapBuildIndex, SIZE - 1);
            heapBuildIndex--;
            return;
        }
    } else {
        if (heapSortEnd <= 0) {
            sortingDone = true;
            heapFocusIndex = -1;
            heapCandidateIndex = -1;
            start_completion_sweep();
            return;
        }

        if (!heapSiftActive) {
            int left = numbers[0];
            int right = numbers[heapSortEnd];
            numbers[0] = right;
            numbers[heapSortEnd] = left;
            statSwaps++;
            play_swap_sound(left, right, 0, heapSortEnd);
            if (heapSortEnd > 1) {
                play_sorted_sound();
            }
            heapSortEnd--;

            if (heapSortEnd <= 0) {
                sortingDone = true;
                heapFocusIndex = -1;
                heapCandidateIndex = -1;
                start_completion_sweep();
                return;
            }

            heap_start_sift(0, heapSortEnd);
            return;
        }
    }

    int root = heapSiftRoot;
    int left = 2 * root + 1;

    if (left > heapSiftEnd) {
        heapSiftActive = false;
        heapFocusIndex = -1;
        heapCandidateIndex = -1;
        return;
    }

    int swapIdx = root;
    heapFocusIndex = root;
    heapCandidateIndex = root;

    play_compare_sound(numbers[swapIdx], numbers[left], swapIdx, left);
    statComparisons++;
    if (numbers[left] > numbers[swapIdx]) {
        swapIdx = left;
        heapCandidateIndex = left;
    }

    int right = left + 1;
    if (right <= heapSiftEnd) {
        statComparisons++;
        play_compare_sound(numbers[swapIdx], numbers[right], swapIdx, right);
        if (numbers[right] > numbers[swapIdx]) {
            swapIdx = right;
            heapCandidateIndex = right;
        }
    }

    if (swapIdx != root) {
        int a = numbers[root];
        int b = numbers[swapIdx];
        numbers[root] = b;
        numbers[swapIdx] = a;
        statSwaps++;
        play_swap_sound(a, b, root, swapIdx);
        heapSiftRoot = swapIdx;
        heapFocusIndex = swapIdx;
    } else {
        heapSiftActive = false;
        heapFocusIndex = -1;
        heapCandidateIndex = -1;
    }
}

void quick_sort_step(void)
{
    if (sortingDone) {
        return;
    }

    if (!quickPartitionActive) {
        while (quickTop >= 0) {
            int low = quickStackLow[quickTop];
            int high = quickStackHigh[quickTop];
            quickTop--;

            if (low >= high) {
                continue;
            }

            quickLow = low;
            quickHigh = high;
            quickPivotValue = numbers[high];
            quickPivotIndex = high;
            quickI = low - 1;
            quickJ = low;
            quickPartitionActive = true;
            return;
        }

        sortingDone = true;
        quickPivotIndex = -1;
        quickI = -1;
        quickJ = -1;
        start_completion_sweep();
        return;
    }

    if (quickJ < quickHigh) {
        statComparisons++;
        play_compare_sound(numbers[quickJ], quickPivotValue, quickJ, quickPivotIndex);

        if (numbers[quickJ] < quickPivotValue) {
            quickI++;
            if (quickI != quickJ) {
                int left = numbers[quickI];
                int right = numbers[quickJ];
                numbers[quickI] = right;
                numbers[quickJ] = left;
                statSwaps++;
                play_swap_sound(left, right, quickI, quickJ);
            }
        }

        quickJ++;
        return;
    }

    int pivotPos = quickI + 1;
    if (pivotPos != quickHigh) {
        int left = numbers[pivotPos];
        int right = numbers[quickHigh];
        numbers[pivotPos] = right;
        numbers[quickHigh] = left;
        statSwaps++;
        play_swap_sound(left, right, pivotPos, quickHigh);
    }
    play_sorted_sound();

    int leftLow = quickLow;
    int leftHigh = pivotPos - 1;
    int rightLow = pivotPos + 1;
    int rightHigh = quickHigh;

    if (leftLow < leftHigh) {
        quick_push_range(leftLow, leftHigh);
    }
    if (rightLow < rightHigh) {
        quick_push_range(rightLow, rightHigh);
    }

    quickPartitionActive = false;
    quickPivotIndex = -1;
    quickI = -1;
    quickJ = -1;
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
        SetMasterVolume(masterVolume);
        compareSound = create_tone_sound(700.0f, 0.012f);
        swapSound = create_tone_sound(660.0f, 0.06f);
        sortedSound = create_tone_sound(880.0f, 0.05f);
        completionSweepSound = create_tone_sound(440.0f, 0.03f);
        confirmationSound = create_tone_sound(523.25f, 0.08f);
        if (IsSoundValid(compareSound)) {
            SetSoundVolume(compareSound, 0.12f);
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

        if (IsKeyPressed(KEY_V)) {
            showValues = !showValues;
        }

        if (IsKeyPressed(KEY_L)) {
            showLegend = !showLegend;
        }

        if (IsKeyPressed(KEY_H)) {
            showHud = !showHud;
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

        if (IsKeyDown(KEY_RIGHT_BRACKET)) {
            masterVolume += masterVolumeStep * dt;
        }

        if (IsKeyDown(KEY_LEFT_BRACKET)) {
            masterVolume -= masterVolumeStep * dt;
        }

        if (masterVolume < 0.0f) masterVolume = 0.0f;
        if (masterVolume > 1.0f) masterVolume = 1.0f;
        if (audioReady) SetMasterVolume(masterVolume);

        if (IsKeyPressed(KEY_TAB)) {
            cycle_sort_mode();
            stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_R)) {
            reset_sort_state(true);
            stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_K)) {
            save_preset(speedMultiplier);
        }

        if (IsKeyPressed(KEY_O)) {
            if (load_preset(&speedMultiplier)) {
                reset_sort_state(true);
                stepTimer = 0.0f;
            }
        }

        if(IsKeyDown(KEY_UP)) speedMultiplier += speedStep;
        if(IsKeyDown(KEY_DOWN)) speedMultiplier -= speedStep;

        if(speedMultiplier < 0.1f) speedMultiplier = 0.1f;
        if(speedMultiplier > 10.0f) speedMultiplier = 10.0f;

        if (!paused && !sortingDone) {
            statElapsed += dt;
            stepTimer += dt;
            float animationDelay = baseDelay / speedMultiplier;
            while(stepTimer >= animationDelay && !sortingDone){
                if (currentSort == SORT_BUBBLE) {
                    bubble_sort_step();
                } else if (currentSort == SORT_INSERTION) {
                    insertion_sort_step();
                } else if (currentSort == SORT_SELECTION) {
                    selection_sort_step();
                } else if (currentSort == SORT_HEAP) {
                    heap_sort_step();
                } else {
                    quick_sort_step();
                }
                statSteps++;
                stepTimer -= animationDelay;
            }
        }

        if (sortingDone && !completionSweepActive && !paused) {
            autoNextSortTimer += dt;
            if (autoNextSortTimer >= autoNextSortDelay) {
                cycle_sort_mode();
                stepTimer = 0.0f;
            }
        }

        update_completion_sweep(dt);

        if (statusTimer > 0.0f) {
            statusTimer -= dt;
            if (statusTimer < 0.0f) {
                statusTimer = 0.0f;
            }
        }

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
