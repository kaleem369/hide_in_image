# hide_in_image
Encode data into an image using [LSB-Steganography](http://en.wikipedia.org/wiki/Steganography_tools), and output as a PNG file.
```
Usage: hii [OPTION]... [FILE] [-m MSGFILE | -t MSGTEXT] [-o OUTFILE]
       hii -x [OPTION]... [FILE] [-m MSGFILE]
       hii -s [OPTION]... [FILE]
Encode data into an image using LSB-Steganography, and output as a PNG image.
Input image supports JPEG, PNG, BMP, PSD, TGA, GIF, HDR, PIC, PNM format.

Available options are:
    -o, --outfile OUTFILE    write output to OUTFILE.
    -m, --msgfile MSGFILE    read/write msg from MSGFILE.
    -t, --msgtext MSGTEXT    specify msg in cli.
    -x, --extract            extract msg to MSGFILE.
    -b, --bit N              insert N bit msg per byte(subpixel), 0 < N < 8
    -v, --verbose            verbose mode.
    -q, --quiet              suppress error messages.
    -s, --show-info          show some info about FILE, can be combined with -x.
    -O 0-9, --optimize       optimize png file for size.
    -h, -?, --help           give this help.
    -V, --version            show version.

Default MSGFILE is "-" (stdin or stdout).

Examples:
    # encode msg.txt into g.png, write output to g.out.png
    hii g.png -m msg.txt -o g.out.png
    # extract message to stdout from g.out.png
    hii -x g.out.png
    # show some information about g.out.png
    hii -s g.out.png

    # encode 'abcdefg' into g.png
    hii g.png -t 'abcdefg' -o g.out.png
    hii -x g.out.png

Reference:
    http://en.wikipedia.org/wiki/Steganography_tools
```
hii v0.1

# install
`make`

or

`make no_optpng=1`
