#include "controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

#include "ui.h"

static int sanitize_array_size(int requested, int maxSize, bool *usedDefault)
{
    if (requested < 2 || requested > maxSize) {
        *usedDefault = true;
        return 50;
    }

    *usedDefault = false;
    return requested;
}

void controller_handle_input(ControllerContext *ctx, float dt)
{
    Rectangle sizeBox = ui_get_size_input_box(ctx->windowHeight);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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

        if (IsKeyPressed(KEY_ESCAPE)) {
            *ctx->sizeInputActive = false;
        }
    }

    if (!*ctx->sizeInputActive) {
        if (IsKeyPressed(KEY_SPACE)) {
            *ctx->paused = !*ctx->paused;
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
