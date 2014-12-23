/*
 * Copyright (C)2013  iCub Facility - Istituto Italiano di Tecnologia
 * Author: Marco Randazzo
 * email:  marco.randazzo@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Time.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>

#include <yarp/os/Semaphore.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Thread.h>

// #include <iCub/iDyn/iDyn.h>
// #include <iCub/iDyn/iDynBody.h>
#include <iCub/ctrl/adaptWinPolyEstimator.h>

#include "constants.h"
#include "scriptModule.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <math.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::math;
using namespace iCub::ctrl;

// #define VCTP_TIME VOCAB4('t','i','m','e')
// #define VCTP_OFFSET VOCAB3('o','f','f')
// #define VCTP_CMD_NOW VOCAB4('c','t','p','n')
// #define VCTP_CMD_QUEUE VOCAB4('c','t','p','q')
// #define VCTP_CMD_FILE VOCAB4('c','t','p','f')
// #define VCTP_POSITION VOCAB3('p','o','s')
// #define VCTP_WAIT VOCAB4('w','a','i','t')
//
// #define ACTION_IDLE    0
// #define ACTION_START   1
// #define ACTION_RUNNING 2

int main(int argc, char *argv[])
{
    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("ctpService/conf");
    rf.configure("ICUB_ROOT",argc,argv);

    if (rf.check("help"))
    {
        cout << "Options:" << endl << endl;
        cout << "\t--name         <moduleName>: set new module name" << endl;
        cout << "\t--robot        <robotname>:  robot name"          << endl;
        cout << "\t--file         <filename>:   the positions file (with both legs)"  << endl;
        cout << "\t--filename2    <filename>:   to specifiy to use two files (left and leg separate). _left.txt and _right.txt automatically appended"  << endl;
        cout << "\t--execute      activate the iPid->setReference() control"  << endl;
        cout << "\t--period       <period>: the period in ms of the internal thread (default 5)"  << endl;
        cout << "\t--speed        <factor>: speed factor (default 1.0 normal, 0.5 double speed, 2.0 half speed etc)"  << endl;
        return 0;
    }

    Network yarp;

    if (!yarp.checkNetwork())
    {
        cout << "ERROR: yarp.checkNetwork() failed."  << endl;
        return -1;
    }

    scriptModule mod;

    return mod.runModule(rf);
}



