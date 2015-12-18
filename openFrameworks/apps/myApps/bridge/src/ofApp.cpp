#include "ofApp.h"

int lastShapeAddedTime = 0;
int shapeAddDelay = 10; // milliseconds
int nowTime = 0;
int lastPixelGrab = 0;
int pixelGrabDelay = 100;
int lastTransmit = 0;
int transmitDelay = 300;
int eraseDelay = 1000;
int nextEraserDelay = 1;
int lastEraserAddTime = 0;
ofFloatPixels pix;
ofColor colortouse;
float allottedWidth;
float blockMargin = 0;
float blockWidth;
float blockHeight;
float topPadding;
string message = "";
string last_message = "";
string last_message_message = "";

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    ofEnableAlphaBlending();
    
    // Framebuffer so we can look at output without bits that are stacked on top
    my_buffer.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
    my_buffer.begin();
    ofClear(0,0,0,0);
    my_buffer.end();
    
    leap.open();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    ofSetVerticalSync(true);
    ofBackgroundHex(0x000000);
    
    box2d.init();
    box2d.setGravity(0, 200);
    box2d.setFPS(60.0);
    udpConnection.Create();
    // Modify this to suit your needs
    udpConnection.Connect("IPADDRESS OF BRIDGE SERVER RUNNING PYTON SCRIPT",65000);
    udpConnection.SetNonBlocking(true);
    
    allottedWidth = ofGetWidth() / 188.0;
    blockWidth = allottedWidth - blockMargin;
    blockHeight = blockWidth * 5.0;
    topPadding = 4.0/5.0 * ofGetHeight();
    
    // there are a 6 ways to control what's going on with ofxBlur
    // 4 parameters are in the setup() function:
    // - radius corresponds to how many points you want to use when blurring
    //   this usually corresponds to how big you want your blur to be
    //   so a 10 pixel radius means you want 10 samples to have perfect blur
    //   if you set this too high, the blur will start to get slow
    // - shape corresponds to the variance of the gaussian function used for
    //   the blur kernel. a larger variance means you will have more like a
    //   box blur, a smaller variance means it will be more "gaussian" blur
    //   this has no effect on the speed of the blur
    // - passes corresponds to how many downsampling blur passes are used
    //   setting this number higher will give more of a "fog" or "bloom" look
    //   if you set this too high, the blur will start to get slow
    // - downsample (not shown here, defaults to .5) corresponds to the scale
    //   factor when downsampling when passes is greater than 1
    //   this has no effect on the speed of the blur
    int radius = 20;
    float shape = .2;
    int passes = 1;
    int downsample = .5;
    blur.setup(ofGetWidth(),ofGetHeight(), radius, shape, passes, downsample);
    
}

