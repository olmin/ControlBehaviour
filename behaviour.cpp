/* 
 * File:   behaviour.cpp
 * Author: olmin
 * 
 * Created on 13 de septiembre de 2011, 13:15
 */

#include "behaviour.h"
#include "vec3.h"
#include "MySQLConnector.h"

 /*! Deep copy the Behaviour data. */
  void Behaviour::copyTo(Behaviour& rhs) const{
      memcpy(&rhs, this, sizeof(Behaviour));
  }
  
  /*! Initializes de Behaviour class*/
  void Behaviour::initialize() {
      printf ("----Initialize------\n");
      int pos;
      int behaviourDuration;
      
      cout<<endl<<"             Choose the behaviour to be done:"<<endl;
      for(int i=0;i<NumActions;i++){
          cout<<i<<"- "<<listActions[i]<<endl;
      }
      cout<<endl<<"Insert number of the behaviour: ";
      cin>>pos;          
      
      cout<<"How many seconds is going to last the behaviour?: ";
      cin>>behaviourDuration;    
    
      this->name=listActions[pos];
      this->specificDuration=behaviourDuration;
      this->count=0;
      printf ("----End Initialize------\n");
  }
  
  /*! Start the behaviour*/
  void Behaviour::start() {
      printf ("----Start------\n");
      //initial time of the behaviour
      this->dateIni= time(NULL);      
  }
  
  
  /*! Saves the actual skeleton detected into the Behaviour data*/
  bool Behaviour::saveData(ntk::Skeleton* skeleton) {
   
    if (this->specificDuration*FPS == this->count) {
        cout << "Size of frames exceed" << endl;
        return true;//finished
    } else {
        
        Point3f head = skeleton->getJoint(skeleton->NTK_SKEL_HEAD);
        Point3f neck = skeleton->getJoint(skeleton->NTK_SKEL_NECK);
        Point3f torso = skeleton->getJoint(skeleton->NTK_SKEL_TORSO);

        Point3f left_shoulder = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_SHOULDER);
        Point3f left_elbow = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_ELBOW);
        Point3f left_hand = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_HAND);
        Point3f left_knee = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_KNEE);
        Point3f left_foot = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_FOOT);
        Point3f left_hip = skeleton->getJoint(skeleton->NTK_SKEL_LEFT_HIP);

        Point3f right_shoulder = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_SHOULDER);
        Point3f right_elbow = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_ELBOW);
        Point3f right_hand = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_HAND);
        Point3f right_knee = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_KNEE);
        Point3f right_foot = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_FOOT);
        Point3f right_hip = skeleton->getJoint(skeleton->NTK_SKEL_RIGHT_HIP);


        //initializing the skeleton we are going to save
        SimpleSkeleton simpleSkeleton = SimpleSkeleton();

        simpleSkeleton.timestamp = time(NULL);
        simpleSkeleton.pos3D = torso; // the 3d position is taken from the central point of the person

        //now we calculate the angles of the joints
        simpleSkeleton.angles[0] = calcAngle(head, neck, left_shoulder, right_shoulder); //angle head-torso     

        simpleSkeleton.angles[1] = calcAngle(left_elbow, left_shoulder, left_shoulder, right_shoulder); //angle left elbow-shoulders
        simpleSkeleton.angles[2] = calcAngle(left_hand, left_elbow, left_elbow, left_shoulder); //angle left hand-elbow
        simpleSkeleton.angles[3] = calcAngle(left_knee, left_hip, left_hip, right_hip); //angle left knee-hips
        simpleSkeleton.angles[4] = calcAngle(left_foot, left_knee, left_knee, left_hip); //angle left ankle-knee

        simpleSkeleton.angles[5] = calcAngle(right_elbow, right_shoulder, right_shoulder, left_shoulder); //angle right elbow-shoulders
        simpleSkeleton.angles[6] = calcAngle(right_hand, right_elbow, right_elbow, right_shoulder); //angle right hand-elbow
        simpleSkeleton.angles[7] = calcAngle(right_knee, right_hip, right_hip, left_hip); //angle right knee-hips
        simpleSkeleton.angles[8] = calcAngle(right_foot, right_knee, right_knee, right_hip); //angle right ankle-knee


        simpleSkeleton.angles[9] = calcAngle(left_shoulder, right_shoulder, torso, neck); //angle of rotation of the shoulders
        simpleSkeleton.angles[10] = calcAngle(left_hip, right_hip, torso, neck); //angle of rotation of the hip

        //       
        //    printf ("----Left arm: %f------\n", simpleSkeleton.angles[2]);
        //     printf ("----Right arm: %f------\n", simpleSkeleton.angles[6]);

        this->positions[this->count++] = simpleSkeleton;
        
        return false;//not full yet
    }
//    printf("----End Save Data Skeleton, Pos 3d: %f, %f, %f------\n",
//            this->positions[this->count - 1].pos3D.x,
//            this->positions[this->count - 1].pos3D.y,
//            this->positions[this->count - 1].pos3D.z
//            );
  }
  
  /*! Ends the behaviour and saves all its data into the database*/
  void Behaviour::finish() {      
      this->dateEnd=time(NULL);
      saveInDB();         
  }
  
  float Behaviour::calcAngle(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End){
      
      vec3 v1= vec3(p1End.x -p1Ini.x, p1End.y - p1Ini.y, p1End.z - p1Ini.z);
      vec3 v2= vec3(p2End.x -p2Ini.x, p2End.y - p2Ini.y, p2End.z - p2Ini.z);
      
      float angle=angle2vectors(v1,v2)*180 / 3.1415926;//radians to degrees
          
      return angle; 
  }
  
    float Behaviour::calcAngleRef(cv::Point3f p1Ini, cv::Point3f p1End, 
                cv::Point3f p2Ini, cv::Point3f p2End){
      
      vec3 v1= vec3(p1End.x -p1Ini.x, p1End.y - p1Ini.y, p1End.z - p1Ini.z);
      vec3 v2= vec3(p2End.x -p2Ini.x, p2End.y - p2Ini.y, p2End.z - p2Ini.z);
      
      vec3 vRef= vec3(v2);//right_hip.x -left_hip.x, right_hip.y - left_hip.y, right_hip.z - left_hip.z);
      
      float angle=signed_angle(v1,v2, vRef)*180 / 3.1415926;//radians to degrees
          
      return angle; 
  }
    
   
  /*! Saves in the database the behaviour*/
  void Behaviour::saveInDB(){
      printf ("----Saving in DB------\n");
      MySQLConnector conn;
      conn.save(this);
      conn.closeConnection();
      //delete this;
      printf ("-----END Saving in DB-----\n");
  }