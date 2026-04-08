#include "controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

#include "ui.h"

enum {
    MENU_RESUME = 0,
    MENU_TOGGLE_HUD,
    MENU_TOGGLE_SETTINGS_OVERLAY,
    MENU_TOGGLE_LEGEND,
    MENU_TOGGLE_VALUES,
    MENU_TOGGLE_TELEMETRY,
    MENU_TOGGLE_SIZE_BOX,
    MENU_TOGGLE_MINIMAL,
    MENU_TOGGLE_COMPARE_AUDIO,
    MENU_TOGGLE_SWAP_AUDIO,
    MENU_TOGGLE_PROGRESS_AUDIO,
    MENU_TOGGLE_FINISH_AUDIO,
    MENU_SAVE_PRESET,
    MENU_LOAD_PRESET,
    MENU_SET_ARRAY_SIZE,
    MENU_CLOSE_APP,
    MENU_ITEM_COUNT
};

static int sanitize_array_size(int requested, int maxSize, bool *usedDefault)
{
    if (requested < 2 || requested > maxSize) {
        *usedDefault = true;
        return 50;
    }

    *usedDefault = false;
    return requested;
}

static void execute_pause_menu_action(ControllerContext *ctx)
{
    switch (*ctx->pauseMenuSelection) {
        case MENU_RESUME:
            *ctx->pauseMenuActive = false;
            *ctx->paused = *ctx->pausedBeforeMenu;
            break;
        case MENU_TOGGLE_HUD:
            *ctx->showHud = !*ctx->showHud;
            if (!*ctx->showHud) *ctx->sizeInputActive = false;
            break;
        case MENU_TOGGLE_SETTINGS_OVERLAY:
            *ctx->showSettingsOverlay = !*ctx->showSettingsOverlay;
            break;
        case MENU_TOGGLE_LEGEND:
            *ctx->showLegend = !*ctx->showLegend;
            break;
        case MENU_TOGGLE_VALUES:
            *ctx->showValues = !*ctx->showValues;
            break;
        case MENU_TOGGLE_TELEMETRY:
            *ctx->showTelemetry = !*ctx->showTelemetry;
            break;
        case MENU_TOGGLE_SIZE_BOX:
            *ctx->showSizeInputBox = !*ctx->showSizeInputBox;
            if (!*ctx->showSizeInputBox) *ctx->sizeInputActive = false;
            break;
        case MENU_TOGGLE_MINIMAL:
            *ctx->minimalUiMode = !*ctx->minimalUiMode;
            if (*ctx->minimalUiMode) *ctx->sizeInputActive = false;
            break;
        case MENU_TOGGLE_COMPARE_AUDIO:
            *ctx->compareAudioEnabled = !*ctx->compareAudioEnabled;
            break;
        case MENU_TOGGLE_SWAP_AUDIO:
            *ctx->swapAudioEnabled = !*ctx->swapAudioEnabled;
            break;
        case MENU_TOGGLE_PROGRESS_AUDIO:
            *ctx->progressAudioEnabled = !*ctx->progressAudioEnabled;
            break;
        case MENU_TOGGLE_FINISH_AUDIO:
            *ctx->finishAudioEnabled = !*ctx->finishAudioEnabled;
            break;
        case MENU_SAVE_PRESET:
            if (ctx->savePreset != NULL) {
                ctx->savePreset(*ctx->speedMultiplier);
            }
            break;
        case MENU_LOAD_PRESET: {
            bool loaded = false;
            if (ctx->loadPreset != NULL) {
                loaded = ctx->loadPreset(ctx->speedMultiplier);
            }
            if (loaded && ctx->resetSort != NULL) {
                ctx->resetSort();
                *ctx->stepTimer = 0.0f;
                *ctx->pauseMenuActive = false;
                *ctx->paused = *ctx->pausedBeforeMenu;
            }
            break;
        }
        case MENU_SET_ARRAY_SIZE:
            *ctx->sizeInputActive = true;
            snprintf(ctx->sizeInput, (size_t)ctx->sizeInputCapacity, "%d", *ctx->arraySize);
            *ctx->sizeInputLen = (int)strlen(ctx->sizeInput);
            break;
        case MENU_CLOSE_APP:
            *ctx->requestClose = true;
            break;
        default:
            break;
    }
}

void controller_handle_input(ControllerContext *ctx, float dt)
{
    if (IsKeyPressed(KEY_SPACE) && !*ctx->sizeInputActive && !ctx->benchmarkOverlayActive) {
        if (!*ctx->pauseMenuActive) {
            *ctx->pausedBeforeMenu = *ctx->paused;
            *ctx->paused = true;
            *ctx->pauseMenuSelection = 0;
            *ctx->pauseMenuActive = true;
        } else {
            *ctx->pauseMenuActive = false;
            *ctx->paused = *ctx->pausedBeforeMenu;
        }
    }

    bool allowSizeBoxInteraction = *ctx->showHud && *ctx->showSizeInputBox && !*ctx->minimalUiMode && !*ctx->pauseMenuActive && !ctx->benchmarkOverlayActive;
    Rectangle sizeBox = ui_get_size_input_box(ctx->windowHeight);
    if (allowSizeBoxInteraction && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, sizeBox)) {
            *ctx->sizeInputActive = true;
            snprintf(ctx->sizeInput, (size_t)ctx->sizeInputCapacity, "%d", *ctx->arraySize);
            *ctx->sizeInputLen = (int)strlen(ctx->sizeInput);
        } else {
            *ctx->sizeInputActive = false;
        }
    }

    if (*ctx->sizeInputActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= '0' && key <= '9' && *ctx->sizeInputLen < ctx->sizeInputCapacity - 1) {
                ctx->sizeInput[*ctx->sizeInputLen] = (char)key;
                (*ctx->sizeInputLen)++;
                ctx->sizeInput[*ctx->sizeInputLen] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && *ctx->sizeInputLen > 0) {
            (*ctx->sizeInputLen)--;
            ctx->sizeInput[*ctx->sizeInputLen] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER)) {
            bool usedDefault = false;
            int requested = atoi(ctx->sizeInput);
            int newSize = sanitize_array_size(requested, ctx->maxSize, &usedDefault);
            if (ctx->applyArraySize != NULL) {
                ctx->applyArraySize(newSize, usedDefault);
            }
            *ctx->stepTimer = 0.0f;
            *ctx->sizeInputActive = false;
        }

    }

    if (*ctx->pauseMenuActive && !*ctx->sizeInputActive) {
        int itemCount = ui_get_pause_menu_item_count();
        Vector2 mousePos = GetMousePosition();
        int hoveredIndex = -1;
        for (int i = 0; i < itemCount; i++) {
            Rectangle itemRect = ui_get_pause_menu_item_rect(ctx->windowWidth, ctx->windowHeight, i);
            if (CheckCollisionPointRec(mousePos, itemRect)) {
                hoveredIndex = i;
                *ctx->pauseMenuSelection = i;
                break;
            }
        }

        if (IsKeyPressed(KEY_UP)) {
            (*ctx->pauseMenuSelection)--;
            if (*ctx->pauseMenuSelection < 0) *ctx->pauseMenuSelection = MENU_ITEM_COUNT - 1;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            (*ctx->pauseMenuSelection)++;
            if (*ctx->pauseMenuSelection >= MENU_ITEM_COUNT) *ctx->pauseMenuSelection = 0;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            execute_pause_menu_action(ctx);
        }

        if (hoveredIndex >= 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            execute_pause_menu_action(ctx);
        }
    }

    if (!*ctx->sizeInputActive && !*ctx->pauseMenuActive && !ctx->benchmarkOverlayActive) {
        if (IsKeyPressed(KEY_M)) {
            *ctx->stepMode = !*ctx->stepMode;
            *ctx->stepOnceRequested = false;
        }

        if (IsKeyPressed(KEY_N) && (*ctx->paused || *ctx->stepMode)) {
            *ctx->stepOnceRequested = true;
        }

        if (IsKeyPressed(KEY_V)) {
            *ctx->showValues = !*ctx->showValues;
        }

        if (IsKeyPressed(KEY_L)) {
            *ctx->showLegend = !*ctx->showLegend;
        }

        if (IsKeyPressed(KEY_H)) {
            *ctx->showHud = !*ctx->showHud;
        }

        if (IsKeyPressed(KEY_G)) {
            *ctx->showSettingsOverlay = !*ctx->showSettingsOverlay;
        }

        if (IsKeyPressed(KEY_T)) {
            *ctx->showTelemetry = !*ctx->showTelemetry;
        }

        if (IsKeyPressed(KEY_B)) {
            *ctx->showSizeInputBox = !*ctx->showSizeInputBox;
            if (!*ctx->showSizeInputBox) {
                *ctx->sizeInputActive = false;
            }
        }

        if (IsKeyPressed(KEY_U)) {
            *ctx->minimalUiMode = !*ctx->minimalUiMode;
            if (*ctx->minimalUiMode) {
                *ctx->sizeInputActive = false;
            }
        }

        if (IsKeyPressed(KEY_C)) {
            *ctx->compareAudioEnabled = !*ctx->compareAudioEnabled;
        }

        if (IsKeyPressed(KEY_S)) {
            *ctx->swapAudioEnabled = !*ctx->swapAudioEnabled;
        }

        if (IsKeyPressed(KEY_P)) {
            *ctx->progressAudioEnabled = !*ctx->progressAudioEnabled;
        }

        if (IsKeyPressed(KEY_F)) {
            *ctx->finishAudioEnabled = !*ctx->finishAudioEnabled;
        }

        if (IsKeyDown(KEY_RIGHT_BRACKET)) {
            *ctx->masterVolume += ctx->masterVolumeStep * dt;
        }

        if (IsKeyDown(KEY_LEFT_BRACKET)) {
            *ctx->masterVolume -= ctx->masterVolumeStep * dt;
        }

        if (IsKeyPressed(KEY_TAB)) {
            if (ctx->cycleSort != NULL) {
                ctx->cycleSort();
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_D)) {
            if (ctx->cycleDistribution != NULL) {
                ctx->cycleDistribution();
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_ONE)) {
            if (ctx->applyArraySize != NULL) {
                ctx->applyArraySize(200, false);
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_TWO)) {
            if (ctx->applyArraySize != NULL) {
                ctx->applyArraySize(500, false);
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_THREE)) {
            if (ctx->applyArraySize != NULL) {
                ctx->applyArraySize(1000, false);
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_R)) {
            if (ctx->resetSort != NULL) {
                ctx->resetSort();
            }
            *ctx->stepTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_K)) {
            if (ctx->savePreset != NULL) {
                ctx->savePreset(*ctx->speedMultiplier);
            }
        }

        if (IsKeyPressed(KEY_O)) {
            bool loaded = false;
            if (ctx->loadPreset != NULL) {
                loaded = ctx->loadPreset(ctx->speedMultiplier);
            }
            if (loaded) {
                if (ctx->resetSort != NULL) {
                    ctx->resetSort();
                }
                *ctx->stepTimer = 0.0f;
            }
        }

        if (IsKeyDown(KEY_UP)) *ctx->speedMultiplier += ctx->speedStep;
        if (IsKeyDown(KEY_DOWN)) *ctx->speedMultiplier -= ctx->speedStep;
    }

    if (*ctx->masterVolume < 0.0f) *ctx->masterVolume = 0.0f;
    if (*ctx->masterVolume > 1.0f) *ctx->masterVolume = 1.0f;
    if (*ctx->speedMultiplier < 0.1f) *ctx->speedMultiplier = 0.1f;
    if (*ctx->speedMultiplier > 10.0f) *ctx->speedMultiplier = 10.0f;
}
