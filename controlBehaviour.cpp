/* 
 * File:   main.cpp
 * Author: olmin
 *
 * Created on 21 de septiembre de 2011, 13:08
 */

#include <ntk/ntk.h>
#include <ntk/utils/debug.h>
#include <ntk/camera/openni_grabber.h>
#include <ntk/gesture/body_event.h>

#include <QApplication>
#include <QDir>
#include <QMutex>


#include <ntk/camera/rgbd_image.h>
#include <ntk/gesture/skeleton.h>
#include <openni/Source/External/LibJPEG/jmorecfg.h>

#include "behaviour.h"


using namespace cv;
using namespace ntk;

namespace opt
{
ntk::arg<bool> high_resolution("--highres", "High resolution color image.", 0);
ntk::arg<int> kinect_id("--kinect-id", "Kinect id", 0);
}

int main(int argc, char **argv)
{
    // Parse command line options.
    arg_base::set_help_option("-h");
    arg_parse(argc, argv);

    // Set debug level to 1.
    ntk::ntk_debug_level = 0;

    // Set current directory to application directory.
    // This is to find Nite config in config/ directory.
    QApplication app (argc, argv);
    QDir::setCurrent(QApplication::applicationDirPath());

    // Declare the global OpenNI driver. Only one can be instantiated in a program.
    OpenniDriver ni_driver;

    // Declare the frame grabber.
    OpenniGrabber grabber(ni_driver, opt::kinect_id());

    // High resolution 1280x1024 RGB Image.
    if (opt::high_resolution())
        grabber.setHighRgbResolution(true);

    // Use mirrored images.
    grabber.setMirrored(true);
    
    // Start the grabber.
    grabber.connectToDevice();
    grabber.start();

    // Holder for the current image.
    RGBDImage image;

    // Image post processor. Compute mappings when RGB resolution is 1280x1024.
    OpenniRGBDProcessor post_processor;
   
    //Init vars
    Behaviour behaviour;
    behaviour.initialize();    
    bool first=true;   
    int count=0;
    bool complete;
    char last_c = 0;      
            
    while (true && (last_c != 27))
    {
        // Wait for a new frame, get a local copy and postprocess it.
        grabber.waitForNextFrame();
        grabber.copyImageTo(image);
        post_processor.processImage(image);

        // Prepare the color view (mapped to depth frame), with skeleton and handpoint.
        cv::Mat3b debug_color_img;
        image.mappedRgb().copyTo(debug_color_img);
        if (image.skeleton())
            image.skeleton()->drawOnImage(debug_color_img);

        
        /*****************************CONTROL BEHAVIOUR******************************************************/
        Point3f iniPoint=Point3f(0,0,0);        
        if (image.skeleton() && (image.skeleton()->getJoint(image.skeleton()->NTK_SKEL_TORSO)!= iniPoint)){//tracking user                   
            if(first){
                 behaviour.start();
                 first=false;
            } 
   
               complete= behaviour.saveData(image.skeletonRef());                    
               if(complete)
                   break;
      
        }
        /*****************************END CONTROL BEHAVIOUR******************************************************/
        
        /*shows the image*/
        imshow("color", debug_color_img);
 
        //waits for escape key
        last_c = (cv::waitKey(10) & 0xff);      
        
    }
    //Once the loop is finished we save the behaviour
    behaviour.finish();
}
