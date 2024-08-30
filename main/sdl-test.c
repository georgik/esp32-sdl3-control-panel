#include <stdio.h>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "graphics.h"
#include "filesystem.h"
#include "text.h"

typedef struct {
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    SDL_Texture *current_texture;
    SDL_Texture *next_texture;
} TransitionRegion;

void slide_transition(SDL_Renderer *renderer, SDL_Texture *current_texture, SDL_Texture *next_texture, int duration_ms) {
    int steps = duration_ms / 10;
    for (int i = 0; i <= steps; i++) {
        float progress = (float)i / steps;
        int offset = (int)(320 * progress);  // Assuming screen width is 320

        // Render current texture sliding out
        clear_screen(renderer);
        draw_image(renderer, current_texture, -offset, 0, 320.0f, 240.0f);

        // Render next texture sliding in
        draw_image(renderer, next_texture, 320 - offset, 0, 320.0f, 240.0f);

        SDL_RenderPresent(renderer);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

SDL_Texture* handle_transition(int x, int y, SDL_Texture *current_texture, TransitionRegion *transitions, int num_transitions) {
    for (int i = 0; i < num_transitions; i++) {
        if (transitions[i].current_texture == current_texture &&
            x >= transitions[i].x_min && x <= transitions[i].x_max &&
            y >= transitions[i].y_min && y <= transitions[i].y_max) {
            return transitions[i].next_texture;
        }
    }
    return current_texture;
}

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

    TransitionRegion transitions[] = {
        {200, 0, 320, 240, dashboard_texture, controls_texture},
        {0, 120, 200, 240, dashboard_texture, apps_texture},
        {0, 0, 320, 240, controls_texture, dashboard_texture},
        {0, 0, 320, 240, apps_texture, settings_texture},
        {0, 0, 320, 240, settings_texture, dashboard_texture}
    };

    int num_transitions = sizeof(transitions) / sizeof(transitions[0]);

    draw_screen(renderer, current_texture);

    SDL_Event event;

    while (1) {
        bool should_draw = false;
        SDL_Texture *new_texture = current_texture;
        // Poll all events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    // Handle quit event (if needed)
                    break;

                case SDL_EVENT_FINGER_UP:
                    int x = event.tfinger.x;
                    int y = event.tfinger.y;

                    printf("Finger up [%i, %i]\n", x, y);

                    new_texture = handle_transition(x, y, current_texture, transitions, num_transitions);
                    if (new_texture != current_texture) {
                        slide_transition(renderer, current_texture, new_texture, 300); // 500ms slide
                        current_texture = new_texture;
                    }
                    break;

                // Handle other events if necessary
            }
        }

        if (new_texture != current_texture) {
            draw_screen(renderer, current_texture);
        }

        vTaskDelay(pdMS_TO_TICKS(100));

    }
}
