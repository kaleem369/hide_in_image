/*
 * encode data in the least significant bits of pixel
 *
 * functions return false on fail
 */

#include "libhii.h"

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"

#if ! _NO_OPTPNG
#include "opnglib/opnglib.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// todo: ignore alpha channel

typedef struct
{
    unsigned char *byte;
    int bit;
} bitptr;

// todo: optimize by using int16_t (endian?)
static unsigned char bit_read(bitptr *ptr, int nbit)
{
    assert(nbit > 0);
    assert(nbit < 8);

    unsigned char ret;

    if(ptr->bit + nbit <= 8)
    {
        ret = *(ptr->byte);
        ret <<= ptr->bit;
        ret >>= ptr->bit + (8 - ptr->bit - nbit);
    }
    else
    {
        unsigned char p1, p2;
        p1 = *(ptr->byte);
        p1 <<= ptr->bit;
        p1 >>= 8 - nbit;
        p2 = *(ptr->byte + 1);
        int rest = nbit - (8 - ptr->bit);
        p2 >>= 8 - rest;
        ret = p1 | p2;
    }

    ptr->bit += nbit;
    if(ptr->bit >= 8)
    {
        ptr->byte++;
        ptr->bit -= 8;
    }
    return ret;
}

#if 0
// not working
unsigned char bit_read_o(bitptr *ptr, int nbit)
{
    assert(nbit > 0);
    assert(nbit < 8);

    uint16_t u16 = (uint16_t)ptr->byte[1] + ((uint16_t)ptr->byte[0] << 8);
    u16 <<= ptr->bit;
    u16 >>= 16 - nbit;

    ptr->bit += nbit;
    ptr->byte += ptr->bit / 8;
    ptr->bit %= 8;

    return (unsigned char)u16;
}
#endif // 0

static void bit_write(bitptr *ptr, unsigned char bits, int nbit)
{
    assert(nbit > 0);
    assert(nbit < 8);

    int rest = 8 - ptr->bit;
    if(rest >= nbit)
    {
        unsigned char p1 = *(ptr->byte), p2 = bits, p3 = *(ptr->byte);
        p1 >>= 8 - ptr->bit;
        p1 <<= 8 - ptr->bit;
        p2 <<= rest - nbit;
        p3 <<= ptr->bit + nbit;
        p3 >>= ptr->bit + nbit;
        *(ptr->byte) = p1 | p2 | p3;
    }
    else
    {
        unsigned char p1 = bits, p2 = bits;
        p1 >>= (nbit - rest);
        p2 <<= (8 - (ptr->bit + nbit - 8));

        unsigned char value1 = *(ptr->byte);
        value1 >>= rest;
        value1 <<= rest;
        value1 |= p1;
        *(ptr->byte) = value1;

        unsigned char value2 = *(ptr->byte + 1);
        value2 <<= nbit - rest;
        value2 >>= nbit - rest;
        value2 |= p2;
        *(ptr->byte + 1) = value2;
    }

    ptr->bit += nbit;
    if(ptr->bit >= 8)
    {
        ptr->byte++;
        ptr->bit -= 8;
    }
}

#if 0
// not working
void bit_write_o(bitptr *ptr, unsigned char bits, int nbit)
{
    assert(nbit > 0);
    assert(nbit < 8);

    uint16_t u16 = (uint16_t)ptr->byte[1] + ((uint16_t)ptr->byte[0] << 8);
    uint32_t mask = 0xffff;
    mask <<= ptr->bit;
    mask >>= ptr->bit;
    mask >>= 16 - ptr->bit - nbit;
    mask <<= 16 - ptr->bit - nbit;
    mask = ~mask;
    u16 &= mask;
    uint16_t add = (uint16_t)bits << (16 - ptr->bit - nbit);
    u16 |= add;

    ptr->byte[0] = u16 >> 8;
    ptr->byte[1] = u16 & 0x00ff;

    ptr->bit += nbit;
    ptr->byte += ptr->bit / 8;
    ptr->bit %= 8;
}

#define bit_read bit_read_o
#define bit_write bit_write_o
#endif // 0

unsigned char subpixel_apply_bits(unsigned char subpixel, unsigned char bits, int nbit)
{
    unsigned char orig = subpixel;
    subpixel >>= nbit;
    subpixel <<= nbit;
    subpixel |= bits;

    if(subpixel - orig > (1 << (nbit - 1)))
    {
        int t = subpixel - (1 << nbit);
        if(t >= 0)
            subpixel = (unsigned char)t;
    }
    else if(subpixel - orig < -(1 << (nbit - 1)))
    {
        int t = subpixel + (1 << nbit);
        if(t <= 255)
            subpixel = (unsigned char)t;
    }
    return subpixel;
}

