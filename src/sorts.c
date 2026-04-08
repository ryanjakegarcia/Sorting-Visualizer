#include "sorts.h"

static void mark_all_sorted(bool *knownSorted, int arraySize)
{
    for (int i = 0; i < arraySize; i++) {
        knownSorted[i] = true;
    }
}

static void heap_start_sift(
    int root,
    int end,
    int *heapSiftRoot,
    int *heapSiftEnd,
    int *heapFocusIndex,
    int *heapCandidateIndex,
    bool *heapSiftActive
)
{
    *heapSiftRoot = root;
    *heapSiftEnd = end;
    *heapFocusIndex = root;
    *heapCandidateIndex = -1;
    *heapSiftActive = true;
}

static void quick_push_range(int low, int high, int *quickStackLow, int *quickStackHigh, int *quickTop, int quickStackCapacity)
{
    if (*quickTop >= quickStackCapacity - 1) {
        return;
    }

    (*quickTop)++;
    quickStackLow[*quickTop] = low;
    quickStackHigh[*quickTop] = high;
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*bubblePass >= arraySize - 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*bubbleIndex < arraySize - *bubblePass - 1) {
        int left = numbers[*bubbleIndex];
        int right = numbers[*bubbleIndex + 1];
        (*statComparisons)++;
        playCompareSound(left, right, *bubbleIndex, *bubbleIndex + 1);
        if (numbers[*bubbleIndex] > numbers[*bubbleIndex + 1]) {
            int temp = numbers[*bubbleIndex];
            numbers[*bubbleIndex] = numbers[*bubbleIndex + 1];
            numbers[*bubbleIndex + 1] = temp;
            (*statSwaps)++;
            playSwapSound(left, right, *bubbleIndex, *bubbleIndex + 1);
        }
        (*bubbleIndex)++;
    } else {
        *bubbleIndex = 0;
        (*bubblePass)++;
        if (*bubblePass > 0 && *bubblePass <= arraySize) {
            knownSorted[arraySize - *bubblePass] = true;
        }

        if (*bubblePass < arraySize - 1) {
            playSortedSound();
        }
    }

    if (*bubblePass >= arraySize - 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*insertionIndex >= arraySize) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (!*insertionHoldingKey) {
        *insertionKey = numbers[*insertionIndex];
        *insertionPos = *insertionIndex - 1;
        *insertionHoldingKey = true;
        return;
    }

    if (*insertionPos >= 0) {
        (*statComparisons)++;
        playCompareSound(numbers[*insertionPos], *insertionKey, *insertionPos, *insertionIndex);
    }

    if (*insertionPos >= 0 && numbers[*insertionPos] > *insertionKey) {
        int movedValue = numbers[*insertionPos];
        numbers[*insertionPos + 1] = numbers[*insertionPos];
        (*insertionPos)--;
        (*statSwaps)++;
        playSwapSound(movedValue, *insertionKey, *insertionPos + 1, *insertionPos + 2);
    } else {
        numbers[*insertionPos + 1] = *insertionKey;
        *insertionHoldingKey = false;
        (*insertionIndex)++;
        if (*insertionIndex > 0 && *insertionIndex - 1 < arraySize) {
            knownSorted[*insertionIndex - 1] = true;
        }

        if (*insertionIndex < arraySize) {
            playSortedSound();
        }
    }

    if (*insertionIndex >= arraySize) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*selectionI >= arraySize - 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*selectionJ < arraySize) {
        (*statComparisons)++;
        playCompareSound(numbers[*selectionJ], numbers[*selectionMin], *selectionJ, *selectionMin);
        if (numbers[*selectionJ] < numbers[*selectionMin]) {
            *selectionMin = *selectionJ;
        }
        (*selectionJ)++;
        return;
    }

    if (*selectionMin != *selectionI) {
        int left = numbers[*selectionI];
        int right = numbers[*selectionMin];
        int temp = numbers[*selectionI];
        numbers[*selectionI] = numbers[*selectionMin];
        numbers[*selectionMin] = temp;
        (*statSwaps)++;
        playSwapSound(left, right, *selectionI, *selectionMin);
    }

    (*selectionI)++;
    if (*selectionI > 0 && *selectionI - 1 < arraySize) {
        knownSorted[*selectionI - 1] = true;
    }
    if (*selectionI < arraySize - 1) {
        playSortedSound();
    }

    *selectionMin = *selectionI;
    *selectionJ = *selectionI + 1;

    if (*selectionI >= arraySize - 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*heapBuilding) {
        if (!*heapSiftActive) {
            if (*heapBuildIndex < 0) {
                *heapBuilding = false;
                *heapSortEnd = arraySize - 1;
                *heapFocusIndex = -1;
                *heapCandidateIndex = -1;
                return;
            }

            heap_start_sift(*heapBuildIndex, arraySize - 1, heapSiftRoot, heapSiftEnd, heapFocusIndex, heapCandidateIndex, heapSiftActive);
            (*heapBuildIndex)--;
            return;
        }
    } else {
        if (*heapSortEnd <= 0) {
            *sortingDone = true;
            knownSorted[0] = true;
            mark_all_sorted(knownSorted, arraySize);
            *heapFocusIndex = -1;
            *heapCandidateIndex = -1;
            startCompletionSweep();
            return;
        }

        if (!*heapSiftActive) {
            int left = numbers[0];
            int right = numbers[*heapSortEnd];
            numbers[0] = right;
            numbers[*heapSortEnd] = left;
            knownSorted[*heapSortEnd] = true;
            (*statSwaps)++;
            playSwapSound(left, right, 0, *heapSortEnd);
            if (*heapSortEnd > 1) {
                playSortedSound();
            }
            (*heapSortEnd)--;

            if (*heapSortEnd <= 0) {
                *sortingDone = true;
                knownSorted[0] = true;
                mark_all_sorted(knownSorted, arraySize);
                *heapFocusIndex = -1;
                *heapCandidateIndex = -1;
                startCompletionSweep();
                return;
            }

            heap_start_sift(0, *heapSortEnd, heapSiftRoot, heapSiftEnd, heapFocusIndex, heapCandidateIndex, heapSiftActive);
            return;
        }
    }

    int root = *heapSiftRoot;
    int left = 2 * root + 1;

    if (left > *heapSiftEnd) {
        *heapSiftActive = false;
        *heapFocusIndex = -1;
        *heapCandidateIndex = -1;
        return;
    }

    int swapIdx = root;
    *heapFocusIndex = root;
    *heapCandidateIndex = root;

    playCompareSound(numbers[swapIdx], numbers[left], swapIdx, left);
    (*statComparisons)++;
    if (numbers[left] > numbers[swapIdx]) {
        swapIdx = left;
        *heapCandidateIndex = left;
    }

    int right = left + 1;
    if (right <= *heapSiftEnd) {
        (*statComparisons)++;
        playCompareSound(numbers[swapIdx], numbers[right], swapIdx, right);
        if (numbers[right] > numbers[swapIdx]) {
            swapIdx = right;
            *heapCandidateIndex = right;
        }
    }

    if (swapIdx != root) {
        int a = numbers[root];
        int b = numbers[swapIdx];
        numbers[root] = b;
        numbers[swapIdx] = a;
        (*statSwaps)++;
        playSwapSound(a, b, root, swapIdx);
        *heapSiftRoot = swapIdx;
        *heapFocusIndex = swapIdx;
    } else {
        *heapSiftActive = false;
        *heapFocusIndex = -1;
        *heapCandidateIndex = -1;
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (!*quickPartitionActive) {
        while (*quickTop >= 0) {
            int low = quickStackLow[*quickTop];
            int high = quickStackHigh[*quickTop];
            (*quickTop)--;

            if (low >= high) {
                if (low == high) {
                    knownSorted[low] = true;
                }
                continue;
            }

            *quickLow = low;
            *quickHigh = high;
            *quickPivotValue = numbers[high];
            *quickPivotIndex = high;
            *quickI = low - 1;
            *quickJ = low;
            *quickPartitionActive = true;
            return;
        }

        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        *quickPivotIndex = -1;
        *quickI = -1;
        *quickJ = -1;
        startCompletionSweep();
        return;
    }

    if (*quickJ < *quickHigh) {
        (*statComparisons)++;
        playCompareSound(numbers[*quickJ], *quickPivotValue, *quickJ, *quickPivotIndex);

        if (numbers[*quickJ] < *quickPivotValue) {
            (*quickI)++;
            if (*quickI != *quickJ) {
                int left = numbers[*quickI];
                int right = numbers[*quickJ];
                numbers[*quickI] = right;
                numbers[*quickJ] = left;
                (*statSwaps)++;
                playSwapSound(left, right, *quickI, *quickJ);
            }
        }

        (*quickJ)++;
        return;
    }

    int pivotPos = *quickI + 1;
    if (pivotPos != *quickHigh) {
        int left = numbers[pivotPos];
        int right = numbers[*quickHigh];
        numbers[pivotPos] = right;
        numbers[*quickHigh] = left;
        (*statSwaps)++;
        playSwapSound(left, right, pivotPos, *quickHigh);
    }
    knownSorted[pivotPos] = true;
    playSortedSound();

    int leftLow = *quickLow;
    int leftHigh = pivotPos - 1;
    int rightLow = pivotPos + 1;
    int rightHigh = *quickHigh;

    if (leftLow < leftHigh) {
        quick_push_range(leftLow, leftHigh, quickStackLow, quickStackHigh, quickTop, quickStackCapacity);
    }
    if (rightLow < rightHigh) {
        quick_push_range(rightLow, rightHigh, quickStackLow, quickStackHigh, quickTop, quickStackCapacity);
    }

    *quickPartitionActive = false;
    *quickPivotIndex = -1;
    *quickI = -1;
    *quickJ = -1;
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*shellGap <= 0) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (!*shellHolding) {
        if (*shellI >= arraySize) {
            *shellGap /= 2;
            if (*shellGap <= 0) {
                *sortingDone = true;
                mark_all_sorted(knownSorted, arraySize);
                startCompletionSweep();
                return;
            }

            *shellI = *shellGap;
            playSortedSound();
            return;
        }

        *shellTemp = numbers[*shellI];
        *shellJ = *shellI;
        *shellHolding = true;
        return;
    }

    if (*shellJ >= *shellGap) {
        int cmpIndex = *shellJ - *shellGap;
        (*statComparisons)++;
        playCompareSound(numbers[cmpIndex], *shellTemp, cmpIndex, *shellJ);

        if (numbers[cmpIndex] > *shellTemp) {
            int movedValue = numbers[cmpIndex];
            numbers[*shellJ] = movedValue;
            (*statSwaps)++;
            playSwapSound(movedValue, *shellTemp, cmpIndex, *shellJ);
            *shellJ -= *shellGap;
            return;
        }
    }

    numbers[*shellJ] = *shellTemp;
    *shellHolding = false;
    (*shellI)++;
}
