#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include "jsmn.h"

int verbose = 0;

// used to handle errors outputted by the `handle` function
int error = 0;
char *message = NULL;

void handle(jsmn_parser *p, char *line, size_t size) {
  jsmntok_t t[256]; // tokens
  int r = jsmn_parse(p, line, size, t, sizeof(t) / sizeof(t[0]));

  if(r < 0) {
    switch(r) {
      case JSMN_ERROR_INVAL:
      case JSMN_ERROR_PART:
        // the string is clearly valid, so we print is as raw
        // but ONLY when the mode is `verbose`
        if (verbose) {
          printf("RAW: %s", line);
        }
        return;
      default:
        // otherwise we return with an error and quit the program
        error = 1;
        message = "Fatal: JSON line is too long\n";
        return;
      }
  }

  if(t[0].type != JSMN_OBJECT) {
    printf("%i\n",t[0].type);
    error = 1;
    message = "Fatal: only JSON objects are accepted\n";
    return;
  }

  // iterate over all the fields of the log object
  for(int i = 1; i < r; i++) {

    if(t[i].type == JSMN_STRING) {
      int size = t[i].end - t[i].start;
      char *substr[size];
      memcpy(substr, &line[t[i].start], size);
      printf("string: %s\n", substr);
    }
  }
}

int main(int argc, char *argv[]) {
  char c;
  while((c = getopt(argc, argv, "hv")) != -1) {
    switch(c) {
      case 'h':
        fprintf(stdout, "usage: jv [-h] [-v]\n");
        exit(0);
        break;
      case 'v':
        verbose = 1;
        break;
    }
  }

  char *line = NULL;
  size_t size = 0;

  jsmn_parser p; // parser
  jsmn_init(&p); // initialize the parser only once

  while((getline(&line, &size, stdin)) >= 0) {
    handle(&p, line, size);

    if(error) {
      fprintf(stderr, message);
      break;
    }
  }

  // free the used memory
  free(line);
  line = NULL;
}
