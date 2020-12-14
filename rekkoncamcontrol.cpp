#include "rekkoncamcontrol.h"

/**
 * @brief RekkonCamControl::RekkonCamControl
 * This class is the public interface to the rapberry pi camera module functions.
 * The functions are implemented in the VideoMMALObject class.
 * The class is divided between the 4 mains components that are accessible :
 * - Video Preview
 * - Video Record
 * - Still Preview
 * - Still Record
 */

RekkonCamControl::RekkonCamControl()
{
    m_mmal_instance = VideoMMALObject::instance();
}


RekkonCamControl::~RekkonCamControl(){}

// --------------------------------------------------
// General camera controls
// --------------------------------------------------

/**
 * @brief RekkonCamControl::open
 * Create and open the main Camera component.
 * @return true if opened, false otherwise.
 */
bool RekkonCamControl::open()
{
    return m_mmal_instance->open();
}

/**
 * @brief RekkonCamControl::release
 * Release and delete the camera component.
 * Delete the connected components if they exist.
 */
void RekkonCamControl::release()
{
    m_mmal_instance->release();
}


// --------------------------------------------------
// Controls on Still Preview output
// --------------------------------------------------

/**
 * @brief RekkonCamControl::setStillPreviewSize
 * @param width
 * @param height
 * Define the resolution of the still preview output.
 */
void RekkonCamControl::setStillPreviewSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setStillPreviewSize(width,height);
}

/**
 * @brief RekkonCamControl::setStillPreviewImageFormat
 * @param mmal_image_format (int)
 * Define the format for the output of the Still preview
 * The following formats are supported:
 * MMAL_ENCODING_I420 (YUV420),
 * MMAL_ENCODING_BGR24 (BGR 8 bits per canal),
 * MMAL_ENCODING_BGR24 (BGR 8 bits per canal)
 */
void RekkonCamControl::setStillPreviewImageFormat(int mmal_image_format)
{
    m_mmal_instance->setVideoPreviewImageFormat(mmal_image_format);
}

/**
 * @brief RekkonCamControl::startStillPreview
 * Create and enable the Still preview components.
 * /!\ Do not work if Video components are enabled.
 */
void RekkonCamControl::startStillPreview()
{
    m_mmal_instance->startStillPreview();
}

/**
 * @brief RekkonCamControl::stopStillPreview
 * Stop and destroy still preview related components if they exist.
 */
void RekkonCamControl::stopStillPreview()
{
    m_mmal_instance->stopStillPreview();
}


// --------------------------------------------------
// Controls on Video Preview output
// --------------------------------------------------

/**
 * @brief RekkonCamControl::setVideoPreviewSize
 * @param width
 * @param height
 * Define the resolution of the video preview output.
 */
void RekkonCamControl::setVideoPreviewSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setVideoPreviewSize(width,height);
}

/**
 * @brief RekkonCamControl::setVideoPreviewImageFormat
 * @param mmal_image_format (int)
 * Define the format for the output of the Video preview
 * The following formats are supported:
 * MMAL_ENCODING_I420 (YUV420),
 * MMAL_ENCODING_BGR24 (BGR 8 bits per canal),
 * MMAL_ENCODING_BGR24 (BGR 8 bits per canal)
 */
void RekkonCamControl::setVideoPreviewImageFormat(int mmal_image_format)
{
    m_mmal_instance->setVideoPreviewImageFormat(mmal_image_format);
}


/**
 * @brief RekkonCamControl::startStillPreview
 * Create and enable the Still preview components.
 * /!\ Do not work if still (image) components are enabled.
 */
void RekkonCamControl::startVideoPreview()
{
    m_mmal_instance->startVideoPreview();
}

/**
 * @brief RekkonCamControl::stopVideoPreview
 * Stop and destroy video preview related components if they exist.
 */
void RekkonCamControl::stopVideoPreview()
{
    m_mmal_instance->stopVideoPreview();
}

// --------------------------------------------------
// Controls on both Preview output
// --------------------------------------------------

/**
 * @brief RekkonCamControl::grab
 * Make sure that an image has been capture for the preview components.
 * Works for both Video and Still preview.
 * @return true if output is grabbed, false otherwise.
 */
bool RekkonCamControl::grab()
{
    return m_mmal_instance->grab();
}

