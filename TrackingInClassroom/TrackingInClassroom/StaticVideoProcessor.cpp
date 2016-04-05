#include "stdafx.h"
#include "VideoProcessor.h"

using namespace std;
using namespace cv;

StaticVideoProcessor::StaticVideoProcessor(){

}

StaticVideoProcessor::~StaticVideoProcessor(){
	
}

int StaticVideoProcessor::init(){
	return 0;
}

int StaticVideoProcessor::setParams(const char * path){
	return 0;
}

vector<Rect> StaticVideoProcessor::getTargets(Mat frame){
	vector<Rect> v;
	return v;
}