#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void audioOut(float * output, int bufferSize, int nChannels);
        void radial(ofPoint p, ofColor c);
    
    int cX, cY;
    int num;
    
    int sd1, sd2, sd3, sd4;
    float fw1, fw2, fw3, fw4;
    float rotate;
    
    ofPoint p1, p2, p3, p4;
    ofColor c1, c2, c3, c4;
    
    float amtA, amtB, amtC, amtD, amtE, amtF, AMT;
    maxiOsc oscA, oscB, oscC, oscD, oscE, oscF, lfo;
    maxiDyn cpL, cpR;
};
