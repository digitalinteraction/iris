/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   testmain.cpp
 * Author: tobias
 *
 * Created on March 29, 2016, 11:05 AM
 */

#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sysexits.h>

#define VERSION_STRING "v1.3.8"
/*
#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"


#include "RaspiCamControl.h"
#include "RaspiPreview.h"
#include "RaspiCLI.h"
#include "RaspiTex.h"
*/
#include "Image_Capture.h"
#include <semaphore.h>

#include <wiringPi.h>
#include "tga.h"
#include <unistd.h>

#include "Buffer.h"






using namespace std;
/*static MMAL_STATUS_T create_camera_component(RASPISTILL_STATE *state);
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
static void default_status(RASPISTILL_STATE *state);
static void * begin_capturing(void *queue);*/


Image_Capture::Image_Capture(Buffer *buffer, Buffer *low_res_buffer, Buffer *low_res_requests)
{
    
    this->buf = buffer;
    this->low_res_buffer = low_res_buffer;
    this->low_res_requests = low_res_requests;

    //RASPISTILL_STATE state;
    MMAL_STATUS_T status = MMAL_SUCCESS;
    MMAL_PORT_T *camera_preview_port = NULL;
    MMAL_PORT_T *camera_video_port = NULL;
    MMAL_PORT_T *camera_still_port = NULL;
    MMAL_PORT_T *preview_input_port = NULL;
    bcm_host_init();
    default_status(&state);
    
    //state.raspitex_state.scene_id = RASPITEX_SCENE_SQUARE;
    raspitex_init(&state.raspitex_state);

    if ((status = create_camera_component(&state)) != MMAL_SUCCESS) {
        //vcos_log_error("%s: Failed to create camera component", __func__);
        printf("Failed to create camera component\n");
        //return -1;
    }
    
    
    camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
    camera_video_port = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
    camera_still_port = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
    
    if ((raspitex_start(&state.raspitex_state) != 0)){
        printf("Error in starting raspitex\n");
    }
        //return -1;
    
    
    /*if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
            //vcos_log_error("%s: Failed to start capture", __func__);
    }*/
    /*if (wiringPiSetup () == -1){
        printf("Error in setting up wiringPi\n");
    }*/
    
    /*pinMode (0, OUTPUT);
    pinMode (2, OUTPUT);
    pinMode (7, OUTPUT);
    
    digitalWrite(0, LOW);
    digitalWrite(2, LOW);
    digitalWrite(7, LOW);
    */
    group = 0;
    
    printf("Almost finshed setting up\n");
    
    capturing = 0;
        //return 1 ;
 
}


Image_Capture::~Image_Capture(){
    //digitalWrite(0, LOW);
    //digitalWrite(2, LOW);
    //digitalWrite(7, LOW);
    printf("Shutting capturing down\n");
}

void Image_Capture::run() {
    printf("starting up!\n");fflush(stdout);
    RASPITEX_PATCH ** patches;
    patches = (RASPITEX_PATCH **) calloc(sizeof(RASPITEX_PATCH *)*11, 1);
    patches[0] = (RASPITEX_PATCH *)calloc(sizeof(RASPITEX_PATCH), 1);
    patches[0]->width = LOW_OUTPUT_X;
    patches[0]->height = LOW_OUTPUT_Y;
    uint8_t size_patches = 1;
    
    while (capturing) {
        
        size_patches = 1;
        RASPITEX_PATCH *patch;
        uint8_t group;
        uint8_t cnt = 1;
        if (low_res_requests->getSize() > 0) {
            while (low_res_requests->get(&patch, &group) == 0 && cnt < 11) {
                patches[cnt] = patch;
                patch->select = 1;
                patch->active = 0;
                size_patches++;
            }
            group++;
        }
        
        /*
        if (worker->requests_pending > 0) {
            //printf("request for %d\n", worker->requests_pending);
            for(int i=0;i<worker->requests_pending;i++){
                patches[i+1] = (RASPITEX_PATCH *)calloc(sizeof(RASPITEX_PATCH), 1);
                memcpy(patches[i+1], &worker->requests[i], sizeof(RASPITEX_PATCH));
                patches[i+1]->select = 1;
                patches[i+1]->active = 0;
                //printf("Request (%d, %d) (%d, %d)\n", patches[i+1]->x, patches[i+1]->y, patches[i+1]->width, patches[i+1]->height);
            }
            group++;
            size_patches = worker->requests_pending+1;
            worker->requests_pending = 0;
        }*/

            patches[0]->active = 0;
            patches[0]->buffer = 0;
            //printf("getting image out\n");
            int8_t ret = raspitex_capture(&state.raspitex_state, patches, size_patches);
            
            if(state.raspitex_state.external_images_finished == 0){
                //pthread_mutex_lock(&worker->buffer_lock);
                //memcpy(&worker->low_patch, patches[0], sizeof(RASPITEX_PATCH));
                //worker->new_low_buffer = 1;
                //pthread_mutex_unlock(&worker->buffer_lock);
                
                RASPITEX_PATCH *item = (RASPITEX_PATCH *)calloc(sizeof(RASPITEX_PATCH), 1);
                memcpy(item, patches[0], sizeof(RASPITEX_PATCH));
                low_res_buffer->add(item, 0);
            }
            

        for (int i = 1; i < size_patches; i++) {
            if (patches[i]->buffer) {
                buf->add(patches[i], group);
                patches[i] = 0;
            }
        }
        
            
        
    }
    //digitalWrite(0, LOW);
    //digitalWrite(2, LOW);
    //digitalWrite(7, LOW);
}

int Image_Capture::get_high_res_image(){
   /*state.raspitex_state.patches[0].active = 1;
    state.raspitex_state.patches[0].height = 300;
    state.raspitex_state.patches[0].width = 300;
    state.raspitex_state.patches[0].x = 200;
    state.raspitex_state.patches[0].y = 200;
    
    state.raspitex_state.patches[1].active = 1;
    state.raspitex_state.patches[1].height = 500;
    state.raspitex_state.patches[1].width = 500;
    state.raspitex_state.patches[1].x = 100;
    state.raspitex_state.patches[1].y = 100;
    
    state.raspitex_state.low_buffer_request = 1;
    
    raspitex_capture(&state.raspitex_state, 1, 1);*/
}

 MMAL_STATUS_T Image_Capture::create_camera_component(RASPISTILL_STATE *state)
{
   MMAL_COMPONENT_T *camera = 0;
   MMAL_ES_FORMAT_T *format;
   MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
   MMAL_STATUS_T status;

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Failed to create camera component");
      //goto error;
       return status;
   }

   //status = raspicamcontrol_set_stereo_mode(camera->output[0], &state->camera_parameters.stereo_mode);
   //status += raspicamcontrol_set_stereo_mode(camera->output[1], &state->camera_parameters.stereo_mode);
   //status += raspicamcontrol_set_stereo_mode(camera->output[2], &state->camera_parameters.stereo_mode);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Could not set stereo mode : error %d", status);
      //goto error;
       return status;
   }

   MMAL_PARAMETER_INT32_T camera_num =
      {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, state->cameraNum};

   status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Could not select camera : error %d", status);
      //goto error;
       return status;
   }

   if (!camera->output_num)
   {
      status = MMAL_ENOSYS;
      //vcos_log_error("Camera doesn't have output ports");
      //goto error;
      return status;
   }

   status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, state->sensor_mode);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Could not set sensor mode : error %d", status);
      //goto error;
       return status;
   }

   preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
   still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

   if (state->settings)
   {
      MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
         {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
          MMAL_PARAMETER_CAMERA_SETTINGS, 1};

      status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
      if ( status != MMAL_SUCCESS )
      {
         //vcos_log_error("No camera settings events");
      }
   }

   // Enable the camera, and tell it its control callback function
   status = mmal_port_enable(camera->control, camera_control_callback);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Unable to enable control port : error %d", status);
      //goto error;
       return status;
   }

   //  set up the camera configuration
   {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = state->width,
         .max_stills_h = state->height,
         .stills_yuv422 = 0,
         .one_shot_stills = 1,
         .max_preview_video_w = state->preview_parameters.previewWindow.width,
         .max_preview_video_h = state->preview_parameters.previewWindow.height,
         .num_preview_video_frames = 3,
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
      };

      if (state->fullResPreview)
      {
         cam_config.max_preview_video_w = state->width;
         cam_config.max_preview_video_h = state->height;
      }

      mmal_port_parameter_set(camera->control, &cam_config.hdr);
   }
   
    
   raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

   // Now set up the port formats

   format = preview_port->format;
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   if(state->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }
   else if(state->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 166, 1000 }, {999, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
   }
   if (state->fullResPreview)
   {
      // In this mode we are forcing the preview to be generated from the full capture resolution.
      // This runs at a max of 15fps with the OV5647 sensor.
      format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
      format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
      format->es->video.crop.x = 0;
      format->es->video.crop.y = 0;
      format->es->video.crop.width = state->width;
      format->es->video.crop.height = state->height;
      format->es->video.frame_rate.num = FULL_RES_PREVIEW_FRAME_RATE_NUM;
      format->es->video.frame_rate.den = FULL_RES_PREVIEW_FRAME_RATE_DEN;
   }
   else
   {
      // Use a full FOV 4:3 mode
      format->es->video.width = VCOS_ALIGN_UP(state->preview_parameters.previewWindow.width, 32);
      format->es->video.height = VCOS_ALIGN_UP(state->preview_parameters.previewWindow.height, 16);
      format->es->video.crop.x = 0;
      format->es->video.crop.y = 0;
      format->es->video.crop.width = state->preview_parameters.previewWindow.width;
      format->es->video.crop.height = state->preview_parameters.previewWindow.height;
      format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
      format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;
   }

   status = mmal_port_format_commit(preview_port);
   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera viewfinder format couldn't be set");
      //goto error;
       return status;
   }

   // Set the same format on the video  port (which we dont use here)
   mmal_format_full_copy(video_port->format, format);
   status = mmal_port_format_commit(video_port);

   if (status  != MMAL_SUCCESS)
   {
      //vcos_log_error("camera video format couldn't be set");
      //goto error;
       return status;
   }

   // Ensure there are enough buffers to avoid dropping frames
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   format = still_port->format;

   if(state->camera_parameters.shutter_speed > 6000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(still_port, &fps_range.hdr);
   }
   else if(state->camera_parameters.shutter_speed > 1000000)
   {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 167, 1000 }, {999, 1000}};
        mmal_port_parameter_set(still_port, &fps_range.hdr);
   }
   // Set our stills format on the stills (for encoder) port
   /*format->encoding = MMAL_ENCODING_OPAQUE;
   format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
   format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
   format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;*/


   status = mmal_port_format_commit(still_port);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera still format couldn't be set");
      //goto error;
       return status;
   }

   /* Ensure there are enough buffers to avoid dropping frames */
   //if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
   //   still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   /* Enable component */
   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera component couldn't be enabled");
      //goto error;
       return status;
   }
      status = raspitex_configure_preview_port(&state->raspitex_state, preview_port); //PORT CHANGE
      //status = raspitex_configure_preview_port(&state->raspitex_state, preview_port);
      if (status != MMAL_SUCCESS)
      {
         fprintf(stderr, "Failed to configure preview port for GL rendering");
         printf("Failed to configure preview port for GL rendering\n");
         //goto error;
         return status;
      }

   state->camera_component = camera;

   if (state->verbose)
      fprintf(stderr, "Camera component done\n");

   return status;

}

 void Image_Capture::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id) {
         case MMAL_PARAMETER_CAMERA_SETTINGS:
         {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            /*vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
			settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
            vcos_log_error("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den
                        );*/
         }
         break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
      //vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
   }
   else
      //vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);

   mmal_buffer_header_release(buffer);
}

 void Image_Capture::default_status(RASPISTILL_STATE *state)
{
   if (!state)
   {
      vcos_assert(0);
      return;
   }
   char file[] = "test\0";

   state->timeout = 5000; // 5s delay before take image
   state->width = 2592;
   state->height = 1944;
   state->quality = 85;
   state->wantRAW = 0;
   state->filename = file;
   state->linkname = file;
   state->frameStart = 0;
   state->verbose = 1;
   state->thumbnailConfig.enable = 1;
   state->thumbnailConfig.width = 64;
   state->thumbnailConfig.height = 48;
   state->thumbnailConfig.quality = 35;
   state->demoMode = 0;
   state->demoInterval = 250; // ms
   state->camera_component = NULL;
   state->encoder_component = NULL;
   state->preview_connection = NULL;
   state->encoder_connection = NULL;
   state->encoder_pool = NULL;
   state->encoding = MMAL_ENCODING_JPEG;
   state->numExifTags = 0;
   state->enableExifTags = 1;
   state->timelapse = 0;
   state->fullResPreview = 1;
   state->frameNextMethod = FRAME_NEXT_SINGLE;
   state->glCapture = 1;
   state->settings = 0;
   state->cameraNum = 0;
   state->burstCaptureMode=0;
   state->sensor_mode = 0;
   state->datetime = 0;
   state->timestamp = 0;

   // Setup preview window defaults
   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&state->camera_parameters);
   //TOBIAS
   state->camera_parameters.shutter_speed = 20000;
   state->camera_parameters.ISO = 100;
   //state->camera_parameters.shutter_speed = 0;
   //state->camera_parameters.ISO = 0;

   // Set initial GL preview state
   raspitex_set_defaults(&state->raspitex_state);
}