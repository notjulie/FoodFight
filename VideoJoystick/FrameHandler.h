/*
 * FrameHandler.h
 *
 *  Created on: Oct 20, 2018
 *      Author: pi
 */

#ifndef FRAMEHANDLER_H_
#define FRAMEHANDLER_H_


class FrameHandler
{
public:
	FILE *file_handle;                   /// File handle to write buffer data to.
	int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
	FILE *pts_file_handle;               /// File timestamps
	int frame;
	int64_t starttime;
	int64_t lasttime;
};


#endif /* FRAMEHANDLER_H_ */
