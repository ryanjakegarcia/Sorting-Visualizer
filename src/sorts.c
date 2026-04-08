#include "sorts.h"
#include <stdlib.h>
#include <string.h>

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1 || *mergeWidth <= 0 || *mergeWidth >= arraySize) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        *mergeActive = false;
        *mergeCopying = false;
        startCompletionSweep();
        return;
    }

    if (!*mergeActive) {
        if (*mergeLeft >= arraySize) {
            *mergeWidth *= 2;
            *mergeLeft = 0;
            if (*mergeWidth < arraySize) {
                playSortedSound();
            }
            return;
        }

        *mergeMid = *mergeLeft + *mergeWidth;
        if (*mergeMid > arraySize) {
            *mergeMid = arraySize;
        }

        *mergeRight = *mergeLeft + 2 * (*mergeWidth);
        if (*mergeRight > arraySize) {
            *mergeRight = arraySize;
        }

        if (*mergeMid >= *mergeRight) {
            *mergeLeft += 2 * (*mergeWidth);
            return;
        }

        *mergeI = *mergeLeft;
        *mergeJ = *mergeMid;
        *mergeK = *mergeLeft;
        *mergeCopyIndex = *mergeLeft;
        *mergeActive = true;
        *mergeCopying = false;
        return;
    }

    if (!*mergeCopying) {
        if (*mergeI < *mergeMid && *mergeJ < *mergeRight) {
            (*statComparisons)++;
            playCompareSound(numbers[*mergeI], numbers[*mergeJ], *mergeI, *mergeJ);
        }

        if (*mergeI < *mergeMid && (*mergeJ >= *mergeRight || numbers[*mergeI] <= numbers[*mergeJ])) {
            mergeBuffer[*mergeK] = numbers[*mergeI];
            (*mergeI)++;
        } else if (*mergeJ < *mergeRight) {
            mergeBuffer[*mergeK] = numbers[*mergeJ];
            (*mergeJ)++;
        }
        (*mergeK)++;

        if (*mergeK >= *mergeRight) {
            *mergeCopying = true;
            *mergeCopyIndex = *mergeLeft;
        }
        return;
    }

    if (*mergeCopyIndex < *mergeRight) {
        int oldValue = numbers[*mergeCopyIndex];
        int newValue = mergeBuffer[*mergeCopyIndex];
        numbers[*mergeCopyIndex] = newValue;
        if (oldValue != newValue) {
            (*statSwaps)++;
            playSwapSound(oldValue, newValue, *mergeCopyIndex, *mergeCopyIndex);
        }
        (*mergeCopyIndex)++;
        return;
    }

    *mergeActive = false;
    *mergeCopying = false;
    *mergeLeft += 2 * (*mergeWidth);
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*cocktailStart >= *cocktailEnd) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*cocktailForward) {
        if (*cocktailIndex < *cocktailEnd) {
            int leftIndex = *cocktailIndex;
            int rightIndex = *cocktailIndex + 1;
            (*statComparisons)++;
            playCompareSound(numbers[leftIndex], numbers[rightIndex], leftIndex, rightIndex);
            if (numbers[leftIndex] > numbers[rightIndex]) {
                int left = numbers[leftIndex];
                int right = numbers[rightIndex];
                numbers[leftIndex] = right;
                numbers[rightIndex] = left;
                (*statSwaps)++;
                *cocktailSwapped = true;
                playSwapSound(left, right, leftIndex, rightIndex);
            }
            (*cocktailIndex)++;
            return;
        }

        knownSorted[*cocktailEnd] = true;
        if (!*cocktailSwapped) {
            *sortingDone = true;
            mark_all_sorted(knownSorted, arraySize);
            startCompletionSweep();
            return;
        }

        *cocktailSwapped = false;
        *cocktailForward = false;
        (*cocktailEnd)--;
        *cocktailIndex = *cocktailEnd;
        if (*cocktailEnd > *cocktailStart) {
            playSortedSound();
        }
        return;
    }

    if (*cocktailIndex > *cocktailStart) {
        int leftIndex = *cocktailIndex - 1;
        int rightIndex = *cocktailIndex;
        (*statComparisons)++;
        playCompareSound(numbers[leftIndex], numbers[rightIndex], leftIndex, rightIndex);
        if (numbers[leftIndex] > numbers[rightIndex]) {
            int left = numbers[leftIndex];
            int right = numbers[rightIndex];
            numbers[leftIndex] = right;
            numbers[rightIndex] = left;
            (*statSwaps)++;
            *cocktailSwapped = true;
            playSwapSound(left, right, leftIndex, rightIndex);
        }
        (*cocktailIndex)--;
        return;
    }

    knownSorted[*cocktailStart] = true;
    (*cocktailStart)++;
    if (!*cocktailSwapped) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    *cocktailSwapped = false;
    *cocktailForward = true;
    *cocktailIndex = *cocktailStart;
    if (*cocktailStart < *cocktailEnd) {
        playSortedSound();
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1 || *gnomeIndex >= arraySize) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*gnomeIndex <= 0) {
        *gnomeIndex = 1;
        return;
    }

    int leftIndex = *gnomeIndex - 1;
    int rightIndex = *gnomeIndex;
    (*statComparisons)++;
    playCompareSound(numbers[leftIndex], numbers[rightIndex], leftIndex, rightIndex);

    if (numbers[rightIndex] >= numbers[leftIndex]) {
        knownSorted[leftIndex] = true;
        (*gnomeIndex)++;
        if (*gnomeIndex < arraySize) {
            playSortedSound();
        }
        if (*gnomeIndex >= arraySize) {
            *sortingDone = true;
            mark_all_sorted(knownSorted, arraySize);
            startCompletionSweep();
        }
        return;
    }

    int left = numbers[leftIndex];
    int right = numbers[rightIndex];
    numbers[leftIndex] = right;
    numbers[rightIndex] = left;
    (*statSwaps)++;
    playSwapSound(left, right, leftIndex, rightIndex);
    (*gnomeIndex)--;
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*combGap < 1) {
        *combGap = 1;
    }

    if (*combIndex + *combGap >= arraySize) {
        if (*combGap == 1) {
            if (!*combSwapped) {
                *sortingDone = true;
                mark_all_sorted(knownSorted, arraySize);
                startCompletionSweep();
                return;
            }

            *combSwapped = false;
            *combIndex = 0;
            return;
        }

        int nextGap = (*combGap * 10) / 13;
        if (nextGap < 1) {
            nextGap = 1;
        }
        *combGap = nextGap;
        *combIndex = 0;
        *combSwapped = false;
        playSortedSound();
        return;
    }

    int leftIndex = *combIndex;
    int rightIndex = *combIndex + *combGap;
    (*statComparisons)++;
    playCompareSound(numbers[leftIndex], numbers[rightIndex], leftIndex, rightIndex);

    if (numbers[leftIndex] > numbers[rightIndex]) {
        int left = numbers[leftIndex];
        int right = numbers[rightIndex];
        numbers[leftIndex] = right;
        numbers[rightIndex] = left;
        (*statSwaps)++;
        *combSwapped = true;
        playSwapSound(left, right, leftIndex, rightIndex);
    }

    (*combIndex)++;
}

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
)
{
    (void)timSortEnd;  // Currently unused but kept for API consistency
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    // Phase 1: Insertion sort on runs
    if (*timInsertActive) {
        if (*timSortIndex >= arraySize) {
            *timInsertActive = false;
            *timMergeActive = true;
            *timMergeWidth = *timRunSize;
            *timMergeIndex = 0;
            playSortedSound();
            return;
        }

        int runEnd = (*timSortIndex + *timRunSize <= arraySize) ? *timSortIndex + *timRunSize : arraySize;
        
        for (int i = *timSortIndex; i < runEnd; i++) {
            int key = numbers[i];
            int j = i - 1;
            
            while (j >= *timSortIndex) {
                (*statComparisons)++;
                playCompareSound(numbers[j], key, j, i);
                
                if (numbers[j] > key) {
                    numbers[j + 1] = numbers[j];
                    (*statSwaps)++;
                    playSwapSound(numbers[j], key, j, j + 1);
                    j--;
                } else {
                    break;
                }
            }
            
            if (j + 1 != i) {
                numbers[j + 1] = key;
                (*statSwaps)++;
            }
        }
        
        *timSortIndex += *timRunSize;
        return;
    }

    // Phase 2: Merge sorted runs
    if (*timMergeActive) {
        if (*timMergeIndex >= arraySize) {
            if (*timMergeWidth >= arraySize) {
                *sortingDone = true;
                mark_all_sorted(knownSorted, arraySize);
                startCompletionSweep();
                return;
            }
            
            *timMergeWidth *= 2;
            *timMergeIndex = 0;
            playSortedSound();
            return;
        }

        *timLeft = *timMergeIndex;
        *timMid = *timLeft + *timMergeWidth;
        if (*timMid > arraySize) *timMid = arraySize;
        *timRight = *timMid + *timMergeWidth;
        if (*timRight > arraySize) *timRight = arraySize;

        if (*timMid >= arraySize) {
            *timMergeIndex += *timMergeWidth * 2;
            return;
        }

        // Merge two runs: [left, mid) and [mid, right)
        *timI = *timLeft;
        *timJ = *timMid;
        *timK = 0;

        while (*timI < *timMid && *timJ < *timRight) {
            (*statComparisons)++;
            playCompareSound(numbers[*timI], numbers[*timJ], *timI, *timJ);
            
            if (numbers[*timI] <= numbers[*timJ]) {
                timBuffer[(*timK)++] = numbers[(*timI)++];
            } else {
                timBuffer[(*timK)++] = numbers[(*timJ)++];
            }
        }

        while (*timI < *timMid) {
            timBuffer[(*timK)++] = numbers[(*timI)++];
        }

        while (*timJ < *timRight) {
            timBuffer[(*timK)++] = numbers[(*timJ)++];
        }

        for (int i = 0; i < *timK; i++) {
            numbers[*timLeft + i] = timBuffer[i];
            (*statSwaps)++;
        }

        *timMergeIndex += *timMergeWidth * 2;
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    // Calculate max bits needed on first call
    if (*radixMaxBit == 0 && *radixBit == 0) {
        int maxValue = 0;
        for (int i = 0; i < arraySize; i++) {
            if (numbers[i] > maxValue) {
                maxValue = numbers[i];
            }
        }
        
        // Find highest bit position needed
        int bitsNeeded = 0;
        for (int i = 0; i < 31; i++) {
            if ((maxValue >> i) & 1) {
                bitsNeeded = i + 1;
            }
        }
        *radixMaxBit = (bitsNeeded > 0) ? bitsNeeded : 1;
    }

    // Process one bit position at a time, only up to maxBit
    if (*radixBit >= *radixMaxBit) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    // Count elements with bit 0 and bit 1 at position radixBit
    if (*radixPass == 0) {
        radixCount[0] = 0;
        radixCount[1] = 0;

        for (int i = 0; i < arraySize; i++) {
            int bit = (numbers[i] >> *radixBit) & 1;
            radixCount[bit]++;
            (*statComparisons)++;
            playCompareSound(numbers[i], bit, i, *radixBit);
        }

        *radixPass = 1;
        *radixIndex = 0;
        return;
    }

    // Distribute elements into buffer based on bit
    if (*radixPass == 1) {
        int zeros = 0;
        int ones = radixCount[0];

        for (int i = 0; i < arraySize; i++) {
            int bit = (numbers[i] >> *radixBit) & 1;
            
            if (bit == 0) {
                radixBuffer[zeros++] = numbers[i];
            } else {
                radixBuffer[ones++] = numbers[i];
            }
            
            (*statSwaps)++;
            playSwapSound(numbers[i], bit, i, ones);
        }

        for (int i = 0; i < arraySize; i++) {
            numbers[i] = radixBuffer[i];
        }

        *radixBit += 1;
        *radixPass = 0;
        playSortedSound();
        return;
    }
}

static int random_range(int min, int max)
{
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
}

static void introsort_push_range(
    int low,
    int high,
    int depth,
    int *introStackLow,
    int *introStackHigh,
    int *introStackDepth,
    int *introTop,
    int introStackCapacity
)
{
    if (*introTop >= introStackCapacity - 1) {
        return;
    }

    (*introTop)++;
    introStackLow[*introTop] = low;
    introStackHigh[*introTop] = high;
    introStackDepth[*introTop] = depth;
}

void bogosort_step(
    int *numbers,
    bool *knownSorted,
    int arraySize,
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
)
{
    if (*sortingDone) {
        return;
    }

    int bogoSize = (arraySize < 10) ? arraySize : 10;

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    // Phase 1: Check if first bogoSize elements are sorted
    if (!*bogoIsChecking) {
        *bogoCheckIndex = 0;
        *bogoIsChecking = true;
        (*bogoAttempts)++;
        playSortedSound();
        return;
    }

    // Check one comparison per step
    if (*bogoCheckIndex < bogoSize - 1) {
        (*statComparisons)++;
        playCompareSound(numbers[*bogoCheckIndex], numbers[*bogoCheckIndex + 1], *bogoCheckIndex, *bogoCheckIndex + 1);

        if (numbers[*bogoCheckIndex] > numbers[*bogoCheckIndex + 1]) {
            // Not sorted, need to shuffle
            *bogoIsChecking = false;
            *bogoCheckIndex = 0;
            
            // Fisher-Yates shuffle on first bogoSize elements
            for (int i = bogoSize - 1; i > 0; i--) {
                int j = random_range(0, i);
                int temp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = temp;
                (*statSwaps)++;
                playSwapSound(temp, numbers[i], i, j);
            }
            
            return;
        }

        (*bogoCheckIndex)++;
        return;
    }

    // If we get here, first bogoSize elements are sorted!
    // Mark all as sorted and finish
    *sortingDone = true;
    mark_all_sorted(knownSorted, arraySize);
    startCompletionSweep();
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        *oddEvenIndex = 0;
        *oddEvenStart = 0;
        *oddEvenSwappedThisRound = false;
        startCompletionSweep();
        return;
    }

    if (*oddEvenIndex + 1 < arraySize) {
        int leftIndex = *oddEvenIndex;
        int rightIndex = leftIndex + 1;
        int left = numbers[leftIndex];
        int right = numbers[rightIndex];
        (*statComparisons)++;
        playCompareSound(left, right, leftIndex, rightIndex);

        if (left > right) {
            numbers[leftIndex] = right;
            numbers[rightIndex] = left;
            (*statSwaps)++;
            *oddEvenSwappedThisRound = true;
            playSwapSound(left, right, leftIndex, rightIndex);
        }

        *oddEvenIndex += 2;
        return;
    }

    if (*oddEvenStart == 1) {
        if (!*oddEvenSwappedThisRound) {
            *sortingDone = true;
            mark_all_sorted(knownSorted, arraySize);
            *oddEvenIndex = 0;
            *oddEvenStart = 0;
            startCompletionSweep();
            return;
        }

        *oddEvenSwappedThisRound = false;
        *oddEvenStart = 0;
    } else {
        *oddEvenStart = 1;
    }

    *oddEvenIndex = *oddEvenStart;
    playSortedSound();
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*pancakeCurrentSize <= 1 || arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        *pancakePhase = 0;
        *pancakeFlipIndex = 0;
        startCompletionSweep();
        return;
    }

    if (*pancakePhase == 0) {
        if (*pancakeScanIndex < *pancakeCurrentSize) {
            (*statComparisons)++;
            playCompareSound(numbers[*pancakeScanIndex], numbers[*pancakeMaxIndex], *pancakeScanIndex, *pancakeMaxIndex);
            if (numbers[*pancakeScanIndex] > numbers[*pancakeMaxIndex]) {
                *pancakeMaxIndex = *pancakeScanIndex;
            }
            (*pancakeScanIndex)++;
            return;
        }

        if (*pancakeMaxIndex == *pancakeCurrentSize - 1) {
            knownSorted[*pancakeCurrentSize - 1] = true;
            (*pancakeCurrentSize)--;
            *pancakeMaxIndex = 0;
            *pancakeScanIndex = 1;
            *pancakeFlipIndex = 0;
            *pancakePhase = 0;
            playSortedSound();
            return;
        }

        if (*pancakeMaxIndex > 0) {
            *pancakePhase = 1;
            *pancakeFlipIndex = 0;
            return;
        }

        *pancakePhase = 2;
        *pancakeFlipIndex = 0;
        return;
    }

    if (*pancakePhase == 1 || *pancakePhase == 2) {
        int end = (*pancakePhase == 1) ? *pancakeMaxIndex : (*pancakeCurrentSize - 1);
        int half = (end + 1) / 2;
        if (*pancakeFlipIndex < half) {
            int leftIndex = *pancakeFlipIndex;
            int rightIndex = end - *pancakeFlipIndex;
            int left = numbers[leftIndex];
            int right = numbers[rightIndex];
            numbers[leftIndex] = right;
            numbers[rightIndex] = left;
            (*statSwaps)++;
            playSwapSound(left, right, leftIndex, rightIndex);
            (*pancakeFlipIndex)++;
            return;
        }

        if (*pancakePhase == 1) {
            *pancakePhase = 2;
            *pancakeFlipIndex = 0;
            return;
        }

        knownSorted[*pancakeCurrentSize - 1] = true;
        (*pancakeCurrentSize)--;
        *pancakePhase = 0;
        *pancakeMaxIndex = 0;
        *pancakeScanIndex = 1;
        *pancakeFlipIndex = 0;
        playSortedSound();
    }
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (arraySize <= 1) {
        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        startCompletionSweep();
        return;
    }

    if (*countingPhase == 0) {
        if (*countingIndex < arraySize) {
            int value = numbers[*countingIndex];
            if (value < 1) value = 1;
            if (value > arraySize) value = arraySize;

            if (*countingIndex == 0) {
                *countingMinValue = value;
                *countingMaxValue = value;
            } else {
                (*statComparisons)++;
                playCompareSound(value, *countingMinValue, *countingIndex, *countingIndex);
                if (value < *countingMinValue) {
                    *countingMinValue = value;
                }

                (*statComparisons)++;
                playCompareSound(value, *countingMaxValue, *countingIndex, *countingIndex);
                if (value > *countingMaxValue) {
                    *countingMaxValue = value;
                }
            }

            countingCounts[value]++;
            playCompareSound(value, value, *countingIndex, *countingIndex);
            (*countingIndex)++;
            return;
        }

        *countingPhase = 1;
        *countingWriteValue = *countingMinValue;
        *countingWriteIndex = 0;
        playSortedSound();
        return;
    }

    if (*countingPhase == 1) {
        while (*countingWriteValue <= *countingMaxValue && countingCounts[*countingWriteValue] == 0) {
            (*countingWriteValue)++;
        }

        if (*countingWriteValue > *countingMaxValue || *countingWriteIndex >= arraySize) {
            *countingPhase = 2;
            *countingIndex = 0;
            playSortedSound();
            return;
        }

        countingOutput[*countingWriteIndex] = *countingWriteValue;
        countingCounts[*countingWriteValue]--;
        (*countingWriteIndex)++;
        return;
    }

    if (*countingIndex < arraySize) {
        int oldValue = numbers[*countingIndex];
        numbers[*countingIndex] = countingOutput[*countingIndex];
        knownSorted[*countingIndex] = true;
        (*statSwaps)++;
        playSwapSound(oldValue, numbers[*countingIndex], *countingIndex, *countingIndex);

        if (((*countingIndex + 1) % 16) == 0 || *countingIndex == arraySize - 1) {
            playSortedSound();
        }

        (*countingIndex)++;
        return;
    }

    *sortingDone = true;
    mark_all_sorted(knownSorted, arraySize);
    startCompletionSweep();
}

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
)
{
    if (*sortingDone) {
        return;
    }

    if (*introHeapFallbackActive) {
        heap_sort_step(
            numbers,
            knownSorted,
            arraySize,
            sortingDone,
            introHeapBuildIndex,
            introHeapSortEnd,
            introHeapSiftRoot,
            introHeapSiftEnd,
            introHeapFocusIndex,
            introHeapCandidateIndex,
            introHeapBuilding,
            introHeapSiftActive,
            statComparisons,
            statSwaps,
            playCompareSound,
            playSwapSound,
            playSortedSound,
            startCompletionSweep
        );
        return;
    }

    if (!*introPartitionActive) {
        while (*introTop >= 0) {
            int low = introStackLow[*introTop];
            int high = introStackHigh[*introTop];
            int depth = introStackDepth[*introTop];
            (*introTop)--;

            if (low >= high) {
                if (low == high) {
                    knownSorted[low] = true;
                }
                continue;
            }

            if (depth <= 0) {
                *introHeapFallbackActive = true;
                *introPartitionActive = false;
                *introPivotIndex = -1;
                *introI = -1;
                *introJ = -1;
                *introHeapBuildIndex = (arraySize / 2) - 1;
                *introHeapSortEnd = arraySize - 1;
                *introHeapSiftRoot = 0;
                *introHeapSiftEnd = 0;
                *introHeapFocusIndex = -1;
                *introHeapCandidateIndex = -1;
                *introHeapBuilding = true;
                *introHeapSiftActive = false;
                return;
            }

            *introLow = low;
            *introHigh = high;
            *introDepthLimit = depth;
            *introPivotValue = numbers[high];
            *introPivotIndex = high;
            *introI = low - 1;
            *introJ = low;
            *introPartitionActive = true;
            return;
        }

        *sortingDone = true;
        mark_all_sorted(knownSorted, arraySize);
        *introPivotIndex = -1;
        *introI = -1;
        *introJ = -1;
        startCompletionSweep();
        return;
    }

    if (*introJ < *introHigh) {
        (*statComparisons)++;
        playCompareSound(numbers[*introJ], *introPivotValue, *introJ, *introPivotIndex);

        if (numbers[*introJ] < *introPivotValue) {
            (*introI)++;
            if (*introI != *introJ) {
                int left = numbers[*introI];
                int right = numbers[*introJ];
                numbers[*introI] = right;
                numbers[*introJ] = left;
                (*statSwaps)++;
                playSwapSound(left, right, *introI, *introJ);
            }
        }

        (*introJ)++;
        return;
    }

    int pivotPos = *introI + 1;
    if (pivotPos != *introHigh) {
        int left = numbers[pivotPos];
        int right = numbers[*introHigh];
        numbers[pivotPos] = right;
        numbers[*introHigh] = left;
        (*statSwaps)++;
        playSwapSound(left, right, pivotPos, *introHigh);
    }
    knownSorted[pivotPos] = true;
    playSortedSound();

    int nextDepth = *introDepthLimit - 1;
    int leftLow = *introLow;
    int leftHigh = pivotPos - 1;
    int rightLow = pivotPos + 1;
    int rightHigh = *introHigh;

    if (leftLow < leftHigh) {
        introsort_push_range(leftLow, leftHigh, nextDepth, introStackLow, introStackHigh, introStackDepth, introTop, introStackCapacity);
    }
    if (rightLow < rightHigh) {
        introsort_push_range(rightLow, rightHigh, nextDepth, introStackLow, introStackHigh, introStackDepth, introTop, introStackCapacity);
    }

    *introPartitionActive = false;
    *introPivotIndex = -1;
    *introI = -1;
    *introJ = -1;
}
