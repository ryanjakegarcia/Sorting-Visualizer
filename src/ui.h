#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <raylib.h>
#include "app_types.h"

typedef struct UiDrawContext {
    int width;
    int height;
    int maxSize;
    int arraySize;

    const int *numbers;
    const bool *knownSorted;

    SortMode currentSort;
    bool sortingDone;
    bool completionSweepActive;
    int completionSweepIndex;

    int quickPivotIndex;
    int quickJ;
    int quickI;
    int heapCandidateIndex;
    int heapFocusIndex;
    int bubbleIndex;
    int insertionPos;
    int selectionMin;
    int selectionJ;
    int bubblePass;
    int insertionIndex;
    int selectionI;
    int heapSortEnd;
    bool heapBuilding;

    bool insertionHoldingKey;
    int insertionKey;

    bool showValues;
    bool showHud;
    bool sizeInputActive;
    const char *sizeInput;

    const char *sortName;
    const char *distributionName;
    float speedMultiplier;
    float masterVolume;

    bool compareAudioEnabled;
    bool swapAudioEnabled;
    bool progressAudioEnabled;
    bool finishAudioEnabled;

    unsigned long long statComparisons;
    unsigned long long statSwaps;
    unsigned long long statSteps;
    float statElapsed;

    float statusTimer;
    const char *statusText;

    bool showLegend;
    bool paused;
    bool stepMode;
    bool minimalUiMode;
    bool showInfo;

    float autoNextSortDelay;
    float autoNextSortTimer;
} UiDrawContext;

Rectangle ui_get_size_input_box(int height);
void draw_elements(const UiDrawContext *ctx);

#endif
