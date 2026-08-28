// ---------------------------------------------------------------------------------------------- //

#ifndef dictionary_included

// ---------------------------------------------------------------------------------------------- //

#define dictionary_included

// ---------------------------------------------------------------------------------------------- //

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <ctype.h>

// ---------------------------------------------------------------------------------------------- //

#define MAX_DICTIONARY_WORDS 20000

#define MAX_WORD_LENGTH 256

char *dictionary_words[MAX_DICTIONARY_WORDS];

int dictionary_word_count = 0;

// ---------------------------------------------------------------------------------------------- //

void initialize_dictionary(const char *filepath) {
    
    FILE *file = fopen(filepath, "r");
    
    if (!file) {
        
        printf("[ ERROR ] | dictionary.c | Could not open dictionary file: %s\n", filepath);
        
        return;
        
    }

    char buffer[MAX_WORD_LENGTH];
    
    while (fgets(buffer, sizeof(buffer), file) && dictionary_word_count < MAX_DICTIONARY_WORDS) {
        
        size_t len = strlen(buffer);
        
        if (len == sizeof(buffer) - 1 && buffer[len - 1] != '\n') {
            
            int c;
            
            while ((c = fgetc(file)) != '\n' && c != EOF);
            
        }
        
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            
            buffer[len - 1] = '\0';
            
            len--;
            
        }

        if (len == 0) continue;

        for (size_t index = 0; index < len; index++) {
            
            buffer[index] = toupper((unsigned char) buffer[index]);
            
        }

        dictionary_words[dictionary_word_count] = strdup(buffer);
        
        if (dictionary_words[dictionary_word_count]) {
            
            dictionary_word_count++;
            
        }
        
    }

    fclose(file);
    
    printf("[ INFO ] | dictionary.c | initialize_dictionary() | Loaded %d words total after reading %s\n", dictionary_word_count, filepath);
    
}

// ---------------------------------------------------------------------------------------------- //

const char *get_prediction(const char *current_text) {
    
    if (!current_text || current_text[0] == '\0') return "";

    const char *last_word = current_text;
    
    for (int index = (int) strlen(current_text) - 1; index >= 0; index--) {
        
        if (current_text[index] == ' ' || current_text[index] == '-') {
            
            last_word = &current_text[index + 1];
            
            break;
            
        }
        
    }

    size_t len = strlen(last_word);
    
    if (len == 0 || len >= MAX_WORD_LENGTH) return "";

    char upper_last_word[MAX_WORD_LENGTH];
    
    strncpy(upper_last_word, last_word, sizeof(upper_last_word) - 1);
    
    upper_last_word[sizeof(upper_last_word) - 1] = '\0';
    
    for (int i = 0; upper_last_word[i]; i++) {
        
        upper_last_word[i] = toupper((unsigned char) upper_last_word[i]);
        
    }

    for (int index = 0; index < dictionary_word_count; index++) {
        
        if (strncmp(dictionary_words[index], upper_last_word, len) == 0) {
            
            // return the predicted part, matching the case of what the user typed?
            // Actually, we can just return the uppercase dictionary word's remainder.
            return dictionary_words[index] + len;
            
        }
        
    }

    return "";
    
}

// ---------------------------------------------------------------------------------------------- //

void cleanup_dictionary() {
    
    for (int index = 0; index < dictionary_word_count; index++) {
        
        free(dictionary_words[index]);
        
    }
    
    dictionary_word_count = 0;
    
}

// ---------------------------------------------------------------------------------------------- //

#endif

// ---------------------------------------------------------------------------------------------- //
