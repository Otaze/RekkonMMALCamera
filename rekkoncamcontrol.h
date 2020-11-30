#ifndef REKKONCAMCONTROL_H
#define REKKONCAMCONTROL_H


#include "videommalobject.h"

#include <string>

class RekkonCamControl
{
public:
    RekkonCamControl();
    ~RekkonCamControl();

    // Controls on Video Preview output
    void setVideoPreviewResolution(unsigned int width, unsigned int height);
    void startVideoPreview();
    void stopVideoPreview();
    bool grab();
    void retrieve(unsigned char *data);
    unsigned int getVideoPreviewWidth() { return m_mmal_instance->getVideoPreviewWidth();};
    unsigned int getVideoPreviewHeight() { return m_mmal_instance->getVideoPreviewHeight();};


    // Controls on Video Record output
    void setVideoRecordResolution(unsigned int width, unsigned int height);
    void startVideoRecord(string filename);
    void stopVideoRecord();
    unsigned int getVideoRecordWidth() { return m_mmal_instance->getVideoRecordWidth();};
    unsigned int getVideoRecordHeight() { return m_mmal_instance->getVideoRecordHeight();};

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
