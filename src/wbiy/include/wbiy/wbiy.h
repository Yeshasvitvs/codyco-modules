
/*
 * Copyright (C) 2011  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Marco Randazzo
 * email: marco.randazzo@iit.it
 *
 * Further modifications
 *
 * Copyright (C) 2013 CoDyCo Consortium
 * Author: Serena Ivaldi
 * email: serena.ivaldi@isir.upmc.fr
 *
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

#ifndef WBIY_H
#define WBIY_H

#include <yarp/dev/all.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/os/Semaphore.h>
#include <iCub/ctrl/adaptWinPolyEstimator.h>
#include <wbi/wbi.h>
#include <map>
#if __APPLE__
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif
#include <vector>

/* CODE UNDER DEVELOPMENT */

namespace wbiy
{
    /** Return true if the robotName is "icubSim", false otherwise. */
    bool isRobotSimulator(const std::string &robotName);
    
    /** Open a remote control board driver for the specified body part. */
    bool openPolyDriver(const std::string &localName, const std::string &robotName, yarp::dev::PolyDriver *&pd, const std::string &bodyPartName);
    
    
    /*
     * Class for reading the sensors of a robot that is accessed through
     * a yarp interface (i.e. PolyDrivers)
     */
    class yarpWholeBodySensors: public wbi::iWholeBodySensors
    {
    protected:
        bool                        initDone;
        int                         dof;            // number of degrees of freedom
        std::string                 name;           // name used as root for the local ports
        std::string                 robot;          // name of the robot
        std::vector<int>            bodyParts;      // list of the body parts
        std::vector<std::string>    bodyPartNames;  // names of the body parts
        std::map<int,unsigned int>  bodyPartAxes;   // number of axes for each body part
        wbi::LocalIdList            jointIdList;    // list of the joint ids
        
        // last reading data
        std::map<int, yarp::sig::Vector>  qLastRead;
        std::map<int, yarp::sig::Vector>  qStampLastRead;
        std::map<int, yarp::sig::Vector>  pwmLastRead;

        // yarp interfaces
        std::map<int, yarp::dev::IEncodersTimed*>       ienc;
        std::map<int, yarp::dev::IOpenLoopControl*>     iopl;
        std::map<int, yarp::dev::PolyDriver*>           dd;
        
        bool openDrivers(int bodyPart);
        void setBodyPartName(int bodyPart, const std::string &nameBodyPart);
        
    public:
        // *** CONSTRUCTORS ***
        /**
          * @param _name Local name of the interface (used as stem of port names)
          * @param _robotName Name of the robot
          * @param _bodyPartNames Array of names of the body part (used when opening the polydrivers)
          */
        yarpWholeBodySensors(const char* _name, const char* _robotName, const std::vector<std::string> &_bodyPartNames);

        virtual bool init();
        virtual bool close();
        virtual bool removeJoint(const wbi::LocalId &j);
        virtual bool addJoint(const wbi::LocalId &j);
        virtual int addJoints(const wbi::LocalIdList &j);
        virtual wbi::LocalIdList getJointList(){ return jointIdList; }
        virtual int getDoFs(){ return dof; }
        
        virtual bool readEncoders(double *q, double *stamps, bool wait=true);
        virtual bool readPwm(double *pwm, double *stamps, bool wait=true);
        virtual bool readInertial(double *inertial, double *stamps, bool wait=true);
        virtual bool readFTsensors(double *ftSens, double *stamps, bool wait=true);
    };
    
    
    /*
     * Class for interfacing the actuators of a robot that is accessed through
     * a yarp interface (polydrivers etc)
     */
    class yarpWholeBodyActuators : public wbi::iWholeBodyActuators
    {
    protected:
        bool                        initDone;
        int                         dof;            // number of degrees of freedom considered
        std::string                 name;           // name used as root for the local ports
        std::string                 robot;          // name of the robot
        std::vector<int>            bodyParts;      // list of the body parts
        std::vector<std::string>    bodyPartNames;  // names of the body parts
        wbi::LocalIdList            jointIdList;    // list of the joint ids
        
        // yarp drivers
        std::map<int, yarp::dev::IPositionControl*>     ipos;
        std::map<int, yarp::dev::ITorqueControl*>       itrq;
        std::map<int, yarp::dev::IImpedanceControl*>    iimp;
        std::map<int, yarp::dev::IControlMode*>         icmd;
        std::map<int, yarp::dev::IVelocityControl*>     ivel;
        std::map<int, yarp::dev::IOpenLoopControl*>     iopl;
        std::map<int, yarp::dev::PolyDriver*>           dd;
        
