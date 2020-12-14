#include "rekkoncamcontrol.h"

/**
 * @brief RekkonCamControl::RekkonCamControl
 * This class is the public interface to the rapberry pi camera module functions.
 * The functions are implemented in the VideoMMALObject class.
 * The class is divided between the 4 mains components that are accessible :
 * - Video Preview
 * - Video Record
 * - Still Preview (Yet to be done)
 * - Still Record (Yet to be done)
 */

RekkonCamControl::RekkonCamControl()
{
    m_mmal_instance = VideoMMALObject::instance();
}


RekkonCamControl::~RekkonCamControl(){}

// --------------------------------------------------
// General camera controls
// --------------------------------------------------

bool RekkonCamControl::open()
{
    return m_mmal_instance->open();
}

void RekkonCamControl::release()
{
    m_mmal_instance->release();
}


// --------------------------------------------------
// Controls on Still Preview output
// --------------------------------------------------


void RekkonCamControl::setStillPreviewSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setStillPreviewSize(width,height);
}

/**
 * @brief RekkonCamControl::startStillPreview
 * Create and enable the Still preview components
 * /!\ Do not work if Video components enabled
 */
void RekkonCamControl::startStillPreview()
{
    m_mmal_instance->startStillPreview();
}

void RekkonCamControl::stopStillPreview()
{
    m_mmal_instance->stopStillPreview();
}


// --------------------------------------------------
// Controls on Video Preview output
// --------------------------------------------------

void RekkonCamControl::setVideoPreviewSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setVideoPreviewSize(width,height);
}

void RekkonCamControl::startVideoPreview()
{
    m_mmal_instance->startVideoPreview();
}
void RekkonCamControl::stopVideoPreview()
{
    m_mmal_instance->stopVideoPreview();
}

// --------------------------------------------------
// Controls on both Preview output
// --------------------------------------------------

bool RekkonCamControl::grab()
{
    return m_mmal_instance->grab();
}

void RekkonCamControl::retrieve(unsigned char *data)
{
    m_mmal_instance->retrieve(data);
}

// --------------------------------------------------
// Controls on Video Record output
// --------------------------------------------------

void RekkonCamControl::setVideoRecordSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setVideoRecordSize(width,height);
}
void RekkonCamControl::startVideoRecord(string filename)
{
    m_mmal_instance->startVideoRecord(filename);
}
void RekkonCamControl::stopVideoRecord()
{
    m_mmal_instance->stopVideoRecord();
}

// --------------------------------------------------
// Controls on Still Record output
// --------------------------------------------------

void RekkonCamControl::setStillRecordSize(unsigned int width, unsigned int height)
{
    m_mmal_instance->setStillRecordSize(width,height);
}
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
