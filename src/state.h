#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "app_types.h"

typedef struct PresetSettings {
    SortMode sortMode;
    float speedMultiplier;
    bool compareAudioEnabled;
    bool swapAudioEnabled;
    bool progressAudioEnabled;
    bool finishAudioEnabled;
    bool showValues;
    bool showLegend;
    bool showHud;
    float masterVolume;
} PresetSettings;

void preset_apply_defaults(PresetSettings *settings);
bool preset_save_to_file(const char *path, const PresetSettings *settings);
bool preset_load_from_file(const char *path, PresetSettings *settings);
void preset_clamp(PresetSettings *settings);

#endif
