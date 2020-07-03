#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include <json-c/json.h>
#include "colors.h"

#define LINE GREEN_TEXT "%-14s" COLOR_RESET " %s%-5s" COLOR_RESET " %s%s" COLOR_RESET "\n"
#define EXTRA_FORMAT "%s=%s" COLOR_RESET "; " LIGHT_BLACK_TEXT

int strict = 0;

// used to handle errors outputted by the `handle` function
int line_count = 0;
char message[256];

json_object *get(json_object *obj, char *key) {
  json_object *value;
  json_object_object_get_ex(obj, key, &value);
  return value;
}

int allocate_length(char **pair, const char *key, const char *value) {
  int len = snprintf(NULL, 0, EXTRA_FORMAT, key, value);
  if(!(*pair= malloc((len + 1)))) {
    snprintf(message, sizeof message, "could not get memory for pair");
    return -1;
  }
  
  return len;
}

char *realloc_extra(char *extra, int new_len) {
  char *new_extra = realloc(extra, new_len);
  if(new_extra == NULL) {
    snprintf(message, sizeof message, "could not get memory for extra line");
    free(extra);
    return NULL;
  }

  return new_extra;
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
        snprintf(message, sizeof message, "invalid JSON at line: %i", line_count);
        return 1;
      } else {
        printf(LINE, "", MAGENTA_TEXT, "raw", line, "");
        return 0;
      }
  }

  // we assume that "time", "level" and "message" are available in each line
  json_object *raw_time = get(root, "time");
  time_t time = (time_t) json_object_get_int64(raw_time);
  char time_str[32];
  strftime(time_str, sizeof time_str, "%D %R", localtime(&time));

  json_object *raw_level = get(root, "level");
  char *color = LIGHT_BLACK_TEXT; 
  char *level = (char *) json_object_get_string(raw_level);

  // convert the `level` to lowercase
  for(int i = 0; level[i]; i++) {
    level[i] = tolower(level[i]);
  }

  if(strcmp(level, "debug") == 0) {
    color = LIGHT_BLUE_TEXT;
  } else if (strcmp(level, "info") == 0) {
    color = BLUE_TEXT;
  } else if (strcmp(level, "success") == 0) {
    color = GREEN_TEXT;
  } else if (strcmp(level, "warn") == 0) {
    color = YELLOW_TEXT;
  } else if (strcmp(level, "error") == 0) {
    color = RED_TEXT;
  }

  json_object *raw_msg = get(root, "message");
  int len = json_object_get_string_len(raw_msg);
  char *partial_message = (char *) json_object_get_string(raw_msg);

  // with the other parameters we create an extra string
  // to be printed in the next line 
  int extra_initial_len = strlen(LIGHT_BLACK_TEXT) + 1;
  int extra_len = extra_initial_len;
  char *extra = malloc(extra_len);
  strcpy(extra, LIGHT_BLACK_TEXT);

  json_object_object_foreach(root, key, raw_value) {
    if(strcmp(key, "time") == 0 ||
        strcmp(key, "level") == 0 ||
        strcmp(key, "message") == 0) {
      continue;
    }

    const char *value = json_object_get_string(raw_value);
    char *pair;
    int len = allocate_length(&pair, key, value);
    len = snprintf(pair, len + 1, EXTRA_FORMAT, key, value);
    if(len < 0) {
      return 1;
    }

    int new_len = (extra_len + len);
    char *new_extra = realloc_extra(extra, new_len); 
    if(new_extra == NULL) {
      free(pair);
      return 1;
    }

    strcat(new_extra, pair);
    extra = new_extra;
    extra_len = new_len;
  }

  int have_extra = extra_len > extra_initial_len;
  char *msg = NULL;
  // if we have `extra` stuff we append `\n` to the msg line
  if(have_extra) {
    if(!(msg = malloc((len + 2)))) {
      snprintf(message, sizeof message, "could not get memory for log message");
      free(extra);
      return 1;
    }

    snprintf(msg, len + 2, "%s\n", partial_message);
  } else {
    msg = partial_message;
  }

  // finally print the generated line
  printf(LINE, time_str, color, level, msg, extra);

  // if the msg buffer was allocated we must free it afterwards
  if (msg) {
    free(msg);
  }
  free(extra);
  return 0;
}

int main(int argc, char *argv[]) {
  char c;
  while((c = getopt(argc, argv, "hs")) != -1) {
    switch(c) {
      case 'h':
        fprintf(stdout, "usage: jv [-h] [-s]\n");
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
