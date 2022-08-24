// Copyright (c) 2022 Social Cognition in Human-Robot Interaction,
//                    Istituto Italiano di Tecnologia, Genova
// Licence: GPLv2 (please see LICENSE file)

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <limits>
#include <random>
#include <fstream>

#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Value.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/os/RpcServer.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Image.h>
#include <yarp/sig/PointCloud.h>
#include <yarp/math/Math.h>

#include "rpc_IDL.h"

#include <yarp/cv/Cv.h>

using namespace cv;

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp> 

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;

/******************************************************************************/
class ProcessingModule : public RFModule, public rpc_IDL {

    BufferedPort<ImageOf<PixelRgb>> requestPort; // in: vector xyz where to fixate
    BufferedPort<ImageOf<PixelRgb>> responsePort; // out: vector xyz where we are fixated

    double requestedPeriod = 0.010;

    bool attach(RpcServer& source) override {
        return this->yarp().attachAsServer(source);
    }

    /**************************************************************************/
    bool configure(ResourceFinder& rf) override {
        // const string name = rf.check("name", 
        //                    Value("/imageProcessing"), 
        //                    "module name (string)").asString();
        const string name = "imagecopy";

        requestPort.open("/"+name+"/in");
        responsePort.open("/"+name+"/out");

        return true;
    }

    bool start() override {
        return false;
    }

    bool stop() override {
        return false;
    }

    bool running() override {
        return false;
    }

    bool set_rate(double rate) override {
        if( rate != 0.0 ) {
            requestedPeriod = 1.0/rate;
        } else {
            return false;
        }
        return true;
    }

    /**************************************************************************/
    double getPeriod() override {
        return requestedPeriod;
    }

    /**************************************************************************/
    bool updateModule() override {

        // VERSION 1: clean copy

        // // don't wait, just snatch if available
        // if( ImageOf<PixelRgb>* image1 = requestPort.read(false) ) {
        //     ImageOf<PixelRgb>& image2 = responsePort.prepare(); // get an empty bottle for BufferedPort
        //     image2.copy( *image1 );
        //     responsePort.write();
        // }

        // VERSION 2: Through cv::Mat

        // don't wait, just snatch if available
        if( ImageOf<PixelRgb>* image1 = requestPort.read(false) ) {
            ImageOf<PixelRgb>& image2 = responsePort.prepare(); // get an empty bottle for BufferedPort

            cv::Mat inputMatrix = yarp::cv::toCvMat(*image1);
            cv::Mat img;
            //cv::blur( inputMatrix, img, Size( 7, 7 ), Point(-1,-1) );
            int threshold_value = 3;
            int threshold_type = 0;
            int const max_binary_value = 255;
            cv::threshold( inputMatrix, img, threshold_value, max_binary_value, threshold_type );
            ImageOf<PixelRgb> image_tmp = yarp::cv::fromCvMat<PixelRgb>( img );

            image2.copy( image_tmp );
            responsePort.write();
        }


        return true;
    }

    bool interruptModule() override {
        // interrupt blocking read
        requestPort.interrupt();
        responsePort.interrupt();
        return true;
    }

    /**************************************************************************/
    bool close() override {
        requestPort.close();
        responsePort.close();

        return true;
    }
};

/******************************************************************************/
int main(int argc, char *argv[]) {
    Network yarp;
    if (!yarp.checkNetwork()) {
        yError() << "Unable to find YARP server!";
        return EXIT_FAILURE;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    ProcessingModule module;
    return module.runModule(rf);
}
