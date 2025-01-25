
/**
 * \file RaspiVidYUV.c
 * Command line program to capture a camera video stream and save file
 * as uncompressed YUV420 data
 * Also optionally display a preview/viewfinder of current camera input.
 *
 * \date 7th Jan 2014
 * \Author: James Hughes
 *
 * Description
 *
 * 2 components are created; camera and preview.
 * Camera component has three ports, preview, video and stills.
 * Preview is connected using standard mmal connections, the video output
 * is written straight to the file in YUV 420 format via the requisite buffer
 * callback. Still port is not used
 *
 * We use the RaspiCamControl code to handle the specific camera settings.
 * We use the RaspiPreview code to handle the generic preview
 */

// We use some GNU extensions (basename)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <stdexcept>


// project includes
#include "CommandProcessor.h"
#include "FrameGrabber.h"
#include "FrameHandler.h"
#include "SocketListener.h"
#include "SPIDAC.h"



extern "C" {
	static void signal_handler(int signal_number);
};


static bool terminateRequested = false;


/**
 * Handler for sigint signals
 *
 * @param signal_number ID of incoming signal.
 *
 */
static void signal_handler(int signal_number)
{
	if (signal_number == SIGUSR1)
	{
		// Handle but ignore - prevents us dropping out if started in none-signal mode
		// and someone sends us the USR1 signal anyway
	}
	else
	{
		// Going to abort on all other signals
		std::cerr << "Aborting program" << std::endl;
		exit(130);
	}

}


/**
 * main
 */
int main(int argc, const char **argv)
{
	// create a command handler; we can add commands to it as we go along
	CommandProcessor commander;

	// set up our socket listener and tell it how to process commands it receives...
	// this is basically a TCP command line for diagnostics
	SocketListener socketListener([&commander](const std::string &s){return commander.ProcessCommand(s);});

	// add our command handlers
	commander.AddHandler("shutdown", [](std::string){ terminateRequested = true; return std::string(); });

	// initialize the SPIDAC and add a couple commands
	SPIDAC spiDac;
	commander.AddHandler("setX", [&spiDac](std::string param){ spiDac.sendX(atol(param.c_str())); return std::string(); });
	commander.AddHandler("setY", [&spiDac](std::string param){ terminateRequested = true; spiDac.sendY(atol(param.c_str())); return std::string(); });

	// testing... a loop that lets the SocketListener run but doesn't
	// bother with any of the camera stuff
#if 0
	for (;;)
	{
		if (terminateRequested)
			return 0;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
#endif

	// Our main objects..
	FrameHandler frameHandler;

	// command handlers supported by camera functions
	commander.AddHandler("getImage", [&](std::string){ return frameHandler.GetImageAsString(); });

	signal(SIGINT, signal_handler);

	// Disable USR1 for the moment - may be reenabled if go in to signal capture mode
	signal(SIGUSR1, SIG_IGN);


	// Now set up our components
   try
   {
        FrameGrabber frameGrabber;
        frameGrabber.CreateCameraComponent();

        // Enable the camera video port and tell it its callback function
        frameGrabber.SetupFrameCallback([&](const std::shared_ptr<VideoFrame> &frame)
        {
            // process it
            frameHandler.HandleFrame(frame);
        });

        // start grabbing frames
        frameGrabber.StartCapturing();

        // watch for signal to exit
        while (!terminateRequested)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
   }
   catch (std::exception &e)
   {
      std::cerr << e.what() << std::endl;
   }

	return 0;
}

