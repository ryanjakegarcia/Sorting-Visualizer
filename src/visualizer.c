#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <raylib.h>
#include "app_types.h"
#include "app_state.h"
#include "audio.h"
#include "controller.h"
#include "sorts.h"
#include "state.h"
#include "ui.h"

#define MAX_SIZE 200

static const int WIDTH = 1800;
static const int HEIGHT = 1000;
static const char* TITLE = "Sorting Visualizer";

int numbers[MAX_SIZE];
bool knownSorted[MAX_SIZE];
static RuntimeState app;

static int quickStackLow[MAX_SIZE];
static int quickStackHigh[MAX_SIZE];

static const float masterVolumeStep = 0.05f;
static const float autoNextSortDelay = 3.0f;
static const char *presetPath = "presets/visualizer_preset.cfg";

static void init_numbers_sequential(void);

static void set_status_text(const char *text)
{
    snprintf(app.statusText, sizeof(app.statusText), "%s", text);
    app.statusTimer = 2.0f;
}

static void reset_stats(void)
{
    app.statComparisons = 0;
    app.statSwaps = 0;
    app.statSteps = 0;
    app.statElapsed = 0.0f;
}

static void reset_known_sorted(void)
{
    for (int i = 0; i < MAX_SIZE; i++) {
        knownSorted[i] = false;
    }
}

static void save_preset(float speedMultiplier)
{
    PresetSettings settings = {
        .sortMode = app.currentSort,
        .speedMultiplier = speedMultiplier,
        .compareAudioEnabled = app.compareAudioEnabled,
        .swapAudioEnabled = app.swapAudioEnabled,
        .progressAudioEnabled = app.progressAudioEnabled,
        .finishAudioEnabled = app.finishAudioEnabled,
        .showValues = app.showValues,
        .showLegend = app.showLegend,
        .showHud = app.showHud,
        .masterVolume = app.masterVolume
    };

    if (!preset_save_to_file(presetPath, &settings)) {
        set_status_text("Preset save failed");
        return;
    }
    set_status_text("Preset saved");
}

static bool load_preset(float *speedMultiplier)
{
    PresetSettings settings;
    preset_apply_defaults(&settings);
    settings.sortMode = app.currentSort;
    settings.speedMultiplier = *speedMultiplier;
    settings.compareAudioEnabled = app.compareAudioEnabled;
    settings.swapAudioEnabled = app.swapAudioEnabled;
    settings.progressAudioEnabled = app.progressAudioEnabled;
    settings.finishAudioEnabled = app.finishAudioEnabled;
    settings.showValues = app.showValues;
    settings.showLegend = app.showLegend;
    settings.showHud = app.showHud;
    settings.masterVolume = app.masterVolume;

    if (!preset_load_from_file(presetPath, &settings)) {
        set_status_text("Preset file missing");
        return false;
    }

    app.currentSort = settings.sortMode;
    *speedMultiplier = settings.speedMultiplier;
    app.compareAudioEnabled = settings.compareAudioEnabled;
    app.swapAudioEnabled = settings.swapAudioEnabled;
    app.progressAudioEnabled = settings.progressAudioEnabled;
    app.finishAudioEnabled = settings.finishAudioEnabled;
    app.showValues = settings.showValues;
    app.showLegend = settings.showLegend;
    app.showHud = settings.showHud;
    app.masterVolume = settings.masterVolume;

    audio_set_master_volume(&app.audio, app.masterVolume);

    set_status_text("Preset loaded");
    return true;
}

static void shuffle_numbers(void)
{
    if (app.distributionMode == DIST_REVERSED) {
        for (int i = 0; i < app.arraySize; i++) {
            numbers[i] = app.arraySize - i;
        }
        return;
    }

    if (app.distributionMode == DIST_FEW_UNIQUE) {
        int uniqueCount = app.arraySize / 12;
        if (uniqueCount < 3) uniqueCount = 3;
        if (uniqueCount > 16) uniqueCount = 16;

        for (int i = 0; i < app.arraySize; i++) {
            int bucket = rand() % uniqueCount;
            float t = (float)bucket / (float)(uniqueCount - 1);
            int value = 1 + (int)(t * (float)(app.arraySize - 1));
            numbers[i] = value;
        }
        return;
    }

    if (app.distributionMode == DIST_SAWTOOTH) {
        int period = app.arraySize / 10;
        if (period < 4) period = 4;
        if (period > 24) period = 24;

        for (int i = 0; i < app.arraySize; i++) {
            int step = i % period;
            float t = (float)step / (float)(period - 1);
            int value = 1 + (int)(t * (float)(app.arraySize - 1));
            numbers[i] = value;
        }
        return;
    }

    init_numbers_sequential();

    for (int i = app.arraySize - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }

    if (app.distributionMode == DIST_NEARLY_SORTED) {
        int swaps = app.arraySize / 10;
        if (swaps < 1) swaps = 1;
        for (int s = 0; s < swaps; s++) {
            int i = rand() % app.arraySize;
            int delta = (rand() % 7) - 3;
            int j = i + delta;
            if (j < 0) j = 0;
            if (j >= app.arraySize) j = app.arraySize - 1;
            int tmp = numbers[i];
            numbers[i] = numbers[j];
            numbers[j] = tmp;
        }
    }
}