        bool openDrivers(int bodyPart);
        
    public:
        // *** CONSTRUCTORS ***
        yarpWholeBodyActuators(const char* _name, const char* _robotName, const std::vector<std::string> &_bodyPartNames);
        
        virtual bool init();
        virtual bool close();
        virtual int getDoFs(){ return dof; }
        virtual bool removeJoint(const wbi::LocalId &j);
        virtual bool addJoint(const wbi::LocalId &j);
        virtual int addJoints(const wbi::LocalIdList &j);
        virtual wbi::LocalIdList getJointList(){ return jointIdList; }
        
        virtual bool setControlMode(int controlMode, int joint=-1);
        virtual bool setTorqueRef(double *taud, int joint=-1);
        virtual bool setPosRef(double *qd, int joint=-1);
        virtual bool setVelRef(double *dqd, int joint=-1);
        virtual bool setPwmRef(double *pwmd, int joint=-1);
        virtual bool setReferenceSpeed(double *rspd, int joint=-1);
    };
    
    /** Thread that estimates the state of the iCub robot. */
    class icubWholeBodyEstimator: public yarp::os::RateThread
    {
    protected:
        yarpWholeBodySensors        *sensors;
        double                      estWind;        // time window for the estimation
        
        iCub::ctrl::AWLinEstimator  *dqFilt;        // joint velocity filter
        iCub::ctrl::AWQuadEstimator *d2qFilt;       // joint acceleration filter
        int dqFiltWL, d2qFiltWL;                    // window lengths of adaptive window filters
        double dqFiltTh, d2qFiltTh;                 // threshold of adaptive window filters
        
        yarp::sig::Vector lastQ, q, qStamps;        // last joint position estimation
        yarp::sig::Vector lastDq;                   // last joint velocity estimation
        yarp::sig::Vector lastD2q;                  // last joint acceleration estimation
        
        yarp::os::Semaphore         mutex;          // mutex for access to class global variables
        
        /** Copy the content of vector src into vector dest. */
        bool copyVector(const yarp::sig::Vector &src, double *dest);
        /** Take the mutex and copy the content of src into dest. */
        bool lockAndCopyVector(const yarp::sig::Vector &src, double *dest);
        /* Resize all vectors using current number of DoFs. */
        void resizeAll(int n);
        void lockAndResizeAll(int n);
        
    public:
        icubWholeBodyEstimator(int _period, yarpWholeBodySensors *_sensors);
        
        /** Set the parameters of the adaptive window filter used for velocity estimation. */
        void setVelFiltParams(int windowLength, double threshold);
        /** Set the parameters of the adaptive window filter used for acceleration estimation. */
        void setAccFiltParams(int windowLength, double threshold);

        bool threadInit();
        void run();
        void threadRelease();
        
        virtual bool removeJoint(const wbi::LocalId &j);
        virtual bool addJoint(const wbi::LocalId &j);
        virtual int addJoints(const wbi::LocalIdList &j);
        
        bool getQ(double *q);
        bool getDq(double *dq);
        bool getD2q(double *d2q);
    };
    
    
    
    /**
     * Class to access the estimates of the states of a robot with a yarp interface.
     */
    class icubWholeBodyStates : public wbi::iWholeBodyStates
    {
    protected:
        yarpWholeBodySensors        *sensors;       // interface to access the robot sensors
        icubWholeBodyEstimator      *estimator;     // estimation thread
        double                      estWind;        // time window for the estimation
        
    public:
        // *** CONSTRUCTORS ***
        icubWholeBodyStates(const char* _name, const char* _robotName, double estimationTimeWindow);
        
        virtual bool init();
        virtual bool close();
        virtual int getDoFs(){                              return sensors->getDoFs(); }
        virtual bool removeJoint(const wbi::LocalId &j){    return estimator->removeJoint(j); }
        virtual bool addJoint(const wbi::LocalId &j){       return estimator->addJoint(j); }
        virtual int addJoints(const wbi::LocalIdList &j){   return estimator->addJoints(j); }
        virtual wbi::LocalIdList getJointList(){            return sensors->getJointList(); }
        
