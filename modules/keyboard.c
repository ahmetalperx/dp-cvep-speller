// ---------------------------------------------------------------------------------------------- //

#ifndef keyboard_included

// ---------------------------------------------------------------------------------------------- //

#define keyboard_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3_ttf/SDL_ttf.h>

#include <SDL3/SDL.h>

#include <stdlib.h>

#include "lsl.c"

#include <stdio.h>

// ---------------------------------------------------------------------------------------------- //

unsigned char sequence_matrix_buffer[256 * 256];

int load_sequence_from_txt(const char* filepath, int* out_num_keys, int* out_num_bits) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open sequence file: %s\n", filepath);
        return 0;
    }
    int rows = 0;
    int cols = 0;
    int current_cols = 0;
    int total_elements = 0;
    int ch;
    int last_char_was_newline = 1;
    
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '0' || ch == '1') {
            current_cols++;
            last_char_was_newline = 0;
        } else if (ch == '\n') {
            if (!last_char_was_newline) {
                rows++;
                if (cols == 0) cols = current_cols;
                current_cols = 0;
                last_char_was_newline = 1;
            }
        }
    }
    if (!last_char_was_newline) {
        rows++;
        if (cols == 0) cols = current_cols;
    }
    if (rows == 0 || cols == 0) {
        printf("Empty or invalid sequence file.\n");
        fclose(file);
        return 0;
    }
    *out_num_keys = rows;
    *out_num_bits = cols;

    rewind(file);
    total_elements = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '0') {
            sequence_matrix_buffer[total_elements++] = 0;
        } else if (ch == '1') {
            sequence_matrix_buffer[total_elements++] = 1;
        }
    }
    fclose(file);
    printf("Successfully loaded sequence from %s (%d keys, %d bits)\n", filepath, rows, cols);
    return 1;
}

// ---------------------------------------------------------------------------------------------- //

typedef struct key_s { 
    
    char key_letter; 
    
    float key_x; 
    
    float key_y; 
    
    float key_width; 
    
    float key_height; 
    
    float key_border_thickness; 
    
    TTF_Text *key_ttf_text;

} key_t;

// ---------------------------------------------------------------------------------------------- //

typedef enum keyboard_state_e {

    keyboard_state_idle,

    keyboard_state_cue,

    keyboard_state_flashing,

    keyboard_state_feedback

} keyboard_state_t;

// ---------------------------------------------------------------------------------------------- //

typedef enum keyboard_mode_e {

    keyboard_mode_training,

    keyboard_mode_online

} keyboard_mode_t;

// ---------------------------------------------------------------------------------------------- //

typedef struct keyboard_key_colors_s {
    
    unsigned char border_color[4];
    
    unsigned char background_color[4];
    
    unsigned char text_color[4];
    
} keyboard_key_colors_t;

// ---------------------------------------------------------------------------------------------- //

typedef struct keyboard_s {

    key_t keyboard_keys[28];
    
    int keyboard_key_count; 
    
    keyboard_key_colors_t keyboard_key_colors[4]; 
    
    keyboard_state_t keyboard_state; 
    
    int keyboard_key_index; 
    
    keyboard_mode_t keyboard_mode;
    
    int cue_count;
    
    int current_trial;
    
    int is_sequence_running;
    
    int state_start_frame_index;
    
    float state_idle_duration;
    
    float state_cue_duration;
    
    float state_flashing_duration;
    
    float state_feedback_duration;
    
    const unsigned char *keyboard_sequence_matrix;
    
    int keyboard_sequence_num_keys;
    
    int keyboard_sequence_num_bits;

} keyboard_t;

// ---------------------------------------------------------------------------------------------- //

