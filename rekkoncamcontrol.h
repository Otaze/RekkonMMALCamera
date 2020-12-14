#ifndef REKKONCAMCONTROL_H
#define REKKONCAMCONTROL_H


#include "videommalobject.h"

#include <string>

class RekkonCamControl
{
public:
    RekkonCamControl();
    ~RekkonCamControl();

    // General camera controls

    bool open();
    void release();

    // Controls on Still Preview output
    void setStillPreviewSize(unsigned int width, unsigned int height);
    void startStillPreview();
    void stopStillPreview();
    unsigned int getStillPreviewWidth() { return m_mmal_instance->getStillPreviewWidth();};
    unsigned int getStillPreviewHeight() { return m_mmal_instance->getStillPreviewHeight();};
    bool isStillPreviewOpened(){ return m_mmal_instance->isStillPreviewOpened();}

    // Controls on Video Preview output
    void setVideoPreviewSize(unsigned int width, unsigned int height);
    void startVideoPreview();
    void stopVideoPreview();
    bool grab();
    void retrieve(unsigned char *data);
    unsigned int getVideoPreviewWidth() { return m_mmal_instance->getVideoPreviewWidth();};
    unsigned int getVideoPreviewHeight() { return m_mmal_instance->getVideoPreviewHeight();};
    bool isVideoPreviewOpened(){ return m_mmal_instance->isVideoPreviewOpened();}


    // Controls on Video Record output
    void setVideoRecordSize(unsigned int width, unsigned int height);
    void startVideoRecord(string filename);
    void stopVideoRecord();
    unsigned int getVideoRecordWidth() { return m_mmal_instance->getVideoRecordWidth();};
    unsigned int getVideoRecordHeight() { return m_mmal_instance->getVideoRecordHeight();};

    // Controls on Still Record output
    void setStillRecordSize(unsigned int width, unsigned int height);
    void startStillRecord(string filename);
    unsigned int getStillRecordWidth() { return m_mmal_instance->getStillRecordWidth();};
    unsigned int getStillRecordHeight() { return m_mmal_instance->getStillRecordHeight();};

    // Controls on Camera components

    void setVideoStabilization(bool v);
    void setBrightness(unsigned int brightness); // [0;100]
    void setShutterSpeed(unsigned  int shutter); //
    void setRotation(int rotation); // 0 = auto
    void setISO(int iso);
    void setSharpness(int sharpness) ;// [-100;100]
    void setContrast(int contrast); // [-100;100]
    void setSaturation(int saturation); // [-100;100]
    void setAWB_RB(float red_g, float blue_g);
    void setExposure(MMAL_PARAM_EXPOSUREMODE_T exposure);
    void setAWB(MMAL_PARAM_AWBMODE_T awb);
    void setImageEffect(MMAL_PARAM_IMAGEFX_T imageEffect);
    void setMetering(MMAL_PARAM_EXPOSUREMETERINGMODE_T metering);
    void setExposureCompensation(int val); // [-10;10]

    void setHorizontalFlip(bool hFlip);
    void setVerticalFlip(bool vFlip);
    void setFrameRate(unsigned int framerate); // [1;30->60]


private:
    VideoMMALObject * m_mmal_instance;
};

#endif // REKKONCAMCONTROL_H
