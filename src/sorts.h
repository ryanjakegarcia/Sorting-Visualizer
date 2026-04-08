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

void timsort_step(
    int *numbers,
    int *timBuffer,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *timRunSize,
    int *timMergeWidth,
    int *timLeft,
    int *timMid,
    int *timRight,
    int *timSortIndex,
    int *timSortEnd,
    int *timMergeIndex,
    int *timI,
    int *timJ,
    int *timK,
    bool *timInsertActive,
    bool *timMergeActive,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void radixsort_step(
    int *numbers,
    int *radixBuffer,
    int *radixCount,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *radixBit,
    int *radixMaxBit,
    int *radixPass,
    int *radixIndex,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void bogosort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool bogoUseFullArray,
    bool *sortingDone,
    int *bogoAttempts,
    int *bogoCheckIndex,
    bool *bogoIsChecking,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void odd_even_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *oddEvenIndex,
    int *oddEvenStart,
    bool *oddEvenSwappedThisRound,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void pancake_sort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *pancakeCurrentSize,
    int *pancakePhase,
    int *pancakeMaxIndex,
    int *pancakeScanIndex,
    int *pancakeFlipIndex,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void counting_sort_step(
    int *numbers,
    int *countingOutput,
    int *countingCounts,
    bool *knownSorted,
    int arraySize,
    bool *sortingDone,
    int *countingPhase,
    int *countingIndex,
    int *countingMinValue,
    int *countingMaxValue,
    int *countingWriteValue,
    int *countingWriteIndex,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

void introsort_step(
    int *numbers,
    bool *knownSorted,
    int *introStackLow,
    int *introStackHigh,
    int *introStackDepth,
    int introStackCapacity,
    int arraySize,
    bool *sortingDone,
    int *introTop,
    int *introLow,
    int *introHigh,
    int *introPivotValue,
    int *introPivotIndex,
    int *introI,
    int *introJ,
    int *introDepthLimit,
    bool *introPartitionActive,
    bool *introHeapFallbackActive,
    int *introHeapBuildIndex,
    int *introHeapSortEnd,
    int *introHeapSiftRoot,
    int *introHeapSiftEnd,
    int *introHeapFocusIndex,
    int *introHeapCandidateIndex,
    bool *introHeapBuilding,
    bool *introHeapSiftActive,
    unsigned long long *statComparisons,
    unsigned long long *statSwaps,
    CompareSoundFn playCompareSound,
    SwapSoundFn playSwapSound,
    SortedSoundFn playSortedSound,
    CompletionSweepFn startCompletionSweep
);

#endif
