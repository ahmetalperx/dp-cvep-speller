// ---------------------------------------------------------------------------------------------- //

#ifndef lsl_included

// ---------------------------------------------------------------------------------------------- //

#define lsl_included

// ---------------------------------------------------------------------------------------------- //

#include <lsl_c.h>

#include <stdio.h>

// ---------------------------------------------------------------------------------------------- //

#include "events.c"

// ---------------------------------------------------------------------------------------------- //

typedef struct lsl_s {

    char *lsl_marker_stream_name;

    lsl_streaminfo lsl_stream_info;

    lsl_outlet lsl_outlet;

    char *lsl_decoder_stream_name;

    lsl_inlet lsl_inlet;

    Uint64 lsl_last_resolve_time;

    char lsl_marker_buffer[256];

} lsl_t;

// ---------------------------------------------------------------------------------------------- //

lsl_t lsl = {

    .lsl_marker_stream_name = "cvep-speller-stream",

    .lsl_stream_info = NULL,

    .lsl_outlet = NULL,

    .lsl_decoder_stream_name = "cvep-decoder-stream",

    .lsl_inlet = NULL,

    .lsl_last_resolve_time = 0,

    .lsl_marker_buffer = { 0 },

};

// ---------------------------------------------------------------------------------------------- //

void initialize_lsl(lsl_t *lsl) {

    lsl -> lsl_stream_info = lsl_create_streaminfo(lsl -> lsl_marker_stream_name, "markers", 1, LSL_IRREGULAR_RATE, cft_string, "");

    lsl -> lsl_outlet = lsl_create_outlet(lsl -> lsl_stream_info, 0, 360);

    printf("\n[ INFO ] | lsl.c | initialize_lsl() | %s | %s\n", lsl -> lsl_marker_stream_name, lsl -> lsl_decoder_stream_name);

}

// ---------------------------------------------------------------------------------------------- //

void destroy_lsl(lsl_t *lsl) {

    if (lsl -> lsl_stream_info) lsl_destroy_streaminfo(lsl -> lsl_stream_info);

    if (lsl -> lsl_outlet) lsl_destroy_outlet(lsl -> lsl_outlet);

    if (lsl -> lsl_inlet) lsl_destroy_inlet(lsl -> lsl_inlet);

}

// ---------------------------------------------------------------------------------------------- //

void update_lsl(lsl_t *lsl) {

    lsl_streaminfo results[1];

    if (!lsl -> lsl_inlet) {

        Uint64 current_time = SDL_GetTicks();

        if (current_time - lsl -> lsl_last_resolve_time > 5000) {

            lsl -> lsl_last_resolve_time = current_time;

            if (lsl_resolve_byprop(results, 1, "name", lsl -> lsl_decoder_stream_name, 1, 0.0) > 0) {

                printf("\n[ INFO ] | lsl.c | update_lsl() | connected to %s\n", lsl -> lsl_decoder_stream_name);

                fflush(stdout);

                lsl -> lsl_inlet = lsl_create_inlet(results[0], 360, LSL_NO_PREFERENCE, 1);

            }

        }

    }

    if (lsl -> lsl_inlet) {

        char predicted_index;

        int errcode = 0, last_valid = -1;

        double ts;

        while ((ts = lsl_pull_sample_c(lsl -> lsl_inlet, &predicted_index, 1, 0.0, &errcode)) != 0.0) {

            if (!errcode && predicted_index >= 0) last_valid = predicted_index;

        }
        
        if (last_valid >= 0) {
            
            push_event_feedback(last_valid);
            
        }

    }

}

// ---------------------------------------------------------------------------------------------- //

void send_lsl_marker(lsl_t *lsl, const char *format, ...) {

    if (!lsl -> lsl_outlet) return;

    va_list args;
    
    va_start(args, format);

    vsnprintf(lsl -> lsl_marker_buffer, sizeof(lsl -> lsl_marker_buffer), format, args);
    
    va_end(args);

    const char *marker_ptr = lsl -> lsl_marker_buffer;
    
    lsl_push_sample_str(lsl -> lsl_outlet, &marker_ptr);

    // printf("\n[ INFO ] | lsl.c | send_lsl_marker() | %s\n", lsl -> lsl_marker_buffer);

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //