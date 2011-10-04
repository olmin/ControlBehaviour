/* 
 * File:   behaviour.h
 * Author: olmin
 *
 * Created on 13 de septiembre de 2011, 13:15
 */

#ifndef BEHAVIOUR_H
#define	BEHAVIOUR_H


#include <time.h>
#include <iostream>
#include <string>
#include <ntk/gesture/skeleton.h>

/*! Number of joints angles. */
#define Num_Angles 11
 /*! Frames per second */
#define FPS 30
/*!Number of Actions*/
#define NumActions 15

using namespace std;
using namespace cv;
using namespace ntk;

/*!List of actions that can be done*/
 static const string listActions[NumActions] =  {
    "nothing",
    "check watch",
    "cross arms",
    "scratch head",
    "sit down",
    "get up",
    "turn around",
    "walk",
    "wave",
    "punch",
    "kick",
    "point",
    "pick up",
    "throw (over head)",
    "throw (from bottom up)"
 };

  
class SimpleSkeleton{
public:
    SimpleSkeleton(){        
    }

     cv::Point3f pos3D;
     time_t timestamp;
     float angles[Num_Angles];
};

class Behaviour{
    
private:
   
    /*! Behaviour total duraion in seconds. */
   enum{BehaviourMaxDuration= 2*60 };//2 minutes

   /*!Specific duration of the behaviour*/
   int specificDuration;
          
public:
    string name;
    time_t dateIni;
    time_t dateEnd;
    int count;
    SimpleSkeleton positions[BehaviourMaxDuration*FPS];

  /*! Deep copy the Behaviour data. */
  void copyTo(Behaviour& rhs) const;
  
  /*! Initializes de Behaviour class*/
  void initialize() ;
  
  /*! To start the behaviour*/
  void start() ;
  
  /*! Saves the actual skeleton detected into the Behaviour data*/
  bool saveData(Skeleton* skeleton) ;
  
  /*! Ends the behaviour and saves all its data into the database*/
  void finish() ;
  
  /*!Returns the FPS rate*/
  int getFPS(){return FPS;};
  
  /*!Asks through shell to user the behaviour to be recorded*/
  void getBehaviourName();
  
private:
   float calcAngle(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End); 
   float calcAngleRef(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End);
   void saveInDB();
};

#endif	/* BEHAVIOUR_H */

