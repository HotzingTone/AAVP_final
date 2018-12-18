#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofScale(0.4);
    
    ofxMaxiSettings::setup(44100, 2, 512);
    ofSoundStreamSetup(2, 0, 44100, 512, 4);
    
    // X and Y of the screen centre
    cX = ofGetWidth()/2;
    cY = ofGetHeight()/2;
    
    // number of radial beams in each circle
    num = 144;
    
    // randomly pick 4 seeds for perlin noise
    sd1 = ofRandom(10000);
    sd2 = ofRandom(10000);
    sd3 = ofRandom(10000);
    sd4 = ofRandom(10000);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    /* this drives the rotation as well as the distance
     from each circle centre to the screen centre */
    rotate += 0.0015;
    
    // the first-order perlin noise output
    fw1 += pow(ofNoise(rotate+sd1),4)*0.01;
    fw2 += pow(ofNoise(rotate+sd2),4)*0.01;
    fw3 += pow(ofNoise(rotate+sd3),4)*0.01;
    fw4 += pow(ofNoise(rotate+sd4),4)*0.01;
    
    /* calculate the centre positions of the 4 circles,
     using the rotation and the second-order perlin noise
     to alter their positional relation */
    p1.x = cos(rotate)*ofNoise(fw1+sd1)*100+cX;
    p1.y = sin(rotate)*ofNoise(fw1+sd1)*100+cY;
    p2.x = cos(rotate/1.5+PI)*ofNoise(fw2+sd2)*100+cX;
    p2.y = sin(rotate/1.5+PI)*ofNoise(fw2+sd2)*100+cY;
    p3.x = cos(-rotate/2)*ofNoise(fw3+sd3)*100+cX;
    p3.y = sin(-rotate/2)*ofNoise(fw3+sd3)*100+cY;
    p4.x = cos(-rotate/2.5+PI)*ofNoise(fw4+sd4)*100+cX;
    p4.y = sin(-rotate/2.5+PI)*ofNoise(fw4+sd4)*100+cY;
    
    /* use the distances between the circle centres
     as the modulation source for sound */
    amtA = pow(1-p1.distance(p2)/200,8);
    amtB = pow(1-p3.distance(p4)/200,8);
    amtC = pow(1-p1.distance(p3)/200,0.5);
    amtD = pow(1-p1.distance(p4)/200,0.5);
    amtE = pow(1-p2.distance(p3)/200,0.25);
    amtF = pow(1-p2.distance(p4)/200,0.25);
    
    // the overall modulation amount
    AMT = (amtA+amtB)*(amtC+amtD+amtE+amtF)*10;
}

//--------------------------------------------------------------
void ofApp::draw(){

    // modulate the colours of the 4 circles
    c1.set(ofClamp(72*AMT,0,255), 0, 0);
    c2.set(ofClamp(16*AMT,0,255), ofClamp(24*AMT,0,255), 0);
    c3.set(0, 0, ofClamp(48+16*AMT,0,255));
    c4.set(0, ofClamp(16+16*AMT,0,255), 0);
    
    ofBackground(c3);
    
    // draw 2 circles in ADD blending mode
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    radial(p1,c1);
    radial(p2,c2);
    
    // add a circle with no blending
    ofDisableBlendMode();
    radial(p3,c3);
    
    // add the last circle on top in ADD blending mode
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    radial(p4,c4);
}

//--------------------------------------------------------------
void ofApp::radial(ofPoint p, ofColor c){
    
    // circle drawing function
    // draw with coloured radial beams
    ofPushMatrix();
    ofTranslate(p);
    ofSetColor(c);
    for (int i=0; i<num; i++){
        ofRotateDeg(360.0/num);
        ofDrawTriangle(25, 0, 1000, -5, 1000, 5);
        ofDrawTriangle(150, 0, 1000, -10, 1000, 10);
        ofDrawTriangle(275, 0, 1000, -15, 1000, 15);
    }
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels) {
    
    // oscillator outputs
    // A and B as modulators
    // C, D, E, F as carriers
    // LFO deals with tremolo
    float modA, modB;
    float outC, outD, outE, outF;
    float outLFO;
    
    // stereo output
    float L, R;
    
    // stereo pans
    float panC=0.1, panD=0.7, panE=0.4, panF=0.5;
    
    // base freqency
    float base=35;
    
    // freqencies of each oscillator,
    // according to their ratios to the base freqency
    float freqA=base, freqB=base*14;
    float freqC=base*14/3, freqD=base*7/2;
    float freqE=base*4/3, freqF=base*3/5;
    
    // FM depth
    float depth = 64;
    
    for (unsigned int i = 0; i < bufferSize; i++) {
        
        // oscillator A and B as the freqency modulators
        modA = oscA.triangle(freqA)*amtA*freqA;
        modB = oscB.sinewave(freqB)*amtB*freqB;
        
        // oscillator C, D, E, F as the carriers
        outC = oscC.sinewave(freqC+(modA*0.7+modB*0.7)*amtC*depth)*amtC*0.2;
        outD = oscD.sinewave(freqD+(modA*0.6+modB*0.8)*amtD*depth)*amtD*0.4;
        outE = oscE.sinewave(freqE+(modA*0.5+modB*0.9)*amtE*depth)*amtE*0.5;
        outF = oscF.sinewave(freqF+(modA*0.9+modB*0.5)*amtF*depth)*amtF*0.8;
        
        // the speed of tremolo follows the overall modulation amount
        outLFO = lfo.sinebuf(3*AMT);
        
        // stereo pan
        L = outC*(1-panC)+outD*(1-panD)+outE*(1-panE)+outF*(1-panF);
        R = outC*panC+outD*panD+outE*panE+outF*panF;
        
        // tremolo
        L = L*(0.7+outLFO*0.12);
        R = R*(0.7+(1-outLFO)*0.12);
        
        // compress the signal before outputting
        output[i*nChannels] = cpL.compressor(L,0.25,0.8,0.1,0.9999);
        output[i*nChannels+1] = cpR.compressor(R,0.25,0.8,0.1,0.9999);
    }
}
