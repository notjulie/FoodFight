
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

static XREF_T  initial_map[] =
{
		{"record",     0},
		{"pause",      1},
};

static int initial_map_size = sizeof(initial_map) / sizeof(initial_map[0]);


/// Command ID's and Structure defining our command line options
#define CommandWidth        1
#define CommandHeight       2
#define CommandFramerate    7
#define CommandInitialState 11
#define CommandCamSelect    12
#define CommandSettings     13
#define CommandSensorMode   14

static COMMAND_LIST cmdline_commands[] =
{
		{ CommandWidth,         "-width",      "w",  "Set image width <size>. Default 1920", 1 },
		{ CommandHeight,        "-height",     "h",  "Set image height <size>. Default 1080", 1 },
		{ CommandFramerate,     "-framerate",  "fps","Specify the frames per second to record", 1},
		{ CommandInitialState,  "-initial",    "i",  "Initial state. Use 'record' or 'pause'. Default 'record'", 1},
		{ CommandCamSelect,     "-camselect",  "cs", "Select camera <number>. Default 0", 1 },
		{ CommandSettings,      "-settings",   "set","Retrieve camera settings and write to stdout", 0},
		{ CommandSensorMode,    "-mode",       "md", "Force sensor mode. 0=auto. See docs for other modes available", 1},
};

static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);

static bool terminateRequested = false;

/**
 * Assign a default set of parameters to the state passed in
 *
 * @param state Pointer to state structure to assign defaults to
 */
static void default_status(FrameGrabber *state)
{
	if (!state)
	{
		vcos_assert(0);
		return;
	}

	// Default everything to zero
	memset(state, 0, sizeof(FrameGrabber));

	// Now set anything non-zero
	state->width = 1920;       // Default to 1080p
	state->height = 1080;
	state->framerate = VIDEO_FRAME_RATE_NUM;

	state->bCapturing = 0;

	state->cameraNum = 0;
	state->settings = 0;
	state->sensor_mode = 0;

	// Set up the camera_parameters to default
	raspicamcontrol_set_defaults(&state->camera_parameters);
}


/**
 * Parse the incoming command line and put resulting parameters in to the state
 *
 * @param argc Number of arguments in command line
 * @param argv Array of pointers to strings from command line
 * @param state Pointer to state structure to assign any discovered parameters to
 * @return Non-0 if failed for some reason, 0 otherwise
 */
static int parse_cmdline(int argc, const char **argv, FrameGrabber *state)
{
	// Parse the command line arguments.
	// We are looking for --<something> or -<abbreviation of something>

	int valid = 1;
	int i;

	for (i = 1; i < argc && valid; i++)
	{
		int command_id, num_parameters;

		if (!argv[i])
			continue;

		if (argv[i][0] != '-')
		{
			valid = 0;
			continue;
		}

		// Assume parameter is valid until proven otherwise
		valid = 1;

		command_id = raspicli_get_command_id(cmdline_commands, cmdline_commands_size, &argv[i][1], &num_parameters);

		// If we found a command but are missing a parameter, continue (and we will drop out of the loop)
		if (command_id != -1 && num_parameters > 0 && (i + 1 >= argc) )
			continue;

		//  We are now dealing with a command line option
		switch (command_id)
		{
		case CommandWidth: // Width > 0
		if (sscanf(argv[i + 1], "%u", &state->width) != 1)
			valid = 0;
		else
			i++;
		break;

		case CommandHeight: // Height > 0
			if (sscanf(argv[i + 1], "%u", &state->height) != 1)
				valid = 0;
			else
				i++;
			break;

		case CommandFramerate: // fps to record
		{
			if (sscanf(argv[i + 1], "%u", &state->framerate) == 1)
			{
				// TODO : What limits do we need for fps 1 - 30 - 120??
				i++;
			}
			else
				valid = 0;
			break;
		}

		case CommandInitialState:
		{
			state->bCapturing = raspicli_map_xref(argv[i + 1], initial_map, initial_map_size);

			if( state->bCapturing == -1)
				state->bCapturing = 0;

			i++;
			break;
		}

		case CommandCamSelect:  //Select camera input port
		{
			if (sscanf(argv[i + 1], "%u", &state->cameraNum) == 1)
			{
				i++;
			}
			else
				valid = 0;
			break;
		}

		case CommandSettings:
			state->settings = 1;
			break;

		case CommandSensorMode:
		{
			if (sscanf(argv[i + 1], "%u", &state->sensor_mode) == 1)
			{
				i++;
			}
			else
				valid = 0;
			break;
		}

		default:
		{
			// Try parsing for any image specific parameters
			// result indicates how many parameters were used up, 0,1,2
			// but we adjust by -1 as we have used one already
			const char *second_arg = (i + 1 < argc) ? argv[i + 1] : NULL;
			int parms_used = (raspicamcontrol_parse_cmdline(&state->camera_parameters, &argv[i][1], second_arg));

			// If no parms were used, this must be a bad parameters
			if (!parms_used)
				valid = 0;
			else
				i += parms_used - 1;

			break;
		}
		}
	}

	if (!valid)
	{
		fprintf(stderr, "Invalid command line option (%s)\n", argv[i-1]);
		return 1;
	}

	return 0;
}



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

	bcm_host_init();

	// Register our application with the logging system
	vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

	signal(SIGINT, signal_handler);

	// Disable USR1 for the moment - may be reenabled if go in to signal capture mode
	signal(SIGUSR1, SIG_IGN);

	default_status(&frameGrabber);

	// Parse the command line and put options in to our status structure
	if (parse_cmdline(argc, argv, &frameGrabber))
	{
		status = (MMAL_STATUS_T)-1;
		exit(EX_USAGE);
	}

	// OK, we have a nice set of parameters. Now set up our components
	// We have two components. Camera, Preview

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

			while (!terminateRequested)
			{
				// Change state

				frameGrabber.bCapturing = !frameGrabber.bCapturing;

				if (mmal_port_parameter_set_boolean(frameGrabber.GetVideoPort(), MMAL_PARAMETER_CAPTURE, frameGrabber.bCapturing) != MMAL_SUCCESS)
				{
					// How to handle?
				}

				// watch for signal to exit... currently just an infinite loop
				while (!terminateRequested)
				{
					vcos_sleep(ABORT_INTERVAL);
				}
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

	return exit_code;
}

