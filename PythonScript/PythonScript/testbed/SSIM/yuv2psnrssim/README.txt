yuv2psnrssim


yuv2psnrssim calculates PSNR and SSIM objetive quality metrics of two given
uncompressed video sequences. These sequences can be either in uncompressed
YUV format or in uncompressed Y4M format. Both formats hold the same
video information, Y4M having an internal header used to store useful
metadata like resolution, colorspace, frame rate, etc.

yuv2psnrssim supports up to 16 bits per sample, 4:2:0, 4:2:2 and 4:4:4
chroma subsampling schemes and can output results in the Excel friendly
CSV format.


Example usage:

    -orig <original file> -comp <compressed file> [other options]


Options:

    -o, -orig, -original <string>      Relative path to the original file.
                                       Required.

    -c, -comp, -compressed <string>    Relative path to the compressed file.
                                       Required.

    -r, -res, -resolution <intxint>    Resolution of the two files. They must
                                       be the same. Required for YUV input.
                                       Format: widthxheight

    -csp, -colorspace <string>         Colorspace of the two files. They must
                                       be the same. It can be yuv420, yuv422 or
                                       yuv444. Default is yuv420.

    -ob, -orig_bit_depth <integer>     Sample bit depth of the original file.
                                       Default is 8 bits.

    -cb, -comp_bit_depth <integer>     Sample bit depth of the compressed file.
                                       Default is 8 bits.

    -csv_output_file <string>          Relative path to the .csv results file.

    -csv_decimal_separator <string>    Decimal separator used in the .csv file.
                                       Use "comma" or "," in European regions,
                                       "dot" or "." in American regions.
                                       Default is "." dot.

    -no_results                        Don't print results to the console.