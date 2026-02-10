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

static uint16_t extract_btf_width(const unsigned char *input) {
  return (uint16_t)((input[3] << 8) | input[4]);
}

static uint16_t extract_btf_height(const unsigned char *input) {
  return (uint16_t)((input[5] << 8) | input[6]);
}

static size_t btf_pixel_bytes(uint16_t width, uint16_t height) {
  uint32_t pixels = (uint32_t)width * (uint32_t)height;
  return (size_t)((pixels + 7) / 8);
}

unsigned char *get_btf_data(const char *input, size_t *out_size, char output_mode) { // get all data from binary (metadata + pixel data)
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

  unsigned char *output = malloc((size_t)file_size);
  if (output == NULL) {
    fclose(file);
    return NULL;
  }

  size_t read_len = fread(output, 1, (size_t)file_size, file);
  fclose(file);
  if (read_len != (size_t)file_size) {
    free(output);
    return NULL;
  }

  if (output[0] != 'B' || output[1] != 'T' || output[2] != 'F') {
    free(output);
    return NULL;
  }

  uint16_t width = extract_btf_width(output);
  uint16_t height = extract_btf_height(output);
  size_t expected_size = 7 + btf_pixel_bytes(width, height);
  if ((size_t)file_size != expected_size) {
    free(output);
    return NULL;
  }

  if (out_size != NULL) {
    *out_size = (size_t)file_size;
  }

  if (output_mode == "d") { // image data
    return output;
  } else if (output_mode == "w") { // width
    return width;
  } else if (output_mode == "h") { // height
    return height;
  } else { // none
    return NULL;
  }
}

unsigned char *extract_btf_pdata(const unsigned char *input, uint16_t width, uint16_t height) {
  size_t pixel_bytes = btf_pixel_bytes(width, height);
  unsigned char *output = malloc(pixel_bytes);
  if (output == NULL) {
    return NULL;
  }

  memcpy(output, input + 7, pixel_bytes);
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
    size_t image_size = 0;
    unsigned char *image_data = get_btf_data(argv[2], &image_size, "d"); // when there's no option
    if (image_data == NULL) {
      printf("Error: null returned....\n");
      return 3;
    }

    // view mode
    int image_width = get_btf_data(argv[2], &image_size, "w"); // get image width
    int image_height = get_btf_data(argv[2], &image_size, "h")
    int image_bytes = btf_pixel_bytes(image_width, image_height);
    for (int i = 0; i < image_bytes; i++) {
      // later
    }

    free(image_data);
  }

  return 0;
}
