#include "ui.h"

#include <stdio.h>
#include <stdlib.h>

static const float padding = 20.0f;
static const float gap = 4.0f;

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

Rectangle ui_get_size_input_box(int height)
{
    Rectangle box = { 20.0f, (float)height - 56.0f, 260.0f, 34.0f };
    return box;
}

int ui_get_pause_menu_item_count(void)
{
    return 16;
}

static void get_pause_menu_layout(int width, int height, int *menuX, int *menuY, int *menuWidth, int *menuHeight, int *itemLineHeight)
{
    int itemCount = ui_get_pause_menu_item_count();
    int mw = 460;
    int ilh = 30;
    int mh = 96 + itemCount * ilh;
    int mx = (width - mw) / 2;
    int my = (height - mh) / 2;
    if (my < 20) {
        my = 20;
    }

    *menuX = mx;
    *menuY = my;
    *menuWidth = mw;
    *menuHeight = mh;
    *itemLineHeight = ilh;
}

Rectangle ui_get_pause_menu_item_rect(int width, int height, int itemIndex)
{
    int itemCount = ui_get_pause_menu_item_count();
    if (itemIndex < 0 || itemIndex >= itemCount) {
        return (Rectangle){ 0 };
    }

    int menuX = 0;
    int menuY = 0;
    int menuWidth = 0;
    int menuHeight = 0;
    int itemLineHeight = 0;
    get_pause_menu_layout(width, height, &menuX, &menuY, &menuWidth, &menuHeight, &itemLineHeight);

    (void)menuWidth;
    (void)menuHeight;
    return (Rectangle){ (float)(menuX + 20), (float)(menuY + 86 + itemIndex * itemLineHeight), 420.0f, (float)itemLineHeight };
}

static void draw_pause_menu(const UiDrawContext *ctx)
{
    const int itemCount = ui_get_pause_menu_item_count();
    int itemLineHeight = 0;
    const int itemFontSize = 22;

    int menuWidth = 0;
    int menuHeight = 0;
    int menuX = 0;
    int menuY = 0;
    get_pause_menu_layout(ctx->width, ctx->height, &menuX, &menuY, &menuWidth, &menuHeight, &itemLineHeight);

    DrawRectangle(0, 0, ctx->width, ctx->height, Fade(BLACK, 0.55f));
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, Fade(DARKGRAY, 0.90f));
    DrawRectangleLinesEx((Rectangle){ (float)menuX, (float)menuY, (float)menuWidth, (float)menuHeight }, 2.0f, SKYBLUE);
    DrawText("Pause Menu", menuX + 20, menuY + 16, 30, YELLOW);
    DrawText("UP/DOWN + ENTER, SPACE to close", menuX + 20, menuY + 52, 18, LIGHTGRAY);

    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < itemCount; i++) {
        char itemText[96] = { 0 };
        Color c = (i == ctx->pauseMenuSelection) ? YELLOW : RAYWHITE;
        Rectangle itemRect = ui_get_pause_menu_item_rect(ctx->width, ctx->height, i);
        bool hovered = CheckCollisionPointRec(mousePos, itemRect);
        bool pressed = hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        if (hovered) {
            DrawRectangleRec(itemRect, Fade(SKYBLUE, pressed ? 0.35f : 0.20f));
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }

        switch (i) {
            case 0:
                snprintf(itemText, sizeof(itemText), "Resume");
                break;
            case 1:
                snprintf(itemText, sizeof(itemText), "HUD: %s", ctx->showHud ? "ON" : "OFF");
                break;
            case 2:
                snprintf(itemText, sizeof(itemText), "Settings Overlay: %s", ctx->showSettingsOverlay ? "ON" : "OFF");
                break;
            case 3:
                snprintf(itemText, sizeof(itemText), "Legend: %s", ctx->showLegend ? "ON" : "OFF");
                break;
            case 4:
                snprintf(itemText, sizeof(itemText), "Values: %s", ctx->showValues ? "ON" : "OFF");
                break;
            case 5:
                snprintf(itemText, sizeof(itemText), "Telemetry: %s", ctx->showTelemetry ? "ON" : "OFF");
                break;
            case 6:
                snprintf(itemText, sizeof(itemText), "Size Input Box: %s", ctx->showSizeInputBox ? "ON" : "OFF");
                break;
            case 7:
                snprintf(itemText, sizeof(itemText), "Minimal UI: %s", ctx->minimalUiMode ? "ON" : "OFF");
                break;
            case 8:
                snprintf(itemText, sizeof(itemText), "Compare Audio: %s", ctx->compareAudioEnabled ? "ON" : "OFF");
                break;
            case 9:
                snprintf(itemText, sizeof(itemText), "Swap Audio: %s", ctx->swapAudioEnabled ? "ON" : "OFF");
                break;
            case 10:
                snprintf(itemText, sizeof(itemText), "Progress Audio: %s", ctx->progressAudioEnabled ? "ON" : "OFF");
                break;
            case 11:
                snprintf(itemText, sizeof(itemText), "Finish Audio: %s", ctx->finishAudioEnabled ? "ON" : "OFF");
                break;
            case 12:
                snprintf(itemText, sizeof(itemText), "Save Preset");
                break;
            case 13:
                snprintf(itemText, sizeof(itemText), "Load Preset");
                break;
            case 14:
                snprintf(itemText, sizeof(itemText), "Set Array Size");
                break;
            case 15:
                snprintf(itemText, sizeof(itemText), "Close App");
                break;
            default:
                itemText[0] = '\0';
                break;
        }

        DrawText(itemText, menuX + 24, menuY + 88 + i * itemLineHeight, itemFontSize, c);
    }
}