unsigned char subpixel_extract_bits(unsigned char subpixel, int nbit)
{
    subpixel <<= 8 - nbit;
    subpixel >>= 8 - nbit;
    return subpixel;
}

bool image_load_file(const char *filename, image *img)
{
    img->data = stbi_load(filename, &img->x, &img->y, &img->comp, 0);
    if(!img->data)
        return false;

    return true;
}

bool image_save_file(const char *filename, image *img)
{
    return (bool)stbi_write_png(filename, img->x, img->y, img->comp, img->data, 0);
}

void image_free(image *img)
{
    stbi_image_free(img->data);
}

int image_usable_bytes(image *img)
{
    return img->x * img->y * img->comp;
}

bool image_data_apply_stream(unsigned char *data, unsigned char *msg_raw, int msg_len, int bit_per_byte)
{
    assert(bit_per_byte < 8);

    bitptr ptr = {.byte = msg_raw, .bit = 0};

    int remain_bits = msg_len * 8;
    while(remain_bits > 0)
    {
        unsigned char bits = bit_read(&ptr, bit_per_byte);
        if(remain_bits < bit_per_byte)
        {
            bits >>= bit_per_byte - remain_bits;
            bits <<= bit_per_byte - remain_bits;
        }
        *data = subpixel_apply_bits(*data, bits, bit_per_byte);
        data++;
        remain_bits -= bit_per_byte;
    }

    return true;
}

bool image_data_extract_stream(unsigned char *data, unsigned char *buf, int buf_len, int bit_per_byte)
{
    assert(buf_len > 0);
    assert(bit_per_byte < 8);

    bitptr ptr = {.byte = buf, .bit = 0};
    int n = buf_len * 8 / bit_per_byte;
    for(int i = 0; i < n; i++)
    {
        unsigned char bits = subpixel_extract_bits(*data++, bit_per_byte);
        bit_write(&ptr, bits, bit_per_byte);
    }
    int remain_bits = buf_len * 8 - n * bit_per_byte;
    if(remain_bits)
    {
        unsigned char bits = subpixel_extract_bits(*data++, bit_per_byte);
        bits >>= bit_per_byte - remain_bits;
        bit_write(&ptr, bits, remain_bits);
    }

    return true;
}

bool image_get_stream_info(image *img, image_stream_info *info)
{
    const int buf_len = 4;
    unsigned char buf[buf_len];
    unsigned char bit_per_byte =  subpixel_extract_bits(img->data[0], 1)
                               + (subpixel_extract_bits(img->data[1], 1) << 1)
                               + (subpixel_extract_bits(img->data[2], 1) << 2);
    if(!(bit_per_byte > 0 && bit_per_byte < 8))
       return false;
    info->bit_per_byte = bit_per_byte;

    bool status = image_data_extract_stream(img->data + 3, buf, buf_len, bit_per_byte);
    if(!status)
        return false;
    info->len = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

    if(info->len > image_capacity(img, info->bit_per_byte))
        return false;

    return true;
}

bool image_set_stream_info(image *img, image_stream_info *info)
{
    unsigned char bit_per_byte = info->bit_per_byte;
    unsigned char b1 = (bit_per_byte & 0b00000001),
                  b2 = !!(bit_per_byte & 0b00000010),
                  b3 = !!(bit_per_byte & 0b00000100);
    img->data[0] = subpixel_apply_bits(img->data[0], b1, 1);
    img->data[1] = subpixel_apply_bits(img->data[1], b2, 1);
    img->data[2] = subpixel_apply_bits(img->data[2], b3, 1);

    unsigned char buf[4] = {
        info->len & 0x000000ff,
        (info->len & 0x0000ff00) >> 8,
        (info->len & 0x00ff0000) >> 16,
        (info->len & 0xff000000) >> 24
    };
    bool status = image_data_apply_stream(img->data + 3, buf, 4, info->bit_per_byte);
    if(!status)
        return false;

    return true;
}

int image_stream_info_len(image_stream_info *info)
{
    return 3 + 32 / info->bit_per_byte + !!(32 % info->bit_per_byte);
}

int image_capacity(image *img, int bit_per_byte)
{
    image_stream_info info = {.bit_per_byte = bit_per_byte};
    return (image_usable_bytes(img) - image_stream_info_len(&info)) * bit_per_byte / 8;
}

