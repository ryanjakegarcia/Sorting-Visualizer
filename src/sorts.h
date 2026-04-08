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

#endif
