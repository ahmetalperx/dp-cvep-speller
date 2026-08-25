// ---------------------------------------------------------------------------------------------- //

#ifndef photodiode_included

// ---------------------------------------------------------------------------------------------- //

#define photodiode_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------------------------- //

typedef enum photodiode_state_e {

    photodiode_state_off,
    
    photodiode_state_on,

} photodiode_state_t;

// ---------------------------------------------------------------------------------------------- //

typedef struct photodiode_s {

    float photodiode_x;

    float photodiode_y;

    float photodiode_width;

    float photodiode_height;

    int photodiode_is_visible;

    photodiode_state_t photodiode_state;

    unsigned char photodiode_state_off_color[4];

    unsigned char photodiode_state_on_color[4];

} photodiode_t;

// ---------------------------------------------------------------------------------------------- //

photodiode_t photodiode_refresh_rate = {

    .photodiode_x = 0.0,

    .photodiode_y = 0.0,

    .photodiode_width = 64.0,

    .photodiode_height = 64.0,

    .photodiode_is_visible = 1,

    .photodiode_state = photodiode_state_on,

    .photodiode_state_off_color = { 0, 0, 0, 255 },

    .photodiode_state_on_color = { 255, 255, 255, 255 },

};

// ---------------------------------------------------------------------------------------------- //

photodiode_t photodiode_presentation_rate = {

    .photodiode_x = 1856.0,

    .photodiode_y = 0.0,

    .photodiode_width = 64.0,

    .photodiode_height = 64.0,

    .photodiode_is_visible = 1,

    .photodiode_state = photodiode_state_on,

    .photodiode_state_off_color = { 0, 0, 0, 255 },

    .photodiode_state_on_color = { 255, 255, 255, 255 },

};

// ---------------------------------------------------------------------------------------------- //

void render_photodiode(SDL_Renderer *renderer, photodiode_t *photodiode) {

    if (!photodiode -> photodiode_is_visible) return;

    unsigned char *photodiode_color;

    if (photodiode -> photodiode_state == photodiode_state_off) photodiode_color = photodiode -> photodiode_state_off_color;

    if (photodiode -> photodiode_state == photodiode_state_on) photodiode_color = photodiode -> photodiode_state_on_color;

    SDL_SetRenderDrawColor(renderer, photodiode_color[0], photodiode_color[1], photodiode_color[2], photodiode_color[3]);

    SDL_RenderFillRect(renderer, &(SDL_FRect) { photodiode -> photodiode_x, photodiode -> photodiode_y, photodiode -> photodiode_width, photodiode -> photodiode_height });

}

// ---------------------------------------------------------------------------------------------- //

void toggle_photodiode_visibility(photodiode_t *photodiode) {

    photodiode -> photodiode_is_visible = !photodiode -> photodiode_is_visible;

}

// ---------------------------------------------------------------------------------------------- //

void set_photodiode_state(photodiode_t *photodiode, photodiode_state_t state) {

    photodiode -> photodiode_state = state;

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //