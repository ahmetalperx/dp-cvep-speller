// ---------------------------------------------------------------------------------------------- //

#ifndef background_included

// ---------------------------------------------------------------------------------------------- //

#define background_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------------------------- //

typedef struct background_s {

    unsigned char background_color[4];

} background_t;

// ---------------------------------------------------------------------------------------------- //

background_t background = {

    .background_color = { 0, 0, 0, 255 },

};

// ---------------------------------------------------------------------------------------------- //

void render_background(SDL_Renderer *renderer, background_t *background) {

    SDL_SetRenderDrawColor(renderer, background -> background_color[0], background -> background_color[1], background -> background_color[2], background -> background_color[3]);

    SDL_RenderClear(renderer);

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //