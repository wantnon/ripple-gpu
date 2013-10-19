

#include "SceneNode.h"
#include <iostream>
#include "support/ccUtils.h"

using namespace cocos2d ;
using namespace std ;

SceneNode::SceneNode()
{
	texWidth=256;
	texHeight=256;
	step_s=1.0/texWidth;
	step_t=1.0/texHeight;

	texSource=0;
	texDest=0;
	texTemp=0;

	uniLoc_texSource=-100;
	uniLoc_texDest=-100;
	uniLoc_texTemp=-100;
	uniLoc_step_s=-100;
	uniLoc_step_t=-100;

	hFBO=0;

	pProgram_updateRipple=NULL;
	pProgram_renderRipple=NULL;

	touchPos=CCPoint(-1,-1);
	touchValid=false;
}

SceneNode::~SceneNode()
{
	//?????

}


bool SceneNode::initWithTexture(std::string textureName) 
{
	glClearColor(1,0,0,1);
	//create texture
    _texture = CCTextureCache::sharedTextureCache()->addImage(textureName.c_str()) ;//???_texture??retainCount?1
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
		{//?????uniform????
			uniLoc_texSource = glGetUniformLocation(pProgram->getProgram(),"texSource");
			uniLoc_texDest = glGetUniformLocation(pProgram->getProgram(),"texDest");
			uniLoc_texTemp = glGetUniformLocation(pProgram->getProgram(),"texTemp");
			uniLoc_step_s = glGetUniformLocation(pProgram->getProgram(),"step_s");
			uniLoc_step_t = glGetUniformLocation(pProgram->getProgram(),"step_t");
			uniLoc_touchPos = glGetUniformLocation(pProgram->getProgram(),"touchPos");
			uniLoc_touchValid = glGetUniformLocation(pProgram->getProgram(),"touchValid");

		}
		CCShaderCache::sharedShaderCache()->addProgram(pProgram,"updateRipple");
		pProgram->release();
		pProgram_updateRipple=pProgram;
		CHECK_GL_ERROR_DEBUG();
	}
/*	//renderRipple shader
	{
		GLchar * fragSource = (GLchar*) CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("renderRipple.fsh").c_str())->getCString();
		CCGLProgram* pProgram = new CCGLProgram();
		pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
		pProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		pProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		pProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);	
		pProgram->link();//link must after bindAttribute
		pProgram->updateUniforms();//????cocos2d-x????uniform????
		{//?????uniform????
			uniLoc_texSource = glGetUniformLocation(pProgram->getProgram(),"texSource");
		}
		CCShaderCache::sharedShaderCache()->addProgram(pProgram,"renderRipple");
		pProgram->release();
		pProgram_renderRipple=pProgram;
		CHECK_GL_ERROR_DEBUG();
	}*/
	//????fbo
	glGenFramebuffers(1,&hFBO);
	//model
	float posArray[8]={0,0,texWidth,0,texWidth,texHeight,0,texHeight};
	int indexArray[6]={0,1,2,2,3,0};
//	float texCoordArray[8]={0,1,1,1,1,0,0,0};
	float texCoordArray[8]={0,0,1,0,1,1,0,1};
	//create and init indexVBO
	CindexVBO::enableAttribArrays();
	_indexVBO=new CindexVBO();
	_indexVBO->genBuffers();
	//submit data to vbo
	_indexVBO->submitPos(posArray,8,GL_STATIC_DRAW);
	_indexVBO->submitIndex(indexArray,6,GL_STATIC_DRAW);
	_indexVBO->submitTexCoord(texCoordArray,8,GL_STATIC_DRAW);
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
		this->setShaderProgram(pProgram_updateRipple);
		ccGLEnable(m_eGLServerState); 	
		//??cocos2d-x????uniform?
        getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 
		//???????uniform?
		glUniform1f(uniLoc_step_s,step_s);
		glUniform1f(uniLoc_step_t,step_t);
		float touchPosArray[]={touchPos.x,touchPos.y};
		glUniform2fv(uniLoc_touchPos,1,touchPosArray);
		glUniform1i(uniLoc_touchValid,touchValid);
		touchValid=false;//��touchʧЧ
		//?????????
		glUniform1i(uniLoc_texSource,1);
		glUniform1i(uniLoc_texDest,2);
		//??????
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSource);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texDest);
        glActiveTexture(GL_TEXTURE0);//???texture0
		//draw
		_indexVBO->setPointers();
		_indexVBO->draw(GL_TRIANGLES);  
	//	//recover oldRBO
	//	glBindRenderbuffer(GL_RENDERBUFFER, oldRBO);  
		//???????fbo
		glBindFramebuffer(GL_FRAMEBUFFER,oldFBO);
	//	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	//	glViewport(0,0,winSize.width,winSize.height); 
		//draw
		setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture));
		ccGLEnable(m_eGLServerState);
		getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 

		glBindTexture(GL_TEXTURE_2D,texTemp);
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
        
        CCPoint location = touch->getLocationInView();
        
        location = CCDirector::sharedDirector()->convertToGL(location);
    //    cout<<"mos pos:"<<location.x<<" "<<location.y<<endl;
	
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
        
        CCPoint location = touch->getLocationInView();
        
        location = CCDirector::sharedDirector()->convertToGL(location);
    //    cout<<"mos pos:"<<location.x<<" "<<location.y<<endl;
		if(location.x>0&&location.x<texWidth&&location.y>0&&location.y<texHeight){
			touchPos=location;
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
        
        CCPoint location = touch->getLocationInView();
        
        location = CCDirector::sharedDirector()->convertToGL(location);
   //     cout<<"mos pos:"<<location.x<<" "<<location.y<<endl;
		if(location.x>0&&location.x<texWidth&&location.y>0&&location.y<texHeight){
			touchPos=location;
			touchValid=true;
		}
	
    }
}