        virtual bool getQ(double *q, double time=-1.0, bool wait=false);
        virtual bool getDq(double *dq, double time=-1.0, bool wait=false);
        virtual bool getDqMotors(double *dqM, double time=-1.0, bool wait=false);
        virtual bool getD2q(double *d2q, double time=-1.0, bool wait=false);
        virtual bool getPwm(double *pwm, double time=-1.0, bool wait=false);
        virtual bool getInertial(double *inertial, double time=-1.0, bool wait=false);
        virtual bool getFTsensors(double *ftSens, double time=-1.0, bool wait=false);
        virtual bool getTorques(double *tau, double time=-1.0, bool wait=false);
    };
    
    
    /**
     * Interface to the kinematic/dynamic model of the robot.
     */
    class icubWholeBodyModel: public wbi::iWholeBodyModel
    {
    protected:
        wbi::LocalIdList     jointIdList;
    public:
        virtual bool init();
        virtual bool close();
        virtual int getDoFs();
        virtual bool removeJoint(const wbi::LocalId &j);
        virtual bool addJoint(const wbi::LocalId &j);
        virtual int addJoints(const wbi::LocalIdList &j);
        virtual wbi::LocalIdList getJointList(){ return jointIdList; }
        
        virtual bool getJointLimits(double *qMin, double *qMax, int joint=-1);
        
        /** Compute rototranslation matrix from root reference frame to reference frame associated to the specified link.
         * @param q Joint angles
         * @param xBase Pose of the robot base, 3 values for position and 4 values for orientation
         * @param linkId Id of the link that is the target of the rototranslation
         * @param H Output 4x4 rototranslation matrix (stored by rows)
         * @return True if the operation succeeded, false otherwise (invalid input parameters) */
        virtual bool computeH(double *q, double *xBase, int linkId, double *H);
        
        /** Compute the Jacobian of the specified point of the robot.
         * @param q Joint angles
         * @param xBase Pose of the robot base, 3 values for position and 4 values for orientation
         * @param linkId Id of the link
         * @param J Output 6xN Jacobian matrix (stored by rows), where N=number of joints
         * @param pos 3d position of the point expressed w.r.t the link reference frame
         * @return True if the operation succeeded, false otherwise (invalid input parameters) */
        virtual bool computeJacobian(double *q, double *xBase, int linkId, double *J, double *pos=0);
        
        /** Given a point on the robot, compute the product between the time derivative of its 
         * Jacobian and the joint velocity vector.
         * @param q Joint angles
         * @param xBase Pose of the robot base, 3 values for position and 4 values for orientation
         * @param dq Joint velocities
         * @param linkId Id of the link
         * @param dJdq Output 6-dim vector containing the product dJ*dq 
         * @param pos 3d position of the point expressed w.r.t the link reference frame
         * @return True if the operation succeeded, false otherwise (invalid input parameters) */
        virtual bool computeDJdq(double *q, double *xB, double *dq, double *dxB, int linkId, double *dJdq, double *pos=0);
        
        /** Compute the forward kinematics of the specified joint.
         * @param q Joint angles
         * @param xB Pose of the robot base, 3 values for position and 4 values for orientation
         * @param linkId Id of the link
         * @param x Output 7-dim pose vector (3 for pos, 4 for angle-axis orientation)
         * @return True if operation succeeded, false otherwise */
        virtual bool forwardKinematics(double *q, double *xB, int linkId, double *x);
        
        /** Compute the inverse dynamics.
         * @param q Joint angles
         * @param xB Pose of the robot base, 3 values for position and 4 values for orientation
         * @param dq Joint velocities
         * @param dxB Velocity of the robot base, 3 values for linear velocity and 3 values for angular velocity
         * @param ddq Joint accelerations
         * @param ddxB Acceleration of the robot base, 3 values for linear acceleration and 3 values for angular acceleration
         * @param tau Output joint torques
         * @return True if operation succeeded, false otherwise */
        virtual bool inverseDynamics(double *q, double *xB, double *dq, double *dxB, double *ddq, double *ddxB, double *tau);
        
        /** Compute the direct dynamics.
         * @param q Joint angles
         * @param xB Pose of the robot base, 3 values for position and 4 values for orientation
         * @param dq Joint velocities
         * @param dxB Velocity of the robot base, 3 values for linear velocity and 3 values for angular velocity
         * @param M Output NxN mass matrix, with N=number of joints
         * @param h Output N-dim vector containing all generalized bias forces (gravity+Coriolis+centrifugal) 
         * @return True if operation succeeded, false otherwise */
        virtual bool directDynamics(double *q, double *xB, double *dq, double *dxB, double *M, double *h);
    };
    
    
    /**
     * Class to communicate with iCub
     */
    class icubWholeBodyInterface : public wbi::wholeBodyInterface
    {
    protected:
        icubWholeBodyStates     *stateInt;
        yarpWholeBodyActuators  *actuatorInt;
        icubWholeBodyModel      *modelInt;
        
    public:
        // *** CONSTRUCTORS ***
        icubWholeBodyInterface(const char* _name, const char* _robotName);
        
        virtual bool init();
        virtual bool close();
        virtual int getDoFs();
        virtual bool removeJoint(const wbi::LocalId &j);
        virtual bool addJoint(const wbi::LocalId &j);
        virtual int addJoints(const wbi::LocalIdList &j);
        virtual wbi::LocalIdList getJointList(){ return actuatorInt->getJointList(); }
   
        // ACTUATORS
        virtual bool setControlMode(int controlMode, int joint=-1){ return actuatorInt->setControlMode(controlMode, joint);}
        virtual bool setTorqueRef(double *taud, int joint=-1){      return actuatorInt->setTorqueRef(taud, joint);}
        virtual bool setPosRef(double *qd, int joint=-1){           return actuatorInt->setPosRef(qd, joint);}
        virtual bool setVelRef(double *dqd, int joint=-1){          return actuatorInt->setVelRef(dqd, joint);}
        virtual bool setPwmRef(double *pwmd, int joint=-1){         return actuatorInt->setPwmRef(pwmd, joint);}
        virtual bool setReferenceSpeed(double *rspd, int joint=-1){ return actuatorInt->setReferenceSpeed(rspd, joint);}
        
        // STATES
        virtual bool getQ(double *q, double time=-1.0, bool wait=false){    return stateInt->getQ(q, time, wait); }
        virtual bool getDq(double *dq, double time=-1.0, bool wait=false){  return stateInt->getDq(dq, time, wait); }
        virtual bool getDqMotors(double *dqM, double time=-1.0, bool wait=false){ return stateInt->getDqMotors(dqM, time, wait); }
        virtual bool getD2q(double *d2q, double time=-1.0, bool wait=false){ return stateInt->getD2q(d2q, time, wait); }
        virtual bool getPwm(double *pwm, double time=-1.0, bool wait=false){ return stateInt->getPwm(pwm, time, wait); }
        virtual bool getInertial(double *inertial, double time=-1.0, bool wait=false){ return stateInt->getInertial(inertial, time, wait); }
        virtual bool getFTsensors(double *ftSens, double time=-1.0, bool wait=false){ return stateInt->getFTsensors(ftSens, time, wait); }
        virtual bool getTorques(double *tau, double time=-1.0, bool wait=false){ return stateInt->getTorques(tau, time, wait); }
        
        // MODEL
        virtual bool getJointLimits(double *qMin, double *qMax, int joint=-1)
        { return modelInt->getJointLimits(qMin, qMax, joint); }
        virtual bool computeH(double *q, double *xB, int linkId, double *H)
        { return modelInt->computeH(q, xB, linkId, H); }
        virtual bool computeJacobian(double *q, double *xB, int linkId, double *J, double *pos=0)
        { return modelInt->computeJacobian(q, xB, linkId, J, pos); }
        virtual bool computeDJdq(double *q, double *xB, double *dq, double *dxB, int linkId, double *dJdq, double *pos=0)
        { return modelInt->computeDJdq(q, xB, dq, dxB, linkId, dJdq, pos); }
        virtual bool forwardKinematics(double *q, double *xB, int linkId, double *x)
        { return modelInt->forwardKinematics(q, xB, linkId, x); }
        virtual bool inverseDynamics(double *q, double *xB, double *dq, double *dxB, double *ddq, double *ddxB, double *tau)
        { return modelInt->inverseDynamics(q, xB, dq, dxB, ddq, ddxB, tau); }
        virtual bool directDynamics(double *q, double *xB, double *dq, double *dxB, double *M, double *h)
        { return modelInt->directDynamics(q, xB, dq, dxB, M, h); }
    };
    
    
    
} // end namespace

#endif