bool image_apply_data(image *img, unsigned char *msg, int msg_len, int bit_per_byte)
{
    image_stream_info info = {.len = msg_len, .bit_per_byte = bit_per_byte};

    bool status = image_set_stream_info(img, &info);
    if(!status)
        return false;
    int offset = image_stream_info_len(&info);
    assert((image_usable_bytes(img) - offset) * bit_per_byte > msg_len * 8);

    status = image_data_apply_stream(img->data + offset, msg, msg_len, bit_per_byte);
    if(!status)
        return false;

    return true;
}

bool image_extract_data(image *img, unsigned char *buf, int buf_len)
{
    image_stream_info info;
    bool status = image_get_stream_info(img, &info);
    if(!status)
        return false;
    if(buf_len < info.len)
        return false;

    int offset = image_stream_info_len(&info);
    status = image_data_extract_stream(img->data + offset, buf, buf_len, info.bit_per_byte);
    if(!status)
        return false;

    return true;
}

bool image_file_extract_data(const char *filename, unsigned char *buf, int buf_len)
{
    image img_st;
    image *img = &img_st;

    bool status = image_load_file(filename, img);
    if(!status)
        return false;

    image_stream_info info;
    status = image_get_stream_info(img, &info);
    if(!status || info.len > buf_len)
    {
        image_free(img);
        return false;
    }

    status = image_extract_data(img, buf, buf_len);
    image_free(img);
    return status;
}

bool image_file_extract_data_to_file(const char *img_file_name, const char *out_file_name)
{
    image img_st;
    image *img = &img_st;

    bool status = image_load_file(img_file_name, img);
    if(!status)
        return false;

    image_stream_info info;
    status = image_get_stream_info(img, &info);
    if(!status)
    {
        image_free(img);
        return false;
    }

    unsigned char *buf = malloc(info.len);
    assert(buf);

    status = image_extract_data(img, buf, info.len);
    image_free(img);
    if(!status)
    {
        free(buf);
        return false;
    }

    FILE *outfile = fopen(out_file_name, "wb+");
    if(!outfile)
    {
        free(buf);
        return false;
    }

    int n = fwrite(buf, 1, info.len, outfile);
    if(n != info.len)
        status = false;

    fclose(outfile);
    return status;
}

bool image_file_apply_data(
    const char *in_file_name, const char *out_file_name,    /* can be NULL */
    unsigned char *buf, int buf_len,
    int bit_per_byte /* set to 0 for automatic detecting suitable value */ )
{
    assert(bit_per_byte < 8 && bit_per_byte >= 0);

    image img_st;
    image *img = &img_st;

    bool status = image_load_file(in_file_name, img);
    if(!status)
        return false;

    if(bit_per_byte == 0)
    {
        for(int i = 1; i < 8; i++)
        {
            int cap = image_capacity(img, i);
            if(cap >= buf_len)
            {
                bit_per_byte = i;
                break;
            }
        }
        if(bit_per_byte == 0)
        {
            image_free(img);
            return false;
        }
    }

    status = image_apply_data(img, buf, buf_len, bit_per_byte);
    if(!status)
    {
        image_free(img);
        return false;
    }

    if(!out_file_name)
        out_file_name = in_file_name;

    status = image_save_file(out_file_name, img);
    image_free(img);

    return status;
}

#if 1
bool image_optimize_pngfile(const char *pngfile, int level, int verbose)
{
#if _NO_OPTPNG
    fprintf(stderr, "OptPNG not available.\n");
    return false;
#else
    opng_set_logging_name("hii");
    if(verbose)
    {
        opng_set_logging_level(OPNG_MSG_INFO);
        opng_set_logging_format(OPNG_MSGFMT_FANCY);
    }
    else
    {
        opng_set_logging_level(OPNG_MSG_WARNING);
        opng_set_logging_format(OPNG_MSGFMT_UNIX);
    }

    opng_optimizer_t *the_optimizer = opng_create_optimizer();
    if (the_optimizer == NULL)
    {
        opng_destroy_optimizer(the_optimizer);
        return false;
    }

    struct opng_options options;
    memset(&options, 0, sizeof(options));
    options.interlace = -1;
    options.optim_level = level;
//    options.verbose = 1;
//    options.out = 1;
    if (opng_set_options(the_optimizer, &options) < 0)
    {
        opng_destroy_optimizer(the_optimizer);
        return false;
    }

    opng_transformer_t *the_transformer = opng_create_transformer();
    if (the_transformer == NULL)
    {
        opng_destroy_transformer(the_transformer);
        opng_destroy_optimizer(the_optimizer);
        return false;
    }
    opng_set_transformer(the_optimizer, the_transformer);

    int ret = opng_optimize_file(the_optimizer, pngfile, pngfile, NULL);

    opng_destroy_optimizer(the_optimizer);
    opng_destroy_transformer(the_transformer);
    return !ret;
#endif
}
#endif // 0