static void init_numbers_sequential(void)
{
    for (int i = 0; i < app.arraySize; i++) {
        numbers[i] = i + 1;
    }
}

static void reset_completion_effects(void)
{
    app.audio.completionSweepActive = false;
    app.audio.completionSweepIndex = 0;
    app.audio.completionSweepTimer = 0.0f;
    app.audio.confirmationPending = false;
}

static void reset_bubble_state(void)
{
    app.bubbleIndex = 0;
    app.bubblePass = 0;
}

static void reset_insertion_state(void)
{
    app.insertionIndex = 1;
    app.insertionPos = 0;
    app.insertionKey = 0;
    app.insertionHoldingKey = false;
}

static void reset_selection_state(void)
{
    app.selectionI = 0;
    app.selectionJ = 1;
    app.selectionMin = 0;
}

static void reset_heap_state(void)
{
    app.heapBuildIndex = (app.arraySize / 2) - 1;
    app.heapSortEnd = app.arraySize - 1;
    app.heapSiftRoot = 0;
    app.heapSiftEnd = 0;
    app.heapFocusIndex = -1;
    app.heapCandidateIndex = -1;
    app.heapBuilding = true;
    app.heapSiftActive = false;
}

static void quick_push_range(int low, int high)
{
    if (app.quickTop >= MAX_SIZE - 1) {
        return;
    }

    app.quickTop++;
    quickStackLow[app.quickTop] = low;
    quickStackHigh[app.quickTop] = high;
}

static void reset_quick_state(void)
{
    app.quickTop = -1;
    app.quickLow = 0;
    app.quickHigh = 0;
    app.quickPivotValue = 0;
    app.quickPivotIndex = -1;
    app.quickI = -1;
    app.quickJ = -1;
    app.quickPartitionActive = false;

    if (app.arraySize > 1) {
        quick_push_range(0, app.arraySize - 1);
    }
}

static void reset_sort_state(bool reshuffle)
{
    app.sortingDone = false;
    app.paused = false;
    app.stepOnceRequested = false;
    app.autoNextSortTimer = 0.0f;
    reset_stats();
    reset_known_sorted();
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
    if (app.currentSort == SORT_BUBBLE) {
        app.currentSort = SORT_INSERTION;
    } else if (app.currentSort == SORT_INSERTION) {
        app.currentSort = SORT_SELECTION;
    } else if (app.currentSort == SORT_SELECTION) {
        app.currentSort = SORT_HEAP;
    } else if (app.currentSort == SORT_HEAP) {
        app.currentSort = SORT_QUICK;
    } else {
        app.currentSort = SORT_BUBBLE;
    }

    reset_sort_state(true);
}

static const char *get_distribution_name(void)
{
    if (app.distributionMode == DIST_NEARLY_SORTED) {
        return "Nearly Sorted";
    }
    if (app.distributionMode == DIST_REVERSED) {
        return "Reversed";
    }
    if (app.distributionMode == DIST_FEW_UNIQUE) {
        return "Few Unique";
    }
    if (app.distributionMode == DIST_SAWTOOTH) {
        return "Sawtooth";
    }
    return "Random";
}

static void cycle_distribution_mode(void)
{
    if (app.distributionMode == DIST_RANDOM) {
        app.distributionMode = DIST_NEARLY_SORTED;
    } else if (app.distributionMode == DIST_NEARLY_SORTED) {
        app.distributionMode = DIST_REVERSED;
    } else if (app.distributionMode == DIST_REVERSED) {
        app.distributionMode = DIST_FEW_UNIQUE;
    } else if (app.distributionMode == DIST_FEW_UNIQUE) {
        app.distributionMode = DIST_SAWTOOTH;
    } else {
        app.distributionMode = DIST_RANDOM;
    }

    reset_sort_state(true);
    set_status_text(TextFormat("Distribution: %s", get_distribution_name()));
}

