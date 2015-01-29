#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#include "libhii.h"

#define APP_NAME "hii"
#define APP_VERSION "0.1"

void test()
{
    image img;
    image_load_file("g.png", &img);
    int b = image_usable_bytes(&img);
    printf("%d bytes usable\n", b);

    int bit_per_byte = 5;
    unsigned char data[1000], ex[1000];
    memset(data, 0b10101010, 1000);
    image_data_apply_stream(img.data, data, 1000, bit_per_byte);

    image_save_file("o.png", &img);
    image_data_extract_stream(img.data, ex, 1000, bit_per_byte);
    printf("memcmp: %d\n", memcmp(data, ex, 1000));

    image_stream_info info, info2;
    info.len = 1000;
    info.bit_per_byte = bit_per_byte;
    image_set_stream_info(&img, &info);
    image_get_stream_info(&img, &info2);
    printf("get set info: %d\n", (info.len == info2.len && info.bit_per_byte == info2.bit_per_byte));

    unsigned char msg[100] = "haha, hehe";
    unsigned char out[100];
    image_apply_data(&img, msg, 100, bit_per_byte);
    image_extract_data(&img, out, 100);
//    printf("strcmp: %d\n", strcmp(msg, out));
    printf("memcmp: %d\n", memcmp(msg, out, 100));
    printf(out);
}

void usage(char opt, const char *self)
{
    if(opt != 0)
    {
        fprintf(stderr, "unknown option -%c\n", opt);
    }

    fprintf(stderr,
            "Usage: %s [OPTION]... [FILE] [-m MSGFILE | -t MSGTEXT] [-o OUTFILE]\n"
            "       %s -x [OPTION]... [FILE] [-m MSGFILE]\n"
            "Insert data into image.\n"
            "\n"
            "Available options are:\n"
            "    -o, --outfile OUTFILE     write output to OUTFILE.\n"
            "    -m, --msgfile MSGFILE     read msg from MSGFILE.\n"
            "    -t, --msgtext MSGTEXT     specify msg in cli.\n"
            "    -x, --extract             extract msg to MSGFILE.\n"
            "    -b, --bit N               insert N bit msg per byte.\n"
            "    -v, --verbose             verbose mode.\n"
            "    -q, --quiet               suppress error messages.\n"
            "    -s, --show-info           show some info about FILE, can be combined with -x.\n"
            "    -h, -?, --help            give this help.\n"
            "    -V, --version             show version.\n"
            "\n"
            "Default MSGFILE is \"-\" (stdin or stdout).\n"
            "\n",
            self, self);
    fprintf(stderr, APP_NAME " v" APP_VERSION "\n");
}

size_t fread_4k(void *buffer, size_t elem_size, size_t number, FILE *f)
{
    char *buf = buffer;
    assert(elem_size < 4096);
    int num = 4096 / elem_size;
    int idx = 0;

    int loop = number / num + !!(number % num);
    for(int i = 0; i < loop; i++)
    {
        int n = fread(&buf[idx], elem_size, num, f);
        idx += n * elem_size;
        if(n < num)
            break;
    }

    return idx / elem_size;
}


