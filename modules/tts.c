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
    
    .text_to_speech_age = 3 

};

// ---------------------------------------------------------------------------------------------- //

void text_to_speech(tts_t *tts, char *text) {

    if (tts -> tts_process_handle != NULL) {

        if (GetExitCodeProcess(tts -> tts_process_handle, &tts -> tts_exit_code) && tts -> tts_exit_code == STILL_ACTIVE) TerminateProcess(tts -> tts_process_handle, 0);

        CloseHandle(tts -> tts_process_handle);

        tts -> tts_process_handle = NULL;

    }

    wsprintfA(tts -> command_string, "powershell -c \"Add-Type -A System.Speech;$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;$s.Volume=%d;$s.Rate=%d;$s.SelectVoiceByHints(%d,%d,0,[System.Globalization.CultureInfo]::new('en-US'));$s.Speak(\\\"%s\\\");Start-Sleep -m 200\"", tts -> text_to_speech_volume, tts -> text_to_speech_speed, tts -> text_to_speech_gender, tts -> text_to_speech_age, text);

    ZeroMemory(&tts -> startup_info, sizeof(tts -> startup_info));

    tts -> startup_info.cb = sizeof(tts -> startup_info);

    ZeroMemory(&tts -> process_information, sizeof(tts -> process_information));

    if (CreateProcess(NULL, tts -> command_string, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &tts -> startup_info, &tts -> process_information)) {

        tts -> tts_process_handle = tts -> process_information.hProcess;

        CloseHandle(tts -> process_information.hThread);

    }

}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //