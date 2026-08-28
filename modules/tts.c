// ---------------------------------------------------------------------------------------------- //

#ifndef tts_included

// ---------------------------------------------------------------------------------------------- //

#define tts_included

// ---------------------------------------------------------------------------------------------- //

#include <windows.h>

// ---------------------------------------------------------------------------------------------- //

typedef struct tts_s {
    
    int text_to_speech_volume;
    
    int text_to_speech_speed;
    
    int text_to_speech_gender;
    
    int text_to_speech_age;

    char command_string[512];

    HANDLE tts_process_handle;

    DWORD tts_exit_code;

    STARTUPINFO startup_info;

    PROCESS_INFORMATION process_information;

} tts_t;

// ---------------------------------------------------------------------------------------------- //

tts_t tts = {
    
    .text_to_speech_volume = 100,
    
    .text_to_speech_speed = 0,
    
    .text_to_speech_gender = 1,
    
    .text_to_speech_age = 3,

    .command_string = { 0 },

    .tts_process_handle = NULL,

    .tts_exit_code = 0,

    .startup_info = { 0 },

    .process_information = { 0 },

};

// ---------------------------------------------------------------------------------------------- //

#include <stdio.h>

typedef struct {
    
    tts_t *tts;
    
    char command_string[1024];
    
} tts_thread_args_t;

// ---------------------------------------------------------------------------------------------- //

DWORD WINAPI tts_thread_func(LPVOID lpParam) {
    
    tts_thread_args_t *args = (tts_thread_args_t *) lpParam;
    
    STARTUPINFO startup_info = { 0 };
    
    startup_info.cb = sizeof(startup_info);
    
    PROCESS_INFORMATION process_information = { 0 };
    
    if (CreateProcessA(NULL, args -> command_string, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_information)) {
        
        args -> tts -> tts_process_handle = process_information.hProcess;
        
        CloseHandle(process_information.hThread);
        
    }
    
    free(args);
    
    return 0;
    
}

// ---------------------------------------------------------------------------------------------- //

void text_to_speech(tts_t *tts, char *text) {

    if (tts -> tts_process_handle != NULL) {

        if (GetExitCodeProcess(tts -> tts_process_handle, &tts -> tts_exit_code) && tts -> tts_exit_code == STILL_ACTIVE) TerminateProcess(tts -> tts_process_handle, 0);

        CloseHandle(tts -> tts_process_handle);

        tts -> tts_process_handle = NULL;

    }

    tts_thread_args_t *args = (tts_thread_args_t *) malloc(sizeof(tts_thread_args_t));
    
    args -> tts = tts;
    
    snprintf(args -> command_string, sizeof(args -> command_string), "powershell -c \"Add-Type -A System.Speech;$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;$s.Volume=%d;$s.Rate=%d;$s.SelectVoiceByHints(%d,%d,0,[System.Globalization.CultureInfo]::new('en-US'));$s.Speak(\\\"%s\\\");Start-Sleep -m 200\"", tts -> text_to_speech_volume, tts -> text_to_speech_speed, tts -> text_to_speech_gender, tts -> text_to_speech_age, text);

    CloseHandle(CreateThread(NULL, 0, tts_thread_func, args, 0, NULL));

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //