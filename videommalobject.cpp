#include "videommalobject.h"


/**
 * Initialize static attributes.
 */
VideoMMALObject* VideoMMALObject::m_instance{nullptr};
std::mutex VideoMMALObject::m_mutex;


VideoMMALObject::VideoMMALObject():
    m_video_preview_format(MMAL_ENCODING_RGB24),
    m_video_preview_width(960),
    m_video_preview_height(540),
    m_video_record_width(1920),
    m_video_record_height(1080),
    m_is_opened(false),
    m_is_recording(false),
    m_is_video_preview_opened(false)
{
    setDefaultsCamParams();

}

/**
 * @brief VideoMMALObject::instance
 * Return the pointer of the singleton
 * @return m_instance : VideoMMALObject singleton
 */
VideoMMALObject* VideoMMALObject::instance()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_instance == nullptr)
    {
        m_instance = new VideoMMALObject();
    }
    return m_instance;
}


void VideoMMALObject::setDefaultsCamParams()
{
    // Default everything to zero
    m_cam_params.framerate 		= 30;
    m_cam_params.sharpness = 0;
    m_cam_params.contrast = 0;
    m_cam_params.brightness = 50;
    m_cam_params.saturation = 0;
    m_cam_params.ISO = 100;
    m_cam_params.videoStabilisation = false;
    m_cam_params.exposureCompensation = 0;
    m_cam_params.exposureMode = MMAL_PARAM_EXPOSUREMODE_AUTO;
    m_cam_params.exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
    m_cam_params.awbMode = MMAL_PARAM_AWBMODE_AUTO;
    m_cam_params.imageEffect = MMAL_PARAM_IMAGEFX_NONE;
    m_cam_params.colourEffects.enable = 0;
    m_cam_params.colourEffects.u = 128;
    m_cam_params.colourEffects.v = 128;
    m_cam_params.rotation = 0;
    m_cam_params.hflip = m_cam_params.vflip = 0;
    m_cam_params.roi.x = m_cam_params.roi.y = 0.0;
    m_cam_params.roi.w = m_cam_params.roi.h = 1.0;
    m_cam_params.shutterSpeed=0;//auto
    m_cam_params.awbg_red=0;
    m_cam_params.awbg_blue=0;
}


VideoMMALObject::~VideoMMALObject()
{}


void VideoMMALObject::setVideoPreviewSize(unsigned int preview_width, unsigned int preview_height)
{
    m_video_preview_width = preview_width;
    m_video_preview_height = preview_height;
}

void VideoMMALObject::setVideoRecordSize(unsigned int record_width, unsigned int record_height)
{
    m_video_record_width = record_width;
    m_video_record_height = record_height;
}


void VideoMMALObject::startVideoPreview()
{
    if (!isOpened()) open();
    createVideoPreviewComponent();
    m_is_video_preview_opened = true;
}
void VideoMMALObject::stopVideoPreview()
{
    if (!isOpened() || !isPreviewVideoOpened()) return;
    destroyVideoPreviewComponent();

    m_is_video_preview_opened = false;
}

void VideoMMALObject::startVideoRecord(std::string filename)
{
    if (!isOpened()) open();
    if (!encoder_callback_data.videoFile) delete encoder_callback_data.videoFile;
    encoder_callback_data.videoFile = new ofstream(filename, ios::out|ios::binary|ios::app);
    createEncoderComponent();

    m_is_recording = true;
}
void VideoMMALObject::stopVideoRecord()
{
    if (!isOpened() || !isRecording()) return;
    destroyEncoderComponent();
    if (encoder_callback_data.videoFile->is_open())
        encoder_callback_data.videoFile->close();


    m_is_recording = false;
}

/**
 * @brief open : Create and initialize the camera main components via MMAL API
 * @return True if components are ready, false otherwise
 */
bool VideoMMALObject::open()
{
    if (isOpened()) return false;
    // Create camera component
    createCameraComponent();
    if ( !camera_component->is_enabled || !splitter_component->is_enabled)
    {
        cerr<<__func__<<" Failed to create camera component"<<__FILE__<<" "<<__LINE__<<endl;
        return false;
    }
    commitParameters();

    if ( mmal_port_parameter_set_boolean ( camera_component->output[MMAL_CAMERA_VIDEO_PORT], MMAL_PARAMETER_CAPTURE, 1 ) != MMAL_SUCCESS ) {
        release();
        return false;
    }

    m_is_opened = true;

    return m_is_opened;
}

