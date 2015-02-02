
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


bool image_load_file(const char *filename, image *img);
bool image_save_file(const char *filename, image *img);
void image_free(image *img);
int  image_usable_bytes(image *img);
bool image_data_apply_stream(unsigned char *data, unsigned char *msg_raw, int msg_len, int bit_per_byte);
bool image_data_extract_stream(unsigned char *data, unsigned char *buf, int buf_len, int bit_per_byte);
bool image_get_stream_info(image *img, image_stream_info *info);
bool image_set_stream_info(image *img, image_stream_info *info);
int  image_stream_info_len(image_stream_info *info);
int  image_capacity(image *img, int bit_per_byte);
bool image_apply_data(image *img, unsigned char *msg, int msg_len, int bit_per_byte);
bool image_extract_data(image *img, unsigned char *buf, int buf_len);
bool image_file_extract_data(const char *filename, unsigned char *buf, int buf_len);
bool image_file_extract_data_to_file(const char *img_file_name, const char *out_file_name);
bool image_file_apply_data(
    const char *in_file_name, const char *out_file_name,    /* can be NULL */
    unsigned char *buf, int buf_len,
    int bit_per_byte /* set to 0 for automatic detecting suitable value */ );
bool image_optimize_pngfile(const char *pngfile, int level, int verbose);

#endif // LIBHII_H
