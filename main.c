#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void usage(void) {
  printf("Usage: btf [option] [file.btf]\n");
  printf("-w | --write : make am image file from your input\n");
  printf("-h | --help : display help\n");
  printf("\n");
  printf("made by github.com/waveper\n");
}

char *get_btf_data(const char *input) {
  FILE *file = fopen(input, "rb");

  unsigned dat_len = strlen(file) - 7; // 7 bytes of ignore metadata, focus on pixel data
  char *output = malloc((dat_len * 8) + 1); // + 1 for null-terminator, also multiply with 8 because each pixel is 1-bit by design and we going to convert data in the file into string of binary

  int i = 0 + 7; // maybe a joke here
  for (i < strlen(file); i++) {
    // uhh, how to convert each bit into output char
  }

  return NULL; // TODO: get file metadata and pixel data
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    usage();
    return 1;
  }

  bool write_mode = false;

  if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
    usage();
    return 0;
  } else if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--write") == 0) {
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
    image_data = get_btf_data(argv[2]); // when there's no option
    if (image_data = NULL) {
      printf("Error: null returned....\n");
      return 3;
    }
  }

  return 0;
}
