#include "app_state.h"

void app_state_init_defaults(RuntimeState *state)
{
    state->arraySize = 50;
    state->sizeInputActive = false;
    state->sizeInput[0] = '\0';
    state->sizeInputLen = 0;
    state->distributionMode = DIST_RANDOM;

    state->currentSort = SORT_BUBBLE;
    state->bubbleIndex = 0;
    state->bubblePass = 0;
    state->insertionIndex = 1;
    state->insertionPos = 0;
    state->insertionKey = 0;
    state->insertionHoldingKey = false;
    state->selectionI = 0;
    state->selectionJ = 1;
    state->selectionMin = 0;
    state->heapBuildIndex = 0;
    state->heapSortEnd = 0;
    state->heapSiftRoot = 0;
    state->heapSiftEnd = 0;
    state->heapFocusIndex = -1;
    state->heapCandidateIndex = -1;
    state->heapBuilding = true;
    state->heapSiftActive = false;
    state->quickTop = -1;
    state->quickLow = 0;
    state->quickHigh = 0;
    state->quickPivotValue = 0;
    state->quickPivotIndex = -1;
    state->quickI = -1;
    state->quickJ = -1;
    state->quickPartitionActive = false;

    state->sortingDone = false;
    state->paused = false;
    state->pausedBeforeMenu = false;
    state->pauseMenuActive = false;
    state->pauseMenuSelection = 0;
    state->requestClose = false;
    state->stepMode = false;
    state->stepOnceRequested = false;

    state->audio = (AudioEngine){ 0 };
    state->compareAudioEnabled = true;
    state->swapAudioEnabled = true;
    state->progressAudioEnabled = true;
    state->finishAudioEnabled = true;
    state->showValues = false;
    state->showLegend = true;
    state->showHud = true;
    state->showTelemetry = true;
    state->showSizeInputBox = true;
    state->minimalUiMode = false;
    state->masterVolume = 0.8f;
    state->autoNextSortTimer = 0.0f;

    state->statComparisons = 0;
    state->statSwaps = 0;
    state->statSteps = 0;
    state->statElapsed = 0.0f;

    state->statusText[0] = '\0';
    state->statusTimer = 0.0f;
}