void draw_elements(const UiDrawContext *ctx)
{
    const float availableWidth = (float)ctx->width - 2.0f * padding;
    const float availableHeight = (float)ctx->height - 2.0f * padding;
    float barWidth = (availableWidth - gap * (ctx->arraySize - 1)) / (float)ctx->arraySize;
    bool useLineMode = (ctx->arraySize >= 120 || barWidth < 3.0f);
    float density = (ctx->arraySize > 0) ? ((float)ctx->arraySize / availableWidth) : 0.0f;
    float densityAlphaBase = clampf(0.95f - density * 0.25f, 0.35f, 0.95f);
    float lineThickness = (ctx->arraySize >= 700) ? 1.0f : ((ctx->arraySize >= 300) ? 1.5f : 2.0f);

    for (int i = 0; i < ctx->arraySize; i++) {
        int displayValue = ctx->numbers[i];
        if (ctx->currentSort == SORT_INSERTION && ctx->insertionHoldingKey && i == ctx->insertionIndex) {
            displayValue = ctx->insertionKey;
        }

        float barHeight = ((float)displayValue / (float)ctx->arraySize) * availableHeight;
        float x = padding + i * (barWidth + gap);
        float y = (float)ctx->height - padding - barHeight;

        Color barColor = WHITE;
        if (ctx->completionSweepActive && i == ctx->completionSweepIndex) {
            barColor = YELLOW;
        } else if (ctx->currentSort == SORT_QUICK && !ctx->sortingDone && i == ctx->quickPivotIndex) {
            barColor = ctx->sortColorC;
        } else if (ctx->currentSort == SORT_QUICK && !ctx->sortingDone && i == ctx->quickJ) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_QUICK && !ctx->sortingDone && i == ctx->quickI) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_HEAP && !ctx->sortingDone && i == ctx->heapCandidateIndex) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_HEAP && !ctx->sortingDone && i == ctx->heapFocusIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_BUBBLE && !ctx->sortingDone && (i == ctx->bubbleIndex || i == ctx->bubbleIndex + 1)) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_INSERTION && !ctx->sortingDone && ctx->insertionHoldingKey && i == ctx->insertionPos) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_SHELL && !ctx->sortingDone && ctx->shellHolding && i == ctx->shellJ) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_SHELL && !ctx->sortingDone && ctx->shellHolding && ctx->shellJ - ctx->shellGap >= 0 && i == ctx->shellJ - ctx->shellGap) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_COCKTAIL && !ctx->sortingDone && ctx->cocktailForward && (i == ctx->cocktailIndex || i == ctx->cocktailIndex + 1)) {
            barColor = (i == ctx->cocktailIndex) ? ctx->sortColorA : ctx->sortColorB;
        } else if (ctx->currentSort == SORT_COCKTAIL && !ctx->sortingDone && !ctx->cocktailForward && (i == ctx->cocktailIndex - 1 || i == ctx->cocktailIndex)) {
            barColor = (i == ctx->cocktailIndex - 1) ? ctx->sortColorA : ctx->sortColorB;
        } else if (ctx->currentSort == SORT_GNOME && !ctx->sortingDone && i == ctx->gnomeIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_GNOME && !ctx->sortingDone && ctx->gnomeIndex > 0 && i == ctx->gnomeIndex - 1) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_COMB && !ctx->sortingDone && i == ctx->combIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_COMB && !ctx->sortingDone && i == ctx->combIndex + ctx->combGap) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_TIMSORT && !ctx->sortingDone && ctx->timInsertActive && i >= ctx->timSortIndex - ctx->timRunSize && i < ctx->timSortIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_TIMSORT && !ctx->sortingDone && ctx->timMergeActive && i >= ctx->timLeft && i < ctx->timRight) {
            if (i < ctx->timMid) barColor = ctx->sortColorA;
            else barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_RADIXSORT && !ctx->sortingDone && i < ctx->radixIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_BOGOSORT && !ctx->sortingDone) {
            int bogoSize = ctx->splashPreviewMode ? ctx->arraySize : ((ctx->arraySize < 10) ? ctx->arraySize : 10);
            if (i < bogoSize) {
                barColor = ctx->sortColorA;  // Highlight the range being sorted
            }
        } else if (ctx->currentSort == SORT_ODD_EVEN && !ctx->sortingDone && i == ctx->oddEvenIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_ODD_EVEN && !ctx->sortingDone && i == ctx->oddEvenIndex + 1) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_PANCAKE && !ctx->sortingDone && ctx->pancakePhase == 0 && i == ctx->pancakeScanIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_PANCAKE && !ctx->sortingDone && ctx->pancakePhase == 0 && i == ctx->pancakeMaxIndex) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_PANCAKE && !ctx->sortingDone && (ctx->pancakePhase == 1 || ctx->pancakePhase == 2) && i <= ((ctx->pancakePhase == 1) ? ctx->pancakeMaxIndex : (ctx->pancakeCurrentSize - 1))) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_COUNTING && !ctx->sortingDone && ctx->countingPhase == 0 && i == ctx->countingIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_COUNTING && !ctx->sortingDone && ctx->countingPhase == 2 && i < ctx->countingIndex) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_INTROSORT && !ctx->sortingDone && ctx->introHeapFallbackActive && i == ctx->introHeapCandidateIndex) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_INTROSORT && !ctx->sortingDone && ctx->introHeapFallbackActive && i == ctx->introHeapFocusIndex) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_INTROSORT && !ctx->sortingDone && !ctx->introHeapFallbackActive && i == ctx->introPivotIndex) {
            barColor = ctx->sortColorC;
        } else if (ctx->currentSort == SORT_INTROSORT && !ctx->sortingDone && !ctx->introHeapFallbackActive && i == ctx->introJ) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_INTROSORT && !ctx->sortingDone && !ctx->introHeapFallbackActive && i == ctx->introI) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_MERGE && !ctx->sortingDone && ctx->mergeActive && !ctx->mergeCopying && i == ctx->mergeI) {
            barColor = ctx->sortColorA;
        } else if (ctx->currentSort == SORT_MERGE && !ctx->sortingDone && ctx->mergeActive && !ctx->mergeCopying && i == ctx->mergeJ) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_MERGE && !ctx->sortingDone && ctx->mergeActive && i == ctx->mergeK) {
            barColor = ctx->sortColorC;
        } else if (ctx->currentSort == SORT_MERGE && !ctx->sortingDone && ctx->mergeCopying && i == ctx->mergeCopyIndex) {
            barColor = ctx->sortColorD;
        } else if (ctx->currentSort == SORT_SELECTION && !ctx->sortingDone && i == ctx->selectionMin) {
            barColor = ctx->sortColorB;
        } else if (ctx->currentSort == SORT_SELECTION && !ctx->sortingDone && i == ctx->selectionJ && ctx->selectionJ < ctx->arraySize) {
            barColor = ctx->sortColorA;
        } else if (ctx->knownSorted[i] || ctx->sortingDone || (ctx->currentSort == SORT_BUBBLE && i >= ctx->arraySize - ctx->bubblePass) || (ctx->currentSort == SORT_INSERTION && i < ctx->insertionIndex) || (ctx->currentSort == SORT_SELECTION && i < ctx->selectionI) || (ctx->currentSort == SORT_HEAP && !ctx->heapBuilding && i > ctx->heapSortEnd) || (ctx->currentSort == SORT_PANCAKE && i >= ctx->pancakeCurrentSize)) {
            barColor = GREEN;
        }

        if (useLineMode) {
            float activity = 0.0f;
            if (ctx->arraySize > 1) {
                int prev = (i > 0) ? ctx->numbers[i - 1] : displayValue;
                int next = (i + 1 < ctx->arraySize) ? ctx->numbers[i + 1] : displayValue;
                activity = ((float)abs(displayValue - prev) + (float)abs(displayValue - next)) * 0.5f / (float)ctx->arraySize;
            }
            float alpha = clampf(densityAlphaBase + activity * 0.35f, 0.30f, 1.0f);
            Color lineColor = Fade(barColor, alpha);
            float centerX = x + barWidth * 0.5f;
            DrawLineEx((Vector2){ centerX, (float)ctx->height - padding }, (Vector2){ centerX, y }, lineThickness, lineColor);
        } else {
            Rectangle bar = { x, y, barWidth, barHeight };
            DrawRectangleRec(bar, barColor);
        }

        if (ctx->showValues && ctx->showHud && !useLineMode) {
            int fontSize = 20;
            const char *valueText = TextFormat("%d", displayValue);
            int textWidth = MeasureText(valueText, fontSize);
            int textX = (int)(x + (barWidth - (float)textWidth) * 0.5f);
            int textY = (int)y - fontSize - 4;
            if (textY < 4) textY = 4;
            DrawText(valueText, textX, textY, fontSize, RAYWHITE);
        }
    }

    if (ctx->benchmarkRunning) {
        int bannerWidth = 560;
        int bannerHeight = 34;
        int bannerX = (ctx->width - bannerWidth) / 2;
        int bannerY = 12;
        if (bannerX < 10) bannerX = 10;
        DrawRectangle(bannerX, bannerY, bannerWidth, bannerHeight, Fade(DARKBLUE, 0.85f));
        DrawRectangleLinesEx((Rectangle){ (float)bannerX, (float)bannerY, (float)bannerWidth, (float)bannerHeight }, 2.0f, SKYBLUE);
        DrawText(ctx->benchmarkStatusText != NULL ? ctx->benchmarkStatusText : "BENCHMARK RUNNING", bannerX + 10, bannerY + 8, 20, YELLOW);
    }

    if (!ctx->showHud) {
        if (ctx->pauseMenuActive) {
            draw_pause_menu(ctx);
        }
        return;
    }

    if (ctx->minimalUiMode) {
        DrawText(TextFormat("Sort: %s", ctx->sortName), 20, 20, 30, YELLOW);
        DrawText(TextFormat("Size: %d", ctx->arraySize), 20, 56, 30, SKYBLUE);
        if (ctx->pauseMenuActive) {
            draw_pause_menu(ctx);
        }
        return;
    }

    if (ctx->showSizeInputBox) {
        Rectangle sizeBox = ui_get_size_input_box(ctx->height);
        DrawRectangleRec(sizeBox, Fade(DARKGRAY, 0.35f));
        DrawRectangleLinesEx(sizeBox, 2.0f, ctx->sizeInputActive ? YELLOW : GRAY);
        DrawText("Array Size:", 30, (int)sizeBox.y + 7, 20, RAYWHITE);
        DrawText(ctx->sizeInputActive ? ctx->sizeInput : TextFormat("%d", ctx->arraySize), 145, (int)sizeBox.y + 7, 20, SKYBLUE);
        DrawText(TextFormat("(2-%d, Enter to apply)", ctx->maxSize), 290, (int)sizeBox.y + 8, 18, LIGHTGRAY);
    }

    DrawText(TextFormat("Sort: %s (TAB)", ctx->sortName), 20, 20, 30, YELLOW);
    DrawText(TextFormat("Speed: %.1fx", ctx->speedMultiplier), 20, 56, 30, YELLOW);
    DrawText(TextFormat("Size: %d", ctx->arraySize), 20, 92, 24, SKYBLUE);
    DrawText(TextFormat("Dist: %s [D]", ctx->distributionName), 20, 122, 24, SKYBLUE);
    DrawText(TextFormat("Vol: %.2f [ / ]", ctx->masterVolume), 20, 152, 24, SKYBLUE);

    if (ctx->showSettingsOverlay) {
        DrawText(TextFormat("Audio C:%s S:%s P:%s F:%s", ctx->compareAudioEnabled ? "ON" : "OFF", ctx->swapAudioEnabled ? "ON" : "OFF", ctx->progressAudioEnabled ? "ON" : "OFF", ctx->finishAudioEnabled ? "ON" : "OFF"), 20, 182, 20, LIGHTGRAY);
        DrawText(TextFormat("Render: %s", useLineMode ? "LINES" : "BARS"), 20, 207, 20, LIGHTGRAY);
        DrawText(TextFormat("Step [M]: %s  One [N]", ctx->stepMode ? "ON" : "OFF"), 20, 232, 20, LIGHTGRAY);
        DrawText(TextFormat("Settings [G]: %s", ctx->showSettingsOverlay ? "ON" : "OFF"), 20, 257, 20, LIGHTGRAY);
        DrawText(TextFormat("Values [V]: %s", ctx->showValues ? "ON" : "OFF"), 20, 282, 20, LIGHTGRAY);
        DrawText(TextFormat("Minimal [U]: %s", ctx->minimalUiMode ? "ON" : "OFF"), 20, 307, 20, LIGHTGRAY);
        DrawText(TextFormat("Legend [L]: %s", ctx->showLegend ? "ON" : "OFF"), 20, 332, 20, LIGHTGRAY);
        DrawText(TextFormat("HUD [H]: %s", ctx->showHud ? "ON" : "OFF"), 20, 357, 20, LIGHTGRAY);
        DrawText(TextFormat("Telemetry [T]: %s", ctx->showTelemetry ? "ON" : "OFF"), 20, 382, 20, LIGHTGRAY);
        DrawText(TextFormat("Size Box [B]: %s", ctx->showSizeInputBox ? "ON" : "OFF"), 20, 407, 20, LIGHTGRAY);
    }
    DrawText(TextFormat("Stats  cmp:%llu  swp:%llu  steps:%llu", ctx->statComparisons, ctx->statSwaps, ctx->statSteps), 360, 20, 20, LIGHTGRAY);
    DrawText(TextFormat("Time: %.2fs  Steps/s: %.1f", ctx->statElapsed, (ctx->statElapsed > 0.0f) ? ((float)ctx->statSteps / ctx->statElapsed) : 0.0f), 360, 40, 20, LIGHTGRAY);

    if (ctx->showTelemetry) {
        int telemetryX = 360;
        int telemetryY = 72;
        DrawText("Telemetry", telemetryX, telemetryY, 22, SKYBLUE);

        if (ctx->telemetryLine1 != NULL && ctx->telemetryLine1[0] != '\0') {
            DrawText(ctx->telemetryLine1, telemetryX, telemetryY + 26, 20, LIGHTGRAY);
        }
        if (ctx->telemetryLine2 != NULL && ctx->telemetryLine2[0] != '\0') {
            DrawText(ctx->telemetryLine2, telemetryX, telemetryY + 50, 20, LIGHTGRAY);
        }
        if (ctx->telemetryLine3 != NULL && ctx->telemetryLine3[0] != '\0') {
            DrawText(ctx->telemetryLine3, telemetryX, telemetryY + 74, 20, LIGHTGRAY);
        }
    }

    if (ctx->showLegend) {
        int legendX = ctx->width - 280;
        int legendY = 20;
        DrawText("Legend", legendX, legendY, 24, SKYBLUE);

        if (ctx->currentSort == SORT_BUBBLE) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Compare Pair", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_INSERTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorB);
            DrawText("Insert Compare", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Prefix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_SELECTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Scan Index", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Current Min", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Finalized Prefix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_SHELL) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Gap Compare", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Gap Insert Pos", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted (final)", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_COCKTAIL) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Forward/Left", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Forward/Right", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Bounds", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_GNOME) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Current", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Previous", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Prefix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_COMB) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Index", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Gap Partner", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Final", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_TIMSORT) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Current Run", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Merge Area", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Runs", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_RADIXSORT) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Processed", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Current Bit", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted by Bit", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_BOGOSORT) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Bogo Range (max 10)", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("During Shuffle", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Checking Order", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_ODD_EVEN) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Current Pair Left", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Current Pair Right", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Final", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_PANCAKE) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Active Prefix", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Current Max", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Locked Suffix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_COUNTING) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Scan / Count Input", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Copied Back Writes", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Prefix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_INTROSORT) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Scan j / Heap Root", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Partition i / Candidate", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, ctx->sortColorC);
            DrawText("Pivot", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_MERGE) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Left Candidate", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Right Candidate", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, ctx->sortColorC);
            DrawText("Merge Write", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, ctx->sortColorD);
            DrawText("Copy Back", legendX + 28, legendY + 116, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 146, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 144, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_HEAP) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Sift Root", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Largest Candidate", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else {
            DrawRectangle(legendX, legendY + 34, 18, 18, ctx->sortColorA);
            DrawText("Scan j", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ctx->sortColorB);
            DrawText("Partition i", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, ctx->sortColorC);
            DrawText("Pivot", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        }
    }

    int messageY = 365;
    if (ctx->paused) {
        DrawText("PAUSED", 20, messageY, 30, YELLOW);
        messageY += 36;
    }

    if (ctx->sortingDone && !ctx->completionSweepActive) {
        float remaining = ctx->autoNextSortDelay - ctx->autoNextSortTimer;
        if (remaining < 0.0f) remaining = 0.0f;
        int timerX = ctx->width - 560;
        if (timerX < 20) timerX = 20;
        DrawText(TextFormat("Next Sort In: %.1fs", remaining), timerX, 64, 24, SKYBLUE);
    }

    if (ctx->statusTimer > 0.0f) {
        DrawText(ctx->statusText, 20, messageY, 22, SKYBLUE);
        messageY += 28;
    }

    if (ctx->showInfo) {
        int infoY = messageY + 8;
        DrawText("Hotkeys (hold I)", 20, infoY, 24, SKYBLUE);
        DrawText("SPACE: Pause/Resume", 20, infoY + 30, 20, LIGHTGRAY);
        DrawText("UP/DOWN: Speed +/-", 20, infoY + 55, 20, LIGHTGRAY);
        DrawText("TAB: Cycle Sort", 20, infoY + 80, 20, LIGHTGRAY);
        DrawText("D: Cycle Distribution", 20, infoY + 105, 20, LIGHTGRAY);
        DrawText("1/2/3: Size 200/500/1000", 20, infoY + 130, 20, LIGHTGRAY);
        DrawText("M: Toggle Step Mode", 20, infoY + 155, 20, LIGHTGRAY);
        DrawText("N: Single Step", 20, infoY + 180, 20, LIGHTGRAY);
        DrawText("U: Toggle Minimal UI", 20, infoY + 205, 20, LIGHTGRAY);
        DrawText("R: Reshuffle/Restart", 20, infoY + 230, 20, LIGHTGRAY);
        DrawText("K: Save Preset", 20, infoY + 255, 20, LIGHTGRAY);
        DrawText("O: Load Preset", 20, infoY + 280, 20, LIGHTGRAY);
        DrawText(TextFormat("V: Values [%s]", ctx->showValues ? "ON" : "OFF"), 20, infoY + 305, 20, LIGHTGRAY);
        DrawText(TextFormat("L: Legend [%s]", ctx->showLegend ? "ON" : "OFF"), 20, infoY + 330, 20, LIGHTGRAY);
        DrawText(TextFormat("H: HUD [%s]", ctx->showHud ? "ON" : "OFF"), 20, infoY + 355, 20, LIGHTGRAY);
        DrawText(TextFormat("G: Settings Overlay [%s]", ctx->showSettingsOverlay ? "ON" : "OFF"), 20, infoY + 380, 20, LIGHTGRAY);
        DrawText(TextFormat("T: Telemetry [%s]", ctx->showTelemetry ? "ON" : "OFF"), 20, infoY + 405, 20, LIGHTGRAY);
        DrawText(TextFormat("B: Size Box [%s]", ctx->showSizeInputBox ? "ON" : "OFF"), 20, infoY + 430, 20, LIGHTGRAY);
        DrawText(TextFormat("C: Compare [%s]", ctx->compareAudioEnabled ? "ON" : "OFF"), 20, infoY + 455, 20, LIGHTGRAY);
        DrawText(TextFormat("S: Swap [%s]", ctx->swapAudioEnabled ? "ON" : "OFF"), 20, infoY + 480, 20, LIGHTGRAY);
        DrawText(TextFormat("P: Progress [%s]", ctx->progressAudioEnabled ? "ON" : "OFF"), 20, infoY + 505, 20, LIGHTGRAY);
        DrawText(TextFormat("F: Finish [%s]", ctx->finishAudioEnabled ? "ON" : "OFF"), 20, infoY + 530, 20, LIGHTGRAY);
        DrawText("[ / ]: Master Volume", 20, infoY + 555, 20, LIGHTGRAY);
        DrawText("X: Benchmark Suite", 20, infoY + 580, 20, LIGHTGRAY);
        DrawText("Benchmark cfg: Z seed on/off, W warmup, -/+ runs, ,/. seed, E export CSV", 20, infoY + 605, 20, LIGHTGRAY);
    }

    if (ctx->pauseMenuActive) {
        draw_pause_menu(ctx);
    }
}
