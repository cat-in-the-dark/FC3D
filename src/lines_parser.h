typedef struct ModelLines {
  int* lines;
  int n_lines;
} ModelLines;

static int is_line_ending(const char* p, unsigned int i, unsigned int end_i) {
  if (p[i] == '\0') return 1;
  if (p[i] == '\n') return 1; /* this includes \r\n */
  if (p[i] == '\r') {
    if (((i + 1) < end_i) && (p[i + 1] != '\n')) { /* detect only \r case */
      return 1;
    }
  }
  return 0;
}

/* http://stackoverflow.com/questions/5710091/how-does-atoi-function-in-c-work
 */
static int my_atoi(const char* c) {
  int value = 0;
  int sign = 1;
  if (*c == '+' || *c == '-') {
    if (*c == '-') sign = -1;
    c++;
  }
  while (((*c) >= '0') && ((*c) <= '9')) { /* isdigit(*c) */
    value *= 10;
    value += (int) (*c - '0');
    c++;
  }
  return value * sign;
}

static void skip_space(const char** token) {
  while ((*token)[0] == ' ' || (*token)[0] == '\t') {
    (*token)++;
  }
}

static int until_space(const char* token) {
  const char* p = token;
  while (p[0] != '\0' && p[0] != ' ' && p[0] != '\t' && p[0] != '\r') {
    p++;
  }

  return (int) (p - token);
}

static int parseInt(const char** token) {
  int i = 0;
  skip_space(token);
  i = my_atoi((*token));
  (*token) += until_space((*token));
  return i;
}

static void skip_line(const char** buf) {
  while ((*buf)[0] != '\0' && (*buf)[0] != '\n' && (*buf)[0] != '\r') {
    (*buf) += 1;
  }
  (*buf) += 1;
}

static int readModelLines(const char* fileText, ModelLines* modelLines) {
  ModelLines ml;
  ml.n_lines = 0;

  if (fileText != NULL) {
    const char* buf = fileText;

    // count number of line-tags
    while (*buf != '\0') {
      if (*buf == 'l' && *(buf + 1) == ' ') {
        ml.n_lines += 1;
        skip_line(&buf);
      } else {
        skip_line(&buf);
      }
    }

    ml.lines = (int*) calloc(ml.n_lines * 2, sizeof(int));

    // read lines data
    buf = fileText;
    int i = 0;
    while (*buf != '\0') {
      if (*buf == 'l' && *(buf + 1) == ' ') {
        buf += 2;  // skip l and space
        ml.lines[i] = parseInt(&buf);
        ml.lines[i + 1] = parseInt(&buf);
        i += 2;
      } else {
        skip_line(&buf);
      }
    }
  } else {
    return -1;  // file not found
  }

  *modelLines = ml;
  return 0;
}

ModelLines LoadModelLines(const char* fileName) {
  ModelLines ml;
  char* fileText = LoadFileText(fileName);
  int err = readModelLines(fileText, &ml);
  if (err != 0) {
    TraceLog(LOG_ERROR, "LINES: err=%d", err);
  }
  for (int i = 0; i < ml.n_lines; i++) {
    printf("[%d %d] ", ml.lines[2 * i], ml.lines[2 * i + 1]);
  }
  printf("\n");
  TraceLog(LOG_INFO, "LINE: [%s] n_lines=%d", fileName, ml.n_lines);
  return ml;
}

void UnloadModelLines(ModelLines modelLines) {
  free(modelLines.lines);
}
