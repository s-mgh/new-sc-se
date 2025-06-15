#include "screen.h"  // برای دسترسی به sc_screen_render_without_border
#include "sshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "util/log.h"
#include "screen.h"

extern struct sc_screen *global_screen;

void generate_screenshot_filename(char *filename, size_t size) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(filename, size, "screenshot_%04d%02d%02d_%02d%02d%02d.bmp",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void take_screenshot(const char *filename) {
    if (!global_screen || !global_screen->window) {
        LOGW("No screen or window available for screenshot");
        return;
    }

    SDL_Renderer *renderer = SDL_GetRenderer(global_screen->window);
    if (!renderer) {
        LOGE("Could not get renderer: %s", SDL_GetError());
        return;
    }

    int width = global_screen->selection_rect.w;
    int height = global_screen->selection_rect.h;
    if (width <= 0 || height <= 0) {
        LOGW("Invalid selection rectangle, using full screen");
        width = global_screen->rect.w;
        height = global_screen->rect.h;
        global_screen->selection_rect.x = 0;
        global_screen->selection_rect.y = 0;
    }

    // ذخیره وضعیت فعلی رندر
    SDL_Rect original_rect = global_screen->selection_rect;

    // رندر بدون کادر سبز
    sc_screen_render_without_border(global_screen);
    SDL_RenderPresent(renderer); // اعمال تغییرات رندر

    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        LOGE("Could not create surface: %s", SDL_GetError());
        return;
    }

    // تنظیم دقیق مختصات برای خواندن پیکسل‌ها
    SDL_Rect read_rect = {
        .x = global_screen->selection_rect.x,
        .y = global_screen->selection_rect.y,
        .w = width,
        .h = height
    };
    LOGD("Reading pixels from rect %d,%d %dx%d", read_rect.x, read_rect.y, read_rect.w, read_rect.h);

    if (SDL_RenderReadPixels(renderer, &read_rect, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch) != 0) {
        LOGE("Could not read render pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    if (SDL_SaveBMP(surface, filename) != 0) {
        LOGE("Could not save screenshot: %s", SDL_GetError());
    } else {
        LOGI("Screenshot saved as %s", filename);
    }

    SDL_FreeSurface(surface);

    // بازگرداندن وضعیت رندر به حالت اولیه
    global_screen->selection_rect = original_rect;
    sc_screen_render(global_screen, false);
    SDL_RenderPresent(renderer);
}

void capture_screenshot(void) {
    char filename[64];
    generate_screenshot_filename(filename, sizeof(filename));
    take_screenshot(filename);
}