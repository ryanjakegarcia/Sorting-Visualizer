#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <raylib.h>
#include "app_types.h"

typedef struct UiDrawContext {
    Color sortColorA;
    Color sortColorB;
    Color sortColorC;
    Color sortColorD;

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
    int quickLow;
    int quickHigh;
    int quickTop;
    int quickPivotValue;
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
    int heapSiftRoot;
    int heapSiftEnd;
    bool heapSiftActive;

    bool insertionHoldingKey;
    int insertionKey;
    int shellGap;
    int shellI;
    int shellJ;
    bool shellHolding;
    int mergeWidth;
    int mergeLeft;
    int mergeMid;
    int mergeRight;
    int mergeI;
    int mergeJ;
    int mergeK;
    int mergeCopyIndex;
    bool mergeActive;
    bool mergeCopying;
    int cocktailStart;
    int cocktailEnd;
    int cocktailIndex;
    bool cocktailForward;
    bool cocktailSwapped;
    int gnomeIndex;
    int combGap;
    int combIndex;
    bool combSwapped;
    int timRunSize;
    int timMergeWidth;
    int timLeft;
    int timMid;
    int timRight;
    int timSortIndex;
    int timSortEnd;
    int timMergeIndex;
    int timI;
    int timJ;
    int timK;
    bool timInsertActive;
    bool timMergeActive;
    int radixBit;
    int radixPass;
    int radixIndex;
    int radixMaxBit;
    int bogoAttempts;
    int bogoCheckIndex;
    bool bogoIsChecking;
    bool splashPreviewMode;
    int oddEvenIndex;
    int oddEvenStart;
    bool oddEvenSwappedThisRound;
    int pancakeCurrentSize;
    int pancakePhase;
    int pancakeMaxIndex;
    int pancakeScanIndex;
    int pancakeFlipIndex;
    int countingPhase;
    int countingIndex;
    int countingMinValue;
    int countingMaxValue;
    int countingWriteValue;
    int countingWriteIndex;
    int introPivotIndex;
    int introI;
    int introJ;
    int introLow;
    int introHigh;
    int introTop;
    int introDepthLimit;
    bool introPartitionActive;
    bool introHeapFallbackActive;
    int introHeapFocusIndex;
    int introHeapCandidateIndex;

    bool showValues;
    bool showHud;
    bool showSettingsOverlay;
    bool showTelemetry;
    const char *telemetryLine1;
    const char *telemetryLine2;
    const char *telemetryLine3;
    bool showSizeInputBox;
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
    bool pauseMenuActive;
    int pauseMenuSelection;
    bool stepMode;
    bool minimalUiMode;
    bool showInfo;

    float autoNextSortDelay;
    float autoNextSortTimer;

    bool benchmarkRunning;
    const char *benchmarkStatusText;
} UiDrawContext;

Rectangle ui_get_size_input_box(int height);
int ui_get_pause_menu_item_count(void);
Rectangle ui_get_pause_menu_item_rect(int width, int height, int itemIndex);
void draw_elements(const UiDrawContext *ctx);

#endif
