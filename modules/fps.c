// ---------------------------------------------------------------------------------------------- //

#ifndef fps_included

// ---------------------------------------------------------------------------------------------- //

#define fps_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3/SDL.h>

#include <stdlib.h>

#include <stdio.h>

#include <time.h>

#include "lsl.c"

// ---------------------------------------------------------------------------------------------- //

typedef struct {
    
    double timestamp_ms;
    
    Uint64 frame_index;
    
    Uint64 sequence_index;
    
    double current_fps;
    
    double difference_fps;
    
    double target_ms;
    
    double current_ms;
    
    double difference_ms;
    
    int is_drop;
    
} fps_log_entry_t;

// ---------------------------------------------------------------------------------------------- //

typedef struct {

    float refresh_rate;

    float presentation_rate;

    Uint64 start_time;

    int prev_refresh_rate_frame_index;

    int refresh_rate_frame_index;

    int presentation_rate_frame_index;

    fps_log_entry_t *logs;

    size_t log_capacity;

    size_t log_count;
    
    Uint64 last_frame_timestamp_in_ticks;

    double target_frame_duration_in_milliseconds;

    int frames_per_stimulus;

    char log_filename[32];

} fps_t;

// ---------------------------------------------------------------------------------------------- //

fps_t fps = { 
    
    .presentation_rate = 60.0f,
    
    .log_filename = "log.csv",

};

// ---------------------------------------------------------------------------------------------- //

void initialize_fps(SDL_Window *window, fps_t *fps) {

    fps -> refresh_rate = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window)) -> refresh_rate;

    fps -> presentation_rate = fps -> refresh_rate / SDL_max(1, (int) (fps -> refresh_rate / fps -> presentation_rate + 0.5f));

    printf("\n[ INFO ] | fps.c | initialize_fps() | refresh rate : %f Hz | presentation rate: %f Hz\n\n", fps -> refresh_rate, fps -> presentation_rate);

    fps -> start_time = SDL_GetPerformanceCounter();

    fps -> prev_refresh_rate_frame_index = -1;

    fps -> refresh_rate_frame_index = 0;

    fps -> refresh_rate_frame_index = 0;

    fps -> presentation_rate_frame_index = 0;

    fps -> log_capacity = 1000000;

    fps -> logs = (fps_log_entry_t *) malloc(fps -> log_capacity * sizeof(fps_log_entry_t));

    fps -> log_count = 0;

    fps -> last_frame_timestamp_in_ticks = SDL_GetPerformanceCounter();

    fps -> target_frame_duration_in_milliseconds = 1000.0 / (double) fps -> refresh_rate;

    fps -> frames_per_stimulus = (fps -> refresh_rate < fps -> presentation_rate) ? 1 : (int) (fps -> refresh_rate / fps -> presentation_rate);

}

// ---------------------------------------------------------------------------------------------- //

void update_fps(fps_t *fps, lsl_t *lsl) {

    Uint64 current_time = SDL_GetPerformanceCounter();

    fps -> refresh_rate_frame_index = (int) (((double) (current_time - fps -> start_time) / (double) SDL_GetPerformanceFrequency()) * fps -> refresh_rate);

    int dropped_frames = 0;

    if (fps -> prev_refresh_rate_frame_index != -1 && fps -> refresh_rate_frame_index > fps -> prev_refresh_rate_frame_index + 1) {

        dropped_frames = fps -> refresh_rate_frame_index - fps -> prev_refresh_rate_frame_index - 1;

        send_lsl_marker(lsl, "frame_dropped");

    }

    fps -> prev_refresh_rate_frame_index = fps -> refresh_rate_frame_index;

    fps -> presentation_rate_frame_index = (int) (fps -> refresh_rate_frame_index * (fps -> presentation_rate / fps -> refresh_rate));

    if (fps -> logs && fps -> log_count < fps -> log_capacity) {

        Uint64 elapsed_time = current_time - fps -> last_frame_timestamp_in_ticks;

        fps -> last_frame_timestamp_in_ticks = current_time;

        double current_timestamp_in_milliseconds = ((double) (current_time - fps -> start_time) * 1000.0) / SDL_GetPerformanceFrequency();

        double current_frame_duration_in_milliseconds = ((double) elapsed_time * 1000.0) / SDL_GetPerformanceFrequency();

        double frame_duration_difference_in_milliseconds = current_frame_duration_in_milliseconds - fps -> target_frame_duration_in_milliseconds;

        double current_fps = elapsed_time > 0 ? ((double) SDL_GetPerformanceFrequency() / (double) elapsed_time) : 0.0;

        double difference_fps = current_fps - fps -> refresh_rate;

        for (int i = 1; i <= dropped_frames && fps -> log_count < fps -> log_capacity; i++) {

            int missed_index = fps -> refresh_rate_frame_index - dropped_frames + i - 1;

            fps -> logs[fps -> log_count].timestamp_ms = current_timestamp_in_milliseconds;

            fps -> logs[fps -> log_count].frame_index = missed_index;

            fps -> logs[fps -> log_count].sequence_index = (int) (missed_index * (fps -> presentation_rate / fps -> refresh_rate));

            fps -> logs[fps -> log_count].current_fps = current_fps;

            fps -> logs[fps -> log_count].difference_fps = difference_fps;

            fps -> logs[fps -> log_count].target_ms = fps -> target_frame_duration_in_milliseconds;

            fps -> logs[fps -> log_count].current_ms = current_frame_duration_in_milliseconds;

            fps -> logs[fps -> log_count].difference_ms = frame_duration_difference_in_milliseconds;

            fps -> logs[fps -> log_count].is_drop = 1;

            fps -> log_count++;
        }

        if (fps -> log_count < fps -> log_capacity) {

            fps -> logs[fps -> log_count].timestamp_ms = current_timestamp_in_milliseconds;

            fps -> logs[fps -> log_count].frame_index = fps -> refresh_rate_frame_index;

            fps -> logs[fps -> log_count].sequence_index = fps -> presentation_rate_frame_index;

            fps -> logs[fps -> log_count].current_fps = current_fps;

            fps -> logs[fps -> log_count].difference_fps = difference_fps;

            fps -> logs[fps -> log_count].target_ms = fps -> target_frame_duration_in_milliseconds;

            fps -> logs[fps -> log_count].current_ms = current_frame_duration_in_milliseconds;

            fps -> logs[fps -> log_count].difference_ms = frame_duration_difference_in_milliseconds;

            fps -> logs[fps -> log_count].is_drop = 0;

            fps -> log_count++;

        }

    }

}

// ---------------------------------------------------------------------------------------------- //

void destroy_fps(fps_t *fps) {

    if (fps -> logs) {

        FILE *file = fopen(fps -> log_filename, "w");

        if (file) {

            fprintf(file, "timestamp_ms, frame_index, sequence_index, refresh_rate, presentation_rate, current_fps, difference_fps, target_ms, current_ms, difference_ms, is_dropped\n");

            for (size_t i = 0; i < fps -> log_count; i++) {

                fprintf(file, "%.3f, %llu, %llu, %.2f, %.2f, %.2f, %+.2f, %.3f, %.3f, %+.3f, %s\n", 

                    fps -> logs[i].timestamp_ms, 

                    (unsigned long long) fps -> logs[i].frame_index, 

                    (unsigned long long) fps -> logs[i].sequence_index, 
                    
                    fps -> refresh_rate, 
                    
                    fps -> presentation_rate, 

                    fps -> logs[i].current_fps, 

                    fps -> logs[i].difference_fps, 

                    fps -> logs[i].target_ms, 

                    fps -> logs[i].current_ms, 

                    fps -> logs[i].difference_ms,

                    fps -> logs[i].is_drop ? "DROP" : ""

                );

            }

            fclose(file);

            printf("\n[ INFO ] | fps.c | destroy_fps() | %s\n\n", fps -> log_filename);

        } else {

            printf("\n[ ERROR ] | fps.c | destroy_fps() | failed to open %s for writing\n\n", fps -> log_filename);

        }

        free(fps -> logs);

        fps -> logs = NULL;

    }

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //