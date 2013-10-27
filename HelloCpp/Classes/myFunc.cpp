//
//  myFunc.cpp
//  HelloCpp
//
//  Created by apple on 13-10-26.
//
//

#include "myFunc.h"
#include "support/ccUtils.h"
CCTexture2D* createCCTexture2DWithSize(const CCSize&size,CCTexture2DPixelFormat eFormat,
                                       float r=0,float g=0,float b=0,float a=0){
    CCTexture2D* pTexture=NULL;
    void *data = NULL;
    do{
        int w = (int)(size.width * CC_CONTENT_SCALE_FACTOR());
        int h = (int)(size.height * CC_CONTENT_SCALE_FACTOR());
        
        // textures must be power of two squared
        unsigned int powW = 0;
        unsigned int powH = 0;
        
        if (CCConfiguration::sharedConfiguration()->supportsNPOT())
        {
            powW = w;
            powH = h;
        }
        else
        {
            powW = ccNextPOT(w);
            powH = ccNextPOT(h);
        }
        
        data = malloc((int)(powW * powH * 4));
        CC_BREAK_IF(! data);
        
        memset(data, 0, (int)(powW * powH * 4));
        
        //填充颜色
        for(int i=0;i<powH;i++){
            for(int j=0;j<powW;j++){
                ((char*)data)[(i*powW+j)*4+0]=r;
                ((char*)data)[(i*powW+j)*4+1]=g;
                ((char*)data)[(i*powW+j)*4+2]=b;
                ((char*)data)[(i*powW+j)*4+3]=a;
            }
        }
        
        pTexture = new CCTexture2D();
        if (pTexture)
        {
            pTexture->initWithData(data, eFormat, powW, powH, CCSizeMake((float)w, (float)h));
        }
    }while (0);
    return pTexture;

}