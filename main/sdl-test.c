#include <stdio.h>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "graphics.h"
#include "filesystem.h"
#include "text.h"

void draw_screen(SDL_Renderer *renderer, SDL_Texture *texture) {
    if (texture == NULL) {
        return;
    }
    clear_screen(renderer);
    draw_image(renderer, texture, 0, 0, 320.0f, 240.0f);
    SDL_RenderPresent(renderer);

}

Uint32 SDLCALL TimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
    vTaskDelay(pdMS_TO_TICKS(10));

    // printf("Timer callback executed!\n");
    return interval; // Return the interval to keep the timer running
}

void app_main(void) {
    printf("Control Panel for ESP32 with SDL3\n");

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return;
    }
    printf("SDL initialized successfully\n");

    SDL_Window *window = SDL_CreateWindow("SDL on ESP32", 320, 240, 0);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return;
    }

    clear_screen(renderer);

    SDL_InitFS();
    // TestFileOpen("/assets/espressif.bmp");

    TTF_Font *font = initialize_font("/assets/FreeSans.ttf", 12);
    if (!font) return;

    // Create a repeating timer with a 1-second interval
    SDL_TimerID timer_id = SDL_AddTimer(1000, TimerCallback, NULL);
    if (timer_id == 0) {
        printf("Failed to create timer: %s\n", SDL_GetError());
    } else {
        printf("Timer created successfully\n");
    }

#ifndef CONFIG_IDF_TARGET_ESP32P4
    SDL_Texture *textTexture = render_text(renderer, font, "Tomik", (SDL_Color){255, 255, 255, 255});
#endif
    SDL_Texture *dashboard_texture = LoadBackgroundImage(renderer, "/assets/screen-dashboard.bmp");
    SDL_Texture *controls_texture = LoadBackgroundImage(renderer, "/assets/screen-controls.bmp");
    SDL_Texture *apps_texture = LoadBackgroundImage(renderer, "/assets/screen-apps.bmp");
    SDL_Texture *settings_texture = LoadBackgroundImage(renderer, "/assets/screen-settings.bmp");
    SDL_Texture *thermostat_texture = LoadBackgroundImage(renderer, "/assets/screen-thermostat.bmp");

    SDL_Texture *current_texture = dashboard_texture;

    draw_screen(renderer, current_texture);

    SDL_Event event;

    while (1) {
        bool should_draw = false;
        // Poll all events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    // Handle quit event (if needed)
                    break;

                case SDL_EVENT_FINGER_UP:
                    printf("Fineger up\n");
                    should_draw = true;
                    current_texture = apps_texture;
                    break;

                // Handle other events if necessary
            }
        }

        if (should_draw) {
           draw_screen(renderer, current_texture);
        }

        vTaskDelay(pdMS_TO_TICKS(100));

    }
}
