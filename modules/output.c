// ---------------------------------------------------------------------------------------------- //

#ifndef output_included

// ---------------------------------------------------------------------------------------------- //

#define output_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3_ttf/SDL_ttf.h>

#include <SDL3/SDL.h>

#include "dictionary.c"

// ---------------------------------------------------------------------------------------------- //

typedef struct output_s {

    char output_text[128];
    
    int output_text_length;

    int output_text_changed;

    float output_text_y; 
    
    int output_is_visible;

    unsigned char output_text_color[4];

    TTF_Text *output_ttf_text;
    
    char predicted_text[64];
    
    unsigned char predicted_text_color[4];
    
    TTF_Text *predicted_ttf_text;

} output_t;

// ---------------------------------------------------------------------------------------------- //

output_t output = {

    .output_text = "WELCOME TO CVEP SPELLER",

    .output_text_length = 23,

    .output_text_changed = 1,

    .output_text_y = 106.0f,

    .output_is_visible = 1,

    .output_text_color = { 0, 255, 0, 255 },

    .output_ttf_text = NULL,
    
    .predicted_text = "",
    
    .predicted_text_color = { 128, 128, 128, 255 },
    
    .predicted_ttf_text = NULL,

};

// ---------------------------------------------------------------------------------------------- //

void render_output(SDL_Renderer *renderer, TTF_TextEngine *text_engine, TTF_Font *font, output_t *output) {

    if (!output -> output_is_visible) return;

    if (output -> output_text_changed || output -> output_ttf_text == NULL) {
        
        if (output -> output_ttf_text != NULL) TTF_DestroyText(output -> output_ttf_text);
        
        output -> output_ttf_text = TTF_CreateText(text_engine, font, output -> output_text, 0);

        if (output -> output_ttf_text != NULL) TTF_SetTextColor(output -> output_ttf_text, output -> output_text_color[0], output -> output_text_color[1], output -> output_text_color[2], output -> output_text_color[3]);
        
        const char *pred = get_prediction(output -> output_text);
        
        strncpy(output -> predicted_text, pred, sizeof(output -> predicted_text) - 1);
        
        output -> predicted_text[sizeof(output -> predicted_text) - 1] = '\0';

        if (output -> output_text_length > 0) {
            
            char last_char = output -> output_text[output -> output_text_length - 1];
            
            if (last_char >= 'a' && last_char <= 'z') {
                
                for (int i = 0; output -> predicted_text[i]; i++) {
                    
                    output -> predicted_text[i] = tolower((unsigned char) output -> predicted_text[i]);
                    
                }
                
            }
            
        }

        if (output -> predicted_ttf_text != NULL) TTF_DestroyText(output -> predicted_ttf_text);
        
        if (strlen(output -> predicted_text) > 0) {
            
            output -> predicted_ttf_text = TTF_CreateText(text_engine, font, output -> predicted_text, 0);
            
            if (output -> predicted_ttf_text != NULL) TTF_SetTextColor(output -> predicted_ttf_text, output -> predicted_text_color[0], output -> predicted_text_color[1], output -> predicted_text_color[2], output -> predicted_text_color[3]);
            
        } 
        else {
            
            output -> predicted_ttf_text = NULL;
            
        }

        output -> output_text_changed = 0;

    }

    int total_width = 0;
    
    int text_height = 0;
    
    int output_width = 0;
    
    int pred_width = 0;

    if (output -> output_ttf_text != NULL) {
        
        TTF_GetTextSize(output -> output_ttf_text, &output_width, &text_height);
        
        total_width += output_width;
        
    }

    if (output -> predicted_ttf_text != NULL) {
        
        int temp_h;
        
        TTF_GetTextSize(output -> predicted_ttf_text, &pred_width, &temp_h);
        
        total_width += pred_width;
        
    }

    int screen_width, screen_height;
    
    SDL_GetRenderOutputSize(renderer, &screen_width, &screen_height);

    float start_x = (screen_width - total_width) / 2.0f;
    
    float start_y = output -> output_text_y - (text_height / 2.0f);

    if (output -> output_ttf_text != NULL) {
        
        TTF_DrawRendererText(output -> output_ttf_text, start_x, start_y);
        
    }

    if (output -> predicted_ttf_text != NULL) {
        
        TTF_DrawRendererText(output -> predicted_ttf_text, start_x + output_width, start_y);
        
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

void accept_prediction(output_t *output) {
    
    if (strlen(output -> predicted_text) > 0) {
        
        size_t pred_len = strlen(output -> predicted_text);
        
        if (output -> output_text_length + pred_len < 127) {
            
            strcat(output -> output_text, output -> predicted_text);
            
            output -> output_text_length += (int) pred_len;
            
            if (output -> output_text_length < 127) {
                
                output -> output_text[output -> output_text_length] = ' ';
                
                output -> output_text[output -> output_text_length + 1] = '\0';
                
                output -> output_text_length++;
                
            }
            
            output -> output_text_changed = 1;
            
            output -> predicted_text[0] = '\0';
            
        }
        
    }
    
}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //