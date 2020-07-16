#include "ofApp.h"
int frame_count = 0;
int output_frame_count = 0;
int output_sequence_length = 600;
ofFbo fbo, output_fbo;
vector<ofVec2f> a, v, p, p_original;
int num_particles = 40;
ofxBlur blur;
int fbo_size = 1920;

adImageSequenceRecorder recorder;

vector<ofVec2f> attractors;
int num_attractors = 25;

void animateStartingPoints() {
	p.clear();
	v.clear();
	a.clear();


	float x_center = fbo_size / 2.;
	float y_center = fbo_size / 2.;
	

	float loop_pct = output_frame_count / float(output_sequence_length);

	float radius = ofMap(sin(loop_pct * TWO_PI), -1, 1, .2*fbo_size, .45*fbo_size);
	float angle_offset = 3. * loop_pct * TWO_PI;
	float x_offset = 2. * loop_pct * ofGetWidth();

	for (int i = 0; i < num_particles; i++) {

		float pct = i / float(num_particles);
		float angle = pct * TWO_PI + angle_offset;
		float x = cos(angle*1.);
		float y = sin(angle*1.);

		x *= radius;
		y *= radius;
		
		x += x_offset + x_center;
		y += y_center;

		x = fmod(x, fbo_size);

		ofVec2f pos = ofVec2f(x, y);
		p.push_back(pos);
		//p_original.push_back(pos);

		//ofVec2f v_i = raw_pos * 1.4;

		//ofVec2f p1 = ofVec2f(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
		//p.push_back(p1);
		//p_original.push_back(p1);

		ofVec2f v_i = ofVec2f(0, 0);


		v.push_back(v_i);
		a.push_back(ofVec2f(0, 0));
	}

}

ofVec2f getAttractorAcceleration(ofVec2f point, ofVec2f attractor) {
	float influence_radius = 100;
	ofVec2f difference = attractor - point;
	float attractorScale = ofClamp((influence_radius - difference.length()) / influence_radius, 0., 1.);
	//difference = difference.normalize();
	difference *= 0.004 * attractorScale / influence_radius;

	return difference;
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	//ofSetFrameRate(100);

	fbo.allocate(fbo_size, fbo_size, GL_RGBA, 4);
	output_fbo.allocate(fbo_size, fbo_size, GL_RGBA, 4);
	blur.setup(ofGetWidth(), ofGetHeight(), 32, 0.2, 1, 0.5);
	blur.setScale(0.);

	for (int i = 0; i < num_attractors; i++) {
		ofVec2f a_temp = ofVec2f(ofRandom(fbo_size), ofRandom(fbo_size));
		attractors.push_back(a_temp);
	}

	recorder.setImage(&output_fbo);
	recorder.setLength(output_sequence_length);

	animateStartingPoints();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo.begin();

	ofDisableBlendMode();
	
	if (frame_count == 0) {
		ofBackground(0);
	}
	else {
		//fbo.draw(0, 0);
	}

	ofEnableBlendMode(OF_BLENDMODE_SCREEN);

	float velocity_limit = 2.;
	float a_level = 0.1;
	float circle_width = ofClamp(2. - frame_count / (fbo_size * 2.), 0, 1.);
	float boundary_acceleration_limit = 0.005;

	for (int i = 0; i < num_particles; i++) {

		ofVec2f boundary_acceleration = p[i] - ofVec2f(fbo_size/2, fbo_size/2);
		float bounds_x_accel = boundary_acceleration.x;
		bounds_x_accel = bounds_x_accel * 2.;
		bounds_x_accel /= fbo_size;
		bounds_x_accel = abs(bounds_x_accel);
		bounds_x_accel = ofMap(bounds_x_accel, 0, 1, -5, 1.1);
		bounds_x_accel = ofClamp(bounds_x_accel, 0, 1);
		if (boundary_acceleration.x > 0) {
			bounds_x_accel *= -1;
		}

		float bounds_y_accel = boundary_acceleration.y;
		bounds_y_accel = bounds_y_accel * 2.;
		bounds_y_accel /= fbo_size;
		bounds_y_accel = abs(bounds_y_accel);
		bounds_y_accel = ofMap(bounds_y_accel, 0, 1, -5, 1.1);
		bounds_y_accel = ofClamp(bounds_y_accel, 0, 1);
		if (boundary_acceleration.y > 0) {
			bounds_y_accel *= -1;
		}

		boundary_acceleration = ofVec2f(bounds_x_accel, bounds_y_accel);
		
		ofVec2f p_i = p[i];
		ofVec2f v_i = v[i];
		ofVec2f a_i = a[i];

		a_i.x = ofSignedNoise(p_i.x / (80. * (i % 2 + 1)) ) * a_level;
		a_i.y = ofSignedNoise(10000 + p_i.y / (80. * (i % 4 + 1)) ) * a_level;

		//float a_angle = ofSignedNoise(p_i.x / 150.) * TWO_PI;
		//float a_amplitude = ofSignedNoise(10000 + p_i.y / 150.) * a_level;

		//a_i.x = sin(a_angle) * a_amplitude;
		//a_i.y = cos(a_angle) * a_amplitude;

		// with attractors
		//a_i = ofVec2f(0, 0);
		//for (int i = 0; i < num_attractors; i++) {
		//	a_i += getAttractorAcceleration(p_i, attractors[i]);
		//}

		v_i += a_i + boundary_acceleration * boundary_acceleration_limit;
		//v_i += a_i;

		v_i.limited(velocity_limit);
		p_i += v_i;

		a[i] = a_i;
		v[i] = v_i;
		p[i] = p_i;

		//if (i / float(p.size()) < 0.5) {
		//	ofSetColor(35, 15, 5);
		//}
		//else {
		//	ofSetColor(5, 22, 35);
		//}

		float i_pct = i / float(num_particles);
		ofColor c = lerpColors(ofColor(35, 8, 5), ofColor(5, 22, 35), i_pct);
		ofSetColor(c);
		
		ofDrawCircle(p_i, circle_width);

		// draws recursive business in the middle
		int num_recursions = 10;
		float bounds = 0.2;
		float circle_width_recursize = circle_width;
		ofVec2f anchor = ofVec2f(fbo_size / 2, fbo_size * 0.3333);
		ofVec2f rec_scalar = ofVec2f(0.7);
		float rec_angle = 0;
		if (i % 2 == 0) {
			rec_scalar = ofVec2f(0.9);
		}
		if (i % 3 == 0) {
			rec_angle = 360 * 0.333333;
			anchor = ofVec2f(fbo_size / 2, fbo_size * 0.6666);
		}
		//rec_angle = (i % 8) * 45;

		if ( (p_i.x > fbo_size * bounds && p_i.x < fbo_size * (1.-bounds) && p_i.y > fbo_size * bounds && p_i.y < fbo_size * (1.-bounds))) {
			for (int j = 0; j < num_recursions; j++) {
				p_i = (p_i - anchor).rotate(rec_angle) * rec_scalar + anchor;
				if (abs(rec_scalar.x) > 1.) circle_width_recursize *= rec_scalar.x;
				//circle_width_recursize *= abs(rec_scalar.x);
				//circle_width_recursize *= 0.95;
				ofDrawCircle(p_i, circle_width_recursize);
			}
		}
	}
	
	//blur.setScale(ofClamp(0.3 - ofGetFrameNum() / 3000., 0., 0.15));
	
	fbo.end();


	

	if (circle_width == 0) {
		output_fbo.begin();
		fbo.draw(0, 0);

		blur.setScale(0.5);
		blur.begin();
		fbo.draw(0, 0);
		blur.end();

		ofEnableBlendMode(OF_BLENDMODE_SCREEN);
		blur.draw();
		ofDisableBlendMode();

		output_fbo.end();

		output_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	else {
		fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	


	ofSetColor(255);
	ofDrawBitmapString(ofGetFrameRate(), 10, 10);
	ofDrawBitmapString(frame_count, 10, 40);

	frame_count++;

	if (circle_width == 0 && recorder.isRecording ) {
		recorder.update();

		fbo.begin(); ofClear(0); fbo.end();

		//float sequence_pct = output_frame_count / float(output_sequence_length);
		//float y_offset = ofGetHeight() / float(output_sequence_length);

		/*for (int i = 0; i < p.size(); i++) {
			float i_pct = i / float(p.size());
			float i_angle = (i_pct + sequence_pct) * TWO_PI;

			ofVec2f new_p_original = p_original[i] + ofVec2f(0, y_offset);
			new_p_original.y = fmod(new_p_original.y, ofGetHeight());
			p[i] = new_p_original;
			p_original[i] = new_p_original;
			v[i] = ofVec2f(0, 0);
			a[i] = ofVec2f(0, 0);
		}*/

		frame_count = 0;
		output_frame_count++;

		animateStartingPoints();
	}
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'r') {
		/*for (int i = 0; i < p.size(); i++) {
			ofVec2f new_p = ofVec2f(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
			p[i] = new_p;
			p_original[i] = new_p;
			v[i] = ofVec2f(0, 0);
			a[i] = ofVec2f(0, 0);
		}*/
		output_frame_count = ofRandom(output_sequence_length);
		animateStartingPoints();
		frame_count = 0;
	}
	if (key == 'g') {
		fbo.begin(); ofClear(0); fbo.end();
		blur.begin(); ofClear(0); blur.end();

		for (int i = 0; i < p.size(); i++) {
			ofVec2f new_p_original = p_original[i] * 0.9999;
			p[i] = new_p_original;
			p_original[i] = new_p_original;
			v[i] = ofVec2f(0, 0);
			a[i] = ofVec2f(0, 0);
		}
		frame_count = 0;
	}
	if (key == 's') {
		recorder.startRecording();
		output_frame_count = 0;
	}
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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
