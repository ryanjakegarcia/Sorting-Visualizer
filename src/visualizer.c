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

#define MAX_SIZE 1000

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

typedef struct SortDescriptor {
    SortMode mode;
    const char *name;
    void (*runStep)(void);
    void (*resetState)(void);
    void (*fillTelemetry)(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
} SortDescriptor;

static void run_bubble_step(void);
static void run_insertion_step(void);
static void run_selection_step(void);
static void run_heap_step(void);
static void run_quick_step(void);
static void run_shell_step(void);
static void reset_bubble_state(void);
static void reset_insertion_state(void);
static void reset_selection_state(void);
static void reset_heap_state(void);
static void reset_quick_state(void);
static void reset_shell_state(void);
static void fill_bubble_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_insertion_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_selection_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_heap_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_quick_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_shell_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_current_sort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);

static const SortDescriptor sortRegistry[] = {
    { SORT_BUBBLE, "Bubble", run_bubble_step, reset_bubble_state, fill_bubble_telemetry },
    { SORT_INSERTION, "Insertion", run_insertion_step, reset_insertion_state, fill_insertion_telemetry },
    { SORT_SHELL, "Shell", run_shell_step, reset_shell_state, fill_shell_telemetry },
    { SORT_SELECTION, "Selection", run_selection_step, reset_selection_state, fill_selection_telemetry },
    { SORT_HEAP, "Heap", run_heap_step, reset_heap_state, fill_heap_telemetry },
    { SORT_QUICK, "Quick", run_quick_step, reset_quick_state, fill_quick_telemetry }
};

#define SORT_REGISTRY_COUNT ((int)(sizeof(sortRegistry) / sizeof(sortRegistry[0])))
#define MAX_BENCHMARK_RESULTS 32

typedef struct BenchmarkResult {
    SortMode mode;
    unsigned long long comparisons;
    unsigned long long swaps;
    unsigned long long steps;
    float elapsed;
} BenchmarkResult;

typedef struct BenchmarkState {
    bool active;
    bool running;
    int currentSortIndex;
    int resultCount;
    BenchmarkResult results[MAX_BENCHMARK_RESULTS];
    int baselineNumbers[MAX_SIZE];
    SortMode previousSort;
    bool previousPaused;
    bool previousStepMode;
} BenchmarkState;

static BenchmarkState benchmark = { 0 };

static void init_numbers_sequential(void);
static const char *get_sort_name_by_mode(SortMode mode);
static int get_sort_index(SortMode mode);
static void benchmark_begin_current_sort(void);
static void benchmark_start(void);
static void benchmark_cancel(void);
static void benchmark_update(void);
static void draw_benchmark_overlay(void);

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
        .showSettingsOverlay = app.showSettingsOverlay,
        .showTelemetry = app.showTelemetry,
        .showSizeInputBox = app.showSizeInputBox,
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
    settings.showSettingsOverlay = app.showSettingsOverlay;
    settings.showTelemetry = app.showTelemetry;
    settings.showSizeInputBox = app.showSizeInputBox;
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
    app.showSettingsOverlay = settings.showSettingsOverlay;
    app.showTelemetry = settings.showTelemetry;
    app.showSizeInputBox = settings.showSizeInputBox;
    if (!app.showSizeInputBox) {
        app.sizeInputActive = false;
    }
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

static void reset_shell_state(void)
{
    app.shellGap = app.arraySize / 2;
    app.shellI = app.shellGap;
    app.shellJ = app.shellGap;
    app.shellTemp = 0;
    app.shellHolding = false;
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
    for (int i = 0; i < SORT_REGISTRY_COUNT; i++) {
        if (sortRegistry[i].resetState != NULL) {
            sortRegistry[i].resetState();
        }
    }

    if (reshuffle) {
        shuffle_numbers();
    }
}

static void cycle_sort_mode(void)
{
    int index = get_sort_index(app.currentSort);
    if (index < 0) {
        index = 0;
    }

    index = (index + 1) % SORT_REGISTRY_COUNT;
    app.currentSort = sortRegistry[index].mode;

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

static int get_sort_index(SortMode mode)
{
    for (int i = 0; i < SORT_REGISTRY_COUNT; i++) {
        if (sortRegistry[i].mode == mode) {
            return i;
        }
    }

    return -1;
}

static const char *get_sort_name_by_mode(SortMode mode)
{
    int index = get_sort_index(mode);
    if (index >= 0) {
        return sortRegistry[index].name;
    }

    return "Unknown";
}

static const char *get_sort_name(void)
{
    return get_sort_name_by_mode(app.currentSort);
}

static void benchmark_begin_current_sort(void)
{
    if (benchmark.currentSortIndex < 0 || benchmark.currentSortIndex >= SORT_REGISTRY_COUNT) {
        benchmark.running = false;
        benchmark.active = false;
        set_status_text("Benchmark failed: invalid sort index");
        return;
    }

    app.currentSort = sortRegistry[benchmark.currentSortIndex].mode;
    memcpy(numbers, benchmark.baselineNumbers, (size_t)app.arraySize * sizeof(numbers[0]));
    reset_sort_state(false);
    app.stepMode = false;
    app.paused = false;
    app.pauseMenuActive = false;
}

static void benchmark_start(void)
{
    if (SORT_REGISTRY_COUNT <= 0) {
        set_status_text("Benchmark unavailable: no sorts registered");
        return;
    }

    if (SORT_REGISTRY_COUNT > MAX_BENCHMARK_RESULTS) {
        set_status_text("Benchmark unavailable: result buffer too small");
        return;
    }

    benchmark.previousSort = app.currentSort;
    benchmark.previousPaused = app.paused;
    benchmark.previousStepMode = app.stepMode;
    benchmark.resultCount = 0;
    benchmark.currentSortIndex = 0;
    benchmark.active = true;
    benchmark.running = true;

    shuffle_numbers();
    memcpy(benchmark.baselineNumbers, numbers, (size_t)app.arraySize * sizeof(numbers[0]));

    benchmark_begin_current_sort();
    set_status_text("Benchmark started");
}

static void benchmark_cancel(void)
{
    benchmark.running = false;
    benchmark.active = false;
    app.currentSort = benchmark.previousSort;
    reset_sort_state(true);
    app.stepMode = benchmark.previousStepMode;
    app.paused = benchmark.previousPaused;
    app.pauseMenuActive = false;
    set_status_text("Benchmark cancelled");
}

static void benchmark_update(void)
{
    if (!benchmark.running) {
        return;
    }

    if (!app.sortingDone) {
        return;
    }

    if (benchmark.resultCount < MAX_BENCHMARK_RESULTS) {
        BenchmarkResult *result = &benchmark.results[benchmark.resultCount++];
        result->mode = app.currentSort;
        result->comparisons = app.statComparisons;
        result->swaps = app.statSwaps;
        result->steps = app.statSteps;
        result->elapsed = app.statElapsed;
    }

    benchmark.currentSortIndex++;
    if (benchmark.currentSortIndex < SORT_REGISTRY_COUNT) {
        benchmark_begin_current_sort();
        return;
    }

    benchmark.running = false;
    benchmark.active = true;
    app.currentSort = benchmark.previousSort;
    memcpy(numbers, benchmark.baselineNumbers, (size_t)app.arraySize * sizeof(numbers[0]));
    reset_sort_state(false);
    app.stepMode = benchmark.previousStepMode;
    app.paused = true;
    app.pauseMenuActive = false;
    set_status_text("Benchmark complete (X to hide)");
}

static void draw_benchmark_overlay(void)
{
    if (!benchmark.active) {
        return;
    }

    int panelX = WIDTH - 700;
    int panelY = 220;
    int panelW = 660;
    int panelH = HEIGHT - panelY - 30;
    if (panelX < 20) panelX = 20;
    if (panelW < 360) panelW = 360;
    if (panelH < 240) panelH = 240;

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.75f));
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, SKYBLUE);

    DrawText("Benchmark Suite [X]", panelX + 16, panelY + 14, 26, YELLOW);
    DrawText(TextFormat("N=%d  Dist=%s  Sorts=%d", app.arraySize, get_distribution_name(), SORT_REGISTRY_COUNT), panelX + 16, panelY + 44, 20, LIGHTGRAY);

    if (benchmark.running) {
        const char *runningName = get_sort_name_by_mode(sortRegistry[benchmark.currentSortIndex].mode);
        DrawText(TextFormat("Running: %s (%d/%d)", runningName, benchmark.currentSortIndex + 1, SORT_REGISTRY_COUNT), panelX + 16, panelY + 70, 20, SKYBLUE);
    } else {
        DrawText("Status: Complete", panelX + 16, panelY + 70, 20, SKYBLUE);
    }

    int tableY = panelY + 102;
    DrawText("Sort", panelX + 16, tableY, 20, YELLOW);
    DrawText("Time(s)", panelX + 170, tableY, 20, YELLOW);
    DrawText("Cmp", panelX + 280, tableY, 20, YELLOW);
    DrawText("Swp", panelX + 400, tableY, 20, YELLOW);
    DrawText("Steps", panelX + 510, tableY, 20, YELLOW);

    int rowY = tableY + 28;
    int maxRows = (panelY + panelH - rowY - 26) / 24;
    if (maxRows < 1) maxRows = 1;

    int rowsToDraw = benchmark.resultCount;
    if (rowsToDraw > maxRows) {
        rowsToDraw = maxRows;
    }

    for (int i = 0; i < rowsToDraw; i++) {
        const BenchmarkResult *result = &benchmark.results[i];
        DrawText(get_sort_name_by_mode(result->mode), panelX + 16, rowY + i * 24, 20, RAYWHITE);
        DrawText(TextFormat("%.3f", result->elapsed), panelX + 170, rowY + i * 24, 20, RAYWHITE);
        DrawText(TextFormat("%llu", result->comparisons), panelX + 280, rowY + i * 24, 20, RAYWHITE);
        DrawText(TextFormat("%llu", result->swaps), panelX + 400, rowY + i * 24, 20, RAYWHITE);
        DrawText(TextFormat("%llu", result->steps), panelX + 510, rowY + i * 24, 20, RAYWHITE);
    }

    if (benchmark.resultCount > rowsToDraw) {
        int hidden = benchmark.resultCount - rowsToDraw;
        DrawText(TextFormat("... and %d more", hidden), panelX + 16, rowY + rowsToDraw * 24, 18, LIGHTGRAY);
    }
}

static void start_completion_sweep(void)
{
    audio_start_completion_sweep(&app.audio, app.finishAudioEnabled, app.arraySize);
}

static void update_completion_sweep(float dt)
{
    audio_update_completion_sweep(&app.audio, dt, app.finishAudioEnabled, app.arraySize);
}

static void run_bubble_step(void)
{
    bubble_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.bubbleIndex, &app.bubblePass, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_insertion_step(void)
{
    insertion_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.insertionIndex, &app.insertionPos, &app.insertionKey, &app.insertionHoldingKey, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_selection_step(void)
{
    selection_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.selectionI, &app.selectionJ, &app.selectionMin, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_heap_step(void)
{
    heap_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.heapBuildIndex, &app.heapSortEnd, &app.heapSiftRoot, &app.heapSiftEnd, &app.heapFocusIndex, &app.heapCandidateIndex, &app.heapBuilding, &app.heapSiftActive, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_quick_step(void)
{
    quick_sort_step(numbers, knownSorted, quickStackLow, quickStackHigh, MAX_SIZE, app.arraySize, &app.sortingDone, &app.quickTop, &app.quickLow, &app.quickHigh, &app.quickPivotValue, &app.quickPivotIndex, &app.quickI, &app.quickJ, &app.quickPartitionActive, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_shell_step(void)
{
    shell_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.shellGap, &app.shellI, &app.shellJ, &app.shellTemp, &app.shellHolding, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void fill_bubble_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Bubble idx: %d  pass: %d", app.bubbleIndex, app.bubblePass);
    snprintf(line2, line2Size, "Compare pair: [%d, %d]", app.bubbleIndex, app.bubbleIndex + 1);
    if (line3Size > 0) line3[0] = '\0';
}

static void fill_insertion_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Insertion idx: %d  pos: %d", app.insertionIndex, app.insertionPos);
    snprintf(line2, line2Size, "Holding key: %s  value: %d", app.insertionHoldingKey ? "yes" : "no", app.insertionKey);
    if (line3Size > 0) line3[0] = '\0';
}

static void fill_selection_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Selection i: %d  j: %d", app.selectionI, app.selectionJ);
    snprintf(line2, line2Size, "Current min idx: %d", app.selectionMin);
    if (line3Size > 0) line3[0] = '\0';
}

static void fill_heap_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Heap phase: %s", app.heapBuilding ? "BUILD" : "EXTRACT");
    snprintf(line2, line2Size, "Sift root/end: %d / %d", app.heapSiftRoot, app.heapSiftEnd);
    snprintf(line3, line3Size, "Sift active: %s  sorted end: %d", app.heapSiftActive ? "yes" : "no", app.heapSortEnd);
}

static void fill_quick_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Quick range: [%d, %d]", app.quickLow, app.quickHigh);
    snprintf(line2, line2Size, "Pivot idx/value: %d / %d", app.quickPivotIndex, app.quickPivotValue);
    snprintf(line3, line3Size, "i: %d  j: %d  stack: %d", app.quickI, app.quickJ, app.quickTop + 1);
}

static void fill_shell_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Gap: %d  i: %d  j: %d", app.shellGap, app.shellI, app.shellJ);
    snprintf(line2, line2Size, "Holding temp: %s  value: %d", app.shellHolding ? "yes" : "no", app.shellTemp);
    snprintf(line3, line3Size, "Compare idx: %d", app.shellJ - app.shellGap);
}

static void fill_current_sort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    int index = get_sort_index(app.currentSort);
    if (line1Size > 0) line1[0] = '\0';
    if (line2Size > 0) line2[0] = '\0';
    if (line3Size > 0) line3[0] = '\0';

    if (index < 0 || sortRegistry[index].fillTelemetry == NULL) {
        snprintf(line1, line1Size, "No telemetry for selected sort");
        return;
    }

    sortRegistry[index].fillTelemetry(line1, line1Size, line2, line2Size, line3, line3Size);
}

static void run_current_sort_step(void)
{
    int index = get_sort_index(app.currentSort);
    if (index < 0 || sortRegistry[index].runStep == NULL) {
        app.sortingDone = true;
        set_status_text("Unknown sort selected");
        return;
    }

    sortRegistry[index].runStep();
}

int main(){
    srand((unsigned int)time(NULL));
    app_state_init_defaults(&app);

    init_numbers_sequential();

    shuffle_numbers();

    InitWindow(WIDTH, HEIGHT, TITLE);
    SetExitKey(KEY_ESCAPE);
    audio_init(&app.audio, app.masterVolume);

    SetTargetFPS(60);
    float speedMultiplier = 1.0f;
    float speedStep = 0.1f;
    float baseDelay = 0.05f;
    float stepTimer = 0.0f;

    reset_sort_state(false);
    
    while(!WindowShouldClose() && !app.requestClose){
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
            .pausedBeforeMenu = &app.pausedBeforeMenu,
            .pauseMenuActive = &app.pauseMenuActive,
            .pauseMenuSelection = &app.pauseMenuSelection,
            .requestClose = &app.requestClose,
            .stepMode = &app.stepMode,
            .stepOnceRequested = &app.stepOnceRequested,
            .showValues = &app.showValues,
            .showLegend = &app.showLegend,
            .showHud = &app.showHud,
            .showSettingsOverlay = &app.showSettingsOverlay,
            .showTelemetry = &app.showTelemetry,
            .showSizeInputBox = &app.showSizeInputBox,
            .minimalUiMode = &app.minimalUiMode,
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

        if (!app.sizeInputActive && !app.pauseMenuActive && IsKeyPressed(KEY_X)) {
            if (benchmark.running) {
                benchmark_cancel();
            } else if (!benchmark.active) {
                benchmark_start();
                stepTimer = 0.0f;
            } else {
                benchmark.active = false;
                app.paused = benchmark.previousPaused;
                set_status_text("Benchmark overlay hidden");
            }
        }

        if (benchmark.running) {
            app.paused = false;
            app.pauseMenuActive = false;
            app.sizeInputActive = false;
            app.stepMode = false;
            app.stepOnceRequested = false;
        }

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

        if (benchmark.running) {
            benchmark_update();
        }

        if (!benchmark.running && app.sortingDone && !audio_completion_sweep_active(&app.audio) && !app.paused && !app.stepMode) {
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

        char telemetryLine1[96] = { 0 };
        char telemetryLine2[96] = { 0 };
        char telemetryLine3[96] = { 0 };
        fill_current_sort_telemetry(telemetryLine1, sizeof(telemetryLine1), telemetryLine2, sizeof(telemetryLine2), telemetryLine3, sizeof(telemetryLine3));

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
            .quickLow = app.quickLow,
            .quickHigh = app.quickHigh,
            .quickTop = app.quickTop,
            .quickPivotValue = app.quickPivotValue,
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
            .heapSiftRoot = app.heapSiftRoot,
            .heapSiftEnd = app.heapSiftEnd,
            .heapSiftActive = app.heapSiftActive,
            .insertionHoldingKey = app.insertionHoldingKey,
            .insertionKey = app.insertionKey,
            .shellGap = app.shellGap,
            .shellI = app.shellI,
            .shellJ = app.shellJ,
            .shellHolding = app.shellHolding,
            .showValues = app.showValues,
            .showHud = app.showHud,
            .showSettingsOverlay = app.showSettingsOverlay,
            .showTelemetry = app.showTelemetry,
            .telemetryLine1 = telemetryLine1,
            .telemetryLine2 = telemetryLine2,
            .telemetryLine3 = telemetryLine3,
            .showSizeInputBox = app.showSizeInputBox,
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
            .pauseMenuActive = app.pauseMenuActive,
            .pauseMenuSelection = app.pauseMenuSelection,
            .stepMode = app.stepMode,
            .minimalUiMode = app.minimalUiMode,
            .showInfo = IsKeyDown(KEY_I),
            .autoNextSortDelay = autoNextSortDelay,
            .autoNextSortTimer = app.autoNextSortTimer
        };
        draw_elements(&ui);
        draw_benchmark_overlay();

        EndDrawing();
    }

    CloseWindow();

    audio_shutdown(&app.audio);

    return 0;
}
