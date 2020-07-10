#include "main.h"

void imageSegmentation(hls::stream<uint_8_side_channel> &input, hls::stream<int_8_side_channel> &output, char threshold)
{
#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=threshold bundle=CRTL_BUS

	// Defining the line buffer and setting the inter dependency to false through pragmas

	// Index used to keep track of row,col


	//int pixConvolved = 0;
	// Calculate delay to fix line-buffer offset
	int waitTicks = (IMG_COLS*(-1)+3)/2;// 241;
	int countWait = 0;
	int sentPixels = 0;


	int_8_side_channel dataOutSideChannel;
	uint_8_side_channel currPixelSideChannel;




	// Iterate on all pixels for our 320x240 image, the HLS PIPELINE improves our latency
	for (int idxPixel = 0; idxPixel < (IMG_COLS*IMG_ROWS); idxPixel++)
	{
#pragma HLS PIPELINE
		// Read and cache (Block here if FIFO sender is empty)
		currPixelSideChannel = input.read();

		// Get the pixel data
		unsigned char pixelIn = currPixelSideChannel.data;


		//Threshold segmentation algorithm
		short valOutput = 0;
		short val = (short)pixelIn;

		//short val = (short)lineBuff.getval(IMG_HEIGHT_OR_ROWS,IMG_WIDTH_OR_COLS);

		if( val > (short)threshold)
		{
			val=(short)threshold;
		}
		else
			val=0;
		valOutput=val;
		if (valOutput < 0)
		{
							valOutput = 0;
		}
//					pixConvolved++;



		countWait++;
		if (countWait > waitTicks)
		{
			dataOutSideChannel.data = valOutput;
			dataOutSideChannel.keep = currPixelSideChannel.keep;
			dataOutSideChannel.strb = currPixelSideChannel.strb;
			dataOutSideChannel.user = currPixelSideChannel.user;
			dataOutSideChannel.last = 0;
			dataOutSideChannel.id = currPixelSideChannel.id;
			dataOutSideChannel.dest = currPixelSideChannel.dest;
			// Send to the stream (Block if the FIFO receiver is full)
			output.write(dataOutSideChannel);
			sentPixels++;
		}
	}

	// Now send the remaining zeros (Just the (Number of delayed ticks)
	for (countWait = 0; countWait < waitTicks; countWait++)
	{
		dataOutSideChannel.data = 0;
		dataOutSideChannel.keep = currPixelSideChannel.keep;
		dataOutSideChannel.strb = currPixelSideChannel.strb;
		dataOutSideChannel.user = currPixelSideChannel.user;
		// Send last on the last item
		if (countWait < waitTicks - 1)
			dataOutSideChannel.last = 0;
		else
			dataOutSideChannel.last = 1;
		dataOutSideChannel.id = currPixelSideChannel.id;
		dataOutSideChannel.dest = currPixelSideChannel.dest;
		// Send to the stream (Block if the FIFO receiver is full)
		output.write(dataOutSideChannel);
	}
}

