// ---------------------------------------------------------------------------------------------- //

#ifndef output_included

// ---------------------------------------------------------------------------------------------- //

#define output_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3_ttf/SDL_ttf.h>

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------------------------- //

typedef struct output_s {

    char output_text[128];
    
    int output_text_length;

    int output_text_changed;

    float output_text_y; 
    
    int output_is_visible;

    unsigned char output_text_color[4];

    TTF_Text *output_ttf_text;

} output_t;

// ---------------------------------------------------------------------------------------------- //

output_t output = {

    .output_text = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG",

    .output_text_length = 43,

    .output_text_changed = 1,

    .output_text_y = 106.0f,

    .output_is_visible = 1,

    .output_text_color = { 0, 255, 0, 255 },

    .output_ttf_text = NULL,

};

// ---------------------------------------------------------------------------------------------- //

void render_output(SDL_Renderer *renderer, TTF_TextEngine *text_engine, TTF_Font *font, output_t *output) {

    if (!output -> output_is_visible) return;

    if (output -> output_text_changed || output -> output_ttf_text == NULL) {
        
        if (output -> output_ttf_text != NULL) TTF_DestroyText(output -> output_ttf_text);
        
        output -> output_ttf_text = TTF_CreateText(text_engine, font, output -> output_text, 0);

        if (output -> output_ttf_text != NULL) TTF_SetTextColor(output -> output_ttf_text, output -> output_text_color[0], output -> output_text_color[1], output -> output_text_color[2], output -> output_text_color[3]);
        
        output -> output_text_changed = 0;

    }

    if (output -> output_ttf_text != NULL) {
        
        int text_width, text_height;
        
        TTF_GetTextSize(output -> output_ttf_text, &text_width, &text_height);
        
        int screen_width, screen_height;
        
        SDL_GetRenderOutputSize(renderer, &screen_width, &screen_height);
        
        TTF_DrawRendererText(output -> output_ttf_text, (screen_width - text_width) / 2.0f, output -> output_text_y - (text_height / 2.0f));

    }

}

// ---------------------------------------------------------------------------------------------- //

void toggle_output_visibility(output_t *output) {

    output -> output_is_visible = !output -> output_is_visible;

}

// ---------------------------------------------------------------------------------------------- //

void add_letter_to_output(output_t *output, char key) {

    if (output -> output_text_length < 127) {
        
        output -> output_text[output -> output_text_length] = key;

        output -> output_text[output -> output_text_length + 1] = '\0';

        output -> output_text_length++;

        output -> output_text_changed = 1;

    }

}

// ---------------------------------------------------------------------------------------------- //

void add_space_to_output(output_t *output) {
    
    if (output -> output_text_length < 127) {
        
        output -> output_text[output -> output_text_length] = ' ';

        output -> output_text[output -> output_text_length + 1] = '\0';

        output -> output_text_length++;

        output -> output_text_changed = 1;

    }
    
}

// ---------------------------------------------------------------------------------------------- //

void delete_letter_from_output(output_t *output) {

    if (output -> output_text_length > 0) {
        
        output -> output_text_length--;

        output -> output_text[output -> output_text_length] = '\0';

        output -> output_text_changed = 1;

    }

}

// ---------------------------------------------------------------------------------------------- //

void clear_output(output_t *output) {

    output -> output_text[0] = '\0';

    output -> output_text_length = 0;

    output -> output_text_changed = 1;

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //