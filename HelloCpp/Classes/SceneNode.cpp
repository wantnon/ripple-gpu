

#include "SceneNode.h"
#include <iostream>
#include <map>
#include "support/ccUtils.h"

using namespace cocos2d ;
using namespace std ;

SceneNode::SceneNode()
{
	texWidth=0;
	texHeight=0;
	step_s=0;
	step_t=0;

	texSource=0;
	texDest=0;
	texTemp=0;

	hFBO=0;

	touchPos_winSpace=CCPoint(0,0);
	touchValid=false;
}

SceneNode::~SceneNode()
{
	//?????

}


bool SceneNode::initWithTexture(std::string textureName) 
{
	//glClearColor(1,0,0,1);
	//create texture
    _texture = CCTextureCache::sharedTextureCache()->addImage(textureName.c_str()) ;//???_texture??retainCount?1
	CCSize texSize=_texture->getContentSize();
	texWidth=texSize.width;
	texHeight=texSize.height;
	step_s=1.0/texWidth;
	step_t=1.0/texHeight;
	//enable touch
	setTouchEnabled( true );
	//????texture
	{
		//
		/*
		glGenTextures(1,&texSource);
		glBindTexture(GL_TEXTURE_2D,texSource);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texWidth,texHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
		glBindTexture(GL_TEXTURE_2D,0);*/
		texSource=CCTextureCache::sharedTextureCache()->addImage("texSource_init.png")->getName();
		glBindTexture(GL_TEXTURE_2D,texSource);
		//ע�⣺���������ò���������Ҫ����������Ⱦfboʱ����ƫ��������
		//GL_NEAREST��GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //��ֹ�����ظ�
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//
		texDest=CCTextureCache::sharedTextureCache()->addImage("texDest_init.png")->getName();
		glBindTexture(GL_TEXTURE_2D,texDest);
		//GL_NEAREST��GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //��ֹ�����ظ�
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//
		texTemp=CCTextureCache::sharedTextureCache()->addImage("texTemp_init.png")->getName();
		glBindTexture(GL_TEXTURE_2D,texTemp);
		//GL_NEAREST��GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //��ֹ�����ظ�
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	}

	//????shader
	//updateRipple shader
	{
		GLchar * fragSource = (GLchar*) CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("updateRipple.fsh").c_str())->getCString();
		CCGLProgram* pProgram = new CCGLProgram();
		pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
		pProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		pProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		pProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
		pProgram->link();//link must after bindAttribute
		pProgram->updateUniforms();//????cocos2d-x????uniform????
		map<string,GLint> myUnifoMap;
		{
			myUnifoMap["texSource"] =glGetUniformLocation(pProgram->getProgram(),"texSource");
			myUnifoMap["texDest"] = glGetUniformLocation(pProgram->getProgram(),"texDest");
			myUnifoMap["step_s"] = glGetUniformLocation(pProgram->getProgram(),"step_s");
			myUnifoMap["step_t"] = glGetUniformLocation(pProgram->getProgram(),"step_t");
			myUnifoMap["touchPos_winSpace"] = glGetUniformLocation(pProgram->getProgram(),"touchPos_winSpace");
			myUnifoMap["touchValid"] = glGetUniformLocation(pProgram->getProgram(),"touchValid");
		}
		CCShaderCache::sharedShaderCache()->addProgram(pProgram,"updateRipple");
		pProgram->release();
		program_updateRipple.program=pProgram;
		program_updateRipple.myUnifoMap=myUnifoMap;
		CHECK_GL_ERROR_DEBUG();
	}
	//renderRipple shader
	{
		GLchar * fragSource = (GLchar*) CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("renderRipple.fsh").c_str())->getCString();
		CCGLProgram* pProgram = new CCGLProgram();
		pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
		pProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		pProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		pProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);	
		pProgram->link();//link must after bindAttribute
		pProgram->updateUniforms();//????cocos2d-x????uniform????
		map<string,GLint> myUnifoMap;
		{
			myUnifoMap["texSource"] = glGetUniformLocation(pProgram->getProgram(),"texSource");
			myUnifoMap["step_s"] = glGetUniformLocation(pProgram->getProgram(),"step_s");
			myUnifoMap["step_t"] = glGetUniformLocation(pProgram->getProgram(),"step_t");
		}
		CCShaderCache::sharedShaderCache()->addProgram(pProgram,"renderRipple");
		pProgram->release();
		program_renderRipple.program=pProgram;
		program_renderRipple.myUnifoMap=myUnifoMap;
		CHECK_GL_ERROR_DEBUG();
	}
	//????fbo
	glGenFramebuffers(1,&hFBO);
	//model
	float posArray[8]={0,0,texWidth,0,texWidth,texHeight,0,texHeight};
	int indexArray[6]={0,1,2,2,3,0};

	//create and init indexVBO
	CindexVBO::enableAttribArrays();
	_indexVBO=new CindexVBO();
	_indexVBO->genBuffers();
	//submit data to vbo
	_indexVBO->submitPos(posArray,8,GL_STATIC_DRAW);
	_indexVBO->submitIndex(indexArray,6,GL_STATIC_DRAW);

	//gl check
	CHECK_GL_ERROR_DEBUG();

	return true ;
}

void SceneNode::draw()
{	

	//??texSource,texDest??????texTemp
	{
		//----????fbo
		//??fbo
		GLint oldFBO=0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING,&oldFBO);
	//	//oldRBO
	//	GLint oldRBO;  
     //   glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRBO);  
		//??????fbo
		glBindFramebuffer(GL_FRAMEBUFFER,hFBO);
		//attach???????
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texTemp,0);
	//	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	//	glViewport(0,0,winSize.width,winSize.height); 
		//----????shader
		this->setShaderProgram(program_updateRipple.program);
		ccGLEnable(m_eGLServerState);//need optim
		//??cocos2d-x????uniform?
        getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 
		//???????uniform?
		glUniform1f(program_updateRipple.myUnifoMap["step_s"],step_s);
		glUniform1f(program_updateRipple.myUnifoMap["step_t"],step_t);
		float touchPosArray[]={touchPos_winSpace.x,touchPos_winSpace.y};
		glUniform2fv(program_updateRipple.myUnifoMap["touchPos_winSpace"],1,touchPosArray);
		glUniform1i(program_updateRipple.myUnifoMap["touchValid"],touchValid);
		touchValid=false;//��touchʧЧ
		//?????????
		glUniform1i(program_updateRipple.myUnifoMap["texSource"],1);
		glUniform1i(program_updateRipple.myUnifoMap["texDest"],2);
		//??????
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSource);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texDest);
        glActiveTexture(GL_TEXTURE0);//back to GL_TEXTURE0
		//draw
		{
			float texCoordArray[8]={0,0,1,0,1,1,0,1};
			_indexVBO->submitTexCoord(texCoordArray,8,GL_DYNAMIC_DRAW);
		}
		_indexVBO->setPointers();
		_indexVBO->draw(GL_TRIANGLES);  
	//	//recover oldRBO
	//	glBindRenderbuffer(GL_RENDERBUFFER, oldRBO);  
		//???????fbo
		glBindFramebuffer(GL_FRAMEBUFFER,oldFBO);
	//	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	//	glViewport(0,0,winSize.width,winSize.height); 

	/*	//draw
		setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture));
		ccGLEnable(m_eGLServerState);
		getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 

		glBindTexture(GL_TEXTURE_2D,texTemp);
		_indexVBO->setPointers();
		_indexVBO->draw(GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D,0);
		*/

		this->setShaderProgram(program_renderRipple.program);
		ccGLEnable(m_eGLServerState);//need optim
		getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 
		//???????uniform?
		glUniform1f(program_renderRipple.myUnifoMap["step_s"],step_s);
		glUniform1f(program_renderRipple.myUnifoMap["step_t"],step_t);
		//pass texture attach point uniform value
		glUniform1i(program_renderRipple.myUnifoMap["texSource"],1);
		//attach texture to texture attach point
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSource);
        glActiveTexture(GL_TEXTURE0);//back to GL_TEXTURE0
		glBindTexture(GL_TEXTURE_2D,_texture->getName());
		//draw
		{
			float texCoordArray[8]={0,1,1,1,1,0,0,0};
			_indexVBO->submitTexCoord(texCoordArray,8,GL_DYNAMIC_DRAW);
		}
		_indexVBO->setPointers();
		_indexVBO->draw(GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D,0);


	


	}
	
	//system("pause");
	//store
	GLuint texSource_store=texSource;
	GLuint texDest_store=texDest;
	GLuint texTemp_store=texTemp;
	//reassign
	texSource=texTemp_store;
	texDest=texSource_store;
	texTemp=texDest_store;

	
	
}
void SceneNode::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    //Add a new body/atlas sprite at the touched location
    CCSetIterator it;
    CCTouch* touch;

    for( it = touches->begin(); it != touches->end(); it++)
    {
        touch = (CCTouch*)(*it);
        
        if(!touch)
            break;
        
        CCPoint loc_winSpace = touch->getLocationInView();
        
        CCPoint loc_GLSpace = CCDirector::sharedDirector()->convertToGL(loc_winSpace);
	
    }
}
void SceneNode::ccTouchesMoved(cocos2d::CCSet* touches , cocos2d::CCEvent* event)
{
    CCSetIterator it;
    CCTouch* touch;
    for( it = touches->begin(); it != touches->end(); it++)
    {
        touch = (CCTouch*)(*it);
        
        if(!touch)
            break;
        
        CCPoint loc_winSpace = touch->getLocationInView();
        
        CCPoint loc_GLSpace = CCDirector::sharedDirector()->convertToGL(loc_winSpace);

		if(loc_winSpace.x>0&&loc_winSpace.x<texWidth&&loc_winSpace.y>0&&loc_winSpace.y<texHeight){
			touchPos_winSpace=loc_winSpace;
			touchValid=true;
		}
		
    }
   
}

void SceneNode::ccTouchesBegan(CCSet* touches, CCEvent* event)
{
    CCSetIterator it;
    CCTouch* touch;
	for( it = touches->begin(); it != touches->end(); it++)
    {
        touch = (CCTouch*)(*it);
        if(!touch)
            break;
        
		CCPoint loc_winSpace = touch->getLocationInView();
        
        CCPoint loc_GLSpace = CCDirector::sharedDirector()->convertToGL(loc_winSpace);

		if(loc_winSpace.x>0&&loc_winSpace.x<texWidth&&loc_winSpace.y>0&&loc_winSpace.y<texHeight){
			touchPos_winSpace=loc_winSpace;
			touchValid=true;
		}
	
    }
}
