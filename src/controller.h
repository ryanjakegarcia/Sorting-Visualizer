#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>

typedef void (*ControllerCycleSortFn)(void);
typedef void (*ControllerCycleDistributionFn)(void);
typedef void (*ControllerResetSortFn)(void);
typedef void (*ControllerSavePresetFn)(float speedMultiplier);
typedef bool (*ControllerLoadPresetFn)(float *speedMultiplier);
typedef void (*ControllerApplyArraySizeFn)(int newSize, bool usedDefault);

typedef struct ControllerContext {
    int windowHeight;
    int maxSize;

    int *arraySize;
    bool *sizeInputActive;
    char *sizeInput;
    int sizeInputCapacity;
    int *sizeInputLen;

    bool *paused;
    bool *pausedBeforeMenu;
    bool *pauseMenuActive;
    int *pauseMenuSelection;
    bool *requestClose;
    bool *stepMode;
    bool *stepOnceRequested;
    bool *showValues;
    bool *showLegend;
    bool *showHud;
    bool *minimalUiMode;
    bool *compareAudioEnabled;
    bool *swapAudioEnabled;
    bool *progressAudioEnabled;
    bool *finishAudioEnabled;

    float *masterVolume;
    float masterVolumeStep;
    float *speedMultiplier;
    float speedStep;
    float *stepTimer;

    ControllerCycleSortFn cycleSort;
    ControllerCycleDistributionFn cycleDistribution;
    ControllerResetSortFn resetSort;
    ControllerSavePresetFn savePreset;
    ControllerLoadPresetFn loadPreset;
    ControllerApplyArraySizeFn applyArraySize;
} ControllerContext;

void controller_handle_input(ControllerContext *ctx, float dt);

#endif
