//********************************************************
//           by yang chao (wantnon) 
//			 2013-10-20
//			 blog: http://350479720.qzone.qq.com
//********************************************************

#ifndef HelloWorld_SceneNode_h
#define HelloWorld_SceneNode_h
#include "cocos2d.h"


#include "indexVBO.h"
using namespace cocos2d;

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
	CCPoint touchPos;
	bool touchValid;
	CindexVBO *_indexVBO;
	CCTexture2D *_texture ;
	
	GLuint hFBO;

	float texWidth,texHeight;
	float step_s,step_t;
    GLuint texSource;
	GLuint texDest;
	GLuint texTemp;

	
	GLint uniLoc_texSource;
	GLint uniLoc_texDest;
	GLint uniLoc_texTemp;
	GLint uniLoc_step_s;
	GLint uniLoc_step_t;
	GLint uniLoc_touchPos;
	GLint uniLoc_touchValid;


};
#endif
