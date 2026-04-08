#ifndef SORTS_H
#define SORTS_H

#include <stdbool.h>

typedef void (*CompareSoundFn)(int firstValue, int secondValue, int leftIndex, int rightIndex);
typedef void (*SwapSoundFn)(int firstValue, int secondValue, int leftIndex, int rightIndex);
typedef void (*SortedSoundFn)(void);
typedef void (*CompletionSweepFn)(void);

void bubble_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *bubbleIndex,
    int *bubblePass,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void insertion_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *insertionIndex,
    int *insertionPos,
    int *insertionKey,
    bool *insertionHoldingKey,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void selection_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *selectionI,
    int *selectionJ,
    int *selectionMin,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void heap_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *heapBuildIndex,
    int *heapSortEnd,
    int *heapSiftRoot,
    int *heapSiftEnd,
    int *heapFocusIndex,
    int *heapCandidateIndex,
    bool *heapBuilding,
    bool *heapSiftActive,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void quick_sort_step(
    int *numbers,
    bool *knownSorted,
    int *quickStackLow,
    int *quickStackHigh,
    int quickStackCapacity,
    int arraySize,
    bool *sortingDone,
    int *quickTop,
    int *quickLow,
    int *quickHigh,
    int *quickPivotValue,
    int *quickPivotIndex,
    int *quickI,
    int *quickJ,
    bool *quickPartitionActive,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void shell_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *shellGap,
    int *shellI,
    int *shellJ,
    int *shellTemp,
    bool *shellHolding,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void merge_sort_step(
    int *numbers,
    int *mergeBuffer,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *mergeWidth,
    int *mergeLeft,
    int *mergeMid,
    int *mergeRight,
    int *mergeI,
    int *mergeJ,
    int *mergeK,
    int *mergeCopyIndex,
    bool *mergeActive,
    bool *mergeCopying,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void cocktail_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *cocktailStart,
    int *cocktailEnd,
    int *cocktailIndex,
    bool *cocktailForward,
    bool *cocktailSwapped,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void gnome_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *gnomeIndex,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void comb_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *combGap,
    int *combIndex,
    bool *combSwapped,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

#endif