static void reset_sort_reshuffle_callback(void)
{
    reset_sort_state(true);
}

static void apply_array_size_callback(int newSize, bool usedDefault)
{
    app.arraySize = newSize;
    init_numbers_sequential();
    reset_sort_state(true);
    if (usedDefault) {
        set_status_text("Invalid size. Using default 50");
    } else {
        set_status_text(TextFormat("Array size set to %d", app.arraySize));
    }
}

static void play_swap_sound(int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    audio_play_swap(&app.audio, app.swapAudioEnabled, app.arraySize, firstValue, secondValue, leftIndex, rightIndex);
}

static void play_compare_sound(int firstValue, int secondValue, int leftIndex, int rightIndex)
{
    audio_play_compare(&app.audio, app.compareAudioEnabled, app.arraySize, firstValue, secondValue, leftIndex, rightIndex);
}

static void play_sorted_sound(void)
{
    audio_play_sorted(&app.audio, app.progressAudioEnabled);
}

static const char *get_sort_name(void)
{
    if (app.currentSort == SORT_BUBBLE) {
        return "Bubble";
    }

    if (app.currentSort == SORT_INSERTION) {
        return "Insertion";
    }

    if (app.currentSort == SORT_SELECTION) {
        return "Selection";
    }

    if (app.currentSort == SORT_HEAP) {
        return "Heap";
    }

    return "Quick";
}

static void start_completion_sweep(void)
{
    audio_start_completion_sweep(&app.audio, app.finishAudioEnabled, app.arraySize);
}

static void update_completion_sweep(float dt)
{
    audio_update_completion_sweep(&app.audio, dt, app.finishAudioEnabled, app.arraySize);
}