//--------------------------------------------------------------
void ofApp::update(){

    fingersFound.clear();
    vector <Hand> hands = leap.getLeapHands();
    
        nowTime = ofGetElapsedTimeMillis();
        
        if((nowTime > lastShapeAddedTime + shapeAddDelay) && leap.isFrameNew() && hands.size() ){
            
            leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
            leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
            leap.setMappingZ(-150, 150, -200, 200);
            
            for(int i = 0; i < hands.size(); i++){
                // just the ones sticking out thank you
                FingerList fingers = hands[i].fingers().extended();
                
                for(int j = 0; j < fingers.count(); j++){
                    //int id = hands[i].fingers[j].id;
                    ofPoint pt;
                    
                    const Finger & finger = fingers[j];
                    //here we convert the Leap point to an ofPoint - with mapping of coordinates
                    //if you just want the raw point - use ofxLeapMotion::getofPoint
                    pt = leap.getMappedofPoint( finger.tipPosition() );
                    
                    //store fingers seen this frame for drawing
                    fingersFound.push_back(finger.id());
                    
                    float r = 20.0;

                    ofFill();
                    
                    float relativeY = (ofGetHeight()/2 - pt.y)/ofGetHeight();
                    if(relativeY > 1){
                        relativeY = 1;
                    }
                    if(relativeY < 0){
                        relativeY = 0;
                    }
                    
                    // choose a hue based on height of each finger
                    colortouse = ofColor::fromHsb((int)(relativeY*255*1.2)%255, 150, 200);
                    
                    customParticles.push_back(ofPtr<CustomParticle>(new CustomParticle));
                    CustomParticle * p = customParticles.back().get();
                    p->setPhysics(.5, 0, 0);
                    p->setup(box2d.getWorld(), ofGetWidth()/2 + pt.x, ofGetHeight()/2 - pt.y, r);
                    p->color = colortouse;
                    
                    lastShapeAddedTime = nowTime;
                }
            }
            
        }
        
        // when no hands are present, drop black circles down to gradually fade it back to black
        if(hands.size() == 0 && nowTime - lastShapeAddedTime > eraseDelay && nowTime - lastEraserAddTime > nextEraserDelay){
            
            float r = 1000;
            customParticles.push_back(ofPtr<CustomParticle>(new CustomParticle));
            CustomParticle * p = customParticles.back().get();
            p->setPhysics(1, 0, 0);
            p->setup(box2d.getWorld(), ofRandom(0,ofGetWidth()), 0, r);
            p->color = ofColor::fromHsb(0,0,0);
            p->color.a = 1;
            
            ofLog(OF_LOG_NOTICE, "Added");
            
            lastEraserAddTime = nowTime;
 
    }
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();
    
    box2d.update();
    // remove shapes offscreen
    ofRemove(boxes, ofxBox2dBaseShape::shouldRemoveOffScreen);
    ofRemove(circles, ofxBox2dBaseShape::shouldRemoveOffScreen);
    ofRemove(customParticles, ofxBox2dBaseShape::shouldRemoveOffScreen);
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    my_buffer.begin();
    
        ofSetHexColor(0xFFFFFF);

        blur.begin();
        for(int i=0; i<customParticles.size(); i++) {
            customParticles[i].get()->draw();
        }
        blur.end();
    
        blur.draw();
    
    my_buffer.end();
    

    
    // output the buffer to the screen
    //my_buffer.draw(0,0, ofGetWidth(), ofGetHeight());
    ofSetHexColor(0xFFFFFF);
    my_buffer.draw(0,0);
    
    // Draw a grid of bridge squares over the top
    if(nowTime > lastPixelGrab + pixelGrabDelay){
        // first read the framebuffer to pixels
        my_buffer.readToPixels(pix);

        glReadPixels(0, 0, ofGetWidth(), ofGetHeight(), GL_RGB, GL_UNSIGNED_BYTE, pix.getPixels());
        lastPixelGrab = nowTime;
    }
    
    for(int i=0; i<=188; i++) {
        float blockX = (i * allottedWidth);
        float blockY = topPadding;
        float blockY2 = blockY + blockHeight + blockMargin;
        float blockCenterX = blockX + allottedWidth / 2.0;
        float blockCenterY = blockY + (blockHeight / 2.0);
        float blockCenterY2 = blockY2 + blockHeight / 2.0;
        
        ofColor color;
        
        // get the color of the underlying center pixel
        ofColor topColor = pix.getColor(blockCenterX, blockCenterY); // top
        ofColor bottomColor = pix.getColor(blockCenterX, blockCenterY2); // bottom
        
        if(message != ""){
            message += ",";
        }
        message += ofToString(i) + "|1|" + ofToString(topColor.r/255.0) + "|" + ofToString(topColor.g/255.0) + "|" + ofToString(topColor.b/255.0);
        message += "," + ofToString(i) + "|0|" + ofToString(topColor.r/255.0) + "|" + ofToString(topColor.g/255.0) + "|" + ofToString(topColor.b/255.0);
        
        // when the message reaches a good size, send it.
        if(message.size()>2500){
            // Comment out for testing
            udpConnection.Send(message.c_str(), message.length());
            
            lastTransmit = nowTime;

            last_message_message = last_message;
            last_message = message;
            message = "";
            
        }
        
        // draw the bridge panels representation rectangles with the colors chosen above
        ofSetColor(topColor);
        ofDrawRectangle(blockX, blockY, blockWidth, blockHeight);
        ofSetColor(bottomColor);
        ofDrawRectangle(blockX, blockY2, blockWidth, blockHeight);
        
        // draw a dot in the center of each bridge panel representation
        ofSetHexColor(0xFFFFFF);
        ofDrawRectangle(blockCenterX, blockCenterY, 2, 2);
        ofDrawRectangle(blockCenterX, blockCenterY2, 2, 2);
        
        // draw black boxes over
        ofSetColor(255, 255, 255, 1);
        ofDrawRectangle(0, 0, ofGetWidth(), topPadding);
        ofDrawRectangle(0, blockY2 + blockHeight, ofGetWidth(), ofGetHeight() - (blockY2 + blockHeight));
        
    }

    string info = "";
    info += "Leap Connected? " + ofToString(leap.isConnected()) + "\n";
    info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
    info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
    info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
    
    ofSetHexColor(0xFFFFFF);
    ofDrawBitmapString(info, 30, 30);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

    // remove shapes
    customParticles.clear();
 
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
