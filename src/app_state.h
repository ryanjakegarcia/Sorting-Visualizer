#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>

#include "app_types.h"
#include "audio.h"

typedef struct RuntimeState {
    int arraySize;
    bool sizeInputActive;
    char sizeInput[16];
    int sizeInputLen;
    DistributionMode distributionMode;

    SortMode currentSort;
    int bubbleIndex;
    int bubblePass;
    int insertionIndex;
    int insertionPos;
    int insertionKey;
    bool insertionHoldingKey;
    int selectionI;
    int selectionJ;
    int selectionMin;
    int heapBuildIndex;
    int heapSortEnd;
    int heapSiftRoot;
    int heapSiftEnd;
    int heapFocusIndex;
    int heapCandidateIndex;
    bool heapBuilding;
    bool heapSiftActive;
    int quickTop;
    int quickLow;
    int quickHigh;
    int quickPivotValue;
    int quickPivotIndex;
    int quickI;
    int quickJ;
    bool quickPartitionActive;
    int shellGap;
    int shellI;
    int shellJ;
    int shellTemp;
    bool shellHolding;

    bool sortingDone;
    bool paused;
    bool pausedBeforeMenu;
    bool pauseMenuActive;
    int pauseMenuSelection;
    bool requestClose;
    bool stepMode;
    bool stepOnceRequested;

    AudioEngine audio;
    bool compareAudioEnabled;
    bool swapAudioEnabled;
    bool progressAudioEnabled;
    bool finishAudioEnabled;
    bool showValues;
    bool showLegend;
    bool showHud;
    bool showSettingsOverlay;
    bool showTelemetry;
    bool showSizeInputBox;
    bool minimalUiMode;
    float masterVolume;
    float autoNextSortTimer;

    unsigned long long statComparisons;
    unsigned long long statSwaps;
    unsigned long long statSteps;
    float statElapsed;

    char statusText[128];
    float statusTimer;
} RuntimeState;

void app_state_init_defaults(RuntimeState *state);

#endif
