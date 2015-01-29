
#if !defined(LIBHII_H)
#define LIBHII_H 1

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int x;
    int y;
    int comp;
    unsigned char *data;
} image;

typedef struct
{
    unsigned int bit_per_byte;
    uint32_t len;
} image_stream_info;

#define STREAM_HEADER_LEN 40     // in bytes

bool image_load_file(const char *filename, image *img);
bool image_save_file(const char *filename, image *img);
void image_free(image *img);
int image_usable_bytes(image *img);
bool image_data_apply_stream(unsigned char *data, unsigned char *msg_raw, int msg_len, int bit_per_byte);
bool image_data_extract_stream(unsigned char *data, unsigned char *buf, int buf_len, int bit_per_byte);
bool image_get_stream_info(image *img, image_stream_info *info);
bool image_set_stream_info(image *img, image_stream_info *info);
int image_stream_info_len(image_stream_info *info);
int image_capacity(image *img, int bit_per_byte);
bool image_apply_data(image *img, unsigned char *msg, int msg_len, int bit_per_byte);
bool image_extract_data(image *img, unsigned char *buf, int buf_len);

#endif // LIBHII_H