/**
 * @brief commitParameters : Commit all cam parameters to the camera components
 */
void VideoMMALObject::commitParameters()
{
    commitSaturation();
    commitSharpness();
    commitContrast();
    commitBrightness();
    commitISO();
    if ( m_cam_params.shutterSpeed!=0 ) {
        commitShutterSpeed();
        m_cam_params.exposureMode=MMAL_PARAM_EXPOSUREMODE_FIXEDFPS;
        commitExposure();
    } else           commitExposure();
    commitExposureCompensation();
    commitMetering();
    commitImageEffect();
    commitRotation();
    commitFlips();
    commitVideoStabilization();
    commitAWB();
    commitAWB_RB();
}

/**
 * @brief release : Release all the camera components.
 * Stop preview and recording if running
 */
void VideoMMALObject::release()
{
    if (isRecording()) stopVideoRecord();
    if (isPreviewVideoOpened()) stopVideoPreview();
    m_is_opened = false;
}

bool VideoMMALObject::grab()
{
    if ( !isOpened() ) return false;
    resize_callback_data.waitForFrame();
    return true;
}

/**
 * @brief VideoMMALObject::retrieve
 * Retrieve the preview image data captured by the camera with the preview image format
 * Supported formats are : YUV420, I420
 * @param data : pointer to array that will be filled with image data
 *
 */
void VideoMMALObject::retrieve(unsigned char *data)
{
    if ( resize_callback_data.buffer_length == 0 ) return;

    unsigned char * imagePtr=resize_callback_data.buffer_data;
    if(  m_video_preview_format  == MMAL_ENCODING_I420){
        for(unsigned int i=0;i<m_video_preview_width+m_video_preview_height/2;i++) {
            memcpy ( data,imagePtr,m_video_preview_width);
            data+=m_video_preview_width;
            imagePtr+=VCOS_ALIGN_UP(m_video_preview_width, 32);//line stride
        }

    }
    else if(  m_video_preview_format  == MMAL_ENCODING_BGR24 || m_video_preview_format  == MMAL_ENCODING_RGB24 ){
        for(unsigned int i=0;i<m_video_preview_height;i++) {
            memcpy ( data,imagePtr,m_video_preview_width*3);
            data+=m_video_preview_width*3;
            imagePtr+=VCOS_ALIGN_UP(m_video_preview_width, 32)*3;//line stride
        }
    }
    delete resize_callback_data.buffer_data;
}

/**
 * @brief VideoMMALObject::connectPorts
 * Create a mmal connection linking the output port and the input ports of 2 mmal components
 * @param output_port
 * @param input_port
 * @param connection
 * @return The result of the connection creation
 */
MMAL_STATUS_T VideoMMALObject::connectPorts ( MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection ) {
    MMAL_STATUS_T status =  mmal_connection_create ( connection, output_port, input_port,  MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT  );
    if ( status == MMAL_SUCCESS ) {
        status =  mmal_connection_enable ( *connection );
        if ( status != MMAL_SUCCESS )
            mmal_connection_destroy ( *connection );
    }
    return status;
}

/**
 * @brief VideoMMALObject::destroyConnection
 * Disable and destroy the link between 2 components and the underlying mmal connection object
 * @param connection
 */
void VideoMMALObject::destroyConnection(MMAL_CONNECTION_T *connection) {
    // disable connection if enabled
    if ( mmal_connection_disable( connection ) != MMAL_SUCCESS ) {
        cerr << ": fail to disable connection\n";
    }

    // destroy connection
    // mmal_connection_disable call is mandatory before calling the destroy function otherwise it fails
    if ( connection && mmal_connection_destroy( connection ) != MMAL_SUCCESS ) {
        cerr << ": fail to destroy connection\n";
    }
}


/**
 * @brief VideoMMALObject::destroyCameraComponent
 * Destroy the camera component and clean involved objects
 */
void VideoMMALObject::destroyCameraComponent() {

    if (splitter_connection )
        destroyConnection(splitter_connection);

    if ( splitter_component ) {
        mmal_component_destroy ( splitter_component );
        splitter_component = NULL;
    }

    if ( camera_component ) {
        mmal_component_destroy ( camera_component );
        camera_component = NULL;
    }

}

/**
 * @brief VideoMMALObject::createCameraComponent
 */
