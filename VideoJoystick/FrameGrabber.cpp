
#include <stdexcept>
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "FrameGrabber.h"

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3


static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
static void check_disable_port(MMAL_PORT_T *port);


/// <summary>
/// Initializes a new instance of class FrameGrabber
/// </summary>
FrameGrabber::FrameGrabber()
{
	// Set up the camera_parameters to default
	memset(&this->camera_parameters, 0, sizeof(this->camera_parameters));
	raspicamcontrol_set_defaults(&this->camera_parameters);
}


/// <summary>
/// Releases resources held by the object
/// </summary>
FrameGrabber::~FrameGrabber()
{
   check_disable_port(GetVideoPort());
	DisableCamera();
	DestroyCameraComponent();
}

void FrameGrabber::SetupFrameCallback(const std::function<void(const std::shared_ptr<VideoFrame> &)> &callback)
{
	this->frameCallback = callback;
	GetVideoPort()->userdata = (struct MMAL_PORT_USERDATA_T *)this;
   MMAL_STATUS_T status = mmal_port_enable(GetVideoPort(), CameraBufferCallbackEntry);

   if (status != MMAL_SUCCESS)
     throw std::runtime_error("Failed to setup camera output");
}

void FrameGrabber::StartCapturing(void)
{
	bCapturing = 1;

    int num = mmal_queue_length(camera_pool->queue);
    int q;
    for (q=0; q<num; q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(camera_pool->queue);

        if (!buffer)
            vcos_log_error("Unable to get a required buffer %d from pool queue", q);

        if (mmal_port_send_buffer(GetVideoPort(), buffer)!= MMAL_SUCCESS)
            vcos_log_error("Unable to send a buffer to camera video port (%d)", q);
    }

	if (mmal_port_parameter_set_boolean(GetVideoPort(), MMAL_PARAMETER_CAPTURE, bCapturing) != MMAL_SUCCESS)
	{
		// How to handle?
	}
}



/// <summary>
/// Static camera callback that just dispatches to our non-static version
/// </summary>
void FrameGrabber::CameraBufferCallbackEntry(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   FrameGrabber *pThis = (FrameGrabber *)port->userdata;
   if (pThis != nullptr)
	   pThis->CameraBufferCallback(port, buffer);
}


/// <summary>
/// Processes the camera buffer
/// </summary>
void FrameGrabber::CameraBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	// create a VideoFrame from the buffer; this way the frame handler can do whatever
	// it wants, including processing it later on a different thread if it so desires
	std::shared_ptr<VideoFrame> frame;
	frame.reset(new VideoFrame(buffer));

	// pass the buffer to the frame handler
	frameCallback(frame);

	// release buffer back to the pool
	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled)
	{
	  MMAL_STATUS_T status;

	  MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(camera_pool->queue);

	  if (new_buffer)
		 status = mmal_port_send_buffer(port, new_buffer);

	  if (!new_buffer || status != MMAL_SUCCESS)
		 vcos_log_error("Unable to return a buffer to the camera port");
	}
}

/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 */
void FrameGrabber::CreateCameraComponent()
{
   MMAL_COMPONENT_T *camera = 0;
   MMAL_STATUS_T status;

   try {
    MMAL_PORT_T *video_port = NULL, *still_port = NULL;
    MMAL_POOL_T *pool;

    /* Create the component */
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

    if (status != MMAL_SUCCESS)
       throw std::runtime_error("Failed to create camera component");

    MMAL_PARAMETER_INT32_T camera_num =
       {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, cameraNum};

    status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

    if (status != MMAL_SUCCESS)
       throw std::runtime_error("Could not select camera");

    if (!camera->output_num)
    {
       throw std::runtime_error("Camera doesn't have output ports");
    }

    status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, sensor_mode);
    if (status != MMAL_SUCCESS)
       throw std::runtime_error("Could not set sensor mode");

    video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

    if (settings)
    {
       MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
          {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
           MMAL_PARAMETER_CAMERA_SETTINGS, 1};

       status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
       if ( status != MMAL_SUCCESS )
       {
          vcos_log_error("No camera settings events");
       }
    }

    // Enable the camera, and tell it its control callback function
    status = mmal_port_enable(camera->control, camera_control_callback);
    if (status != MMAL_SUCCESS)
       throw std::runtime_error("Unable to enable control port");

    //  set up the camera configuration
    {
       MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
       {
          { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
          .max_stills_w = width,
          .max_stills_h = height,
          .stills_yuv422 = 0,
          .one_shot_stills = 0,
          .max_preview_video_w = width,
          .max_preview_video_h = height,
          .num_preview_video_frames = 3,
          .stills_capture_circular_buffer_height = 0,
          .fast_preview_resume = 0,
          .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
       };
       mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }

    // Now set up the port formats

    //enable dynamic framerate if necessary
    if (camera_parameters.shutter_speed)
    {
       if (framerate > 1000000./camera_parameters.shutter_speed)
       {
          framerate=0;
       }
    }

    // Set the encode format on the video  port

    MMAL_ES_FORMAT_T *format = video_port->format;

    if(camera_parameters.shutter_speed > 6000000)
    {
         MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                      { 50, 1000 }, {166, 1000}};
         mmal_port_parameter_set(video_port, &fps_range.hdr);
    }
    else if(camera_parameters.shutter_speed > 1000000)
    {
         MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                      { 167, 1000 }, {999, 1000}};
         mmal_port_parameter_set(video_port, &fps_range.hdr);
    }

    format->encoding = mmal_util_rgb_order_fixed(still_port) ? MMAL_ENCODING_RGB24 : MMAL_ENCODING_BGR24;
    format->encoding_variant = 0;  //Irrelevant when not in opaque mode

    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

    status = mmal_port_format_commit(video_port);
    if (status != MMAL_SUCCESS)
       throw std::runtime_error("camera video format couldn't be set");

    // Ensure there are enough buffers to avoid dropping frames
    if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
       video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

    status = mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    if (status != MMAL_SUCCESS)
       throw std::runtime_error("Failed to select zero copy");

    // Set the encode format on the still  port

    format = still_port->format;

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;

    status = mmal_port_format_commit(still_port);

    if (status != MMAL_SUCCESS)
       throw std::runtime_error("camera still format couldn't be set");

    /* Ensure there are enough buffers to avoid dropping frames */
    if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
       still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

    /* Enable component */
    status = mmal_component_enable(camera);

    if (status != MMAL_SUCCESS)
       throw std::runtime_error("camera component couldn't be enabled");

    raspicamcontrol_set_all_parameters(camera, &camera_parameters);

    /* Create pool of buffer headers for the output port to consume */
    pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size);

    if (!pool)
    {
       vcos_log_error("Failed to create buffer header pool for camera still port %s", still_port->name);
    }

    camera_pool = pool;
    camera_component = camera;
   }
   catch (...)
   {
      // clean up before exiting
      if (camera)
         mmal_component_destroy(camera);
      throw;
   }
}

/**
 * Destroy the camera component
 *
 * @param state Pointer to state control struct
 *
 */
void FrameGrabber::DestroyCameraComponent(void)
{
   if (camera_component)
   {
      mmal_component_destroy(camera_component);
      camera_component = NULL;
   }
}

void FrameGrabber::DisableCamera(void)
{
    if (camera_component)
       mmal_component_disable(camera_component);
}




/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id) {
         case MMAL_PARAMETER_CAMERA_SETTINGS:
         {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
			settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
            vcos_log_error("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den
                        );
         }
         break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
      vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
   }
   else
   {
      vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
   }

   mmal_buffer_header_release(buffer);
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
