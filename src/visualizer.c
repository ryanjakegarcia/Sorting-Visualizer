#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
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
static int mergeBuffer[MAX_SIZE];
static int timBuffer[MAX_SIZE];
static int radixBuffer[MAX_SIZE];
static int radixCount[2];

static const float masterVolumeStep = 0.05f;
static const float autoNextSortDelay = 3.0f;
static const char *presetPath = "presets/visualizer_preset.cfg";
static const char *benchmarkCsvPath = "benchmarks/benchmark_results.csv";

typedef struct SortDescriptor {
    SortMode mode;
    const char *name;
    void (*runStep)(void);
    void (*resetState)(void);
    void (*fillTelemetry)(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
    int colorAId;
    int colorBId;
    int colorCId;
    int colorDId;
} SortDescriptor;

enum {
    COLOR_ID_RED = 0,
    COLOR_ID_ORANGE,
    COLOR_ID_MAROON,
    COLOR_ID_SKYBLUE,
    COLOR_ID_VIOLET,
    COLOR_ID_PINK,
    COLOR_ID_GOLD,
    COLOR_ID_BLUE,
    COLOR_ID_PURPLE,
    COLOR_ID_MAGENTA
};

static void run_bubble_step(void);
static void run_insertion_step(void);
static void run_selection_step(void);
static void run_heap_step(void);
static void run_quick_step(void);
static void run_shell_step(void);
static void run_merge_step(void);
static void run_cocktail_step(void);
static void run_gnome_step(void);
static void run_comb_step(void);
static void run_timsort_step(void);
static void run_radixsort_step(void);
static void run_bogosort_step(void);
static void reset_bubble_state(void);
static void reset_insertion_state(void);
static void reset_selection_state(void);
static void reset_heap_state(void);
static void reset_quick_state(void);
static void reset_shell_state(void);
static void reset_merge_state(void);
static void reset_cocktail_state(void);
static void reset_gnome_state(void);
static void reset_comb_state(void);
static void reset_timsort_state(void);
static void reset_radixsort_state(void);
static void reset_bogosort_state(void);
static void fill_bubble_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_insertion_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_selection_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_heap_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_quick_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_shell_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_merge_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_cocktail_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_gnome_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_comb_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_timsort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_radixsort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_bogosort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static void fill_current_sort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size);
static Color color_from_id(int colorId);

static const SortDescriptor sortRegistry[] = {
    { SORT_BUBBLE, "Bubble", run_bubble_step, reset_bubble_state, fill_bubble_telemetry, COLOR_ID_RED, COLOR_ID_ORANGE, COLOR_ID_MAROON, COLOR_ID_SKYBLUE },
    { SORT_INSERTION, "Insertion", run_insertion_step, reset_insertion_state, fill_insertion_telemetry, COLOR_ID_VIOLET, COLOR_ID_ORANGE, COLOR_ID_PINK, COLOR_ID_SKYBLUE },
    { SORT_SHELL, "Shell", run_shell_step, reset_shell_state, fill_shell_telemetry, COLOR_ID_MAROON, COLOR_ID_GOLD, COLOR_ID_ORANGE, COLOR_ID_SKYBLUE },
    { SORT_MERGE, "Merge", run_merge_step, reset_merge_state, fill_merge_telemetry, COLOR_ID_BLUE, COLOR_ID_ORANGE, COLOR_ID_PURPLE, COLOR_ID_SKYBLUE },
    { SORT_SELECTION, "Selection", run_selection_step, reset_selection_state, fill_selection_telemetry, COLOR_ID_RED, COLOR_ID_MAGENTA, COLOR_ID_MAROON, COLOR_ID_SKYBLUE },
    { SORT_HEAP, "Heap", run_heap_step, reset_heap_state, fill_heap_telemetry, COLOR_ID_RED, COLOR_ID_ORANGE, COLOR_ID_MAROON, COLOR_ID_SKYBLUE },
    { SORT_QUICK, "Quick", run_quick_step, reset_quick_state, fill_quick_telemetry, COLOR_ID_RED, COLOR_ID_PINK, COLOR_ID_ORANGE, COLOR_ID_SKYBLUE },
    { SORT_COCKTAIL, "Cocktail", run_cocktail_step, reset_cocktail_state, fill_cocktail_telemetry, COLOR_ID_ORANGE, COLOR_ID_RED, COLOR_ID_SKYBLUE, COLOR_ID_MAROON },
    { SORT_GNOME, "Gnome", run_gnome_step, reset_gnome_state, fill_gnome_telemetry, COLOR_ID_VIOLET, COLOR_ID_PINK, COLOR_ID_ORANGE, COLOR_ID_SKYBLUE },
    { SORT_COMB, "Comb", run_comb_step, reset_comb_state, fill_comb_telemetry, COLOR_ID_BLUE, COLOR_ID_GOLD, COLOR_ID_PURPLE, COLOR_ID_SKYBLUE },
    { SORT_TIMSORT, "Timsort", run_timsort_step, reset_timsort_state, fill_timsort_telemetry, COLOR_ID_MAGENTA, COLOR_ID_PINK, COLOR_ID_RED, COLOR_ID_SKYBLUE },
    { SORT_RADIXSORT, "Radixsort", run_radixsort_step, reset_radixsort_state, fill_radixsort_telemetry, COLOR_ID_GOLD, COLOR_ID_MAROON, COLOR_ID_ORANGE, COLOR_ID_SKYBLUE },
    { SORT_BOGOSORT, "Bogosort", run_bogosort_step, reset_bogosort_state, fill_bogosort_telemetry, COLOR_ID_PINK, COLOR_ID_MAGENTA, COLOR_ID_PURPLE, COLOR_ID_SKYBLUE }
};

