//********************************************************
//           by yang chao (wantnon) 
//			 2013-10-20
//			 blog: http://350479720.qzone.qq.com
//********************************************************

#ifndef HelloWorld_SceneNode_h
#define HelloWorld_SceneNode_h
#include "cocos2d.h"

#include<iostream>
#include<map>
using namespace std;

#include "indexVBO.h"
using namespace cocos2d;
//if program1 and program2 have a uniform variable with the same name, the two variable's uniform ID (GLint) may be different. 
//so safe way is let each shader program hold his own uniform IDs.
class CGLProgramWithMyUnifos{
public:
	map<string,GLint> myUnifoMap;
	CCGLProgram*program;
	CGLProgramWithMyUnifos(){
		program=NULL;
	}
	virtual ~CGLProgramWithMyUnifos(){
		program->release();
	}
};
class SceneNode : public CCLayer{
public:
	SceneNode() ;
	virtual ~SceneNode() ;
	bool initWithTexture(std::string textureName) ;
	void draw() ;
	//touch
	virtual void ccTouchesBegan(cocos2d::CCSet* touches , cocos2d::CCEvent* event);
	virtual void ccTouchesMoved(cocos2d::CCSet* touches , cocos2d::CCEvent* event);
	virtual void ccTouchesEnded(cocos2d::CCSet* touches , cocos2d::CCEvent* event);

private:
	CCPoint touchPos_winSpace;//origin is window's left up corner
	bool touchValid;
	CindexVBO *_indexVBO;
	CCTexture2D *_texture ;
	
	GLuint hFBO;

	CGLProgramWithMyUnifos program_updateRipple;
	CGLProgramWithMyUnifos program_renderRipple;

	float texWidth,texHeight;
	float step_s,step_t,step_scaleFactor;//step_s和step_t进行一定的缩放再传入shader，可以使水波起伏看起来更大些，因为跨度大些梯度就大些
    CCTexture2D* texSource;
	CCTexture2D* texDest;
	CCTexture2D* texTemp;






};
#endif