void VideoMMALObject::createCameraComponent() {

    MMAL_ES_FORMAT_T *format;
    MMAL_STATUS_T status;
    /* Create the component */
    status = mmal_component_create ( MMAL_COMPONENT_DEFAULT_CAMERA, &camera_component );

    if ( status != MMAL_SUCCESS ) {
        cerr<< ( "Failed to create camera component" );
        return;
    }

    status = mmal_component_create ( MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter_component );

     if ( status != MMAL_SUCCESS ) {
         cerr<< ( "Failed to create splitter component" );
         destroyCameraComponent();
         return;
     }

    if ( !camera_component->output_num ) {
        cerr<< ( "Camera doesn't have output ports" );
        //mmal_component_destroy ( camera );
        destroyCameraComponent();
        return;
    }

    camera_video_output = camera_component->output[MMAL_CAMERA_VIDEO_PORT];
    //  set up the camera configuration

    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
    cam_config.hdr.id=MMAL_PARAMETER_CAMERA_CONFIG;
    cam_config.hdr.size=sizeof ( cam_config );
    cam_config.max_stills_w = MAX_PHOTO_WIDTH;
    cam_config.max_stills_h = MAX_PHOTO_HEIGHT;
    cam_config.stills_yuv422 = 0;
    cam_config.one_shot_stills = 0;
    cam_config.max_preview_video_w = MAX_VIDEO_WIDTH;
    cam_config.max_preview_video_h = MAX_VIDEO_HEIGHT;
    cam_config.num_preview_video_frames = 3 + vcos_max(0, (m_cam_params.framerate-30)/10);
    cam_config.stills_capture_circular_buffer_height = 0;
    cam_config.fast_preview_resume = 0;
    cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
    mmal_port_parameter_set ( camera_component->control, &cam_config.hdr );

    // Set the Camera format on the video port

    cerr << "Set up Camera : " << MAX_VIDEO_WIDTH << ", " << MAX_VIDEO_HEIGHT << endl;
    format = camera_video_output->format;
    format->encoding_variant = MMAL_ENCODING_I420;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(MAX_VIDEO_WIDTH, 32);
    format->es->video.height = VCOS_ALIGN_UP(MAX_VIDEO_HEIGHT, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = MAX_VIDEO_WIDTH;
    format->es->video.crop.height = MAX_VIDEO_HEIGHT;
    format->es->video.frame_rate.num =  m_cam_params.framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

    status = mmal_port_format_commit ( camera_video_output );
    if ( status ) {
        cerr<< ( "camera video format couldn't be set" );
        destroyCameraComponent();
        return;
    }

    camera_video_output->buffer_num = camera_video_output->buffer_num_recommended;
    if (camera_video_output->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
         camera_video_output->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    camera_video_output->buffer_size = camera_video_output->buffer_size_recommended;
    if (camera_video_output->buffer_size < camera_video_output->buffer_size_min)
         camera_video_output->buffer_size = camera_video_output->buffer_size_min;



    splitter_input_port = splitter_component->input[0];
    splitter_output_video_port = splitter_component->output[0];
    splitter_output_record_port = splitter_component->output[1];

    mmal_format_copy(splitter_input_port->format, camera_video_output->format);
    splitter_input_port->buffer_num = splitter_input_port->buffer_num_recommended;

    if (splitter_input_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
         splitter_input_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    splitter_input_port->buffer_size = splitter_input_port->buffer_size_recommended;
    if (splitter_input_port->buffer_size < splitter_input_port->buffer_size_min)
         splitter_input_port->buffer_size = splitter_input_port->buffer_size_min;
    status = mmal_port_format_commit(splitter_input_port);



    for (unsigned int i = 0; i < splitter_component->output_num; i++)
    {
       mmal_format_copy(splitter_component->output[i]->format, splitter_component->input[0]->format);

       format = splitter_component->output[i]->format;
       status = mmal_port_format_commit(splitter_component->output[i]);
       if ( status ) {
           cerr<< ( "camera splitter port format commit error" );
           destroyCameraComponent();
           return;
       }

    }


    status = connectPorts( camera_video_output, splitter_input_port, &splitter_connection  );
     if ( status ) {
         cerr<< ( "camera splitter port connection error" );
         destroyCameraComponent();
         return;
     }


    /* Enable component */
    status = mmal_component_enable ( camera_component );

    if ( status ) {
        cerr<< ( "camera component couldn't be enabled" );
        //mmal_component_destroy ( camera );
        destroyCameraComponent();
        return;
    }

    status = mmal_component_enable ( splitter_component );

    if ( status ) {
        cerr<< ( "splitter component couldn't be enabled" );
        //mmal_component_destroy ( splitter );
        destroyCameraComponent();
        return;
    }
}


/**
 * @brief VideoMMALObject::destroyPreviewComponent
 * Destroy the Preview(Resizer) component and clean involved objects
 */
void VideoMMALObject::destroyVideoPreviewComponent()
{
    // Disable resizer port
    if ( resizer_output_port && resizer_output_port->is_enabled ) {
        mmal_port_disable ( resizer_output_port );
        resizer_output_port = NULL;
    }
    if (resizer_connection )
        destroyConnection(resizer_connection);

    if ( resize_pool )
        mmal_port_pool_destroy ( resizer_component->output[0], resize_pool );

    // Disable all our ports that are not handled by connections
    if ( resizer_component )
        mmal_component_disable ( resizer_component );

    if ( resizer_component ) {
        mmal_component_destroy ( resizer_component );
        resizer_component = NULL;
    }
}

/**
 * @brief VideoMMALObject::createPreviewComponent
 * Create the Preview (Resizer) component.
 */
void VideoMMALObject::createVideoPreviewComponent()
{
    if (!isOpened())
    {
        open();
    }

    MMAL_ES_FORMAT_T *format;
    MMAL_STATUS_T status;

    cerr << "Setup Preview : " << m_video_preview_width << ", "<< m_video_preview_height << endl;
    status = mmal_component_create ( "vc.ril.isp", &resizer_component );

    if ( status != MMAL_SUCCESS ) {
        cerr<< ( "Failed to create resizer component" );
        destroyVideoPreviewComponent();
        return;
    }

    resizer_input_port = resizer_component->input[0];
    resizer_output_port = resizer_component->output[0];

    resizer_output_port->userdata = ( struct MMAL_PORT_USERDATA_T * ) &resize_callback_data;


    mmal_format_copy(resizer_input_port->format, splitter_output_video_port->format);

    resizer_input_port->buffer_num = resizer_input_port->buffer_num_recommended;
    if (resizer_input_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
        resizer_input_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    resizer_input_port->buffer_size = resizer_input_port->buffer_size_recommended;
    if (resizer_input_port->buffer_size < resizer_input_port->buffer_size_min)
        resizer_input_port->buffer_size = resizer_input_port->buffer_size_min;
    status = mmal_port_format_commit(resizer_input_port);

    mmal_format_copy(resizer_output_port->format, resizer_input_port->format);

    format = resizer_output_port->format;
    format->encoding_variant = m_video_preview_format;
    format->encoding = m_video_preview_format;
    format->es->video.width = VCOS_ALIGN_UP(m_video_preview_width, 32);
    format->es->video.height = VCOS_ALIGN_UP(m_video_preview_height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = m_video_preview_width;
    format->es->video.crop.height = m_video_preview_height;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;


    status = mmal_port_format_commit(resizer_output_port);



    status = connectPorts( splitter_output_video_port, resizer_input_port, &resizer_connection  );
    if ( status )
    {
        cerr<< ( "splitter resizer port connection error" );
        destroyVideoPreviewComponent();
        return;
    }


    status = mmal_port_enable ( resizer_output_port,video_preview_buffer_callback );
    if ( status )
    {
        cerr<< ( " Resizer (Preview) callback link error" );
        destroyVideoPreviewComponent();
        return;
    }

    resizer_output_port->buffer_size = resizer_output_port->buffer_size_recommended;
    resizer_output_port->buffer_num = resizer_output_port->buffer_num_recommended;

    if (resizer_output_port->buffer_size < resizer_output_port->buffer_size_min)
        resizer_output_port->buffer_size = resizer_output_port->buffer_size_min;


    if (resizer_output_port->buffer_num < resizer_output_port->buffer_num_min)
        resizer_output_port->buffer_num = resizer_output_port->buffer_num_min;


    resize_pool = mmal_port_pool_create ( resizer_output_port, resizer_output_port->buffer_num, resizer_output_port->buffer_size );
    if ( !resize_pool )
    {
           cerr<< ( "Failed to create buffer header pool for video output port" );
           destroyVideoPreviewComponent();
           return;
    }
    resize_callback_data.resizer_pool = resize_pool;


       // Enable resizer (preview) components
    status = mmal_component_enable ( resizer_component );

    if ( status )
    {
        cerr<< ( "resizer component couldn't be enabled" );
        destroyVideoPreviewComponent();
        return;
    }


    int num = mmal_queue_length ( resize_pool->queue );
    int q;
    for ( q=0; q<num; q++ ) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get ( resize_pool->queue );

        if ( !buffer )
            cerr<<"Unable to get a required buffer"<<q<<" from pool queue"<<endl;

        if ( mmal_port_send_buffer ( resizer_output_port, buffer ) != MMAL_SUCCESS )
            cerr<<"Unable to send a buffer to encoder output port "<< q<<endl;
    }
}

/**
 * @brief VideoMMALObject::destroyEncoderComponent
 * Destroy the Record (Encoder) component and clean involved objects
 */
void VideoMMALObject::destroyEncoderComponent() {

    // Disable encoder_output_port
    if ( encoder_output_port && encoder_output_port->is_enabled ) {
        mmal_port_disable ( encoder_output_port );
        encoder_output_port = NULL;
    }
    //Destroy encoder connection
    if (encoder_connection)
        destroyConnection(encoder_connection);


    if ( encoder_pool ) {
        mmal_port_pool_destroy ( encoder_component->output[0], encoder_pool );
    }

    // Disable all our ports that are not handled by connections
    if ( encoder_component )
        mmal_component_disable ( encoder_component );


    if ( encoder_component ) {
        mmal_component_destroy ( encoder_component );
        encoder_component = NULL;
    }
}

/**
 * @brief VideoMMALObject::createEncoderComponent
 * Create the Record (Encoder) component.
 */
void VideoMMALObject::createEncoderComponent() {
    if (!isOpened())
    {
        open();
    }

    MMAL_STATUS_T status;

    cerr << "create encoder component" << endl;
    if ( mmal_component_create ( MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder_component ) ) {
        cerr  << ": Could not create encoder component.\n";
        destroyEncoderComponent();
        return;
    }
    if ( !encoder_component->input_num || !encoder_component->output_num ) {
        cerr  << ": Encoder does not have input/output ports.\n";
        destroyEncoderComponent();
        return;
    }
    cerr << "encoder component created" << endl;
    encoder_input_port = encoder_component->input[0];
    encoder_output_port = encoder_component->output[0];

    encoder_output_port->userdata = ( struct MMAL_PORT_USERDATA_T * ) &encoder_callback_data;

    mmal_format_copy ( encoder_output_port->format, encoder_input_port->format );
    encoder_output_port->format->encoding = MMAL_ENCODING_H264; // encode to H264
    encoder_output_port->format->bitrate = 17000000;
    encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;

    if ( encoder_output_port->buffer_size < encoder_output_port->buffer_size_min )
        encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
    encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
    if ( encoder_output_port->buffer_num < encoder_output_port->buffer_num_min )
        encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;

    MMAL_PARAMETER_VIDEO_PROFILE_T  param;
    param.hdr.id = MMAL_PARAMETER_PROFILE;
    param.hdr.size = sizeof(param);
    param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_HIGH;
    param.profile[0].level = MMAL_VIDEO_LEVEL_H264_4;


    if (mmal_port_parameter_set(encoder_output_port, &param.hdr) != MMAL_SUCCESS)
    {
        cerr << "Unable to set H264 profile" << endl;
        destroyEncoderComponent();
        return;
     }
     cerr << "encoder output port param created" << endl;

    // We need to set the frame rate on output to 0, to ensure it gets
    // updated correctly from the input framerate when port connected
    encoder_output_port->format->es->video.frame_rate.num = 0;
    encoder_output_port->format->es->video.frame_rate.den = 1;

    if ( mmal_port_format_commit(encoder_output_port) ) {
        cerr  << "Could not set format on encoder output port.\n";
        destroyEncoderComponent();
        return;
    }


    if ( mmal_component_enable(encoder_component)) {
        cerr << "Could not enable encoder component.\n";
        destroyEncoderComponent();
        return;
    }

    cerr << "encoder enabled" << endl;
    encoder_pool = mmal_port_pool_create ( encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size );
    if ( ! ( encoder_pool ) ) {
        cerr  << "Failed to create buffer header pool for encoder output port.\n";
        destroyEncoderComponent();
        return;
    }
    encoder_callback_data.encoder_pool = encoder_pool;

    cerr << "encoder pool created" << endl;

    if ((status = connectPorts(splitter_output_record_port,encoder_input_port,&encoder_connection)) != MMAL_SUCCESS)
    {
        cerr  << "Could not connect splitter output port to encoder input port.\n";
        destroyEncoderComponent();
        return;
    }
    cerr << "encoder connect ports created" << endl;
    if ((status = mmal_port_enable(encoder_output_port, encoder_buffer_callback)) != MMAL_SUCCESS)
    {
        cout << "Failed to enable encoder output port.\n";
        destroyEncoderComponent();
        return;
    }
    cerr << "encoder port enabled" << endl;

    for (unsigned int q=0; q < mmal_queue_length ( encoder_pool->queue ); q++ ) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get ( encoder_pool->queue );

        if ( !buffer )
            cerr<<"Unable to get a required buffer"<<q<<" from pool queue"<<endl;

        if ( mmal_port_send_buffer ( encoder_output_port, buffer ) != MMAL_SUCCESS )
            cerr<<"Unable to send a buffer to encoder output port "<< q<<endl;
    }
        cerr << "encoder setup finished" << endl;
}

void VideoMMALObject::video_preview_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    MMAL_BUFFER_HEADER_T *new_buffer;
    PORT_RESIZER_USERDATA *pData = ( PORT_RESIZER_USERDATA * ) port->userdata;

    bool hasGrabbed=false;
    std::unique_lock<std::mutex> lck ( pData->_mutex );
    if ( pData ) {
        if ( pData->wantToGrab &&  buffer->length ) {
            pData->buffer_data = new unsigned char[buffer->length]();
            mmal_buffer_header_mem_lock ( buffer );
            pData->buffer_length = buffer->length;
            memcpy ( pData->buffer_data,buffer->data,buffer->length );
            pData->wantToGrab = false;
            hasGrabbed=true;
            mmal_buffer_header_mem_unlock ( buffer );
        }
    }
    //pData->_mutex.unlock();
    // if ( hasGrabbed ) pData->Thcond.BroadCast(); //wake up waiting client
    // release buffer back to the pool
    mmal_buffer_header_release ( buffer );
    // and send one back to the port (if still open)
    if ( port->is_enabled ) {
        MMAL_STATUS_T status;

        new_buffer = mmal_queue_get ( pData->resizer_pool->queue );

        if ( new_buffer )
            status = mmal_port_send_buffer ( port, new_buffer );

        if ( !new_buffer || status != MMAL_SUCCESS )
            printf ( "Unable to return a buffer to the encoder port" );
    }

    if ( hasGrabbed ) pData->Broadcast(); //wake up waiting client

}

/**
   *  buffer header callback function for encoder
   *
   *  Callback will dump buffer data to the specific file.
   *  (Doesn't handle segmented mp4 files yet)
   *
   * @param port Pointer to port from which callback originated
   * @param buffer mmal buffer header pointer
   */
void VideoMMALObject::encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
  MMAL_BUFFER_HEADER_T *new_buffer;


  // We pass our file handle and other stuff in via the userdata field.

  PORT_ENCODER_USERDATA *pData = (PORT_ENCODER_USERDATA *)port->userdata;

  if (pData)
  {
      if (!(buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) ) {
          pData->videoFile->write((char *)buffer->data, buffer->length);
      }
  }


  // release buffer back to the pool
  mmal_buffer_header_release(buffer);

  // and send one back to the port (if still open)qs
  if (port->is_enabled)
  {
     MMAL_STATUS_T status;

     new_buffer = mmal_queue_get(pData->encoder_pool->queue);

     if (new_buffer)
        status = mmal_port_send_buffer(port, new_buffer);

     if (!new_buffer || status != MMAL_SUCCESS)
        cout << "Unable to return a buffer to the encoder port\n";
  }
}

void VideoMMALObject::setPreviewVideoImageFormat(int mmal_image_format)
{
    m_video_preview_format = mmal_image_format;
}

void VideoMMALObject::setVideoStabilization(bool v) {
    m_cam_params.videoStabilisation=v;
    if ( isOpened() ) commitVideoStabilization();
}

void VideoMMALObject::setBrightness (unsigned int brightness) {
    if ( brightness > 100 )                brightness = 100 ;
    m_cam_params.brightness = brightness;
    if ( isOpened() ) commitBrightness();
}
void VideoMMALObject::setShutterSpeed (unsigned  int shutter) {
    if ( shutter > 330000 )
        shutter = 330000;
    m_cam_params.shutterSpeed= shutter;
    if ( isOpened() ) commitShutterSpeed();
}

void VideoMMALObject::setRotation(int rotation) {
    while ( rotation < 0 )
        rotation += 360;
    if ( rotation >= 360 )
        rotation = rotation % 360;
    m_cam_params.rotation = rotation;
    if ( isOpened() ) commitRotation();
}

void VideoMMALObject::setISO(int iso) {
    m_cam_params.ISO = iso;
    if ( isOpened() ) commitISO();
}

void VideoMMALObject::setSharpness(int sharpness) {
    if ( sharpness < -100 ) sharpness = -100;
    if ( sharpness > 100 ) sharpness = 100;
    m_cam_params.sharpness = sharpness;
    if ( isOpened() ) commitSharpness();
}

void VideoMMALObject::setContrast(int contrast) {
    if ( contrast < -100 ) contrast = -100;
    if ( contrast > 100 ) contrast = 100;
    m_cam_params.contrast = contrast;
    if ( isOpened() ) commitContrast();
}

void VideoMMALObject::setSaturation(int saturation) {
    if ( saturation < -100 ) saturation = -100;
    if ( saturation > 100 ) saturation = 100;
    m_cam_params.saturation = saturation;
    if ( isOpened() ) commitSaturation();
}

void VideoMMALObject::setAWB_RB(float red_g, float blue_g) {
    m_cam_params.awbg_blue = blue_g;
    m_cam_params.awbg_red = red_g;
    if ( isOpened() ) commitAWB_RB();
}
void VideoMMALObject::setExposure(MMAL_PARAM_EXPOSUREMODE_T exposure) {
    m_cam_params.exposureMode = exposure;
    if ( isOpened() ) commitExposure();
}

void VideoMMALObject::setAWB(MMAL_PARAM_AWBMODE_T awb) {
    m_cam_params.awbMode = awb;
    if ( isOpened() ) commitAWB();
}

void VideoMMALObject::setImageEffect(MMAL_PARAM_IMAGEFX_T imageEffect) {
    m_cam_params.imageEffect = imageEffect;
    if ( isOpened() ) commitImageEffect();
}

void VideoMMALObject::setMetering(MMAL_PARAM_EXPOSUREMETERINGMODE_T metering) {
    m_cam_params.exposureMeterMode = metering;
    if ( isOpened() ) commitMetering();
}
void VideoMMALObject::setExposureCompensation(int val) {
    if ( val < -10 ) val= -10;
    if ( val > 10 ) val = 10;
    m_cam_params.exposureCompensation=val;
    if ( isOpened() ) commitExposureCompensation();
}

void VideoMMALObject::setHorizontalFlip(bool hFlip) {
    m_cam_params.hflip = hFlip;
    if ( isOpened() ) commitFlips();
}

void VideoMMALObject::setVerticalFlip(bool vFlip) {
    m_cam_params.vflip = vFlip;
    if ( isOpened() ) commitFlips();
}

/**
 * @brief VideoMMALObject::setFrameRate
 * @param framerate
 * Set the framerate of the camera component.
 * The changes will be considered when the camera is re-opened.
 */
void VideoMMALObject::setFrameRate(unsigned int framerate)
{
    m_cam_params.framerate = framerate;
}


void VideoMMALObject::commitAWB_RB() {
    MMAL_PARAMETER_AWB_GAINS_T param = {{MMAL_PARAMETER_CUSTOM_AWB_GAINS,sizeof(param)}, {0,0}, {0,0}};
    param.r_gain.num = (unsigned int)(m_cam_params.awbg_red * 65536);
    param.b_gain.num = (unsigned int)(m_cam_params.awbg_blue * 65536);
    param.r_gain.den = param.b_gain.den = 65536;
    if ( mmal_port_parameter_set(camera_component->control, &param.hdr) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set AWBG gains parameter.\n";
}

void VideoMMALObject::commitBrightness() {
    mmal_port_parameter_set_rational ( camera_component->control, MMAL_PARAMETER_BRIGHTNESS, ( MMAL_RATIONAL_T ) {
        m_cam_params.brightness, 100
    } );
}


void VideoMMALObject::commitRotation() {
    int rotation = int ( m_cam_params.rotation / 90 ) * 90;
    mmal_port_parameter_set_int32 ( camera_component->output[0], MMAL_PARAMETER_ROTATION,rotation );
    mmal_port_parameter_set_int32 ( camera_component->output[1], MMAL_PARAMETER_ROTATION,rotation );
    mmal_port_parameter_set_int32 ( camera_component->output[2], MMAL_PARAMETER_ROTATION, rotation );
}

void VideoMMALObject::commitISO() {
    if ( mmal_port_parameter_set_uint32 ( camera_component->control, MMAL_PARAMETER_ISO, m_cam_params.ISO ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set ISO parameter.\n";
}

void VideoMMALObject::commitSharpness() {
    if ( mmal_port_parameter_set_rational ( camera_component->control, MMAL_PARAMETER_SHARPNESS, ( MMAL_RATIONAL_T ){m_cam_params.sharpness, 100} ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set sharpness parameter.\n";
}

void VideoMMALObject::commitShutterSpeed() {
    if ( mmal_port_parameter_set_uint32 ( camera_component->control, MMAL_PARAMETER_SHUTTER_SPEED, m_cam_params.shutterSpeed ) !=  MMAL_SUCCESS )
      cerr << __func__ << ": Failed to set shutter parameter.\n";
}

void VideoMMALObject::commitContrast() {
    if ( mmal_port_parameter_set_rational ( camera_component->control, MMAL_PARAMETER_CONTRAST, ( MMAL_RATIONAL_T ) {m_cam_params.contrast, 100} ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set contrast parameter.\n";
}

void VideoMMALObject::commitSaturation() {
    if ( mmal_port_parameter_set_rational ( camera_component->control, MMAL_PARAMETER_SATURATION, ( MMAL_RATIONAL_T ) {m_cam_params.saturation, 100} ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set saturation parameter.\n";
}

void VideoMMALObject::commitExposure() {
    MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE,sizeof ( exp_mode ) },  m_cam_params.exposureMode };
    if ( mmal_port_parameter_set ( camera_component->control, &exp_mode.hdr ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set exposure parameter.\n";
}

void VideoMMALObject::commitExposureCompensation() {
    if ( mmal_port_parameter_set_int32 ( camera_component->control, MMAL_PARAMETER_EXPOSURE_COMP , m_cam_params.exposureCompensation ) !=MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set Exposure Compensation parameter.\n";
}

void VideoMMALObject::commitAWB() {
    MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE,sizeof ( param ) }, m_cam_params.awbMode };
    if ( mmal_port_parameter_set ( camera_component->control, &param.hdr ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set AWB parameter.\n";
}

void VideoMMALObject::commitImageEffect() {
    MMAL_PARAMETER_IMAGEFX_T imgFX = {{MMAL_PARAMETER_IMAGE_EFFECT,sizeof ( imgFX ) }, m_cam_params.imageEffect };
    if ( mmal_port_parameter_set ( camera_component->control, &imgFX.hdr ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set image effect parameter.\n";
}

void VideoMMALObject::commitMetering() {
    MMAL_PARAMETER_EXPOSUREMETERINGMODE_T meter_mode = {{MMAL_PARAMETER_EXP_METERING_MODE, sizeof ( meter_mode ) }, m_cam_params.exposureMeterMode };
    if ( mmal_port_parameter_set ( camera_component->control, &meter_mode.hdr ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set metering parameter.\n";
}

void VideoMMALObject::commitFlips() {
    MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof ( MMAL_PARAMETER_MIRROR_T ) }, MMAL_PARAM_MIRROR_NONE};
    if ( m_cam_params.hflip && m_cam_params.vflip )
        mirror.value = MMAL_PARAM_MIRROR_BOTH;
    else if ( m_cam_params.hflip )
        mirror.value = MMAL_PARAM_MIRROR_HORIZONTAL;
    else if ( m_cam_params.vflip )
        mirror.value = MMAL_PARAM_MIRROR_VERTICAL;
    if ( mmal_port_parameter_set ( camera_component->output[0], &mirror.hdr ) != MMAL_SUCCESS ||
        mmal_port_parameter_set ( camera_component->output[1], &mirror.hdr ) != MMAL_SUCCESS ||
        mmal_port_parameter_set ( camera_component->output[2], &mirror.hdr ) )
    cerr << __func__ << ": Failed to set horizontal/vertical flip parameter.\n";
}

void VideoMMALObject::commitVideoStabilization() {
    if ( mmal_port_parameter_set_boolean ( camera_component->control, MMAL_PARAMETER_VIDEO_STABILISATION, m_cam_params.videoStabilisation ) != MMAL_SUCCESS )
        cerr << __func__ << ": Failed to set video stabilization parameter.\n";
}
