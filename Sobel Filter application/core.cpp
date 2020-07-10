#include "core.h"

void doImgProc(hls::stream<uint_8_channel> &input, hls::stream<int_8_channel> &output, char kernel[3*3])
{
#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=kernel bundle=KERNEL_BUS

	// Defining the line buffer and setting the inter dependency to false through pragmas
	hls::LineBuffer<3,IMG_WIDTH,unsigned char> lineBuff;
	hls::Window<3,3,short> window;

	// Index used to keep track of row,col
	int idxCol = 0;
	int idxRow = 0;
	int procPix = 0;
//delay to fix line-buffer offset
	int wait = (IMG_WIDTH*(3-1)+3)/2;// 241;
	int countWait = 0;



	int_8_channel dataOutSideChannel;
	uint_8_channel pixelSideChannel;

	// Iterate on all pixels for our 320x240 image, the HLS PIPELINE allows parallelization
	for (int index = 0; index < (IMG_WIDTH*IMG_HEIGHT); index++)
	{
#pragma HLS PIPELINE
		// Read and cache
		pixelSideChannel = input.read();

		// Get the pixel data
		unsigned char pixelIn = pixelSideChannel.data;

		// Put data on the LineBuffer
		lineBuff.shift_up(idxCol);
		lineBuff.insert_top(pixelIn,idxCol);

		// Put data on the window and multiply with the kernel
		for (int wRow = 0; wRow < 3; wRow++)
		{
			for (int wCol = 0; wCol < 3; wCol++)
			{

				short val = (short)lineBuff.getval(wRow,wCol+procPix);

				// Multiply kernel by the sampling window
				val = (short)kernel[(wRow*3) + wCol ] * val;
				window.insert(val,wRow,wCol);
			}
		}

		// Avoid calculate out of the image boundaries and if we can convolve
		short valOutput = 0;
		if ((idxRow >= 3-1) && (idxCol >= 3-1))
		{
				// Convolution
				valOutput = sumWindow(&window);
				valOutput = valOutput / 8;
				// Avoid negative values
				if (valOutput < 0)
					valOutput = 0;
			procPix++;
		}

		// Calculate row and col index
		if (idxCol < IMG_WIDTH-1)
		{
			idxCol++;
		}
		else
		{

			idxCol = 0;
			idxRow++;
			procPix = 0;
		}

		countWait++;
		if (countWait > wait)
		{
			dataOutSideChannel.data = valOutput;
			dataOutSideChannel.keep = pixelSideChannel.keep;
			dataOutSideChannel.strb = pixelSideChannel.strb;
			dataOutSideChannel.user = pixelSideChannel.user;
			dataOutSideChannel.last = 0;
			dataOutSideChannel.id = pixelSideChannel.id;
			dataOutSideChannel.dest = pixelSideChannel.dest;
			// Send to the stream (Block if the FIFO receiver is full)
			output.write(dataOutSideChannel);

		}
	}

	// Now send the remaining zeros (Just the (Number of delayed ticks)
	for (countWait = 0; countWait < wait; countWait++)
	{
		dataOutSideChannel.data = 0;
		dataOutSideChannel.keep = pixelSideChannel.keep;
		dataOutSideChannel.strb = pixelSideChannel.strb;
		dataOutSideChannel.user = pixelSideChannel.user;
		// Send last on the last item
		if (countWait < wait - 1)
			dataOutSideChannel.last = 0;
		else
			dataOutSideChannel.last = 1;
		dataOutSideChannel.id = pixelSideChannel.id;
		dataOutSideChannel.dest = pixelSideChannel.dest;
		// Send to the stream (Block if the FIFO receiver is full)
		output.write(dataOutSideChannel);
	}
}

short sumWindow(hls::Window<3,3,short> *window)
{
	short sum = 0;

	// Iterate on the window multiplying and accumulating the kernel and sampling window
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			sum = sum + (short)window->getval(i,j);
		}
	}
	return sum;
}
