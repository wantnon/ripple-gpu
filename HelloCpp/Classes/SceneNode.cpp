

#include "SceneNode.h"
#include <iostream>
#include <map>
#include "support/ccUtils.h"
#include "myFunc.h"
using namespace cocos2d ;
using namespace std ;

SceneNode::SceneNode()
{
	step_s=0;
	step_t=0;
    
    texBackGround=NULL;
	bufferTexSource=NULL;
	bufferTexDest=NULL;
	bufferTexTemp=NULL;

	hFBO=0;

	touchPos_winSpace=CCPoint(0,0);
	touchValid=false;

	heightMode=false;
}

SceneNode::~SceneNode()
{
	
	_indexVBO->release();
	CCTextureCache::sharedTextureCache()->removeTexture(texBackGround);
	glDeleteFramebuffers(1,&hFBO);
    bufferTexSource->release();
    bufferTexDest->release();
    bufferTexTemp->release();

}


bool SceneNode::initWithTexture(std::string textureName) 
{
    //enable touch
	setTouchEnabled( true );
    //set projection is 2D (default is 3D). if use 3D projection, projection error accumulation may cause ripple effect mess.
    CCDirector::sharedDirector()->setProjection(kCCDirectorProjection2D);
    //get adaptedViewport. adaptedViewport is calculated by cocos2d-x
    //so long as we use this adaptedViewport, we just writting code based on designResolutionSize, no need to worry about the screen adaptation.
    glGetIntegerv(GL_VIEWPORT,adaptedViewport);
    //get screenSize
    //screenSize is the real size of simulator/device screen
    screenSize=CCEGLView::sharedOpenGLView()->getFrameSize();
    CCLOG("screenSize:%f,%f",screenSize.width,screenSize.height);
    //get winSize
    //winSize is equals to designResolutionSize. we only need to writting code based on designResolutionSize (and forget the real screenSize).
    winSize=CCDirector::sharedDirector()->getWinSize();
    CCLOG("winSize:%f,%f",winSize.width,winSize.height);
    //determine bufferTexSize based on winSize
    bufferTexSize=CCSize(winSize.width*0.4,winSize.height*0.4);
    //use bufferTexSize to calculate step_s and step_t
    step_s=1.0/bufferTexSize.width;
	step_t=1.0/bufferTexSize.height;
	//create textures
    texBackGround = CCTextureCache::sharedTextureCache()->addImage(textureName.c_str()) ;
    bufferTexSource=createCCTexture2DWithSize(bufferTexSize,kCCTexture2DPixelFormat_RGBA8888,0.5,0.5,0.5,1);
    bufferTexDest=createCCTexture2DWithSize(bufferTexSize,kCCTexture2DPixelFormat_RGBA8888,0.5,0.5,0.5,1);
    bufferTexTemp=createCCTexture2DWithSize(bufferTexSize,kCCTexture2DPixelFormat_RGBA8888,0.5,0.5,0.5,1);
    //set texture params
    ccGLBindTexture2D(bufferTexSource->getName());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ccGLBindTexture2D(bufferTexDest->getName());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ccGLBindTexture2D(bufferTexTemp->getName());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//create shaders
	//updateRipple shader
	{
		GLchar * fragSource = (GLchar*) CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("updateRipple.fsh").c_str())->getCString();
		CCGLProgram* pProgram = new CCGLProgram();
		pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
        //bind attribute
		pProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		pProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		pProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
        //link  (must after bindAttribute)
		pProgram->link();
        //get cocos2d-x build-in uniforms
		pProgram->updateUniforms();
        //get my own uniforms
		map<string,GLint> myUnifoMap;
        myUnifoMap["texSource"] =glGetUniformLocation(pProgram->getProgram(),"texSource");
        myUnifoMap["texDest"] = glGetUniformLocation(pProgram->getProgram(),"texDest");
        myUnifoMap["step_s"] = glGetUniformLocation(pProgram->getProgram(),"step_s");
        myUnifoMap["step_t"] = glGetUniformLocation(pProgram->getProgram(),"step_t");
        myUnifoMap["touchPos_winSpace"] = glGetUniformLocation(pProgram->getProgram(),"touchPos_winSpace");
        myUnifoMap["touchValid"] = glGetUniformLocation(pProgram->getProgram(),"touchValid");
        myUnifoMap["winSize"] = glGetUniformLocation(pProgram->getProgram(),"winSize");
        myUnifoMap["bufferTexSize"] = glGetUniformLocation(pProgram->getProgram(),"bufferTexSize");
		//make program_updateRipple
		program_updateRipple.myUnifoMap=myUnifoMap;
		program_updateRipple.setProgram(pProgram);
         //program can be released
        pProgram->release();
        //check gl error
		CHECK_GL_ERROR_DEBUG();
	}
	//renderRipple shader
	{
		GLchar * fragSource = (GLchar*) CCString::createWithContentsOfFile(CCFileUtils::sharedFileUtils()->fullPathForFilename("renderRipple.fsh").c_str())->getCString();
		CCGLProgram* pProgram = new CCGLProgram();
		pProgram->initWithVertexShaderByteArray(ccPositionTextureColor_vert, fragSource);
        //bind attribute
		pProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		pProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
		pProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
        //link  (must after bindAttribute)
		pProgram->link();
        //get cocos2d-x build-in uniforms
		pProgram->updateUniforms();
        //get my own uniforms
		map<string,GLint> myUnifoMap;
        myUnifoMap["texSource"] = glGetUniformLocation(pProgram->getProgram(),"texSource");
        myUnifoMap["step_s"] = glGetUniformLocation(pProgram->getProgram(),"step_s");
        myUnifoMap["step_t"] = glGetUniformLocation(pProgram->getProgram(),"step_t");
        //make program_renderRipple
        program_renderRipple.myUnifoMap=myUnifoMap;
        program_renderRipple.setProgram(pProgram);
        //program can be released
        pProgram->release();
        //check gl error
		CHECK_GL_ERROR_DEBUG();
	}
	//create FBO
	glGenFramebuffers(1,&hFBO);
    // create model
	float posArray[8]={0,0,winSize.width,0,winSize.width,winSize.height,0,winSize.height};
	int indexArray[6]={0,1,2,2,3,0};
    float texCoordArray[8]={0,1,1,1,1,0,0,0};
	//create indexVBO
	CindexVBO::enableAttribArrays();
	_indexVBO=new CindexVBO();
	_indexVBO->genBuffers();
	//submit data to VBO
	_indexVBO->submitPos(posArray,8,GL_STATIC_DRAW);
	_indexVBO->submitIndex(indexArray,6,GL_STATIC_DRAW);
    _indexVBO->submitTexCoord(texCoordArray,8,GL_STATIC_DRAW);
	//check gl error
	CHECK_GL_ERROR_DEBUG();

	return true ;
}

