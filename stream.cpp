#include "stream.h"
#include "calibrate_square.h"
#include "calibrate_panorama.h"

using namespace std;
using std::vector;
using namespace cv;

//function prototypes for gstreamer loop
void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer user_data);
gboolean bus_cb(GstBus *bus, GstMessage *msg, gpointer user_data);
GstFlowReturn new_preroll(GstAppSink *asink, gpointer data);
GstFlowReturn new_buffer(GstAppSink *asink, gpointer data);
static void cb_need_data (GstElement *asrc,guint unused_size, gpointer user_data);

//global elements
GMainLoop *loop;
GstElement *pipeline;
GstElement *warped_pipe;
GstElement *src;
GstElement *depay;

GstElement *sink;
GstElement *appsrc;
GstElement *appsink;
gboolean src_linked;
GstElement *mux;
GstElement *enc;
GstElement *appsrc_caps;

calibrate_square *calib_square;
calibrate_panorama *calib_pano;
stream *this_stream;
viewThread *vthread;

stream::stream( )
{
    calib_square = new calibrate_square();
    calib_pano = new calibrate_panorama();
    vthread = new viewThread();
    this_stream = this;
}

bool stream::buildpipeline()
{
    //create pipeline elements
    //gst-launch-0.10 rtspsrc latency=0 location= ! rtph264depay ! h264parse ! ffdec_h264 ! videoscale ! capsfilter caps=video/x-raw-yuv,width=640,height=480 ! ffmpegcolorspace ! capsfilter caps=video/x-raw-rgb,bpp=16,depth=16 ! ffmpegcolorspace ! xvimagesink

    GstElement *parse;
    GstElement *decode;
    GstElement *conv;
    GstElement *conv1;
    GstElement *scale;
    GstElement *yuvfilter;
    GstElement *rgbfilter;

    GstElement *appsrc_conv;

    //create all requred elements
    loop = g_main_loop_new(NULL, false);
    pipeline = gst_pipeline_new("input-pipeline");
    warped_pipe = gst_pipeline_new("warped-pipeline");

    src = gst_element_factory_make("rtspsrc", NULL);
    depay = gst_element_factory_make("rtph264depay", NULL);
    parse = gst_element_factory_make("h264parse", NULL);
    decode = gst_element_factory_make("ffdec_h264", NULL);
    scale = gst_element_factory_make("videoscale", NULL);
    yuvfilter = gst_element_factory_make("capsfilter", NULL);
    conv = gst_element_factory_make("ffmpegcolorspace", NULL);
    conv1 = gst_element_factory_make("ffmpegcolorspace", NULL);
    rgbfilter = gst_element_factory_make("capsfilter", NULL);
    appsink = gst_element_factory_make("appsink", NULL);

    appsrc = gst_element_factory_make("appsrc", NULL);
    appsrc_caps = gst_element_factory_make("capsfilter", NULL);
    appsrc_conv = gst_element_factory_make("ffmpegcolorspace",NULL);
    enc = gst_element_factory_make("ffenc_mpeg4", NULL);
    mux = gst_element_factory_make("mp4mux", NULL);
    sink = gst_element_factory_make("filesink", NULL);

    //set filesink location
    g_object_set(sink, "location", "capture.mp4", NULL);

    //set appsrc to accept rgb, appsrc_caps sepecifies dimensions & details
    g_object_set(G_OBJECT(appsrc), "caps", gst_caps_new_simple("video/x-raw-rgb",
                                                               NULL),NULL);

    //set yuv filter & frame size
    gst_util_set_object_arg(G_OBJECT(yuvfilter), "caps",
                            "video/x-raw-yuv,width=640,height=480");

    //set rgb filter
    gst_util_set_object_arg(G_OBJECT(rgbfilter), "caps",
                            "video/x-raw-rgb");

    //set appsink to match rgb
    GstCaps *caps = gst_caps_from_string("video/x-raw-rgb,width=640,height=480");
    gst_app_sink_set_caps((GstAppSink*)appsink,caps);

    //set rtspsrc location
    //TODO make command line arg
    g_object_set(src, "location", "rtsp://admin:123456@10.10.40.100:554/realmonitor?channel=0.sdp", "latency", 0, NULL);

    //set app sink properties
    gst_app_sink_set_emit_signals((GstAppSink*)appsink, true);
    gst_app_sink_set_drop((GstAppSink*)appsink, true);
    gst_app_sink_set_max_buffers((GstAppSink*)appsink, 1);
    GstAppSinkCallbacks callbacks = {NULL, new_preroll, new_buffer, NULL};
    gst_app_sink_set_callbacks((GstAppSink*)appsink, &callbacks, NULL, NULL);

    //connect the appsrc buffer signal,
    //set to false initially to prevent it requesting data before we are ready
    gst_app_src_set_emit_signals ((GstAppSrc*)appsrc, false);
    g_signal_connect(appsrc, "need-data", G_CALLBACK(cb_need_data), NULL);

    if(!src || !depay || !parse || !decode || !scale || !yuvfilter || !conv || !rgbfilter || !appsink){
        g_printerr("could not create all elements\n");
        return false;
    }
    if(!appsrc || !appsrc_caps || !appsrc_conv || !sink ){
        g_printerr("could not create all elements\n");
        return false;
    }

    //add to pipeline
    gst_bin_add_many(GST_BIN(pipeline), src, depay, parse, decode,  scale,yuvfilter, conv, rgbfilter, conv1, appsink,  NULL);

    //add elements to warped pipeline
    gst_bin_add_many(GST_BIN(warped_pipe), appsrc,appsrc_caps,appsrc_conv, enc,mux,sink, NULL);

    //linki all static pads
    if(!gst_element_link_many(depay, parse, decode, scale, yuvfilter,conv, rgbfilter, conv1, appsink, NULL)){
        g_printerr("could not link all elements\n");
        return false;
    }

    if(!gst_element_link_many(appsrc,appsrc_caps, appsrc_conv, enc, mux,sink, NULL)){
        g_print("could not link appsrc\n");
        return false;
    }

    //set callback for rtspsrc
    src_linked = false;
    g_signal_connect(src, "pad-added", G_CALLBACK(pad_added_handler), loop);

    //add bus callback
    gst_bus_add_watch(GST_ELEMENT_BUS(pipeline), bus_cb, loop);
    gst_bus_add_watch(GST_ELEMENT_BUS(warped_pipe), bus_cb, loop);

    //set to PAUSED so that all elements are prepared for data flow
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    gst_element_set_state(warped_pipe, GST_STATE_PAUSED);

    return true;
}

void stream::startstream()
{
    GstPad* encpad;
    encpad = gst_element_get_static_pad(enc, "sink");
    //set pipeline to PLAYING
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    //start view thread before main loop is entered
    vthread->run();
    //enter main loop
    g_main_loop_run(loop);

    //once exited set to NULL and unref
    //send eos to enc so that mp4 is written correctly
    gst_pad_send_event(encpad, gst_event_new_eos());
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_element_set_state(warped_pipe, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(warped_pipe);

    return;
}

gboolean bus_cb(GstBus *bus, GstMessage *msg, gpointer user_data)
{
    Q_UNUSED(bus);
    Q_UNUSED(user_data);

    //parse bus messages
    switch(GST_MESSAGE_TYPE(msg)){
        case GST_MESSAGE_ERROR:{
            //quit on error
            GError *err = NULL;
            gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            gst_object_default_error(msg->src, err, dbg);
            g_clear_error(&err);
            g_free(dbg);
            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:{
            GstState old_state, pending_state, new_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
            //if message from either pipeline, display state change
            if(GST_OBJECT_NAME(msg->src) == GST_OBJECT_NAME(pipeline) ||GST_OBJECT_NAME(msg->src) == GST_OBJECT_NAME(warped_pipe) ){
                g_print("'%s' state changed from %s to %s. \n", GST_OBJECT_NAME(msg->src), gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
                //if input pipeline is now 'PLAYING' set warped pipeline to 'PLAYING'
                if(GST_OBJECT_NAME(msg->src) == GST_OBJECT_NAME(pipeline) && new_state == GST_STATE_PLAYING){
                    gst_element_set_state(warped_pipe, GST_STATE_PLAYING);
                }
            }
            break;
        }
        default:{
            break;
        }
    }
    return true;

}

GstFlowReturn new_preroll(GstAppSink *asink, gpointer data)
{
    //confirms successful initialisation
    Q_UNUSED(asink);
    Q_UNUSED(data);

    g_print("Got preroll\n");
    return GST_FLOW_OK;
}

GstBuffer *temp;
int fcount;

GstFlowReturn new_buffer(GstAppSink *asink, gpointer data)
{
    fcount++;
    Q_UNUSED(data);
    GstBufferCopyFlags copyflags = GST_BUFFER_COPY_ALL;

    //grab available buffer
    GstBuffer *buffer =  gst_app_sink_pull_buffer(asink);

    //convert GstBuffer to Mat
    Mat frame(Size(640,480), CV_8UC3, (char*)GST_BUFFER_DATA(buffer), Mat::AUTO_STEP);

    Mat frame2;
    //gstreamer buffer is rgb, opencv wants bgr.
    cvtColor(frame, frame2,CV_RGB2BGR);

    //calib_pano for horizontal mount panorama
    Mat warped = calib_pano->do_warp(frame2);

    //calib_square for vertical mount square image
    //Mat warped = calib_square->getmatrixvalues(frame2);

    //load image in opencv format to view thread
    Mat view;
    warped.copyTo(view);
    vthread->showImage(view);

    //convert to rgb for use with gstreamer
    Mat newmat(warped.rows, warped.cols, CV_8UC3);
    cvtColor(warped,newmat, CV_BGR2RGB);

    if(fcount == 1){
        //assign memory block on heap
        char *app_caps = new char [1024];

        //get new dimensions
        int height = frame.rows;
        int width = frame.cols;

        //set caps char
        sprintf(app_caps,"video/x-raw-rgb, width=(int)%d, height=(int)%d, bpp=(int)24, depth=(int)24, endianness=(int)4321,"\
                 "red_mask=(int)16711680, green_mask=(int)65280, blue_mask=(int)255, framerate=(fraction)25/1, interlaced=(boolean)false,"
                 "pixel-aspect-ratio=(fraction)15/16", width, height);

        //assign caps
        gst_util_set_object_arg(G_OBJECT(appsrc_caps), "caps", app_caps);

        //delete memory block
        delete [] app_caps;
    }

    //copy input buffer unchanged
    temp =gst_buffer_copy(buffer);

    //copy metadata tags
    gst_buffer_copy_metadata(temp,buffer,copyflags);

    //tell appsrc buffer is ready
    g_signal_emit_by_name(appsrc, "need-data", appsrc, NULL,NULL);

    //unref buffer so it can be used again
    gst_buffer_unref(buffer);

    return GST_FLOW_OK;
}

static void cb_need_data (GstElement *asrc,guint unused_size, gpointer user_data)
{
    Q_UNUSED(asrc);
    Q_UNUSED(unused_size);
    Q_UNUSED(user_data);
    GstFlowReturn ret;
    GstBuffer *buffer;

    //copy data to local buffer
    buffer = gst_buffer_copy(temp);

    //free temp buffer
    gst_buffer_unref(temp);

    //push buffer down the pipeline
    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);

    //confirm buffer sent properly
    if (ret != GST_FLOW_OK) {
        g_main_loop_quit (loop);
    }else{
        //stop appsrc from instantly requesting another buffer
        gst_app_src_set_emit_signals ((GstAppSrc*)asrc, false);
    }

    //unref buffer for next use
    gst_buffer_unref (buffer);
}

void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer user_data)
{
    GstPadLinkReturn ret;
    GstPad *depay_pad;
    GstCaps *filter = NULL;
    gboolean caps_accepted = false;

    Q_UNUSED(src);
    Q_UNUSED(user_data);

    //check if already linked, if so then exit
    if(src_linked){
        g_print("source already linked\n");
        return;
    }

    //set pad to be linked
    depay_pad = gst_element_get_static_pad(depay, "sink");
    //specify pad caps
    filter = gst_caps_new_simple("application/x-rtp",
                                 "media", G_TYPE_STRING,"video",
                                 NULL);
    //confirm new pad caps
    caps_accepted = gst_pad_accept_caps(new_pad, filter);
    if(!caps_accepted){
        g_printerr("caps not accepted by sink\n");
        return;
    }

    //set caps on pad to be linked
    gst_pad_fixate_caps(depay_pad, filter);

    ret = gst_pad_link(new_pad, depay_pad);
    if(ret != GST_PAD_LINK_OK){
        g_printerr("pad link failed\n");
        return;
    }
    else{
        g_print("pad %s linked to %s successfully\n",GST_PAD_NAME(new_pad), GST_PAD_NAME(depay_pad));
        src_linked = true;
    }
}

viewThread::viewThread()
{
}

void viewThread::run()
{
    qDebug() << "viewThread running";
}

int topleftx, toplefty, bottomrightx, bottomrighty;
//TODO check points in image

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    Q_UNUSED(flags);
    Q_UNUSED(userdata);

    //store top left co-ordinates
    if ( event == EVENT_LBUTTONDOWN){
        cout << "lbutton down, x: " << x << " y: " << y << endl;
        topleftx = x;
        toplefty = y;
    }
    //store bottom right coordinates
    else if ( event == EVENT_LBUTTONUP){
        cout << "lbutton up, x: " << x << " y: " << y << endl;
        bottomrightx = x;
        bottomrighty = y;
        //if top left == bottom right close image
        if(topleftx == bottomrightx || toplefty == bottomrighty){
            topleftx = toplefty = bottomrightx = bottomrighty = 0;
        }
    }
}

void viewThread::showImage(Mat view)
{
    namedWindow("Dewarped");
    setMouseCallback("Dewarped", mouseCallback);

    //if rect has been drawn
    if(bottomrightx != 0 && bottomrighty != 0){
        Point p0(topleftx,toplefty);
        Point p1(bottomrightx,bottomrighty);
        //show rectangle on original image
        rectangle(view, p0,p1,Scalar(0,0,255));
        //show selected image in new window
        Rect ROI(p0,p1);
        Mat roi = view(ROI);
        resize(roi,roi,Size(300,300));
        namedWindow("roi");
        imshow("roi", roi);
    }else{
        destroyWindow("roi");
    }

    imshow("Dewarped", view);

    //if 'esc' key then break
    char c = waitKey(10);
    if(c == 27){
        g_main_loop_quit(loop);
    }
}

