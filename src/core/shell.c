#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../file_system/file_system.h"
#include "../memory/lru.h"
#include "../memory/memory.h"
#include "../scheduling/ready_queue.h"
#include "interpreter.h"

int main(int argc, char* argv[]) {
    // Set stdout to unbuffered to avoid undefined printing behaviour in batch
    // mode
    setvbuf(stdout, NULL, _IONBF, 0);

    // 1 = interactive mode, 0 = batch mode
    int is_interactive = isatty(STDIN_FILENO);

    // omit printing welcome banner if we are in barch mode
    if (is_interactive) {
        printf("Welcome to pico-shell!\n");
        printf(
            "Frame Size = %d, Frame Store Size = %d; Variable Store Size = "
            "%d\n",
            FRAME_SIZE, FRAME_STORE_SIZE, VAR_STORE_SIZE);
        printf("►  Type help to get list of commands\n\n");
    }

    char prompt = '$';               // Shell prompt
    char userInput[MAX_USER_INPUT];  // user's input stored here
    int errorCode = 0;               // zero means no error, default

    // Init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    mem_init();
    fs_init();

    while (1) {
        // omit printing prompt char if we are in batch mode
        if (is_interactive) {
            printf("%c ", prompt);
        }

        // Check if reached end of file when in batch mode
        if (fgets(userInput, MAX_USER_INPUT - 1, stdin) == NULL) {
            break;
        }

        errorCode = parseInput(userInput);

        if (errorCode == -1) {
            exit(99);  // ignore all other errors
        }

        memset(userInput, 0, sizeof(userInput));
    }

    mem_deinit();

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char inp[]) {
    char tmp[200];
    // Array storing individual commands
    char* commands[100];
    // Array to store words of current command
    char* words[100];

    int cmdCount = 0;
    int wordlen;
    int errorCode;

    // Get first command
    char* cmd = strtok(inp, ";");

    while (cmd != NULL && cmdCount < MAX_NUM_COMMANDS) {
        commands[cmdCount] = strdup(cmd);
        // Get next command. We pass in NULL param to indicate we are still
        // parsing same string 'inp'.
        cmd = strtok(NULL, ";");
        cmdCount++;
    }

    // Process each command
    for (int i = 0; i < cmdCount; i++) {
        int ix = 0;
        int wordCount = 0;

        // Skip white spaces
        for (ix = 0; commands[i][ix] == ' ' && ix < MAX_USER_INPUT; ix++);

        while (commands[i][ix] != '\n' && commands[i][ix] != '\0' &&
               ix < MAX_USER_INPUT) {
            // Extract a word
            for (wordlen = 0;
                 !wordEnding(commands[i][ix]) && ix < MAX_USER_INPUT;
                 ix++, wordlen++) {
                tmp[wordlen] = commands[i][ix];
            }

            tmp[wordlen] = '\0';
            words[wordCount] = strdup(tmp);
            wordCount++;

            if (commands[i][ix] == '\0') {
                break;
            }

            ix++;
        }

        errorCode = interpreter(words, wordCount);

        for (int j = 0; j < wordCount; j++) {
            free(words[j]);
        }

        free(commands[i]);
    }

    return errorCode;
}
