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

using namespace std;
using namespace cv;
using namespace ntk;

/*! Number of joints angles. */
enum { Num_Angles = 11};

  
class SimpleSkeleton{
public:
    SimpleSkeleton(){        
    }

     cv::Point3f pos3D;
     time_t timestamp;
     float angles[Num_Angles];
};

class Behaviour{
    
    /*! Behaviour total duraion in seconds. */
    enum { BehaviourDuration = 2*60 };//2 minutes

    /*! Frequency of samples per second. */
    enum { FPS = 15 };//15 fps
          
public:
    string name;
    time_t dateIni;
    time_t dateEnd;
    int count;
    SimpleSkeleton positions[BehaviourDuration*FPS];

  /*! Deep copy the Behaviour data. */
  void copyTo(Behaviour& rhs) const;
  
  /*! Initializes de Behaviour class*/
  void initialize(const string& behaviourName) ;
  
  /*! Saves the actual skeleton detected into the Behaviour data*/
  void saveData(Skeleton* skeleton) ;
  
  /*! Ends the behaviour and saves all its data into the database*/
  void finish() ;
  
  /*!Returns the FPS rate*/
  int getFPS(){return FPS;};
  
private:
   float calcAngle(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End); 
   float calcAngleRef(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End);
   void saveInDB();
};

#endif	/* BEHAVIOUR_H */

