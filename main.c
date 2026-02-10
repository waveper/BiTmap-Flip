#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void usage(void) {
  printf("Usage: btf [option] [file.btf]\n");
  printf("-w | --write : make am image file from your input\n");
  printf("-h | --help : display help\n");
  printf("\n");
  printf("made by github.com/waveper\n");
}

char *get_btf_data(const char *input) { // get all data from binary (metadata + pixel data), it will be extractee later
  // also verify the file
  FILE *file = fopen(input, "rb");
  if (file == NULL) {
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return NULL;
  }
  long file_size = ftell(file);
  if (file_size < 7) {
    fclose(file);
    return NULL;
  }
  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return NULL;
  }

  unsigned dat_len = (unsigned)file_size;
  unsigned char *output = malloc((dat_len * 8) + 1); // + 1 for null-terminator, also multiply with 8 because each pixel is 1-bit by design and we going to convert data in the file into string of binary

  for (unsigned i = 7; i < (unsigned)file_size; i++) {
    snprintf(output, 8, "\X20", file[i]); // TODO: actual functioning read file to binary string
  }
  
  // output as strings of binaries
  return output;
}

char *extract_btf_pdata(const char *input, int pixels) { // extract pixel data from string of binary and output as extracted pixel data (binary string)
  unsigned dat_len = sizeof(input) - (7 * 8); // ignore 7 bytes of metadata, focus on pixel data
  unsigned char *output = malloc(dat_len + 1);

  for (int i = 7 * 8; i < (pixels + (7 * 8)); i++) { // pixels varuable is the numbers of pixels calculated from (width * height)
    output[i] = input[i];
  }
  output[i + 1] = '\0';
  
  return output;
}

int extract_btf_width(const char *input) {
  uint16_t output; // 16-bit output
  char *bin[(2 * 8) + 1]; // binary buffer

  for (int i = 4 * 8; i < (4 * 8) + (2 * 8); i++) {
    bin[i / (4 * 8)] = input[i];
  }
  bin[17] = '\0';
  output  = strtoll(bin, NULL, 2);

  return output;
}

int extract_btf_height(const char *input) {
  uint16_t output; // 16-bit output
  char *bin[(2 * 8) + 1]; // binary buffer

  for (int i = 6 * 8; i < (6 * 8) + (2 * 8); i++) {
    bin[i / (6 * 8)] = input[i];
  }
  bin[17] = '\0';
  output  = strtoll(bin, NULL, 2);

  return output;
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
    if (argc < 3 || argv[2] == NULL) {
      printf("Error: please input the image file\n");
      return 2;
    }
  } else {
    if (argc < 3 || argv[2] == NULL) {
      printf("Error: please input the image file\n");
      return 2;
    }
    char *image_data = get_btf_data(argv[2]); // when there's no option
    if (image_data == NULL) {
      printf("Error: null returned....\n");
      return 3;
    }
  }

  return 0;
}