int main(int argc, char **argv)
{
    struct
    {
        char *infile;
        char *outfile;
        char *msgfile;
        char *msgtext;
        bool extract;
        bool show_info;
        int  msglen;
        int  verbose;
        bool quiet;
        int  bit_per_byte;
    } conf = {
        .infile       = NULL,
        .outfile      = NULL,
        .msgfile      = NULL,
        .msgtext      = NULL,
        .extract      = false,
        .show_info    = false,
        .verbose      = 0,      // todo
        .quiet        = false,  // todo
        .bit_per_byte = 0
    };

    struct option long_opts[] = {
        {"outfile",      1, NULL, 'o'},
        {"verbose",      0, NULL, 'v'},
        {"msgfile",      1, NULL, 'm'},
        {"msgtext",      1, NULL, 't'},
        {"msg",          1, NULL, 't'},
        {"extract",      0, NULL, 'x'},
        {"show-info",    0, NULL, 's'},
        {"quiet",        0, NULL, 'q'},
        {"help",         0, NULL, 'h'},
        {"bits",         1, NULL, 'b'},
        {"bit",          1, NULL, 'b'},
        {"version",      0, NULL, 'V'},
        {NULL,           0, NULL, 0}
    };

    char short_opts[] = "o:vm:t:xsqhb:V";
    int opt;

    while((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch(opt)
        {
        case 'o':
            conf.outfile = optarg;
            break;
        case 'v':
            conf.verbose++;
            break;
        case 'm':
            conf.msgfile = optarg;
            break;
        case 't':
            conf.msgtext = optarg;
            break;
        case 'x':
            conf.extract = true;
            break;
        case 's':
            conf.show_info = true;
            break;
        case 'q':
            conf.quiet = true;
            break;
        case 'h':
        case '?':
            usage(0, argv[0]);
            return EXIT_SUCCESS;
        case 'b':
            conf.bit_per_byte = atoi(optarg);
            if(!(conf.bit_per_byte < 8 && conf.bit_per_byte > 0))
            {
                fprintf(stderr, "0 < bits < 8\n");
                return EXIT_FAILURE;
            }
            break;
        case 'V':
            fprintf(stderr, APP_NAME " v" APP_VERSION "\n");
            return EXIT_SUCCESS;
            break;
        default:
            usage(opt, argv[0]);
            return EXIT_FAILURE;
        }
    }

    if(optind + 1 == argc)
    {
        conf.infile = argv[optind];
    }
    else if(optind == argc)
    {
        usage(0, argv[0]);
        return EXIT_FAILURE;
    }
    else
    {
        usage(0, argv[0]);
        return EXIT_FAILURE;
    }

    if(conf.msgfile && conf.msgtext)
    {
        fprintf(stderr, "--msgfile & --msgtext are conflict\n");
        return EXIT_FAILURE;
    }

    image img_st;
    image *img = &img_st;

    bool status = image_load_file(conf.infile, img);
    if(!status)
    {
        fprintf(stderr, "load image '%s' failed.\n", conf.infile);
        return EXIT_FAILURE;
    }

    if(conf.show_info)
    {
        char *type[] = {
            [0] = "unknown",
            [1] = "grey",
            [2] = "grey-alpha",
            [3] = "rgb",
            [4] = "rgba"
        };
        fprintf(stdout, "x: %d, y: %d, type: %s\n", img->x, img->y, type[img->comp]);

        if(conf.extract)
        {
            image_stream_info info;
            bool status = image_get_stream_info(img, &info);
            if(!status)
            {
                fprintf(stderr, "bad input file.\n");
                return EXIT_FAILURE;
            }
            fprintf(stdout, "msg length: %d, bit per byte: %d, capacity: %d\n",
                    info.len, info.bit_per_byte, image_capacity(img, info.bit_per_byte));
        }
        else
        {
            if(!conf.bit_per_byte)
            {
                fprintf(stdout, "capacities:\n");
                for(int i = 1; i < 8; i++)
                {
                    int cap = image_capacity(img, i);
                    fprintf(stdout, "\t%d bit: %d\n", i, cap);
                }
            }
            else
            {
                fprintf(stdout, "capacity: %d", image_capacity(img, conf.bit_per_byte));
            }
        }

        return EXIT_SUCCESS;
    }
    // else

    if(conf.extract)
    {
        FILE *msgfile;
        if(!conf.msgfile)
        {
            msgfile = stdout;
        }
        else
        {
            msgfile = fopen(conf.msgfile, "wb+");
        }
        if(!msgfile)
        {
            fprintf(stderr, "can't open msgfile.\n");
            return EXIT_FAILURE;
        }

        image_stream_info info;
        bool status = image_get_stream_info(img, &info);
        if(!status)
        {
            fprintf(stderr, "bad input.\n");
            return EXIT_FAILURE;
        }
        char *buf = malloc(info.len);
        assert(buf);
        status = image_extract_data(img, (unsigned char *)buf, info.len);
        int n = fwrite(buf, 1, info.len, msgfile);
        if(n != info.len)
            status = false;

        free(buf);
        image_free(img);
        fclose(msgfile);
        return !status;
    }
    // else

    if(!conf.msgtext)
    {
        if(!conf.msgfile)
        {
            conf.msgfile = "-";
        }

        FILE *msgfile;
        if(0 == strcmp(conf.msgfile, "-"))
        {
            msgfile = stdin;
        }
        else
        {
            msgfile = fopen(conf.msgfile, "rb");
            if(!msgfile)
            {
                fprintf(stderr, "can't open msgfile.\n");
                return EXIT_FAILURE;
            }
        }
        int buf_len;
        if(conf.bit_per_byte)
        {
            buf_len = image_capacity(img, conf.bit_per_byte);
        }
        else
        {
            buf_len = image_capacity(img, 7);
        }
        char *buf = malloc(buf_len);
        assert(buf);
        int n = fread_4k(buf, 1, buf_len, msgfile);
        if(!feof(msgfile))
        {
            free(buf);
            fclose(msgfile);
            fprintf(stderr, "msg too long.\n");
            return EXIT_FAILURE;
        }
        buf_len = n;

        conf.msgtext = buf;
        conf.msglen = buf_len;
    }
    else
    {
        conf.msglen = strlen(conf.msgtext);
    }

    if(!conf.bit_per_byte)
    {
        for(int i = 1; i < 8; i++)
        {
            int cap = image_capacity(img, i);
            if(cap >= conf.msglen)
            {
                conf.bit_per_byte = i;
                break;
            }
        }
    }
    if(!conf.bit_per_byte || image_capacity(img, conf.bit_per_byte) < conf.msglen)
    {
        fprintf(stderr, "msg too long.\n");
        return EXIT_FAILURE;
    }

    status = image_apply_data(img, (unsigned char *)conf.msgtext, conf.msglen, conf.bit_per_byte);
    if(!status)
        goto EXIT;

    if(!conf.outfile)
    {
        int infile_len = strlen(conf.infile);
        char *outfile = malloc(infile_len + 4 + 1);
        strcpy(outfile, conf.infile);
        char *dot = strrchr(outfile, '.');
        if(!dot)
        {
            strcat(outfile, ".out");
        }
        else
        {
            memmove(dot + 4, dot, infile_len - (dot - outfile));
            memcpy(dot, ".out", strlen(".out"));
        }
        outfile[infile_len + 4] = '\0';
        conf.outfile = outfile; // fixme: mem leak
    }
    status = image_save_file(conf.outfile, img);

//    test();

    EXIT:
    if(conf.msgfile)
        free(conf.msgtext);
    image_free(img);
    return !status;
}
