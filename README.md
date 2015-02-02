# hide_in_image
Encode data into an image using [LSB-Steganography](http://en.wikipedia.org/wiki/Steganography_tools), and output as a PNG file.
```
Usage: hii [OPTION]... [FILE] [-m MSGFILE | -t MSGTEXT] [-o OUTFILE]
       hii -x [OPTION]... [FILE] [-m MSGFILE]
Encode data into an image using LSB-Steganography, and output as a PNG file.

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
        hii g.png -m msg.txt -o g.out.png
        hii -x g.out.png
        hii -sx g.out.png

        hii g.png -t abcdefg -o g.out.png
        hii -x g.png
```
hii v0.1
