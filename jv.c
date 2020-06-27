#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>

#include <json-c/json.h>

int strict = 0;

// used to handle errors outputted by the `handle` function
int line_count = 0;
int error = 0;
char *message = NULL;

void handle(char *line, size_t size) {
  enum json_tokener_error error;
  json_object *root = json_tokener_parse_verbose(line, &error);

  switch(error) {
    case json_tokener_success:
    case json_tokener_continue:
      // don't stop. These are file status codes
      break;

    default:
      if(strict) {
        // in strict mode we fail if the json is invalid
        error = 1;
        snprintf(
          message,
          strlen(message),
          "Fatal: invalid JSON at line %i",
          line_count
        ); 
      } else {
        printf("RAW: %s", line);
      }
      return;
  }
  
  // key and val don't exist outside of this bloc
  json_object_object_foreach(root, key, val) {
    printf("key: %s\nval: %s\n", key, json_object_get_string(val));
  }
}

int main(int argc, char *argv[]) {
  char c;
  while((c = getopt(argc, argv, "hs")) != -1) {
    switch(c) {
      case 'h':
        fprintf(stdout, "usage: jv [-h] [-v]\n");
        exit(0);
        break;
      case 's':
        strict = 1;
        break;
    }
  }

  char *line = NULL;
  size_t size = 0;

  while((getline(&line, &size, stdin)) >= 0) {
    line_count++;
    handle(line, size);

    if(error) {
      fprintf(stderr, message);
      break;
    }
  }

  // free the used memory
  free(line);
  line = NULL;
}
