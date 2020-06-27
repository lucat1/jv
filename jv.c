#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

#include <json-c/json.h>

const char *LINE = " %-14s ┃ %-5s ┃ %s\n";

int strict = 0;

// used to handle errors outputted by the `handle` function
int line_count = 0;
char message[256];

json_object *get(json_object *obj, char *key) {
  json_object *value;
  json_object_object_get_ex(obj, key, &value);
  return value;
}

int handle(char *line, size_t size) {
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
        snprintf(
          message,
          sizeof message,
          "invalid JSON at line: %i",
          line_count
          ); 
        return 1;
      } else {
        printf(LINE, "", "raw", line);
        return 0;
      }
  }

  // we assume that "time" and "level" are available in each line
  json_object *raw_time = get(root, "time");
  time_t time = (time_t) json_object_get_int64(raw_time);
  char time_str[32];
  strftime(time_str, sizeof(time_str), "%D %R", localtime(&time));

  json_object *raw_level = get(root, "level");
  const char *level = json_object_get_string(raw_level);

  printf(LINE, time_str, level, "");
  return 0;
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
    if(handle(line, size)) {
      fprintf(stderr, "Fatal: %s\n", message);
      break;
    }
  }

  // free the used memory
  free(line);
  line = NULL;
}
