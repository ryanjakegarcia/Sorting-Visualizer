#include "ui.h"

static const float padding = 20.0f;
static const float gap = 4.0f;

Rectangle ui_get_size_input_box(int height)
{
    Rectangle box = { 20.0f, (float)height - 56.0f, 260.0f, 34.0f };
    return box;
}

void draw_elements(const UiDrawContext *ctx)
{
    const float availableWidth = (float)ctx->width - 2.0f * padding;
    const float availableHeight = (float)ctx->height - 2.0f * padding;
    float barWidth = (availableWidth - gap * (ctx->arraySize - 1)) / (float)ctx->arraySize;

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
            barColor = ORANGE;
        } else if (ctx->currentSort == SORT_QUICK && !ctx->sortingDone && i == ctx->quickJ) {
            barColor = RED;
        } else if (ctx->currentSort == SORT_QUICK && !ctx->sortingDone && i == ctx->quickI) {
            barColor = PINK;
        } else if (ctx->currentSort == SORT_HEAP && !ctx->sortingDone && i == ctx->heapCandidateIndex) {
            barColor = ORANGE;
        } else if (ctx->currentSort == SORT_HEAP && !ctx->sortingDone && i == ctx->heapFocusIndex) {
            barColor = RED;
        } else if (ctx->currentSort == SORT_BUBBLE && !ctx->sortingDone && (i == ctx->bubbleIndex || i == ctx->bubbleIndex + 1)) {
            barColor = RED;
        } else if (ctx->currentSort == SORT_INSERTION && !ctx->sortingDone && ctx->insertionHoldingKey && i == ctx->insertionPos) {
            barColor = ORANGE;
        } else if (ctx->currentSort == SORT_SELECTION && !ctx->sortingDone && i == ctx->selectionMin) {
            barColor = ORANGE;
        } else if (ctx->currentSort == SORT_SELECTION && !ctx->sortingDone && i == ctx->selectionJ && ctx->selectionJ < ctx->arraySize) {
            barColor = RED;
        } else if (ctx->knownSorted[i] || ctx->sortingDone || (ctx->currentSort == SORT_BUBBLE && i >= ctx->arraySize - ctx->bubblePass) || (ctx->currentSort == SORT_INSERTION && i < ctx->insertionIndex) || (ctx->currentSort == SORT_SELECTION && i < ctx->selectionI) || (ctx->currentSort == SORT_HEAP && !ctx->heapBuilding && i > ctx->heapSortEnd)) {
            barColor = GREEN;
        }

        Rectangle bar = { x, y, barWidth, barHeight };
        DrawRectangleRec(bar, barColor);

        if (ctx->showValues && ctx->showHud) {
            int fontSize = 20;
            const char *valueText = TextFormat("%d", displayValue);
            int textWidth = MeasureText(valueText, fontSize);
            int textX = (int)(x + (barWidth - (float)textWidth) * 0.5f);
            int textY = (int)y - fontSize - 4;
            if (textY < 4) textY = 4;
            DrawText(valueText, textX, textY, fontSize, RAYWHITE);
        }
    }

    if (!ctx->showHud) {
        return;
    }

    Rectangle sizeBox = ui_get_size_input_box(ctx->height);
    DrawRectangleRec(sizeBox, Fade(DARKGRAY, 0.35f));
    DrawRectangleLinesEx(sizeBox, 2.0f, ctx->sizeInputActive ? YELLOW : GRAY);
    DrawText("Array Size:", 30, (int)sizeBox.y + 7, 20, RAYWHITE);
    DrawText(ctx->sizeInputActive ? ctx->sizeInput : TextFormat("%d", ctx->arraySize), 145, (int)sizeBox.y + 7, 20, SKYBLUE);
    DrawText(TextFormat("(2-%d, Enter to apply)", ctx->maxSize), 290, (int)sizeBox.y + 8, 18, LIGHTGRAY);

    DrawText(TextFormat("Sort: %s (TAB)", ctx->sortName), 20, 20, 30, YELLOW);
    DrawText(TextFormat("Speed: %.1fx", ctx->speedMultiplier), 20, 56, 30, YELLOW);
    DrawText(TextFormat("Size: %d", ctx->arraySize), 20, 92, 24, SKYBLUE);
    DrawText(TextFormat("Dist: %s [D]", ctx->distributionName), 20, 122, 24, SKYBLUE);
    DrawText(TextFormat("Vol: %.2f [ / ]", ctx->masterVolume), 20, 152, 24, SKYBLUE);
    DrawText(TextFormat("Audio C:%s S:%s P:%s F:%s", ctx->compareAudioEnabled ? "ON" : "OFF", ctx->swapAudioEnabled ? "ON" : "OFF", ctx->progressAudioEnabled ? "ON" : "OFF", ctx->finishAudioEnabled ? "ON" : "OFF"), 20, 182, 20, LIGHTGRAY);
    DrawText(TextFormat("Values [V]: %s", ctx->showValues ? "ON" : "OFF"), 20, 207, 20, LIGHTGRAY);
    DrawText(TextFormat("Legend [L]: %s", ctx->showLegend ? "ON" : "OFF"), 20, 232, 20, LIGHTGRAY);
    DrawText(TextFormat("HUD [H]: %s", ctx->showHud ? "ON" : "OFF"), 20, 257, 20, LIGHTGRAY);
    DrawText(TextFormat("Stats  cmp:%llu  swp:%llu  steps:%llu", ctx->statComparisons, ctx->statSwaps, ctx->statSteps), 360, 20, 20, LIGHTGRAY);
    DrawText(TextFormat("Time: %.2fs  Steps/s: %.1f", ctx->statElapsed, (ctx->statElapsed > 0.0f) ? ((float)ctx->statSteps / ctx->statElapsed) : 0.0f), 360, 40, 20, LIGHTGRAY);

    if (ctx->showLegend) {
        int legendX = ctx->width - 280;
        int legendY = 20;
        DrawText("Legend", legendX, legendY, 24, SKYBLUE);

        if (ctx->currentSort == SORT_BUBBLE) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Compare Pair", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_INSERTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, ORANGE);
            DrawText("Insert Compare", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, GREEN);
            DrawText("Sorted Prefix", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 88, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_SELECTION) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Scan Index", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ORANGE);
            DrawText("Current Min", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Finalized Prefix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else if (ctx->currentSort == SORT_HEAP) {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Sift Root", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, ORANGE);
            DrawText("Largest Candidate", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, GREEN);
            DrawText("Sorted Suffix", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        } else {
            DrawRectangle(legendX, legendY + 34, 18, 18, RED);
            DrawText("Scan j", legendX + 28, legendY + 32, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 62, 18, 18, PINK);
            DrawText("Partition i", legendX + 28, legendY + 60, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 90, 18, 18, ORANGE);
            DrawText("Pivot", legendX + 28, legendY + 88, 20, LIGHTGRAY);
            DrawRectangle(legendX, legendY + 118, 18, 18, YELLOW);
            DrawText("Completion Sweep", legendX + 28, legendY + 116, 20, LIGHTGRAY);
        }
    }

    int messageY = 290;
    if (ctx->paused) {
        DrawText("PAUSED", 20, messageY, 30, YELLOW);
        messageY += 36;
    }

    if (ctx->sortingDone && !ctx->completionSweepActive) {
        float remaining = ctx->autoNextSortDelay - ctx->autoNextSortTimer;
        if (remaining < 0.0f) remaining = 0.0f;
        DrawText(TextFormat("Next Sort In: %.1fs", remaining), 20, messageY, 24, SKYBLUE);
        messageY += 30;
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
        DrawText("R: Reshuffle/Restart", 20, infoY + 130, 20, LIGHTGRAY);
        DrawText("K: Save Preset", 20, infoY + 155, 20, LIGHTGRAY);
        DrawText("O: Load Preset", 20, infoY + 180, 20, LIGHTGRAY);
        DrawText(TextFormat("V: Values [%s]", ctx->showValues ? "ON" : "OFF"), 20, infoY + 205, 20, LIGHTGRAY);
        DrawText(TextFormat("L: Legend [%s]", ctx->showLegend ? "ON" : "OFF"), 20, infoY + 230, 20, LIGHTGRAY);
        DrawText(TextFormat("H: HUD [%s]", ctx->showHud ? "ON" : "OFF"), 20, infoY + 255, 20, LIGHTGRAY);
        DrawText(TextFormat("C: Compare [%s]", ctx->compareAudioEnabled ? "ON" : "OFF"), 20, infoY + 280, 20, LIGHTGRAY);
        DrawText(TextFormat("S: Swap [%s]", ctx->swapAudioEnabled ? "ON" : "OFF"), 20, infoY + 305, 20, LIGHTGRAY);
        DrawText(TextFormat("P: Progress [%s]", ctx->progressAudioEnabled ? "ON" : "OFF"), 20, infoY + 330, 20, LIGHTGRAY);
        DrawText(TextFormat("F: Finish [%s]", ctx->finishAudioEnabled ? "ON" : "OFF"), 20, infoY + 355, 20, LIGHTGRAY);
        DrawText("[ / ]: Master Volume", 20, infoY + 380, 20, LIGHTGRAY);
    }
}
