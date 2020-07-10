#include "hls_video.h"
#define IMG_WIDTH 320
#define IMG_HEIGHT 240

// Image file path
#define INPUT_IMAGE       "C:\\image\\input.bmp"
#define OUTPUT_IMAGE "C:\\image\\result.bmp"



// Use the axi stream side-channel (TLAST,TKEEP,TUSR,TID)
#include <ap_axi_sdata.h>
typedef ap_axiu<8,2,5,6> uint_8_channel;
typedef ap_axis<8,2,5,6> int_8_channel;


// Our IP core
void doImgProc(hls::stream<uint_8_channel> &input, hls::stream<int_8_channel> &output, char kernel[3*3]);
short sumWindow(hls::Window<3,3,short> *window);