void SceneNode::draw()
{
	//use bufferTexSource and bufferTexDest to render bufferTexTemp
	{
		//----render to bufferTexTemp
		//get old FBO
		GLint oldFBO=0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING,&oldFBO);
		//bind FBO
		glBindFramebuffer(GL_FRAMEBUFFER,hFBO);
		//attach bufferTexTemp to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,bufferTexTemp->getName(),0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glViewport(0,0,bufferTexSize.width,bufferTexSize.height);//means, though we draw on a board of designResolutionSize, but the picture will automatically scale into this viewport rect
		//----change shader and pass uniform values
		this->setShaderProgram(program_updateRipple.getProgram());
		ccGLEnable(m_eGLServerState);
		//pass values for cocos2d-x build-in uniforms
        getShaderProgram()->use(); 
        getShaderProgram()->setUniformsForBuiltins(); 
		//pass values for my own uniforms
		glUniform1f(program_updateRipple.myUnifoMap["step_s"],step_s);
		glUniform1f(program_updateRipple.myUnifoMap["step_t"],step_t);
		float touchPos_c[]={touchPos_winSpace.x,touchPos_winSpace.y};
		glUniform2fv(program_updateRipple.myUnifoMap["touchPos_winSpace"],1,touchPos_c);
		glUniform1i(program_updateRipple.myUnifoMap["touchValid"],touchValid);
        float winSize_c[]={winSize.width,winSize.height};
        glUniform2fv(program_updateRipple.myUnifoMap["winSize"],1,winSize_c);
        float bufferTexSize_c[]={bufferTexSize.width,bufferTexSize.height};
        glUniform2fv(program_updateRipple.myUnifoMap["bufferTexSize"],1,bufferTexSize_c);
		//pass texture attach point id to sampler uniform
		glUniform1i(program_updateRipple.myUnifoMap["texSource"],1);
		glUniform1i(program_updateRipple.myUnifoMap["texDest"],2);
		//attach texture to texture attach point
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bufferTexSource->getName());
        glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, bufferTexDest->getName());
        glActiveTexture(GL_TEXTURE0);//back to GL_TEXTURE0
		//draw
		_indexVBO->setPointers();
		_indexVBO->draw(GL_TRIANGLES);  
		//restore to old FBO
		glBindFramebuffer(GL_FRAMEBUFFER,oldFBO);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glViewport(adaptedViewport[0],adaptedViewport[1],adaptedViewport[2],adaptedViewport[3]);//we just draw thing on designResolution board, then the picture will automatically scale into this viewport rect
		if(heightMode){//if we want to see the height image
            //----change shader and pass uniform values
			setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture));
			ccGLEnable(m_eGLServerState);
            //pass values for cocos2d-x build-in uniforms
			getShaderProgram()->use();
			getShaderProgram()->setUniformsForBuiltins();
            //bind texture
            //because we know the current active texture attach point is GL_TEXTURE0, so we call bind directly
			glBindTexture(GL_TEXTURE_2D,bufferTexTemp->getName());
            //draw
			_indexVBO->setPointers();
			_indexVBO->draw(GL_TRIANGLES);
		}else{//if we want to see the final ripple effect
            //----change shader and pass uniform values
			this->setShaderProgram(program_renderRipple.getProgram());
			ccGLEnable(m_eGLServerState);
            //pass values for cocos2d-x build-in uniforms
			getShaderProgram()->use(); 
		    getShaderProgram()->setUniformsForBuiltins(); 
			//pass values for my own uniforms
			glUniform1f(program_renderRipple.myUnifoMap["step_s"],step_s);
			glUniform1f(program_renderRipple.myUnifoMap["step_t"],step_t);
			//pass texture attach point id to sampler uniform
			glUniform1i(program_renderRipple.myUnifoMap["texSource"],1);
			//attach texture to texture attach point
			glActiveTexture(GL_TEXTURE1);
		    glBindTexture(GL_TEXTURE_2D, bufferTexSource->getName());
		    glActiveTexture(GL_TEXTURE0);//back to GL_TEXTURE0
			glBindTexture(GL_TEXTURE_2D,texBackGround->getName());
			//draw
            _indexVBO->setPointers();
			_indexVBO->draw(GL_TRIANGLES);
		}
	}
    //----reassign bufferTexs
	//store refs
	CCTexture2D* bufferTexSource_store=bufferTexSource;
	CCTexture2D* bufferTexDest_store=bufferTexDest;
	CCTexture2D* bufferTexTemp_store=bufferTexTemp;
	//reassign
	bufferTexSource=bufferTexTemp_store;
	bufferTexDest=bufferTexSource_store;
	bufferTexTemp=bufferTexDest_store;

	
}
void SceneNode::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    touchValid=false;
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    
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
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    
    CCSetIterator it;
    CCTouch* touch;
    for( it = touches->begin(); it != touches->end(); it++)
    {
        touch = (CCTouch*)(*it);
        
        if(!touch)
            break;
        
        CCPoint loc_winSpace = touch->getLocationInView();

		if(loc_winSpace.x>0&&loc_winSpace.x<winSize.width&&loc_winSpace.y>0&&loc_winSpace.y<winSize.height){
			touchPos_winSpace=loc_winSpace;
			touchValid=true;
		}
		
    }
   
}

void SceneNode::ccTouchesBegan(CCSet* touches, CCEvent* event)
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    
    CCSetIterator it;
    CCTouch* touch;
	for( it = touches->begin(); it != touches->end(); it++)
    {
        touch = (CCTouch*)(*it);
        if(!touch)
            break;
        
        CCPoint loc_winSpace = touch->getLocationInView();
    //     CCLOG("loc_winSpace:%f,%f",loc_winSpace.x,loc_winSpace.y);
		if(loc_winSpace.x>0&&loc_winSpace.x<winSize.width&&loc_winSpace.y>0&&loc_winSpace.y<winSize.height){
			touchPos_winSpace=loc_winSpace;
			touchValid=true;
		}
	
    }
}
