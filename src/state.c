#include "state.h"

#include <stdio.h>
#include <string.h>

void preset_apply_defaults(PresetSettings *settings)
{
    settings->sortMode = SORT_BUBBLE;
    settings->speedMultiplier = 1.0f;
    settings->compareAudioEnabled = true;
    settings->swapAudioEnabled = true;
    settings->progressAudioEnabled = true;
    settings->finishAudioEnabled = true;
    settings->showValues = false;
    settings->showLegend = true;
    settings->showHud = true;
    settings->showSettingsOverlay = true;
    settings->showTelemetry = true;
    settings->showSizeInputBox = true;
    settings->masterVolume = 0.8f;
}

bool preset_save_to_file(const char *path, const PresetSettings *settings)
{
    FILE *f = fopen(path, "w");
    if (!f) {
        return false;
    }

    fprintf(f, "sort %d\n", (int)settings->sortMode);
    fprintf(f, "speed %.4f\n", settings->speedMultiplier);
    fprintf(f, "compare %d\n", settings->compareAudioEnabled ? 1 : 0);
    fprintf(f, "swap %d\n", settings->swapAudioEnabled ? 1 : 0);
    fprintf(f, "progress %d\n", settings->progressAudioEnabled ? 1 : 0);
    fprintf(f, "finish %d\n", settings->finishAudioEnabled ? 1 : 0);
    fprintf(f, "values %d\n", settings->showValues ? 1 : 0);
    fprintf(f, "legend %d\n", settings->showLegend ? 1 : 0);
    fprintf(f, "hud %d\n", settings->showHud ? 1 : 0);
    fprintf(f, "settings %d\n", settings->showSettingsOverlay ? 1 : 0);
    fprintf(f, "telemetry %d\n", settings->showTelemetry ? 1 : 0);
    fprintf(f, "sizebox %d\n", settings->showSizeInputBox ? 1 : 0);
    fprintf(f, "volume %.4f\n", settings->masterVolume);

    fclose(f);
    return true;
}

bool preset_load_from_file(const char *path, PresetSettings *settings)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        return false;
    }

    char key[32];
    while (fscanf(f, "%31s", key) == 1) {
        if (strcmp(key, "sort") == 0) {
            int mode = 0;
            if (fscanf(f, "%d", &mode) == 1 && mode >= SORT_BUBBLE && mode <= SORT_INTROSORT) {
                settings->sortMode = (SortMode)mode;
            }
        } else if (strcmp(key, "speed") == 0) {
            float value = settings->speedMultiplier;
            if (fscanf(f, "%f", &value) == 1) {
                settings->speedMultiplier = value;
            }
        } else if (strcmp(key, "compare") == 0) {
            int v = settings->compareAudioEnabled ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->compareAudioEnabled = (v != 0);
        } else if (strcmp(key, "swap") == 0) {
            int v = settings->swapAudioEnabled ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->swapAudioEnabled = (v != 0);
        } else if (strcmp(key, "progress") == 0) {
            int v = settings->progressAudioEnabled ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->progressAudioEnabled = (v != 0);
        } else if (strcmp(key, "finish") == 0) {
            int v = settings->finishAudioEnabled ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->finishAudioEnabled = (v != 0);
        } else if (strcmp(key, "values") == 0) {
            int v = settings->showValues ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showValues = (v != 0);
        } else if (strcmp(key, "legend") == 0) {
            int v = settings->showLegend ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showLegend = (v != 0);
        } else if (strcmp(key, "hud") == 0) {
            int v = settings->showHud ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showHud = (v != 0);
        } else if (strcmp(key, "settings") == 0) {
            int v = settings->showSettingsOverlay ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showSettingsOverlay = (v != 0);
        } else if (strcmp(key, "telemetry") == 0) {
            int v = settings->showTelemetry ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showTelemetry = (v != 0);
        } else if (strcmp(key, "sizebox") == 0) {
            int v = settings->showSizeInputBox ? 1 : 0;
            if (fscanf(f, "%d", &v) == 1) settings->showSizeInputBox = (v != 0);
        } else if (strcmp(key, "volume") == 0) {
            float v = settings->masterVolume;
            if (fscanf(f, "%f", &v) == 1) settings->masterVolume = v;
        }
    }

    fclose(f);
    preset_clamp(settings);
    return true;
}

void preset_clamp(PresetSettings *settings)
{
    if (settings->speedMultiplier < 0.1f) settings->speedMultiplier = 0.1f;
    if (settings->speedMultiplier > 10.0f) settings->speedMultiplier = 10.0f;
    if (settings->masterVolume < 0.0f) settings->masterVolume = 0.0f;
    if (settings->masterVolume > 1.0f) settings->masterVolume = 1.0f;
}