/**
 * @brief RekkonCamControl::retrieve
 * @param data (unsigned char *) Pointer to an instantiated array of char or uint8_t
 * Retrieve the output data of the preview and store it in 'data' array.
 * see @setVideoPreviewImageFormat or @setStillPreviewImageFormat to see supported format for the output.
 */
void RekkonCamControl::retrieve(unsigned char *data)
{
    m_mmal_instance->retrieve(data);
}

// --------------------------------------------------
// Controls on Video Record output
// --------------------------------------------------


/**
 * @brief RekkonCamControl::setVideoRecordSize
 * @param width
 * @param height
 * Define the resolution of the video record output.
 */
void RekkonCamControl::setVideoRecordSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setVideoRecordSize(width,height);
}

/**
 * @brief RekkonCamControl::startVideoRecord
 * @param filename (string)
 * Create the video recording components and then record encoded frames into "filename" file.
 * /!\ Do not work if still (image) components are enabled.
 */
void RekkonCamControl::startVideoRecord(string filename)
{
    m_mmal_instance->startVideoRecord(filename);
}

/**
 * @brief RekkonCamControl::stopVideoRecord
 * Stop and destroy video recording related components.
 */
void RekkonCamControl::stopVideoRecord()
{
    m_mmal_instance->stopVideoRecord();
}

// --------------------------------------------------
// Controls on Still Record output
// --------------------------------------------------

/**
 * @brief RekkonCamControl::setStillRecordSize
 * @param width
 * @param height
 * Define the resolution of the still record output.
 */
void RekkonCamControl::setStillRecordSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setStillRecordSize(width,height);
}

/**
 * @brief RekkonCamControl::startStillRecord
 * @param filename (string)
 * Create the still recording components and then record encoded image into "filename" file.
 * When the image is created, components are stopped and destroyed.
 * /!\ Do not work if still (image) components are enabled.
 */
void RekkonCamControl::startStillRecord(string filename)
{
    m_mmal_instance->startStillRecord(filename);
}

// --------------------------------------------------
// Controls on Camera components settings
// --------------------------------------------------

void RekkonCamControl::setVideoStabilization(bool v)
{
    m_mmal_instance->setVideoStabilization(v);
}

void RekkonCamControl::setBrightness(unsigned int brightness)
{
    m_mmal_instance->setBrightness(brightness);
}

void RekkonCamControl::setShutterSpeed(unsigned  int shutter)
{
    m_mmal_instance->setShutterSpeed(shutter);
}

void RekkonCamControl::setRotation(int rotation)
{
    m_mmal_instance->setRotation(rotation);
}

void RekkonCamControl::setISO(int iso)
{
    m_mmal_instance->setISO(iso);
}

void RekkonCamControl::setSharpness(int sharpness)
{
    m_mmal_instance->setSharpness(sharpness);
}

void RekkonCamControl::setContrast(int contrast)
{
    m_mmal_instance->setContrast(contrast);
}

void RekkonCamControl::setSaturation(int saturation)
{
    m_mmal_instance->setSaturation(saturation);
}

void RekkonCamControl::setAWB_RB(float red_g, float blue_g)
{
    m_mmal_instance->setAWB_RB(red_g,blue_g);
}

void RekkonCamControl::setExposure(MMAL_PARAM_EXPOSUREMODE_T exposure)
{
    m_mmal_instance->setExposure(exposure);
}

void RekkonCamControl::setAWB(MMAL_PARAM_AWBMODE_T awb)
{
    m_mmal_instance->setAWB(awb);
}

void RekkonCamControl::setImageEffect(MMAL_PARAM_IMAGEFX_T imageEffect)
{
    m_mmal_instance->setImageEffect(imageEffect);
}

void RekkonCamControl::setMetering(MMAL_PARAM_EXPOSUREMETERINGMODE_T metering)
{
    m_mmal_instance->setMetering(metering);
}

void RekkonCamControl::setExposureCompensation(int val)
{
    m_mmal_instance->setExposureCompensation(val);
}

void RekkonCamControl::setHorizontalFlip(bool hFlip)
{
    m_mmal_instance->setHorizontalFlip(hFlip);
}

void RekkonCamControl::setVerticalFlip(bool vFlip)
{
    m_mmal_instance->setVerticalFlip(vFlip);
}

void RekkonCamControl::setFrameRate(unsigned int framerate)
{
    m_mmal_instance->setFrameRate(framerate);
}
