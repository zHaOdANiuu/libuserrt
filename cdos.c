#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static void printEscaped(FILE* fp, const char* s, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    unsigned char c = s[i];
    switch (c) {
      case '"':
        fputs("\\\"", fp);
        break;
      case '\\':
        fputs("\\\\", fp);
        break;
      case '\n':
        fputs("\\n", fp);
        break;
      case '\r':
        fputs("\\r", fp);
        break;
      case '\t':
        fputs("\\t", fp);
        break;
      case '\b':
        fputs("\\b", fp);
        break;
      case '\f':
        fputs("\\f", fp);
        break;
      default:
        fputc(c, fp);
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 1) {
    printf("Error args");
    return 1;
  }

  FILE* inputFile = fopen(argv[1], "r");
  if (!inputFile) {
    perror(argv[1]);
    return 1;
  }

  printf("[\n");

  bool commentEnd   = false;
  bool keywordStart = false;
  char buf[4096];
  while (fgets(buf, sizeof(buf), inputFile) != NULL) {
    if (!(buf[0] == '/' && buf[1] == '*'))
      continue;

    while (fgets(buf, sizeof(buf), inputFile) != NULL) {
      char* p = buf;
      while (*p == ' ')
        ++p;

      if (*p != '*')
        continue;
      ++p;

      if (*p == '/') {
        if (keywordStart) {
          commentEnd = true;
          printf("\"\n  }");
        }
        break;
      }

      char* c = ++p;
      while (*p == ' ')
        ++p;

      if (*p == '@') {
        char*  keyword    = ++p;
        size_t keywordLen = 0;

        while (*p != ' ' && *p != '\n') {
          ++p;
          ++keywordLen;
        }
        keyword[keywordLen] = '\0';

        char* contentHead = p;
        if (*contentHead == ' ')
          ++contentHead;
        contentHead = *contentHead == '\n' ? "" : contentHead;

        if (keywordStart)
          printf("\"\n  }");

        if (commentEnd)
          printf(",\n");

        printf("  {\n");
        printf("    \"keyword\": \"%s\",\n", keyword);
        printf("    \"content\": \"%s", contentHead);

        keywordStart = true;
      } else if (keywordStart) {
        printEscaped(stdout, c, strlen(c));
      }
    }
    keywordStart = false;
  }

  printf("\n]");

  fclose(inputFile);
  return 0;
}
