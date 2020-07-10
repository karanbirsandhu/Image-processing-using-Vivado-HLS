#include <hls_video.h>
#define IMG_COLS 239
#define IMG_ROWS 240

// Image file path
#define INPUT_IMAGE   "C:\\image\\1918.bmp"
#define OUTPUT_IMAGE  "C:\\image\\result1.bmp"


// Use the axi stream side-channel (TLAST,TKEEP,TUSR,TID)
#include <ap_axi_sdata.h>
typedef ap_axiu<8,2,5,6> uint_8_side_channel;
typedef ap_axis<8,2,5,6> int_8_side_channel;

void imageSegmentation(hls::stream<uint_8_side_channel> &inStream, hls::stream<int_8_side_channel> &outStream,char  threshold);
