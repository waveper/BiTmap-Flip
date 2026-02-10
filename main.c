#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
  BiTmap-Flip (BTF) CLI overview
  - File format: "BTF" signature (3 bytes), width (2 bytes, big-endian),
    height (2 bytes, big-endian), then packed 1-bit pixels.
  - Pixels are stored row-major; the first pixel maps to bit 7 of byte 0,
    next to bit 6, and so on. Total pixel bytes = ceil(width*height/8).
  - View mode: read/validate the file, then print each pixel as '#' or ' '.
  - Write mode (-w): prompt for width/height and a 0/1 string, then pack
    bits and write the BTF header + pixel bytes.
*/

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

// Convert total pixel count to packed 1-bit byte length.
static size_t btf_pixel_bytes(uint16_t width, uint16_t height) {
  uint32_t pixels = (uint32_t)width * (uint32_t)height;
  return (size_t)((pixels + 7) / 8);
}

// Read, validate, and return full BTF file data (caller frees).
unsigned char *get_btf_data(const char *input, size_t *out_size, uint16_t *out_width, uint16_t *out_height) { // get all data from binary (metadata + pixel data)
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
  if (out_width != NULL) {
    *out_width = width;
  }
  if (out_height != NULL) {
    *out_height = height;
  }

  return output;
}

// Copy only the packed pixel bytes from a validated BTF buffer.
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
    if (argc < 3) {
      printf("Error: please input the image file\n");
      return 2;
    }
    uint16_t input_width = 0;
    uint16_t input_height = 0;
    printf("Input width: ");
    if (scanf("%hu", &input_width) != 1) {
      printf("Error: invalid width\n");
      return 3;
    }
    printf("Input height: ");
    if (scanf("%hu", &input_height) != 1) {
      printf("Error: invalid height\n");
      return 3;
    }

    size_t total_pixels = (size_t)input_width * (size_t)input_height;
    if (total_pixels == 0) {
      printf("Error: width and height must be greater than 0\n");
      return 3;
    }

    char *input_data = malloc(total_pixels + 1);
    if (input_data == NULL) {
      printf("Error: out of memory\n");
      return 3;
    }
    printf("Input binary pixel data (%zu chars of 0/1): ", total_pixels);
    if (scanf("%s", input_data) != 1) {
      free(input_data);
      printf("Error: invalid pixel data\n");
      return 3;
    }

    size_t input_len = strlen(input_data);
    if (input_len != total_pixels) {
      free(input_data);
      printf("Error: pixel data length mismatch\n");
      return 3;
    }
    for (size_t i = 0; i < input_len; i++) {
      if (input_data[i] != '0' && input_data[i] != '1') {
        free(input_data);
        printf("Error: invalid binary input\n");
        return 3;
      }
    }

    FILE *file = fopen(argv[2], "wb");
    if (file == NULL) {
      free(input_data);
      printf("Error: could not open output file\n");
      return 3;
    }

    if (fwrite("BTF", 1, 3, file) != 3) {
      fclose(file);
      free(input_data);
      printf("Error: failed to write header\n");
      return 3;
    }

    unsigned char header[4];
    header[0] = (unsigned char)(input_width >> 8);
    header[1] = (unsigned char)(input_width & 0xFF);
    header[2] = (unsigned char)(input_height >> 8);
    header[3] = (unsigned char)(input_height & 0xFF);
    if (fwrite(header, 1, 4, file) != 4) {
      fclose(file);
      free(input_data);
      printf("Error: failed to write header\n");
      return 3;
    }

    size_t pixel_bytes = btf_pixel_bytes(input_width, input_height);
    unsigned char *pixel_data = calloc(pixel_bytes, 1);
    if (pixel_data == NULL) {
      fclose(file);
      free(input_data);
      printf("Error: out of memory\n");
      return 3;
    }
    for (size_t i = 0; i < total_pixels; i++) {
      if (input_data[i] == '1') {
        size_t byte_index = i / 8;
        size_t bit_index = 7 - (i % 8);
        pixel_data[byte_index] |= (unsigned char)(1u << bit_index);
      }
    }
    if (fwrite(pixel_data, 1, pixel_bytes, file) != pixel_bytes) {
      fclose(file);
      free(input_data);
      free(pixel_data);
      printf("Error: failed to write pixel data\n");
      return 3;
    }

    fclose(file);
    free(input_data);
    free(pixel_data);
    printf("Wrote %s\n", argv[2]);

  } else {
    if (argc < 1) {
      printf("Error: please input the image file\n");
      return 2;
    }
    size_t image_size = 0;
    uint16_t image_width = 0;
    uint16_t image_height = 0;
    unsigned char *image_data = get_btf_data(argv[1], &image_size, &image_width, &image_height); // when there's no option
    if (image_data == NULL) {
      printf("Error: null returned....\n");
      return 3;
    }

    // view mode
    size_t total_pixels = (size_t)image_width * (size_t)image_height;
    size_t pixel_bytes = btf_pixel_bytes(image_width, image_height);
    if (image_size < 7 + pixel_bytes) {
      free(image_data);
      printf("Error: invalid image size\n");
      return 3;
    }
    for (size_t i = 0; i < total_pixels; i++) {
      size_t byte_index = i / 8;
      size_t bit_index = 7 - (i % 8);
      unsigned char byte = image_data[7 + byte_index];
      bool is_set = ((byte >> bit_index) & 1u) != 0;
      printf("%c", is_set ? '#' : ' ');
      if ((i + 1) % image_width == 0) {
        printf("\n");
      }
    }

    free(image_data);
  }

  return 0;
}