keyboard_t keyboard = { 
    
    .keyboard_keys = {

        { .key_letter = 'A', .key_x = 368.0f,  .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'B', .key_x = 544.0f,  .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'C', .key_x = 720.0f,  .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'D', .key_x = 896.0f,  .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'E', .key_x = 1072.0f, .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'F', .key_x = 1248.0f, .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'G', .key_x = 1424.0f, .key_y = 212.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'H', .key_x = 368.0f,  .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'I', .key_x = 544.0f,  .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'J', .key_x = 720.0f,  .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'K', .key_x = 896.0f,  .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'L', .key_x = 1072.0f, .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'M', .key_x = 1248.0f, .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'N', .key_x = 1424.0f, .key_y = 388.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'O', .key_x = 368.0f,  .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'P', .key_x = 544.0f,  .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'Q', .key_x = 720.0f,  .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'R', .key_x = 896.0f,  .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'S', .key_x = 1072.0f, .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'T', .key_x = 1248.0f, .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'U', .key_x = 1424.0f, .key_y = 564.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'V', .key_x = 368.0f,  .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'W', .key_x = 544.0f,  .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'X', .key_x = 720.0f,  .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'Y', .key_x = 896.0f,  .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = 'Z', .key_x = 1072.0f, .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = '-', .key_x = 1248.0f, .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f },

        { .key_letter = '<', .key_x = 1424.0f, .key_y = 740.0f, .key_width = 128.0f, .key_height = 128.0f, .key_border_thickness = 4.0f }

    },
    
    .keyboard_key_count = 28, 
    
    .keyboard_key_colors = {

        { .border_color = { 85, 85, 85, 255 }, .background_color = { 16, 16, 16, 255 }, .text_color = { 85, 85, 85, 255 } },

        { .border_color = { 255, 255, 0, 255 }, .background_color = { 16, 16, 16, 255 }, .text_color = { 85, 85, 85, 255 } },

        { .border_color = { 255, 255, 255, 255 }, .background_color = { 255, 255, 255, 255 }, .text_color = { 0, 0, 0, 255 } },

        { .border_color = { 0, 0, 255, 255 }, .background_color = { 16, 16, 16, 255 }, .text_color = { 85, 85, 85, 255 } }

    },
    
    .keyboard_state = keyboard_state_idle, 
    
    .keyboard_key_index = -1,
    
    .keyboard_mode = keyboard_mode_training,
    
    .cue_count = 10,
    
    .current_trial = 0,
    
    .is_sequence_running = 0,
    
    .state_start_frame_index = 0,
    
    .state_idle_duration = 0.3f,
    
    .state_cue_duration = 0.7f,
    
    .state_flashing_duration = 4.2f,
    
    .state_feedback_duration = 0.7f,
    
    .keyboard_sequence_matrix = sequence_matrix_buffer,
    
    .keyboard_sequence_num_keys = 0,
    
    .keyboard_sequence_num_bits = 0,

};

// ---------------------------------------------------------------------------------------------- //

void render_keyboard(SDL_Renderer *renderer, TTF_TextEngine *text_engine, TTF_Font *font, keyboard_t *keyboard, int frame_index) {

    if (keyboard->keyboard_sequence_num_keys == 0) {
        int keys, bits;
        if (load_sequence_from_txt("codes/mgold_61_6521.txt", &keys, &bits)) {
            keyboard->keyboard_sequence_num_keys = keys;
            keyboard->keyboard_sequence_num_bits = bits;
        }
    }

    for (int index = 0; index < keyboard -> keyboard_key_count; index++) {
            
        // ---------------------------------------------------------------------------------------------- //

        int color_index = 0;

        if (keyboard -> keyboard_state == keyboard_state_cue && index == keyboard -> keyboard_key_index) color_index = 1;

        if (keyboard -> keyboard_state == keyboard_state_flashing && keyboard -> keyboard_sequence_matrix[(index % keyboard -> keyboard_sequence_num_keys) * keyboard -> keyboard_sequence_num_bits + (frame_index % keyboard -> keyboard_sequence_num_bits)] == 1) color_index = 2;

        if (keyboard -> keyboard_state == keyboard_state_feedback && index == keyboard -> keyboard_key_index) color_index = 3;

        // ---------------------------------------------------------------------------------------------- //

        SDL_SetRenderDrawColor(renderer, keyboard -> keyboard_key_colors[color_index].border_color[0], keyboard -> keyboard_key_colors[color_index].border_color[1], keyboard -> keyboard_key_colors[color_index].border_color[2], keyboard -> keyboard_key_colors[color_index].border_color[3]);

        SDL_RenderFillRect(renderer, &(SDL_FRect) { keyboard -> keyboard_keys[index].key_x, keyboard -> keyboard_keys[index].key_y, keyboard -> keyboard_keys[index].key_width, keyboard -> keyboard_keys[index].key_height });

        // ---------------------------------------------------------------------------------------------- //
        
        SDL_SetRenderDrawColor(renderer, keyboard -> keyboard_key_colors[color_index].background_color[0], keyboard -> keyboard_key_colors[color_index].background_color[1], keyboard -> keyboard_key_colors[color_index].background_color[2], keyboard -> keyboard_key_colors[color_index].background_color[3]);

        SDL_RenderFillRect(renderer, &(SDL_FRect) { keyboard -> keyboard_keys[index].key_x + keyboard -> keyboard_keys[index].key_border_thickness, keyboard -> keyboard_keys[index].key_y + keyboard -> keyboard_keys[index].key_border_thickness, keyboard -> keyboard_keys[index].key_width - keyboard -> keyboard_keys[index].key_border_thickness * 2.0f, keyboard -> keyboard_keys[index].key_height - keyboard -> keyboard_keys[index].key_border_thickness * 2.0f });

        // ---------------------------------------------------------------------------------------------- //

        if (keyboard -> keyboard_keys[index].key_ttf_text == NULL) {
            
            keyboard -> keyboard_keys[index].key_ttf_text = TTF_CreateText(text_engine, font, (char[2]) { keyboard -> keyboard_keys[index].key_letter, '\0' }, 0);
            
        }
        
        // ---------------------------------------------------------------------------------------------- //

        TTF_SetTextColor(keyboard -> keyboard_keys[index].key_ttf_text, keyboard -> keyboard_key_colors[color_index].text_color[0], keyboard -> keyboard_key_colors[color_index].text_color[1], keyboard -> keyboard_key_colors[color_index].text_color[2], keyboard -> keyboard_key_colors[color_index].text_color[3]);

        // ---------------------------------------------------------------------------------------------- //

        int text_width, text_height;
        
        TTF_GetTextSize(keyboard -> keyboard_keys[index].key_ttf_text, &text_width, &text_height);

        // ---------------------------------------------------------------------------------------------- //

        TTF_DrawRendererText(keyboard -> keyboard_keys[index].key_ttf_text, keyboard -> keyboard_keys[index].key_x + (keyboard -> keyboard_keys[index].key_width - text_width) / 2.0f, keyboard -> keyboard_keys[index].key_y + (keyboard -> keyboard_keys[index].key_height - text_height) / 2.0f);

        // ---------------------------------------------------------------------------------------------- //

    }

}

// ---------------------------------------------------------------------------------------------- //

int get_key_index_by_letter(keyboard_t *keyboard, char letter) {

    for (int index = 0; index < keyboard -> keyboard_key_count; index++) {

        if (keyboard -> keyboard_keys[index].key_letter == letter) return index;

    }

    return -1;

}

// ---------------------------------------------------------------------------------------------- //

void update_keyboard(keyboard_t *keyboard, int presentation_rate_frame_index, float presentation_rate, lsl_t *lsl) {

    if (keyboard -> is_sequence_running) {

        float state_elapsed_time = (float)(presentation_rate_frame_index - keyboard -> state_start_frame_index) / presentation_rate;

        if (keyboard -> keyboard_mode == keyboard_mode_training) { 

            if (keyboard -> keyboard_state == keyboard_state_feedback && state_elapsed_time >= keyboard -> state_feedback_duration) {

                send_lsl_marker(lsl, "stop_feedback");

                keyboard -> keyboard_state = keyboard_state_idle;

                keyboard->state_start_frame_index = presentation_rate_frame_index;

                keyboard -> keyboard_key_index = -1;

                send_lsl_marker(lsl, "start_iti");
            }

            else if (keyboard -> keyboard_state == keyboard_state_cue && state_elapsed_time >= keyboard -> state_cue_duration) {

                send_lsl_marker(lsl, "stop_cue");

                keyboard -> keyboard_state = keyboard_state_flashing;

                keyboard -> state_start_frame_index = presentation_rate_frame_index;

                send_lsl_marker(lsl, "start_trial");
            }

            else if (keyboard -> keyboard_state == keyboard_state_flashing && state_elapsed_time >= keyboard -> state_flashing_duration) {
                
                send_lsl_marker(lsl, "stop_trial");

                keyboard -> keyboard_state = keyboard_state_idle;

                keyboard -> state_start_frame_index = presentation_rate_frame_index;

                send_lsl_marker(lsl, "start_iti");
            }

            else if (keyboard -> keyboard_state == keyboard_state_idle && state_elapsed_time >= keyboard -> state_idle_duration) {

                send_lsl_marker(lsl, "stop_iti");
                
                keyboard -> current_trial++;
                
                if (keyboard -> current_trial < keyboard -> cue_count) {
                    
                    keyboard -> keyboard_key_index = rand() % keyboard -> keyboard_key_count;

                    keyboard -> keyboard_state = keyboard_state_cue;

                    keyboard -> state_start_frame_index = presentation_rate_frame_index;

                    char letter = keyboard -> keyboard_keys[keyboard -> keyboard_key_index].key_letter;

                    char key_str[2] = { letter, '\0' };

                    const char *key_name = (letter == '-') ? "space" : ((letter == '<') ? "backspace" : key_str);

                    send_lsl_marker(lsl, "start_cue;label=%d;key=%s", keyboard -> keyboard_key_index, key_name);

                } else {

                    keyboard -> is_sequence_running = 0;

                }
            }

        } else if (keyboard -> keyboard_mode == keyboard_mode_online) {

            if (keyboard -> keyboard_state == keyboard_state_feedback && state_elapsed_time >= keyboard -> state_feedback_duration) {
                
                send_lsl_marker(lsl, "stop_feedback");

                keyboard -> keyboard_state = keyboard_state_idle;

                keyboard -> state_start_frame_index = presentation_rate_frame_index;

                keyboard -> keyboard_key_index = -1;

                send_lsl_marker(lsl, "start_iti");
            }
            
            else if (keyboard -> keyboard_state == keyboard_state_idle && state_elapsed_time >= keyboard -> state_idle_duration) {
                
                send_lsl_marker(lsl, "stop_iti");

                keyboard -> keyboard_state = keyboard_state_flashing;

                keyboard -> state_start_frame_index = presentation_rate_frame_index;
                
                send_lsl_marker(lsl, "start_trial");
            }

        }

    }

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //