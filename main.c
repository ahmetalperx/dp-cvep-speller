// ---------------------------------------------------------------------------------------------- //

#include <SDL3_ttf/SDL_ttf.h>

#include <SDL3/SDL.h>

#include <winsock2.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <mmsystem.h>

#include <stdlib.h>

#include <avrt.h>

#include <time.h>

// ---------------------------------------------------------------------------------------------- //

#include "modules/background.c"

#include "modules/photodiode.c"

#include "modules/keyboard.c"

#include "modules/events.c"

#include "modules/server.c"

#include "modules/output.c"

#include "modules/lsl.c"

#include "modules/fps.c"

#include "modules/tts.c"

// ---------------------------------------------------------------------------------------------- //

int main() {

    // ---------------------------------------------------------------------------------------------- //

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // ---------------------------------------------------------------------------------------------- //

    SetThreadAffinityMask(GetCurrentThread(), (1ULL << 1));

    // ---------------------------------------------------------------------------------------------- //

    DWORD mmcss_task_index = 0;

    HANDLE mmcss_handle = AvSetMmThreadCharacteristicsW(L"Pro Audio", &mmcss_task_index);

    if (mmcss_handle != NULL) {
        
        AvSetMmThreadPriority(mmcss_handle, AVRT_PRIORITY_CRITICAL);
        
    }

    // ---------------------------------------------------------------------------------------------- //

    timeBeginPeriod(1);

    // ---------------------------------------------------------------------------------------------- //

    srand((unsigned int) time(NULL));

    // ---------------------------------------------------------------------------------------------- //

    SDL_Init(SDL_INIT_VIDEO);

    // ---------------------------------------------------------------------------------------------- //

    SDL_Window *window = SDL_CreateWindow("alper", 0, 0, SDL_WINDOW_FULLSCREEN);

    // ---------------------------------------------------------------------------------------------- //

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // ---------------------------------------------------------------------------------------------- //

    SDL_SetRenderVSync(renderer, 1);

    // ---------------------------------------------------------------------------------------------- //

    TTF_Init();

    // ---------------------------------------------------------------------------------------------- //
    
    TTF_TextEngine *text_engine = TTF_CreateRendererTextEngine(renderer);
    
    // ---------------------------------------------------------------------------------------------- //
    
    TTF_Font *font_montserrat_medium_20 = TTF_OpenFont("fonts/montserrat_medium.ttf", 20);

    TTF_Font *font_montserrat_extrabold_64 = TTF_OpenFont("fonts/montserrat_extrabold.ttf", 64);
    
    TTF_Font *font_awesome_48 = TTF_OpenFont("fonts/fa-solid-900.ttf", 48);
    
    if (!font_montserrat_medium_20 || !font_montserrat_extrabold_64 || !font_awesome_48) {
        
        printf("Failed to load fonts.\n");
        
        return -1;
        
    }
    
    // ---------------------------------------------------------------------------------------------- //

    initialize_dictionary("words/en.txt");
    
    // ---------------------------------------------------------------------------------------------- //
    
    initialize_lsl(&lsl);
    // ---------------------------------------------------------------------------------------------- //
    
    initialize_server(&server);
    
    // ---------------------------------------------------------------------------------------------- //

    initialize_fps(window, &fps);

    // ---------------------------------------------------------------------------------------------- //

    for (int is_running = 1; is_running;) {

        // ---------------------------------------------------------------------------------------------- //

        update_fps(&fps, &lsl);

        // ---------------------------------------------------------------------------------------------- //
        
        update_server(&server);
        
        // ---------------------------------------------------------------------------------------------- //
        
        if (keyboard.keyboard_state != keyboard_state_flashing) {
            
            update_lsl(&lsl);
            
        }
        
        // ---------------------------------------------------------------------------------------------- //

        for (SDL_Event event; SDL_PollEvent(&event);) {

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) push_event_close();

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_1 || event.key.key == SDLK_KP_1) && !event.key.repeat) push_event_idle();

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_2 || event.key.key == SDLK_KP_2) && !event.key.repeat) push_event_training();
            
            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_3 || event.key.key == SDLK_KP_3) && !event.key.repeat) push_event_online();
            
            // ---------------------------------------------------------------------------------------------- //

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_4 || event.key.key == SDLK_KP_4) && !event.key.repeat) toggle_photodiode_visibility(&photodiode_refresh_rate);
            
            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_5 || event.key.key == SDLK_KP_5) && !event.key.repeat) toggle_output_visibility(&output);

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_6 || event.key.key == SDLK_KP_6) && !event.key.repeat) toggle_photodiode_visibility(&photodiode_presentation_rate);

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_9 || event.key.key == SDLK_KP_9) && !event.key.repeat && get_key_index_by_letter(&keyboard, '*') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '*'));

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_8 || event.key.key == SDLK_KP_8 || event.key.key == SDLK_TAB) && !event.key.repeat && get_key_index_by_letter(&keyboard, '>') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '>'));
            
            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_7 || event.key.key == SDLK_KP_7) && !event.key.repeat && get_key_index_by_letter(&keyboard, '#') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '#'));

            if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_0 || event.key.key == SDLK_KP_0 || event.key.key == SDLK_CAPSLOCK) && !event.key.repeat && get_key_index_by_letter(&keyboard, '^') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '^'));

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == SDL_EVENT_KEY_DOWN && (SDLK_A <= event.key.key && event.key.key <= SDLK_Z) && get_key_index_by_letter(&keyboard, (char)('A' + (event.key.key - SDLK_A))) != -1) push_event_feedback(get_key_index_by_letter(&keyboard, (char)('A' + (event.key.key - SDLK_A))));

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE && get_key_index_by_letter(&keyboard, '-') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '-'));

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_BACKSPACE && get_key_index_by_letter(&keyboard, '<') != -1) push_event_feedback(get_key_index_by_letter(&keyboard, '<'));
            
            // ---------------------------------------------------------------------------------------------- //

            if (event.type == custom_event_types.custom_event_type_idle) {

                if (keyboard.keyboard_state == keyboard_state_flashing) send_lsl_marker(&lsl, "stop_trial");

                if (keyboard.keyboard_state == keyboard_state_cue) send_lsl_marker(&lsl, "stop_cue");

                if (keyboard.keyboard_state == keyboard_state_feedback) send_lsl_marker(&lsl, "stop_feedback");

                keyboard.keyboard_state = keyboard_state_idle;

                keyboard.is_sequence_running = 0;

                send_lsl_marker(&lsl, "start_iti");
                
            }

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == custom_event_types.custom_event_type_training) {

                clear_output(&output);

                if (keyboard.keyboard_state == keyboard_state_idle) send_lsl_marker(&lsl, "stop_iti");

                keyboard.keyboard_mode = keyboard_mode_training;

                keyboard.current_trial = 0;

                keyboard.is_sequence_running = 1;

                keyboard.keyboard_key_index = rand() % keyboard.keyboard_key_count;

                keyboard.keyboard_state = keyboard_state_cue;

                keyboard.state_start_frame_index = fps.refresh_rate_frame_index;

                char letter = (keyboard.keyboard_key_index >= 0 && keyboard.keyboard_key_index < keyboard.keyboard_key_count) ? keyboard.keyboard_keys[keyboard.keyboard_key_index].key_letter : '?';

                char key_str[2];
                
                const char *key_name = get_key_name_str(letter, key_str);
                
                send_lsl_marker(&lsl, "start_cue;label=%d;key=%s", keyboard.keyboard_key_index, key_name);

            }

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == custom_event_types.custom_event_type_online) {
                
                clear_output(&output);

                if (keyboard.keyboard_state == keyboard_state_idle) send_lsl_marker(&lsl, "stop_iti");
                
                keyboard.keyboard_mode = keyboard_mode_online;

                keyboard.is_sequence_running = 1;

                keyboard.keyboard_state = keyboard_state_flashing;

                keyboard.state_start_frame_index = fps.refresh_rate_frame_index;
                
                send_lsl_marker(&lsl, "start_trial");
                
            }

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == custom_event_types.custom_event_type_feedback) {

                if (keyboard.keyboard_mode == keyboard_mode_training) continue;

                if (keyboard.keyboard_state == keyboard_state_flashing) send_lsl_marker(&lsl, "stop_trial");

                int key_index = (int) (intptr_t) event.user.data1;

                keyboard.keyboard_key_index = key_index;

                keyboard.keyboard_state = keyboard_state_feedback;

                keyboard.state_start_frame_index = fps.refresh_rate_frame_index;

                char letter = (keyboard.keyboard_key_index >= 0 && keyboard.keyboard_key_index < keyboard.keyboard_key_count) ? keyboard.keyboard_keys[keyboard.keyboard_key_index].key_letter : '?';
                
                char key_str[2];
                
                const char *key_name = get_key_name_str(letter, key_str);
                
                send_lsl_marker(&lsl, "start_feedback;label=%d;key=%s", keyboard.keyboard_key_index, key_name);
                
                if (letter == '-') {
                    
                    add_space_to_output(&output);

                } else if (letter == '<') {

                    delete_letter_from_output(&output);

                } else if (letter == '*') {

                    text_to_speech(&tts, output.output_text);

                } else if (letter == '>') {

                    accept_prediction(&output);
                    
                } else if (letter == '^') {
                    
                    toggle_caps_lock(&keyboard);

                } else if (letter == '#') {

                    output.output_text[0] = '\0';
                    
                    output.output_text_length = 0;
                    
                    output.output_text_changed = 1;

                } else {
                    
                    if (keyboard.is_lowercase && letter >= 'A' && letter <= 'Z') letter += 32;

                    add_letter_to_output(&output, letter);

                }

            }

            // ---------------------------------------------------------------------------------------------- //

            if (event.type == custom_event_types.custom_event_type_close) is_running = 0;

            // ---------------------------------------------------------------------------------------------- //

        }

        // ---------------------------------------------------------------------------------------------- //

        int is_tts_speaking = 0;
        
        if (tts.tts_process_handle != NULL) {
            
            DWORD exit_code;
            
            if (GetExitCodeProcess(tts.tts_process_handle, &exit_code) && exit_code == STILL_ACTIVE) is_tts_speaking = 1;
            
        }

        update_keyboard(&keyboard, fps.refresh_rate_frame_index, fps.refresh_rate, &lsl, is_tts_speaking);

        // ---------------------------------------------------------------------------------------------- //

        int sequence_frame_index = fps.refresh_rate_frame_index - keyboard.state_start_frame_index;

        // ---------------------------------------------------------------------------------------------- //

        render_background(renderer, &background);

        // ---------------------------------------------------------------------------------------------- //
        
        set_photodiode_state(&photodiode_refresh_rate, (fps.refresh_rate_frame_index % 2 == 0) ? photodiode_state_on : photodiode_state_off);

        render_photodiode(renderer, &photodiode_refresh_rate);

        // ---------------------------------------------------------------------------------------------- //

        set_photodiode_state(&photodiode_presentation_rate, (fps.presentation_rate_frame_index % 2 == 0) ? photodiode_state_on : photodiode_state_off);

        render_photodiode(renderer, &photodiode_presentation_rate);

        // ---------------------------------------------------------------------------------------------- //

        render_output(renderer, text_engine, font_montserrat_medium_20, &output);

        // ---------------------------------------------------------------------------------------------- //

        render_keyboard(renderer, text_engine, font_montserrat_extrabold_64, font_awesome_48, &keyboard, sequence_frame_index, fps.frames_per_stimulus);

        // ---------------------------------------------------------------------------------------------- //
        
        SDL_RenderPresent(renderer);

        // ---------------------------------------------------------------------------------------------- //

    }

    // ---------------------------------------------------------------------------------------------- //

    destroy_fps(&fps);

    // ---------------------------------------------------------------------------------------------- //

    destroy_server(&server);
    
    // ---------------------------------------------------------------------------------------------- //
    
    destroy_lsl(&lsl);
    
    // ---------------------------------------------------------------------------------------------- //
    
    cleanup_dictionary();

    // ---------------------------------------------------------------------------------------------- //

    TTF_CloseFont(font_montserrat_extrabold_64);
    
    TTF_CloseFont(font_awesome_48);

    TTF_CloseFont(font_montserrat_medium_20);

    // ---------------------------------------------------------------------------------------------- //

    TTF_DestroyRendererTextEngine(text_engine);

    // ---------------------------------------------------------------------------------------------- //

    TTF_Quit();

    // ---------------------------------------------------------------------------------------------- //

    SDL_DestroyRenderer(renderer);

    // ---------------------------------------------------------------------------------------------- //

    SDL_DestroyWindow(window);

    // ---------------------------------------------------------------------------------------------- //

    SDL_Quit();

    // ---------------------------------------------------------------------------------------------- //

    timeEndPeriod(1);

    // ---------------------------------------------------------------------------------------------- //

    AvRevertMmThreadCharacteristics(mmcss_handle);

    // ---------------------------------------------------------------------------------------------- //

    return 0;

    // ---------------------------------------------------------------------------------------------- //

}

// ---------------------------------------------------------------------------------------------- //