#define SORT_REGISTRY_COUNT ((int)(sizeof(sortRegistry) / sizeof(sortRegistry[0])))
#define MAX_BENCHMARK_RESULTS 32

typedef struct BenchmarkResult {
    SortMode mode;
    unsigned long long totalComparisons;
    unsigned long long totalSwaps;
    unsigned long long totalSteps;
    float totalElapsed;
    float minElapsed;
    float maxElapsed;
    int runs;
} BenchmarkResult;

typedef struct BenchmarkState {
    bool active;
    bool running;
    bool inWarmup;
    int currentSortIndex;
    int currentRunIndex;
    int selectedConfigSortIndex;
    int resultCount;
    int sequenceCount;
    int sequence[MAX_BENCHMARK_RESULTS];
    bool sortEnabled[MAX_BENCHMARK_RESULTS];
    BenchmarkResult results[MAX_BENCHMARK_RESULTS];
    int baselineNumbers[MAX_SIZE];
    SortMode previousSort;
    bool previousPaused;
    bool previousStepMode;
    bool configUseFixedSeed;
    unsigned int configFixedSeed;
    int configRunsPerSort;
    bool configWarmup;
} BenchmarkState;

static BenchmarkState benchmark = { 0 };

static void init_numbers_sequential(void);
static const char *get_sort_name_by_mode(SortMode mode);
static int get_sort_index(SortMode mode);
static void benchmark_begin_current_sort(void);
static void benchmark_start(void);
static void benchmark_cancel(void);
static void benchmark_update(void);
static void benchmark_prepare_run_baseline(void);
static int benchmark_build_sequence(void);
static bool benchmark_export_results_csv(void);
static void benchmark_build_display_order(int *order, int count);
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

static void reset_merge_state(void)
{
    app.mergeWidth = 1;
    app.mergeLeft = 0;
    app.mergeMid = 0;
    app.mergeRight = 0;
    app.mergeI = 0;
    app.mergeJ = 0;
    app.mergeK = 0;
    app.mergeCopyIndex = 0;
    app.mergeActive = false;
    app.mergeCopying = false;
}

static void reset_cocktail_state(void)
{
    app.cocktailStart = 0;
    app.cocktailEnd = app.arraySize - 1;
    app.cocktailIndex = 0;
    app.cocktailForward = true;
    app.cocktailSwapped = false;
}

static void reset_gnome_state(void)
{
    app.gnomeIndex = 1;
}

static void reset_comb_state(void)
{
    app.combGap = app.arraySize;
    app.combIndex = 0;
    app.combSwapped = false;
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

static Color color_from_id(int colorId)
{
    switch (colorId) {
        case COLOR_ID_RED: return RED;
        case COLOR_ID_ORANGE: return ORANGE;
        case COLOR_ID_MAROON: return MAROON;
        case COLOR_ID_SKYBLUE: return SKYBLUE;
        case COLOR_ID_VIOLET: return VIOLET;
        case COLOR_ID_PINK: return PINK;
        case COLOR_ID_GOLD: return GOLD;
        case COLOR_ID_BLUE: return BLUE;
        case COLOR_ID_PURPLE: return PURPLE;
        case COLOR_ID_MAGENTA: return MAGENTA;
        default: return RED;
    }
}

static void benchmark_begin_current_sort(void)
{
    if (benchmark.currentSortIndex < 0 || benchmark.currentSortIndex >= benchmark.sequenceCount) {
        benchmark.running = false;
        benchmark.active = false;
        set_status_text("Benchmark failed: invalid sort index");
        return;
    }

    app.currentSort = sortRegistry[benchmark.sequence[benchmark.currentSortIndex]].mode;
    memcpy(numbers, benchmark.baselineNumbers, (size_t)app.arraySize * sizeof(numbers[0]));
    reset_sort_state(false);
    app.stepMode = false;
    app.paused = false;
    app.pauseMenuActive = false;
}

static int benchmark_build_sequence(void)
{
    int count = 0;
    for (int i = 0; i < SORT_REGISTRY_COUNT; i++) {
        if (i < MAX_BENCHMARK_RESULTS && benchmark.sortEnabled[i]) {
            benchmark.sequence[count++] = i;
        }
    }
    benchmark.sequenceCount = count;
    return count;
}

static int benchmark_compare_display_order(const void *leftPtr, const void *rightPtr)
{
    int leftIndex = *(const int *)leftPtr;
    int rightIndex = *(const int *)rightPtr;

    const BenchmarkResult *left = &benchmark.results[leftIndex];
    const BenchmarkResult *right = &benchmark.results[rightIndex];
    bool leftHasRuns = left->runs > 0;
    bool rightHasRuns = right->runs > 0;

    if (leftHasRuns != rightHasRuns) {
        return leftHasRuns ? -1 : 1;
    }

    if (leftHasRuns && rightHasRuns) {
        float leftAvg = left->totalElapsed / (float)left->runs;
        float rightAvg = right->totalElapsed / (float)right->runs;

        if (leftAvg < rightAvg) return -1;
        if (leftAvg > rightAvg) return 1;
    }

    if (leftIndex < rightIndex) return -1;
    if (leftIndex > rightIndex) return 1;
    return 0;
}

static void benchmark_build_display_order(int *order, int count)
{
    if (count < 1) {
        return;
    }

    for (int i = 0; i < count; i++) {
        order[i] = i;
    }

    qsort(order, (size_t)count, sizeof(int), benchmark_compare_display_order);
}

static bool benchmark_export_results_csv(void)
{
    FILE *file = fopen(benchmarkCsvPath, "w");
    if (file == NULL) {
        set_status_text("Benchmark CSV export failed");
        return false;
    }

    int order[MAX_BENCHMARK_RESULTS] = { 0 };
    benchmark_build_display_order(order, benchmark.resultCount);

    fprintf(file, "rank,sort,enabled,runs,total_elapsed,avg_elapsed,min_elapsed,max_elapsed,total_comparisons,avg_comparisons,total_swaps,avg_swaps,total_steps,avg_steps\n");
    for (int rank = 0; rank < benchmark.resultCount; rank++) {
        int resultIndex = order[rank];
        const BenchmarkResult *result = &benchmark.results[resultIndex];
        bool enabled = benchmark.sortEnabled[resultIndex];
        float avgElapsed = (result->runs > 0) ? (result->totalElapsed / (float)result->runs) : 0.0f;
        float minElapsed = (result->runs > 0) ? result->minElapsed : 0.0f;
        float maxElapsed = (result->runs > 0) ? result->maxElapsed : 0.0f;
        unsigned long long avgComparisons = (result->runs > 0) ? (result->totalComparisons / (unsigned long long)result->runs) : 0ULL;
        unsigned long long avgSwaps = (result->runs > 0) ? (result->totalSwaps / (unsigned long long)result->runs) : 0ULL;
        unsigned long long avgSteps = (result->runs > 0) ? (result->totalSteps / (unsigned long long)result->runs) : 0ULL;
        fprintf(file, "%d,%s,%d,%d,%.6f,%.6f,%.6f,%.6f,%llu,%llu,%llu,%llu,%llu,%llu\n",
            rank + 1,
            sortRegistry[resultIndex].name,
            enabled ? 1 : 0,
            result->runs,
            result->totalElapsed,
            avgElapsed,
            minElapsed,
            maxElapsed,
            result->totalComparisons,
            avgComparisons,
            result->totalSwaps,
            avgSwaps,
            result->totalSteps,
            avgSteps);
    }

    fclose(file);
    set_status_text(TextFormat("Benchmark CSV saved to %s", benchmarkCsvPath));
    return true;
}

static void benchmark_prepare_run_baseline(void)
{
    if (benchmark.configUseFixedSeed) {
        unsigned int seedBase = benchmark.configFixedSeed;
        unsigned int runOffset = (unsigned int)(benchmark.currentRunIndex + 1) * 7919u;
        unsigned int warmupOffset = benchmark.inWarmup ? 123u : 0u;
        srand(seedBase + runOffset + warmupOffset);
    }

    shuffle_numbers();
    memcpy(benchmark.baselineNumbers, numbers, (size_t)app.arraySize * sizeof(numbers[0]));
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

    if (benchmark.configRunsPerSort < 1) {
        benchmark.configRunsPerSort = 1;
    }

    if (benchmark_build_sequence() <= 0) {
        set_status_text("Benchmark unavailable: no sorts selected");
        return;
    }

    benchmark.previousSort = app.currentSort;
    benchmark.previousPaused = app.paused;
    benchmark.previousStepMode = app.stepMode;
    benchmark.resultCount = SORT_REGISTRY_COUNT;
    benchmark.currentSortIndex = 0;
    benchmark.currentRunIndex = benchmark.configWarmup ? -1 : 0;
    benchmark.inWarmup = benchmark.configWarmup;
    benchmark.active = true;
    benchmark.running = true;

    for (int i = 0; i < benchmark.resultCount; i++) {
        benchmark.results[i].mode = sortRegistry[i].mode;
        benchmark.results[i].totalComparisons = 0;
        benchmark.results[i].totalSwaps = 0;
        benchmark.results[i].totalSteps = 0;
        benchmark.results[i].totalElapsed = 0.0f;
        benchmark.results[i].minElapsed = FLT_MAX;
        benchmark.results[i].maxElapsed = 0.0f;
        benchmark.results[i].runs = 0;
    }

    benchmark_prepare_run_baseline();

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

    if (!benchmark.inWarmup && benchmark.currentSortIndex >= 0 && benchmark.currentSortIndex < benchmark.sequenceCount) {
        int resultIndex = benchmark.sequence[benchmark.currentSortIndex];
        BenchmarkResult *result = &benchmark.results[resultIndex];
        result->totalComparisons += app.statComparisons;
        result->totalSwaps += app.statSwaps;
        result->totalSteps += app.statSteps;
        result->totalElapsed += app.statElapsed;
        if (app.statElapsed < result->minElapsed) result->minElapsed = app.statElapsed;
        if (app.statElapsed > result->maxElapsed) result->maxElapsed = app.statElapsed;
        result->runs++;
    }

    benchmark.currentSortIndex++;
    if (benchmark.currentSortIndex < benchmark.sequenceCount) {
        benchmark_begin_current_sort();
        return;
    }

    if (benchmark.inWarmup) {
        benchmark.inWarmup = false;
        benchmark.currentRunIndex = 0;
        benchmark.currentSortIndex = 0;
        benchmark_prepare_run_baseline();
        benchmark_begin_current_sort();
        set_status_text("Benchmark warmup complete");
        return;
    }

    benchmark.currentRunIndex++;
    if (benchmark.currentRunIndex < benchmark.configRunsPerSort) {
        benchmark.currentSortIndex = 0;
        benchmark_prepare_run_baseline();
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
    DrawText(TextFormat("N=%d  Dist=%s  Sorts=%d/%d", app.arraySize, get_distribution_name(), benchmark.sequenceCount, SORT_REGISTRY_COUNT), panelX + 16, panelY + 44, 20, LIGHTGRAY);
    DrawText(TextFormat("Config: Runs[-/=]%d  Seed[Z]:%s  Warmup[W]:%s", benchmark.configRunsPerSort, benchmark.configUseFixedSeed ? "ON" : "OFF", benchmark.configWarmup ? "ON" : "OFF"), panelX + 16, panelY + 68, 18, LIGHTGRAY);
    DrawText(TextFormat("Seed Value [,/.]: %u  Subset [LEFT/RIGHT], Toggle[;], Enter=start", benchmark.configFixedSeed), panelX + 16, panelY + 90, 18, LIGHTGRAY);

    int buttonY = panelY + 118;
    Rectangle runsMinusButton = { (float)(panelX + 16), (float)buttonY, 24.0f, 24.0f };
    Rectangle runsPlusButton = { (float)(panelX + 44), (float)buttonY, 24.0f, 24.0f };
    Rectangle seedMinusButton = { (float)(panelX + 220), (float)buttonY, 24.0f, 24.0f };
    Rectangle seedPlusButton = { (float)(panelX + 248), (float)buttonY, 24.0f, 24.0f };
    Rectangle fixedSeedButton = { (float)(panelX + 330), (float)buttonY, 96.0f, 24.0f };
    Rectangle warmupButton = { (float)(panelX + 430), (float)buttonY, 96.0f, 24.0f };
    Rectangle startButton = { (float)(panelX + 530), (float)buttonY, 114.0f, 24.0f };
    Rectangle exportButton = { (float)(panelX + 72), (float)buttonY, 132.0f, 24.0f };

    Vector2 mousePos = GetMousePosition();
    bool hoverRunsMinus = CheckCollisionPointRec(mousePos, runsMinusButton);
    bool hoverRunsPlus = CheckCollisionPointRec(mousePos, runsPlusButton);
    bool hoverSeedMinus = CheckCollisionPointRec(mousePos, seedMinusButton);
    bool hoverSeedPlus = CheckCollisionPointRec(mousePos, seedPlusButton);
    bool hoverFixedSeed = CheckCollisionPointRec(mousePos, fixedSeedButton);
    bool hoverWarmup = CheckCollisionPointRec(mousePos, warmupButton);
    bool hoverStart = CheckCollisionPointRec(mousePos, startButton);
    bool hoverExport = CheckCollisionPointRec(mousePos, exportButton);

    if (hoverRunsMinus || hoverRunsPlus || hoverSeedMinus || hoverSeedPlus || hoverFixedSeed || hoverWarmup || hoverStart || hoverExport) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }

    DrawRectangleRec(runsMinusButton, Fade(DARKGRAY, hoverRunsMinus ? 0.95f : 0.8f));
    DrawRectangleRec(runsPlusButton, Fade(DARKGRAY, hoverRunsPlus ? 0.95f : 0.8f));
    DrawRectangleRec(seedMinusButton, Fade(DARKGRAY, hoverSeedMinus ? 0.95f : 0.8f));
    DrawRectangleRec(seedPlusButton, Fade(DARKGRAY, hoverSeedPlus ? 0.95f : 0.8f));
    DrawRectangleRec(fixedSeedButton, Fade(benchmark.configUseFixedSeed ? DARKGREEN : DARKGRAY, hoverFixedSeed ? 0.95f : 0.8f));
    DrawRectangleRec(warmupButton, Fade(benchmark.configWarmup ? DARKGREEN : DARKGRAY, hoverWarmup ? 0.95f : 0.8f));
    DrawRectangleRec(startButton, Fade(benchmark.running ? DARKGRAY : DARKBLUE, hoverStart ? 1.0f : 0.85f));
    DrawRectangleRec(exportButton, Fade(benchmark.running ? DARKGRAY : DARKPURPLE, hoverExport ? 1.0f : 0.85f));

    DrawRectangleLinesEx(runsMinusButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(runsPlusButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(seedMinusButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(seedPlusButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(fixedSeedButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(warmupButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(startButton, 1.0f, LIGHTGRAY);
    DrawRectangleLinesEx(exportButton, 1.0f, LIGHTGRAY);

    DrawText("-", (int)runsMinusButton.x + 8, (int)runsMinusButton.y + 2, 20, RAYWHITE);
    DrawText("+", (int)runsPlusButton.x + 7, (int)runsPlusButton.y + 1, 20, RAYWHITE);
    DrawText("-", (int)seedMinusButton.x + 8, (int)seedMinusButton.y + 2, 20, RAYWHITE);
    DrawText("+", (int)seedPlusButton.x + 7, (int)seedPlusButton.y + 1, 20, RAYWHITE);
    DrawText("Fixed Seed", (int)fixedSeedButton.x + 8, (int)fixedSeedButton.y + 4, 16, RAYWHITE);
    DrawText("Warmup", (int)warmupButton.x + 18, (int)warmupButton.y + 4, 16, RAYWHITE);
    DrawText("Start", (int)startButton.x + 34, (int)startButton.y + 4, 16, RAYWHITE);
    DrawText("Export CSV", (int)exportButton.x + 14, (int)exportButton.y + 4, 16, RAYWHITE);

    int statusY = buttonY + 34;

    if (benchmark.running) {
        const char *runningName = get_sort_name_by_mode(sortRegistry[benchmark.sequence[benchmark.currentSortIndex]].mode);
        if (benchmark.inWarmup) {
            DrawText(TextFormat("Running Warmup: %s (%d/%d)", runningName, benchmark.currentSortIndex + 1, benchmark.sequenceCount), panelX + 16, statusY, 20, SKYBLUE);
        } else {
            DrawText(TextFormat("Running: %s  run %d/%d  (%d/%d)", runningName, benchmark.currentRunIndex + 1, benchmark.configRunsPerSort, benchmark.currentSortIndex + 1, benchmark.sequenceCount), panelX + 16, statusY, 20, SKYBLUE);
        }
    } else {
        bool hasResults = false;
        for (int i = 0; i < benchmark.resultCount; i++) {
            if (benchmark.results[i].runs > 0) {
                hasResults = true;
                break;
            }
        }
        DrawText(hasResults ? "Status: Complete" : "Status: Config", panelX + 16, statusY, 20, SKYBLUE);
    }

    int tableY = panelY + 182;
    DrawText("Sort", panelX + 16, tableY, 20, YELLOW);
    DrawText("Avg", panelX + 150, tableY, 20, YELLOW);
    DrawText("Min", panelX + 235, tableY, 20, YELLOW);
    DrawText("Max", panelX + 320, tableY, 20, YELLOW);
    DrawText("Cmp", panelX + 405, tableY, 20, YELLOW);
    DrawText("Swp", panelX + 490, tableY, 20, YELLOW);
    DrawText("Runs", panelX + 565, tableY, 20, YELLOW);

    int rowY = tableY + 28;
    int maxRows = (panelY + panelH - rowY - 26) / 24;
    if (maxRows < 1) maxRows = 1;

    int rowsToDraw = benchmark.resultCount;
    if (rowsToDraw > maxRows) {
        rowsToDraw = maxRows;
    }

    int displayOrder[MAX_BENCHMARK_RESULTS] = { 0 };
    benchmark_build_display_order(displayOrder, benchmark.resultCount);

    for (int i = 0; i < rowsToDraw; i++) {
        int resultIndex = displayOrder[i];
        const BenchmarkResult *result = &benchmark.results[resultIndex];
        bool enabled = benchmark.sortEnabled[resultIndex];
        bool selected = (!benchmark.running && benchmark.active && i == benchmark.selectedConfigSortIndex);
        Rectangle rowRect = { (float)(panelX + 10), (float)(rowY + i * 24 - 2), (float)(panelW - 20), 24.0f };
        bool hoveredRow = CheckCollisionPointRec(mousePos, rowRect);
        if (hoveredRow) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        float avg = (result->runs > 0) ? (result->totalElapsed / (float)result->runs) : 0.0f;
        float min = (result->runs > 0) ? result->minElapsed : 0.0f;
        float max = (result->runs > 0) ? result->maxElapsed : 0.0f;
        unsigned long long avgCmp = (result->runs > 0) ? (result->totalComparisons / (unsigned long long)result->runs) : 0ULL;
        unsigned long long avgSwp = (result->runs > 0) ? (result->totalSwaps / (unsigned long long)result->runs) : 0ULL;
        Color rowColor = enabled ? RAYWHITE : GRAY;
        if (selected) {
            DrawRectangle(panelX + 10, rowY + i * 24 - 2, panelW - 20, 24, Fade(SKYBLUE, hoveredRow ? 0.40f : 0.25f));
        } else if (hoveredRow) {
            DrawRectangle(panelX + 10, rowY + i * 24 - 2, panelW - 20, 24, Fade(SKYBLUE, 0.15f));
        }
        DrawText(TextFormat("[%c] %s", enabled ? 'x' : ' ', sortRegistry[resultIndex].name), panelX + 16, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%.3f", avg), panelX + 150, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%.3f", min), panelX + 235, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%.3f", max), panelX + 320, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%llu", avgCmp), panelX + 405, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%llu", avgSwp), panelX + 490, rowY + i * 24, 20, rowColor);
        DrawText(TextFormat("%d", result->runs), panelX + 575, rowY + i * 24, 20, rowColor);
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

static void run_merge_step(void)
{
    merge_sort_step(numbers, mergeBuffer, knownSorted, app.arraySize, &app.sortingDone, &app.mergeWidth, &app.mergeLeft, &app.mergeMid, &app.mergeRight, &app.mergeI, &app.mergeJ, &app.mergeK, &app.mergeCopyIndex, &app.mergeActive, &app.mergeCopying, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_cocktail_step(void)
{
    cocktail_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.cocktailStart, &app.cocktailEnd, &app.cocktailIndex, &app.cocktailForward, &app.cocktailSwapped, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_gnome_step(void)
{
    gnome_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.gnomeIndex, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_comb_step(void)
{
    comb_sort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.combGap, &app.combIndex, &app.combSwapped, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
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

static void fill_merge_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Width: %d  left/mid/right: %d/%d/%d", app.mergeWidth, app.mergeLeft, app.mergeMid, app.mergeRight);
    snprintf(line2, line2Size, "i:%d  j:%d  k:%d", app.mergeI, app.mergeJ, app.mergeK);
    snprintf(line3, line3Size, "Phase: %s", app.mergeCopying ? "COPY BACK" : (app.mergeActive ? "MERGE" : "PREPARE"));
}

static void fill_cocktail_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Cocktail start/end: %d/%d", app.cocktailStart, app.cocktailEnd);
    snprintf(line2, line2Size, "Index: %d  dir: %s", app.cocktailIndex, app.cocktailForward ? "FWD" : "BWD");
    snprintf(line3, line3Size, "Swapped this pass: %s", app.cocktailSwapped ? "yes" : "no");
}

static void fill_gnome_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Gnome index: %d", app.gnomeIndex);
    snprintf(line2, line2Size, "Compare pair: [%d, %d]", app.gnomeIndex - 1, app.gnomeIndex);
    snprintf(line3, line3Size, "Mode: adjacent walk");
}

static void fill_comb_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Gap: %d  Index: %d", app.combGap, app.combIndex);
    snprintf(line2, line2Size, "Compare pair: [%d, %d]", app.combIndex, app.combIndex + app.combGap);
    snprintf(line3, line3Size, "Swapped this pass: %s", app.combSwapped ? "yes" : "no");
}

static void run_timsort_step(void)
{
    timsort_step(numbers, timBuffer, knownSorted, app.arraySize, &app.sortingDone, &app.timRunSize, &app.timMergeWidth, &app.timLeft, &app.timMid, &app.timRight, &app.timSortIndex, &app.timSortEnd, &app.timMergeIndex, &app.timI, &app.timJ, &app.timK, &app.timInsertActive, &app.timMergeActive, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void run_radixsort_step(void)
{
    radixsort_step(numbers, radixBuffer, radixCount, knownSorted, app.arraySize, &app.sortingDone, &app.radixBit, &app.radixMaxBit, &app.radixPass, &app.radixIndex, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void reset_timsort_state(void)
{
    app.timRunSize = 32;
    app.timMergeWidth = 1;
    app.timLeft = 0;
    app.timMid = 0;
    app.timRight = 0;
    app.timSortIndex = 0;
    app.timSortEnd = 32;
    app.timMergeIndex = 0;
    app.timI = 0;
    app.timJ = 0;
    app.timK = 0;
    app.timInsertActive = true;
    app.timMergeActive = false;
}

static void reset_radixsort_state(void)
{
    app.radixBit = 0;
    app.radixPass = 0;
    app.radixIndex = 0;
    app.radixMaxBit = 0;
}

static void fill_timsort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    if (app.timInsertActive) {
        snprintf(line1, line1Size, "Phase: INSERTION  Run size: %d", app.timRunSize);
        snprintf(line2, line2Size, "Sorting run ending at: %d", app.timSortEnd);
        snprintf(line3, line3Size, "Index: %d", app.timSortIndex);
    } else {
        snprintf(line1, line1Size, "Phase: MERGE  Width: %d", app.timMergeWidth);
        snprintf(line2, line2Size, "Merge range: [%d, %d)", app.timLeft, app.timRight);
        if (line3Size > 0) line3[0] = '\0';
    }
}

static void fill_radixsort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    snprintf(line1, line1Size, "Bit position: %d / %d", app.radixBit, app.radixMaxBit);
    snprintf(line2, line2Size, "Pass: %d  (0=count, 1=distribute)", app.radixPass);
    snprintf(line3, line3Size, "Zeros: %d  Ones: %d", radixCount[0], radixCount[1]);
}

static void run_bogosort_step(void)
{
    bogosort_step(numbers, knownSorted, app.arraySize, &app.sortingDone, &app.bogoAttempts, &app.bogoCheckIndex, &app.bogoIsChecking, &app.statComparisons, &app.statSwaps, play_compare_sound, play_swap_sound, play_sorted_sound, start_completion_sweep);
}

static void reset_bogosort_state(void)
{
    app.bogoAttempts = 0;
    app.bogoCheckIndex = 0;
    app.bogoIsChecking = false;
}

static void fill_bogosort_telemetry(char *line1, size_t line1Size, char *line2, size_t line2Size, char *line3, size_t line3Size)
{
    int bogoSize = (app.arraySize < 10) ? app.arraySize : 10;
    snprintf(line1, line1Size, "Bogo attempts: %d", app.bogoAttempts);
    snprintf(line2, line2Size, "Sorting up to: %d elements", bogoSize);
    if (app.bogoIsChecking) {
        snprintf(line3, line3Size, "Checking: index %d / %d", app.bogoCheckIndex, bogoSize - 1);
    } else {
        snprintf(line3, line3Size, "Shuffling...");
    }
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

    benchmark.configUseFixedSeed = true;
    benchmark.configFixedSeed = 1337u;
    benchmark.configRunsPerSort = 3;
    benchmark.configWarmup = false;
    benchmark.selectedConfigSortIndex = 0;
    for (int i = 0; i < SORT_REGISTRY_COUNT && i < MAX_BENCHMARK_RESULTS; i++) {
        benchmark.sortEnabled[i] = true;
    }
    benchmark_build_sequence();

    reset_sort_state(false);
    
    while(!WindowShouldClose() && !app.requestClose){
        float dt = GetFrameTime();
        audio_begin_frame(&app.audio);

        ControllerContext controller = {
            .windowWidth = WIDTH,
            .windowHeight = HEIGHT,
            .maxSize = MAX_SIZE,
            .benchmarkOverlayActive = benchmark.active,
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

        if (!benchmark.running && !app.sizeInputActive && !app.pauseMenuActive) {
            if (IsKeyPressed(KEY_Z)) {
                benchmark.configUseFixedSeed = !benchmark.configUseFixedSeed;
                set_status_text(benchmark.configUseFixedSeed ? "Benchmark fixed seed ON" : "Benchmark fixed seed OFF");
            }

            if (IsKeyPressed(KEY_W)) {
                benchmark.configWarmup = !benchmark.configWarmup;
                set_status_text(benchmark.configWarmup ? "Benchmark warmup ON" : "Benchmark warmup OFF");
            }

            if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
                benchmark.configRunsPerSort++;
                if (benchmark.configRunsPerSort > 20) benchmark.configRunsPerSort = 20;
                set_status_text(TextFormat("Benchmark runs: %d", benchmark.configRunsPerSort));
            }

            if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
                benchmark.configRunsPerSort--;
                if (benchmark.configRunsPerSort < 1) benchmark.configRunsPerSort = 1;
                set_status_text(TextFormat("Benchmark runs: %d", benchmark.configRunsPerSort));
            }

            if (IsKeyPressed(KEY_PERIOD)) {
                benchmark.configFixedSeed += 1u;
                set_status_text(TextFormat("Benchmark seed: %u", benchmark.configFixedSeed));
            }

            if (IsKeyPressed(KEY_COMMA)) {
                if (benchmark.configFixedSeed > 0u) {
                    benchmark.configFixedSeed -= 1u;
                }
                set_status_text(TextFormat("Benchmark seed: %u", benchmark.configFixedSeed));
            }

            if (benchmark.active) {
                int panelX = WIDTH - 700;
                int panelY = 220;
                int panelW = 660;
                int panelH = HEIGHT - panelY - 30;
                if (panelX < 20) panelX = 20;
                if (panelW < 360) panelW = 360;
                if (panelH < 240) panelH = 240;

                int buttonY = panelY + 118;
                Rectangle runsMinusButton = { (float)(panelX + 16), (float)buttonY, 24.0f, 24.0f };
                Rectangle runsPlusButton = { (float)(panelX + 44), (float)buttonY, 24.0f, 24.0f };
                Rectangle seedMinusButton = { (float)(panelX + 220), (float)buttonY, 24.0f, 24.0f };
                Rectangle seedPlusButton = { (float)(panelX + 248), (float)buttonY, 24.0f, 24.0f };
                Rectangle fixedSeedButton = { (float)(panelX + 330), (float)buttonY, 96.0f, 24.0f };
                Rectangle warmupButton = { (float)(panelX + 430), (float)buttonY, 96.0f, 24.0f };
                Rectangle startButton = { (float)(panelX + 530), (float)buttonY, 114.0f, 24.0f };
                Rectangle exportButton = { (float)(panelX + 72), (float)buttonY, 132.0f, 24.0f };

                int tableY = panelY + 182;
                int rowY = tableY + 28;
                int maxRows = (panelY + panelH - rowY - 26) / 24;
                if (maxRows < 1) maxRows = 1;
                int rowsToDraw = benchmark.resultCount;
                if (rowsToDraw > maxRows) rowsToDraw = maxRows;

                int displayOrder[MAX_BENCHMARK_RESULTS] = { 0 };
                benchmark_build_display_order(displayOrder, benchmark.resultCount);

                Vector2 mousePos = GetMousePosition();
                bool mouseClick = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                bool consumedClick = false;

                if (mouseClick) {
                    if (CheckCollisionPointRec(mousePos, runsMinusButton)) {
                        benchmark.configRunsPerSort--;
                        if (benchmark.configRunsPerSort < 1) benchmark.configRunsPerSort = 1;
                        set_status_text(TextFormat("Benchmark runs: %d", benchmark.configRunsPerSort));
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, runsPlusButton)) {
                        benchmark.configRunsPerSort++;
                        if (benchmark.configRunsPerSort > 20) benchmark.configRunsPerSort = 20;
                        set_status_text(TextFormat("Benchmark runs: %d", benchmark.configRunsPerSort));
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, seedMinusButton)) {
                        if (benchmark.configFixedSeed > 0u) {
                            benchmark.configFixedSeed -= 1u;
                        }
                        set_status_text(TextFormat("Benchmark seed: %u", benchmark.configFixedSeed));
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, seedPlusButton)) {
                        benchmark.configFixedSeed += 1u;
                        set_status_text(TextFormat("Benchmark seed: %u", benchmark.configFixedSeed));
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, fixedSeedButton)) {
                        benchmark.configUseFixedSeed = !benchmark.configUseFixedSeed;
                        set_status_text(benchmark.configUseFixedSeed ? "Benchmark fixed seed ON" : "Benchmark fixed seed OFF");
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, warmupButton)) {
                        benchmark.configWarmup = !benchmark.configWarmup;
                        set_status_text(benchmark.configWarmup ? "Benchmark warmup ON" : "Benchmark warmup OFF");
                        consumedClick = true;
                    } else if (CheckCollisionPointRec(mousePos, startButton)) {
                        benchmark_start();
                        stepTimer = 0.0f;
                        consumedClick = true;
                    } else if (!benchmark.running && CheckCollisionPointRec(mousePos, exportButton)) {
                        benchmark_export_results_csv();
                        consumedClick = true;
                    }
                }

                int hoveredRow = -1;
                for (int i = 0; i < rowsToDraw; i++) {
                    Rectangle rowRect = { (float)(panelX + 10), (float)(rowY + i * 24 - 2), (float)(panelW - 20), 24.0f };
                    if (CheckCollisionPointRec(mousePos, rowRect)) {
                        hoveredRow = i;
                        break;
                    }
                }

                if (hoveredRow >= 0) {
                    benchmark.selectedConfigSortIndex = hoveredRow;
                }

                if (!consumedClick && hoveredRow >= 0 && mouseClick) {
                    benchmark.selectedConfigSortIndex = hoveredRow;
                    if (mousePos.x <= (float)(panelX + 48)) {
                        int idx = displayOrder[hoveredRow];
                        benchmark.sortEnabled[idx] = !benchmark.sortEnabled[idx];
                        benchmark_build_sequence();
                        set_status_text(TextFormat("Benchmark subset: %d selected", benchmark.sequenceCount));
                    }
                }

                if (IsKeyPressed(KEY_LEFT)) {
                    benchmark.selectedConfigSortIndex--;
                    if (benchmark.selectedConfigSortIndex < 0) {
                        benchmark.selectedConfigSortIndex = SORT_REGISTRY_COUNT - 1;
                    }
                }

                if (IsKeyPressed(KEY_RIGHT)) {
                    benchmark.selectedConfigSortIndex++;
                    if (benchmark.selectedConfigSortIndex >= SORT_REGISTRY_COUNT) {
                        benchmark.selectedConfigSortIndex = 0;
                    }
                }

                if (IsKeyPressed(KEY_SEMICOLON)) {
                    int idx = displayOrder[benchmark.selectedConfigSortIndex];
                    if (idx >= 0 && idx < SORT_REGISTRY_COUNT) {
                        benchmark.sortEnabled[idx] = !benchmark.sortEnabled[idx];
                        benchmark_build_sequence();
                        set_status_text(TextFormat("Benchmark subset: %d selected", benchmark.sequenceCount));
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    benchmark_start();
                    stepTimer = 0.0f;
                }

                if (!benchmark.running && IsKeyPressed(KEY_E)) {
                    benchmark_export_results_csv();
                }
            }
        }

        if (!app.sizeInputActive && !app.pauseMenuActive && IsKeyPressed(KEY_X)) {
            if (benchmark.running) {
                benchmark_cancel();
            } else if (!benchmark.active) {
                benchmark.active = true;
                benchmark.running = false;
                benchmark.selectedConfigSortIndex = 0;
                benchmark.resultCount = SORT_REGISTRY_COUNT;
                benchmark_build_sequence();
                set_status_text("Benchmark config open (Enter to start)");
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
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        ClearBackground(BLACK);

        char telemetryLine1[96] = { 0 };
        char telemetryLine2[96] = { 0 };
        char telemetryLine3[96] = { 0 };
        char benchmarkStatus[160] = { 0 };
        fill_current_sort_telemetry(telemetryLine1, sizeof(telemetryLine1), telemetryLine2, sizeof(telemetryLine2), telemetryLine3, sizeof(telemetryLine3));

        if (benchmark.running && benchmark.currentSortIndex >= 0 && benchmark.currentSortIndex < benchmark.sequenceCount) {
            const char *runningName = get_sort_name_by_mode(sortRegistry[benchmark.sequence[benchmark.currentSortIndex]].mode);
            if (benchmark.inWarmup) {
                snprintf(benchmarkStatus, sizeof(benchmarkStatus), "BENCHMARK RUNNING: Warmup %s (%d/%d)", runningName, benchmark.currentSortIndex + 1, benchmark.sequenceCount);
            } else {
                snprintf(benchmarkStatus, sizeof(benchmarkStatus), "BENCHMARK RUNNING: %s  run %d/%d  (%d/%d)", runningName, benchmark.currentRunIndex + 1, benchmark.configRunsPerSort, benchmark.currentSortIndex + 1, benchmark.sequenceCount);
            }
        }

        int currentSortIndex = get_sort_index(app.currentSort);
        if (currentSortIndex < 0) {
            currentSortIndex = 0;
        }

        UiDrawContext ui = {
            .sortColorA = color_from_id(sortRegistry[currentSortIndex].colorAId),
            .sortColorB = color_from_id(sortRegistry[currentSortIndex].colorBId),
            .sortColorC = color_from_id(sortRegistry[currentSortIndex].colorCId),
            .sortColorD = color_from_id(sortRegistry[currentSortIndex].colorDId),
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
            .mergeWidth = app.mergeWidth,
            .mergeLeft = app.mergeLeft,
            .mergeMid = app.mergeMid,
            .mergeRight = app.mergeRight,
            .mergeI = app.mergeI,
            .mergeJ = app.mergeJ,
            .mergeK = app.mergeK,
            .mergeCopyIndex = app.mergeCopyIndex,
            .mergeActive = app.mergeActive,
            .mergeCopying = app.mergeCopying,
            .cocktailStart = app.cocktailStart,
            .cocktailEnd = app.cocktailEnd,
            .cocktailIndex = app.cocktailIndex,
            .cocktailForward = app.cocktailForward,
            .cocktailSwapped = app.cocktailSwapped,
            .gnomeIndex = app.gnomeIndex,
            .combGap = app.combGap,
            .combIndex = app.combIndex,
            .combSwapped = app.combSwapped,
            .timRunSize = app.timRunSize,
            .timMergeWidth = app.timMergeWidth,
            .timLeft = app.timLeft,
            .timMid = app.timMid,
            .timRight = app.timRight,
            .timSortIndex = app.timSortIndex,
            .timSortEnd = app.timSortEnd,
            .timMergeIndex = app.timMergeIndex,
            .timI = app.timI,
            .timJ = app.timJ,
            .timK = app.timK,
            .timInsertActive = app.timInsertActive,
            .timMergeActive = app.timMergeActive,
            .radixBit = app.radixBit,
            .radixPass = app.radixPass,
            .radixIndex = app.radixIndex,
            .radixMaxBit = app.radixMaxBit,
            .bogoAttempts = app.bogoAttempts,
            .bogoCheckIndex = app.bogoCheckIndex,
            .bogoIsChecking = app.bogoIsChecking,
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
            .autoNextSortTimer = app.autoNextSortTimer,
            .benchmarkRunning = benchmark.running,
            .benchmarkStatusText = benchmarkStatus
        };
        draw_elements(&ui);
        draw_benchmark_overlay();

        EndDrawing();
    }

    CloseWindow();

    audio_shutdown(&app.audio);

    return 0;
}