static void run_current_sort_step(void)
{
    if (app.currentSort == SORT_BUBBLE) {
        bubble_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.bubbleIndex, &app.bubblePass, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
    } else if (app.currentSort == SORT_INSERTION) {
        insertion_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.insertionIndex, &app.insertionPos, &app.insertionKey, &app.insertionHoldingKey, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
    } else if (app.currentSort == SORT_SELECTION) {
        selection_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.selectionI, &app.selectionJ, &app.selectionMin, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
    } else if (app.currentSort == SORT_HEAP) {
        heap_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.heapBuildIndex, &app.heapSortEnd, &app.heapSiftRoot, &app.heapSiftEnd, &app.heapFocusIndex, &app.heapCandidateIndex, &app.heapBuilding, &app.heapSiftActive, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
    } else {
        quick_sort_step(numbers, knownSorted, quickStackLow, quickStackHigh, MAX_SIZE, app.arraySize, &app.sortingDone, &app.quickTop, &app.quickLow, &app.quickHigh, &app.quickPivotValue, &app.quickPivotIndex, &app.quickI, &app.quickJ, &app.quickPartitionActive, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
    }
}

int main(){
    srand((unsigned int)time(NULL));
    app_state_init_defaults(&app);

    init_numbers_sequential();

    shuffle_numbers();

    InitWindow(WIDTH, HEIGHT, TITLE);
    audio_init(&app.audio, app.masterVolume);

    SetTargetFPS(60);
    float speedMultiplier = 1.0f;
    float speedStep = 0.1f;
    float baseDelay = 0.05f;
    float stepTimer = 0.0f;

    reset_sort_state(false);
    
    while(!WindowShouldClose()){
        float dt = GetFrameTime();
        audio_begin_frame(&app.audio);

        ControllerContext controller = {
            .windowHeight = HEIGHT,
            .maxSize = MAX_SIZE,
            .arraySize = &app.arraySize,
            .sizeInputActive = &app.sizeInputActive,
            .sizeInput = app.sizeInput,
            .sizeInputCapacity = (int)sizeof(app.sizeInput),
            .sizeInputLen = &app.sizeInputLen,
            .paused = &app.paused,
            .stepMode = &app.stepMode,
            .stepOnceRequested = &app.stepOnceRequested,
            .showValues = &app.showValues,
            .showLegend = &app.showLegend,
            .showHud = &app.showHud,
            .compareAudioEnabled = &app.compareAudioEnabled,
            .swapAudioEnabled = &app.swapAudioEnabled,
            .progressAudioEnabled = &app.progressAudioEnabled,
            .finishAudioEnabled = &app.finishAudioEnabled,
            .masterVolume = &app.masterVolume,
            .masterVolumeStep = masterVolumeStep,
            .speedMultiplier = &speedMultiplier,
            .speedStep = speedStep,
            .stepTimer = &stepTimer,
            .cycleSort = cycle_sort_mode,
            .cycleDistribution = cycle_distribution_mode,
            .resetSort = reset_sort_reshuffle_callback,
            .savePreset = save_preset,
            .loadPreset = load_preset,
            .applyArraySize = apply_array_size_callback
        };
        controller_handle_input(&controller, dt);

        audio_set_master_volume(&app.audio, app.masterVolume);

        if (!app.paused && !app.sortingDone && !app.stepMode) {
            app.statElapsed += dt;
            stepTimer += dt;
            float animationDelay = baseDelay / speedMultiplier;
            while(stepTimer >= animationDelay && !app.sortingDone){
                run_current_sort_step();
                app.statSteps++;
                stepTimer -= animationDelay;
            }
        }

        if (app.stepOnceRequested && !app.sortingDone) {
            run_current_sort_step();
            app.statSteps++;
            app.stepOnceRequested = false;
        }

        if (app.sortingDone && !audio_completion_sweep_active(&app.audio) && !app.paused && !app.stepMode) {
            app.autoNextSortTimer += dt;
            if (app.autoNextSortTimer >= autoNextSortDelay) {
                cycle_sort_mode();
                stepTimer = 0.0f;
            }
        }

        update_completion_sweep(dt);

        if (app.statusTimer > 0.0f) {
            app.statusTimer -= dt;
            if (app.statusTimer < 0.0f) {
                app.statusTimer = 0.0f;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        UiDrawContext ui = {
            .width = WIDTH,
            .height = HEIGHT,
            .maxSize = MAX_SIZE,
            .arraySize = app.arraySize,
            .numbers = numbers,
            .knownSorted = knownSorted,
            .currentSort = app.currentSort,
            .sortingDone = app.sortingDone,
            .completionSweepActive = audio_completion_sweep_active(&app.audio),
            .completionSweepIndex = audio_completion_sweep_index(&app.audio),
            .quickPivotIndex = app.quickPivotIndex,
            .quickJ = app.quickJ,
            .quickI = app.quickI,
            .heapCandidateIndex = app.heapCandidateIndex,
            .heapFocusIndex = app.heapFocusIndex,
            .bubbleIndex = app.bubbleIndex,
            .insertionPos = app.insertionPos,
            .selectionMin = app.selectionMin,
            .selectionJ = app.selectionJ,
            .bubblePass = app.bubblePass,
            .insertionIndex = app.insertionIndex,
            .selectionI = app.selectionI,
            .heapSortEnd = app.heapSortEnd,
            .heapBuilding = app.heapBuilding,
            .insertionHoldingKey = app.insertionHoldingKey,
            .insertionKey = app.insertionKey,
            .showValues = app.showValues,
            .showHud = app.showHud,
            .sizeInputActive = app.sizeInputActive,
            .sizeInput = app.sizeInput,
            .sortName = get_sort_name(),
            .distributionName = get_distribution_name(),
            .speedMultiplier = speedMultiplier,
            .masterVolume = app.masterVolume,
            .compareAudioEnabled = app.compareAudioEnabled,
            .swapAudioEnabled = app.swapAudioEnabled,
            .progressAudioEnabled = app.progressAudioEnabled,
            .finishAudioEnabled = app.finishAudioEnabled,
            .statComparisons = app.statComparisons,
            .statSwaps = app.statSwaps,
            .statSteps = app.statSteps,
            .statElapsed = app.statElapsed,
            .statusTimer = app.statusTimer,
            .statusText = app.statusText,
            .showLegend = app.showLegend,
            .paused = app.paused,
            .stepMode = app.stepMode,
            .showInfo = IsKeyDown(KEY_I),
            .autoNextSortDelay = autoNextSortDelay,
            .autoNextSortTimer = app.autoNextSortTimer
        };
        draw_elements(&ui);

        EndDrawing();
    }

    CloseWindow();

    audio_shutdown(&app.audio);

    return 0;
}
