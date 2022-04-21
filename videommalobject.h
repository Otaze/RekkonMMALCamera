#ifndef VIDEOMMALOBJECT_H
#define VIDEOMMALOBJECT_H

#include <mutex>
#include <iostream>
#include <fstream>

#include "mmal/mmal.h"
#include "mmal/util/mmal_util.h"
#include "mmal/util/mmal_util_params.h"
#include "mmal/util/mmal_default_components.h"
#include "mmal/util/mmal_connection.h"
#include "mmal/mmal_buffer.h"
#include <condition_variable>
#include "interface/vcos/vcos.h"



using namespace std;

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_STILL_PORT 2

#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 3

#define MAX_VIDEO_WIDTH 1920
#define MAX_VIDEO_HEIGHT 1080

#define MAX_STILL_WIDTH 4056
#define MAX_STILL_HEIGHT 3040
/* Structures from
 * https://github.com/raspberrypi/userland/blob/master/host_applications/linux/apps/raspicam/RaspiCamControl.h
 */

// There isn't actually a MMAL structure for the following, so make one
struct MMAL_PARAM_COLOURFX_T
{
   int enable;       /// Turn colourFX on or off
   int u,v;          /// U and V to use
};

struct MMAL_PARAM_THUMBNAIL_CONFIG_T
{
   int enable;
   int width,height;
   int quality;
};

struct PARAM_FLOAT_RECT_T
{
   double x;
   double y;
   double w;
   double h;
};


struct CAMERA_PARAMETERS
{

    int framerate;
    int sharpness;             /// -100 to 100
    int contrast;              /// -100 to 100
    int brightness;            ///  0 to 100
    int saturation;            ///  -100 to 100
    int ISO;                   ///  TODO : what range? 100 to 6400?
    bool videoStabilisation;    /// 0 or 1 (false or true)
    int exposureCompensation;  /// -10 to +10 ?
    int shutterSpeed;
    MMAL_PARAM_EXPOSUREMODE_T   exposureMode;
    MMAL_PARAM_EXPOSUREMETERINGMODE_T   exposureMeterMode;
    MMAL_PARAM_AWBMODE_T  awbMode;
    MMAL_PARAM_IMAGEFX_T imageEffect;
    MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imageEffectsParameters;
    MMAL_PARAM_COLOURFX_T colourEffects;
    MMAL_PARAM_FLICKERAVOID_T flickerAvoidMode;
    int rotation;              /// 0-359
    int hflip;                 /// 0 or 1
    int vflip;                 /// 0 or 1
    PARAM_FLOAT_RECT_T  roi;   /// region of interest to use on the sensor. Normalised [0,1] values in the rect
    float awbg_red;//white balance red and blue
    float awbg_blue;
};


/** Struct used to pass information in encoder port userdata to callback
*/

struct PORT_PREVIEW_USERDATA
{
    PORT_PREVIEW_USERDATA() {
        wantToGrab=false;
    }
    void waitForFrame() {
        //_mutex.lock();
        std::unique_lock<std::mutex> lck ( _mutex );

        wantToGrab=true;
        ready = false;
        while ( !ready ) cv.wait ( lck ); //this will unlock the mutex and wait atomically
    };
    void Broadcast() {
        ready = true;
        cv.notify_all();
    };


    MMAL_POOL_T *pool;
    std::mutex _mutex;
    bool ready;
    condition_variable cv;
    bool wantToGrab;
    unsigned int buffer_length;
    unsigned char * buffer_data;

};
struct PORT_ENCODER_USERDATA
{
   std::ofstream * file;
   MMAL_POOL_T * encoder_pool;
   bool encode_completed;/// Pointer to the pool of buffers used by encoder output port
};

class VideoMMALObject
{
public:

    static VideoMMALObject* instance();

    void setStillPreviewSize(unsigned int preview_width, unsigned int preview_height);
    void setStillPreviewImageFormat(int mmal_image_format);
    int getStillPreviewImageFormat() { return m_still_preview_format;};
    void startStillPreview();
    void stopStillPreview();
    unsigned int getStillPreviewWidth(){ return m_still_preview_width;};
    unsigned int getStillPreviewHeight(){ return m_still_preview_height;};
    bool isStillPreviewOpened(){ return m_is_still_preview_opened;}


    void setVideoPreviewSize(unsigned int preview_width, unsigned int preview_height);
    void setVideoPreviewImageFormat(int mmal_image_format);
    int getVideoPreviewImageFormat() { return m_video_preview_format;};
    void startVideoPreview();
    void stopVideoPreview();
    unsigned int getVideoPreviewWidth(){ return m_video_preview_width;};
    unsigned int getVideoPreviewHeight(){ return m_video_preview_height;};
    bool isVideoPreviewOpened(){ return m_is_video_preview_opened;}
    bool grab();
    void retrieve(unsigned char *data);


    void setVideoRecordSize(unsigned int record_width, unsigned int record_height);
    unsigned int getVideoRecordWidth(){ return m_video_record_width;};
    unsigned int getVideoRecordHeight(){ return m_video_record_height;};
    void startVideoRecord(std::string filename);
    void stopVideoRecord();

    void setStillRecordSize(unsigned int record_width, unsigned int record_height);
    unsigned int getStillRecordWidth(){ return m_still_record_width;};
    unsigned int getStillRecordHeight(){ return m_still_record_height;};
    void startStillRecord(std::string filename);
    //void stopStillRecord();



    VideoMMALObject(const VideoMMALObject&) = delete;
    VideoMMALObject& operator=(const VideoMMALObject&) = delete;

    bool isStillRecording(){ return m_is_still_recording;}
    bool isVideoRecording(){ return m_is_video_recording;}
    bool areVideoComponentsReady(){ return m_are_video_components_ready;}
    bool isOpened(){ return m_is_opened;}

    bool open();
    void release();


    void setVideoStabilization(bool v);
    void setBrightness(unsigned int brightness);
    void setShutterSpeed(unsigned  int shutter);
    void setRotation(int rotation);
    void setISO(int iso);
    void setSharpness(int sharpness) ;
    void setContrast(int contrast);
    void setSaturation(int saturation);
    void setAWB_RB(float red_g, float blue_g);
    void setExposure(MMAL_PARAM_EXPOSUREMODE_T exposure);
    void setAWB(MMAL_PARAM_AWBMODE_T awb);
    void setImageEffect(MMAL_PARAM_IMAGEFX_T imageEffect);
    void setMetering(MMAL_PARAM_EXPOSUREMETERINGMODE_T metering);
    void setExposureCompensation(int val);

    void setHorizontalFlip(bool hFlip);
    void setVerticalFlip(bool vFlip);
    void setFrameRate(unsigned int framerate);



private:
    VideoMMALObject();
    ~VideoMMALObject();

    static VideoMMALObject *m_instance;
    static std::mutex m_mutex;

    int m_still_preview_format;
    unsigned int m_still_preview_width;
    unsigned int m_still_preview_height;
    bool m_is_still_preview_opened;


    int m_video_preview_format;
    unsigned int m_video_preview_width;
    unsigned int m_video_preview_height;
    bool m_is_video_preview_opened;

    unsigned int m_video_record_width;
    unsigned int m_video_record_height;
    bool m_is_video_recording;

    unsigned int m_still_record_width;
    unsigned int m_still_record_height;
    bool m_is_still_recording;


    bool m_is_opened;
    bool m_are_video_components_ready;


    CAMERA_PARAMETERS m_cam_params;

    void setDefaultsCamParams();
    void commitParameters();

    void destroyCameraComponent();
    void createCameraComponent();

    void destroyVideoComponents();
    void createVideoComponents();

    void createStillEncoderComponent();
    void destroyStillEncoderComponent();

    void createVideoEncoderComponent();
    void destroyVideoEncoderComponent();

    void destroyVideoPreviewComponent();
    void createVideoPreviewComponent();

    void destroyStillPreviewComponent();
    void createStillPreviewComponent();


    MMAL_STATUS_T connectPorts ( MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection );
    void destroyConnection(MMAL_CONNECTION_T *connection);

    static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void preview_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);



    MMAL_COMPONENT_T *camera_component;
    MMAL_PORT_T *camera_video_output_port;
    MMAL_PORT_T *camera_preview_output_port;
    MMAL_PORT_T *camera_still_output_port;

    /* Used in Video components */
    MMAL_PORT_T *splitter_output_record_port;
    MMAL_PORT_T *splitter_output_video_port;
    MMAL_PORT_T *splitter_input_port;
    MMAL_COMPONENT_T *splitter_component;
    MMAL_CONNECTION_T *splitter_connection; // Connection from the camera to the splitter


    /* Used in Video record */
    MMAL_COMPONENT_T *video_encoder_component;	/// Pointer to the video_encoder component
    MMAL_CONNECTION_T *video_encoder_connection; // Connection from the splitter to the video_encoder
    MMAL_PORT_T *video_encoder_input_port;
    MMAL_PORT_T *video_encoder_output_port;
    MMAL_POOL_T *video_encoder_pool;

    /* Used in Still record */
    MMAL_COMPONENT_T *still_encoder_component;	/// Pointer to the video_encoder component
    MMAL_CONNECTION_T *still_encoder_connection; // Connection from the splitter to the video_encoder
    MMAL_PORT_T *still_encoder_input_port;
    MMAL_PORT_T *still_encoder_output_port;
    MMAL_POOL_T *still_encoder_pool;

    /* Used in records */
    PORT_ENCODER_USERDATA encoder_callback_data;

    /* Used in preview video*/
    MMAL_PORT_T *resizer_output_port;
    MMAL_PORT_T *resizer_input_port;
    MMAL_COMPONENT_T *resizer_component;
    MMAL_CONNECTION_T *resizer_connection; // Connection from the splitter to the resizer
    MMAL_POOL_T *resize_pool;


    /* Used in preview Still*/
    MMAL_POOL_T *still_preview_pool;

    // used in both preview
    PORT_PREVIEW_USERDATA preview_callback_data;


    void commitSaturation();
    void commitSharpness();
    void commitContrast();
    void commitBrightness();
    void commitISO();
    void commitShutterSpeed();
    void commitExposure();
    void commitExposureCompensation();
    void commitMetering();
    void commitImageEffect();
    void commitRotation();
    void commitFlips();
    void commitVideoStabilization();
    void commitAWB();
    void commitAWB_RB();
};

#endif // VIDEOMMALOBJECT_H
