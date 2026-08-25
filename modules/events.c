// ---------------------------------------------------------------------------------------------- //

#ifndef events_included

// ---------------------------------------------------------------------------------------------- //

#define events_included

// ---------------------------------------------------------------------------------------------- //

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------------------------- //

typedef struct custom_event_types_s {
    
    Uint32 custom_event_type_idle;
    
    Uint32 custom_event_type_training;
    
    Uint32 custom_event_type_online;
    
    Uint32 custom_event_type_feedback;
    
    Uint32 custom_event_type_close;

} custom_event_types_t;

// ---------------------------------------------------------------------------------------------- //

custom_event_types_t custom_event_types = { 0 };    

// ---------------------------------------------------------------------------------------------- //

void push_custom_event(Uint32 *custom_event_type, int data_1, int data_2) {

    if (*custom_event_type == 0) *custom_event_type = SDL_RegisterEvents(1);

    SDL_Event event = { 0 };

    event.type = *custom_event_type;

    event.user.type = *custom_event_type;

    event.user.data1 = (void*) (intptr_t) data_1;

    event.user.data2 = (void*) (intptr_t) data_2;
    
    SDL_PushEvent(&event);

}

// ---------------------------------------------------------------------------------------------- //

void push_event_idle() { push_custom_event(&custom_event_types.custom_event_type_idle, -999, -999); }

void push_event_training() { push_custom_event(&custom_event_types.custom_event_type_training, -999, -999); }

void push_event_online() { push_custom_event(&custom_event_types.custom_event_type_online, -999, -999); }

void push_event_feedback(int key_index) { push_custom_event(&custom_event_types.custom_event_type_feedback, key_index, -999); }

void push_event_close() { push_custom_event(&custom_event_types.custom_event_type_close, -999, -999); }

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //