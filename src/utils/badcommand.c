#include <stdio.h>

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

int badcommandMissingArguments() {
    printf("Bad command: Missing arguments\n");
    return 2;
}

int badcommandTooManyTokens() {
    printf("Bad command: Too many tokens\n");
    return 3;
}

int badCommandErrorOccurred() {
    printf("Bad command: An error occurred\n");
    return 4;
}

int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 5;
}

int badcommandVariableDoesNotExist() {
    printf("Bad command: Variable does not exist in shell memory\n");
    return 6;
}

int badcommandNotAlphanum() {
    printf("Bad command: Directory/file name not alphanumeric\n");
    return 7;
}

int badcommandDirDoesNotExist() {
    printf("Bad command: Directory name does not exist\n");
    return 8;
}

int badCommandProcessTableFull() {
    printf("Bad command: Process table is full\n");
    return 9;
}

int badcommandInvalidPolicy() {
    printf("Bad command: Invalid policy selected\n");
    return 10;
}
