#include <stdio.h>
#include <stdbool.h>

void usage(void) {
  printf("Usage: btf [option] [file.btf]\n");
  printf("-w | --write : make am image file from your input\n");
  printf("-h | --help : display help\n");
  printf("\n");
  printf("made by github.com/waveper\n");
}

char *get_btf_data(const char *input) {
  return NULL; // TODO: get file metadata and pixel data
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    usage();
    return 1;
  }

  bool write_mode = false;

  if (argv[1] == "-h" | argv[1] == "--help") {
    usage();
    return 0;
  } else if (argv[1] == "-w" | argv[1] == "--write") {
    write_mode = true;
  }

  if (write_mode) {
    if (argv[2] == NULL) {
      printf("Error: please input the image file\n");
      return 2;
    }
  } else {
    if (argv[2] == NULL) {
      printf("Error: please input the image file\n");
      return 2;
    }
    get_btf_data(argv[2]); // when there's no option
  }

  return 0;
}
