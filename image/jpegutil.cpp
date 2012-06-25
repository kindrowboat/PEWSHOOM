// $jpegutil.cpp 3.0 milbo$ JPEG libray hooks

#include "stasm.hpp"

extern "C"
{
#define XMD_H   /* get cdjpeg.h to not issue compiler warnings */
#include "cdjpeg.h"
#ifdef _WIN32
extern djpeg_dest_ptr jinit_write_bmp1(j_decompress_ptr cinfo, boolean is_os2); // TODO why is this needed?
#endif
}

#define JMESSAGE(code,string)   string ,

static const char * const cdjpeg_message_table[] = /* the add-on message string table */
{
    #include "../jpeg/cderror.h"
    NULL
};

static const char *sgJpegFile;  // for error reporting

//-----------------------------------------------------------------------------
// TODO this is just a hack but it at least prints a
// decent error msg before dying

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
char s[SLEN];
(*cinfo->err->format_message)(cinfo, s);
Err("%s: %s", sGetBaseExt(sgJpegFile), s);
exit(-1);   // needed under Windows where Err doesn't exit
}

//-----------------------------------------------------------------------------
// Return NULL on success else return an error msg

char *sConvertJpgFileToBmpFile (const char sJpegFile[], FILE *pOutFile, int Width)
{
sgJpegFile = sJpegFile;                     // for error reporting
static struct jpeg_decompress_struct cinfo;
static struct jpeg_error_mgr jerr;
djpeg_dest_ptr dest_mgr = NULL;
FILE *pInFile = NULL;
JDIMENSION num_scanlines;

/* Initialize the CONVERT_JPEG_TO_BMP decompression object with default error handling. */
cinfo.err = jpeg_std_error(&jerr);
jerr.error_exit = my_error_exit;
jpeg_create_decompress(&cinfo);

/* Add some application-specific error messages (from cderror.h) */
jerr.addon_message_table = cdjpeg_message_table;
jerr.first_addon_message = JMSG_FIRSTADDONCODE;
jerr.last_addon_message = JMSG_LASTADDONCODE;

pInFile = fopen(sJpegFile, READ_BINARY);
if (!pInFile)
    {
    static char sErr[SLEN];
    sprintf(sErr, "Can't open %s", sJpegFile);
    return sErr;
    }

/* Specify data source for decompression */
jpeg_stdio_src(&cinfo, pInFile);

/* Read file header, set default decompression parameters */
(void)jpeg_read_header(&cinfo, true);

cinfo.err->trace_level = 0;
cinfo.dct_method = JDCT_ISLOW;  // milbo: this is the default in their vc libs

extern djpeg_dest_ptr jinit_write_bmp1 (j_decompress_ptr cinfo, boolean is_os2); // TODO why is this needed?
dest_mgr = jinit_write_bmp1(&cinfo, false);

dest_mgr->output_file = pOutFile;

if (Width && int(cinfo.image_width) != Width)
    {
    cinfo.scale_num = Width;
    cinfo.scale_denom = cinfo.output_width;
    }

(void)jpeg_start_decompress(&cinfo);            /* start decompressor */

(*dest_mgr->start_output)(&cinfo, dest_mgr);    /* write output file header */

while (cinfo.output_scanline < cinfo.output_height)     /* process data */
    {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
                                        dest_mgr->buffer_height);

    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
    }

/* Finish decompression and release memory.
 * I must do it in this order because output module has allocated memory
 * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
 */
(*dest_mgr->finish_output) (&cinfo, dest_mgr);
(void)jpeg_finish_decompress(&cinfo);

jpeg_destroy_decompress(&cinfo);

fclose(pInFile);

return NULL;    // success
}
