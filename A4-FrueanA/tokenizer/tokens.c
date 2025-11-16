/* TODO: Provide implementations of your tokenize function here 
* Splits an input line into tokens based on shell rules:
* Special tokens: ()<>;|
* Strings in quotes preserve special characters and whitespace
* Whitespace separates tokens unless they are inside quotes
* Uses the vect_t dynamic for storing tokens
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../vector/vect.h"
#include "tokens.h"

// Check if character is special
int special_char(char c) {
    switch (c) {
        case '(':
        case ')':
        case '<':
        case '>':
        case ';':
        case '|':
            return 1;
        default:
            return 0;
    }
}

// commit the array's item to the token list if it has an item
void push_token(vect_t *tokens, char *buffer, int *index) {
    if (*index > 0) {
        buffer[*index] = '\0';
        vect_add(tokens, buffer);
        *index = 0;
    }
}

// tokenize
vect_t *tokenize(const char *input) {
    assert(input != NULL);

    vect_t *result = vect_new();
    assert(result != NULL);

    size_t n = strlen(input);
    char *buffer = malloc(n + 1);
    assert(buffer != NULL);

    int buff_len = 0;
    int in_quotes = 0;

    for (size_t i = 0; i < n; i++) {
        char ch = input[i];

        if (in_quotes) {
            if (ch == '"') {
                push_token(result, buffer, &buff_len);
                in_quotes = 0;
            } else {
                buffer[buff_len++] = ch;
            }
        } else {
            if (ch == '"') {
                push_token(result, buffer, &buff_len);
                in_quotes = 1;
            } else if (special_char(ch)) {
                push_token(result, buffer, &buff_len);
                buffer[0] = ch;
                buffer[1] = '\0';
                vect_add(result, buffer);

                buff_len = 0;
            } else if (ch == ' ' || ch == '\t' || ch == '\n') {
                push_token(result, buffer, &buff_len);
            } else {
                buffer[buff_len++] = ch;
            }
        }
    }

    // final push if buffer is not empty
    push_token(result, buffer, &buff_len);

    free(buffer);
    return result;
}
