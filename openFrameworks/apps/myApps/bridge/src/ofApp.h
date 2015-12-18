#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxBox2d.h"
#include "ofxNetwork.h"
#include "ofxBlur.h"

// ------------------------------------------------- a simple extended box2d circle
class CustomParticle : public ofxBox2dCircle {
    
public:
    CustomParticle() {
    }
    ofColor color;
    void draw() {
        float radius = getRadius();
        
        glPushMatrix();
        glTranslatef(getPosition().x, getPosition().y, 0);
        
        ofSetColor(color);
        ofFill();
        ofCircle(0, 0, radius);
        glPopMatrix();
        
    }
};


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    
    ofxBox2d                                box2d;			  //	the box2d world
    ofPolyline                              drawing;		  //	we draw with this first
    ofxBox2dEdge                            edgeLine;		  //	the box2d edge/line shape (min 2 points)
    vector		<ofPtr<ofxBox2dCircle> >	circles;		  //	default box2d circles
    vector		<ofPtr<ofxBox2dRect> >		boxes;			  //	defalut box2d rects
    vector		<ofPtr<CustomParticle> >    customParticles;  //	this is a custom particle the extends a cirlce
    
    ofFbo my_buffer;
    ofxBlur blur;
    
    float                                   px, py;
    ofxLeapMotion leap;
    vector <ofxLeapMotionSimpleHand> simpleHands;
    vector <int> fingersFound;

    ofxUDPManager udpConnection;
    
};