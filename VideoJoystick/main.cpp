
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

#include <stdexcept>

#define VERSION_STRING "v1.3.15"

#include <bcm2835.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_connection.h"

extern "C" {
	#include "RaspiCamControl.h"
	#include "RaspiCLI.h"
};

#include <semaphore.h>
#include "CommandProcessor.h"
#include "FrameGrabber.h"
#include "FrameHandler.h"
#include "SocketListener.h"


/// Interval at which we check for an failure abort during capture
const int ABORT_INTERVAL = 100; // ms


extern "C" {
	int mmal_status_to_int(MMAL_STATUS_T status);
	static void signal_handler(int signal_number);
};


static bool terminateRequested = false;



/**
 * Checks if specified port is valid and enabled, then disables it
 *
 * @param port  Pointer the port
 *
 */
static void check_disable_port(MMAL_PORT_T *port)
{
	if (port && port->is_enabled)
		mmal_port_disable(port);
}

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
		vcos_log_error("Aborting program\n");
		exit(130);
	}

}


/**
 * main
 */
int main(int argc, const char **argv)
{
	// initialize access to the GPU
	bcm_host_init();

	// initialize the bcm2835 library
	if (!bcm2835_init())
	{
		printf("Error initializing bcm2835 library\n");
		return -1;
	}

	bcm2835_pwm_set_data(18, BCM2835_GPIO_FSEL_ALT5);

	bcm2835_pwm_set_clock(1000);
	bcm2835_pwm_set_mode(0,1,1);
	bcm2835_pwm_set_range(0,1024);
	bcm2835_pwm_set_data(0,300);

	// set up our socket listener... basically a TCP command line for diagnostics
	SocketListener socketListener([](const std::string &s){return Commander.ProcessCommand(s);});

	// Our main objects..
	FrameGrabber frameGrabber;
	FrameHandler frameHandler;
	int exit_code = EX_OK;

	// add our command handlers
	Commander.AddHandler("shutdown", [](std::string){ terminateRequested = true; return std::string(); });
	Commander.AddHandler("getImage", [&](std::string){ return frameHandler.GetImageAsString(); });

	MMAL_STATUS_T status = MMAL_SUCCESS;

	// Register our application with the logging system
	vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

	signal(SIGINT, signal_handler);

	// Disable USR1 for the moment - may be reenabled if go in to signal capture mode
	signal(SIGUSR1, SIG_IGN);


	// Now set up our components
	if ((status = frameGrabber.CreateCameraComponent()) != MMAL_SUCCESS)
	{
		vcos_log_error("%s: Failed to create camera component", __func__);
		exit_code = EX_SOFTWARE;
	}
	else
	{
		status = MMAL_SUCCESS;

		if (status == MMAL_SUCCESS)
		{
			// Enable the camera video port and tell it its callback function
			status = frameGrabber.SetupFrameCallback([&](const std::shared_ptr<VideoFrame> &frame) {
				// process it
				frameHandler.HandleFrame(frame);
			});

			if (status != MMAL_SUCCESS)
			{
				vcos_log_error("Failed to setup camera output");
				goto error;
			}

			// Send all the buffers to the camera video port
			{
				int num = mmal_queue_length(frameGrabber.camera_pool->queue);
				int q;
				for (q=0;q<num;q++)
				{
					MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(frameGrabber.camera_pool->queue);

					if (!buffer)
						vcos_log_error("Unable to get a required buffer %d from pool queue", q);

					if (mmal_port_send_buffer(frameGrabber.GetVideoPort(), buffer)!= MMAL_SUCCESS)
						vcos_log_error("Unable to send a buffer to camera video port (%d)", q);
				}
			}

			// start grabbing frames
			frameGrabber.StartCapturing();

			// watch for signal to exit
			while (!terminateRequested)
			{
				vcos_sleep(ABORT_INTERVAL);
			}
		}
		else
		{
			mmal_status_to_int(status);
			vcos_log_error("%s: Failed to connect camera to preview", __func__);
		}

		error:

		mmal_status_to_int(status);

		// Disable all our ports that are not handled by connections
		check_disable_port(frameGrabber.GetVideoPort());
		frameGrabber.DisableCamera();

		frameGrabber.DestroyCameraComponent();
	}

	if (status != MMAL_SUCCESS)
		raspicamcontrol_check_configuration(128);

	bcm2835_close();
	return exit_code;
}

