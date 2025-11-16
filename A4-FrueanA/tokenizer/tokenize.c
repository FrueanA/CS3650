#include <stdio.h>

#include "../vector/vect.h"
#include "tokens.h"

const int BUFFER_SIZE = 1024;

int main(int argc, char **argv) {
  char buffer[BUFFER_SIZE];

  if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
    vect_t *tokens = tokenize(buffer);

    if (tokens != NULL) {
      unsigned int n = vect_size(tokens);
      for (unsigned int k = 0; k < n; k++) {
        const char *word = vect_get(tokens, k);
        
        printf("%s\n", word);
      }
      vect_delete(tokens);
    }
  }

  return 0;
}
