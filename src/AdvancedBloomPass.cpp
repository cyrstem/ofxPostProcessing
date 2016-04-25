/*
 *  AdvancedBloomPass.cpp
 *
 *  Copyright (c) 2012, Neil Mendoza, http://www.neilmendoza.com
 *  Copyright (c) 2016, Patrick F�rst, http://www.patrickfuerst.at
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met: 
 *  
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of Neil Mendoza nor the names of its contributors may be used 
 *    to endorse or promote products derived from this software without 
 *    specific prior written permission. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE. 
 *
 */
#include "AdvancedBloomPass.h"
#include "ofMain.h"

namespace itg
{
	AdvancedBloomPass::AdvancedBloomPass(const ofVec2f& aspect, bool arb, const ofVec2f& xBlur, const ofVec2f& yBlur, unsigned resolution, bool aspectCorrect, bool useHDR) : RenderPass(aspect, arb, "advanced bloom")
    {
        currentReadFbo = 0;
        if (resolution != ofNextPow2(resolution)) ofLogWarning() << "Resolution " << resolution << " is not a power of two, using " << ofNextPow2(resolution);
        
        xConv = ConvolutionPass::Ptr(new ConvolutionPass(aspect, arb, xBlur));
        yConv = ConvolutionPass::Ptr(new ConvolutionPass(aspect, arb, (aspectCorrect?aspect.x / aspect.y:1.f) * yBlur));
        
		brightnessFilter = BrightnessFilterPass::Ptr(new BrightnessFilterPass(aspect, arb));

        ofFbo::Settings s;
		s.internalformat = useHDR ? GL_RGBA16F : GL_RGBA8;
        if (arb)
        {
            s.width = resolution;
            s.height = resolution * aspect.y / aspect.x;
            s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
        }
        else
        {
            s.width = ofNextPow2(resolution);
            s.height = ofNextPow2(resolution);
            s.textureTarget = GL_TEXTURE_2D;
            
        }
        s.useDepth = true;
        
        for (int i = 0; i < 2; ++i) fbos[i].allocate(s);
    }

    
    void AdvancedBloomPass::render(ofFbo& readFbo, ofFbo& writeFbo)
    {
		if (brightnessFilter->getEnabled()) {

			brightnessFilter->render(readFbo, fbos[1]);
			xConv->render(fbos[1], fbos[0]);
			yConv->render(fbos[0], fbos[1]);
		}
		else {
			
			xConv->render(readFbo, fbos[0]);
			yConv->render(fbos[0], fbos[1]);
		}
        
        writeFbo.begin();
        ofClear(0, 0, 0, 0);
        ofSetColor(255, 255, 255);
        readFbo.draw(0, 0);
        ofEnableAlphaBlending();
        glBlendFunc(GL_ONE, GL_ONE);
        fbos[1].draw(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ofDisableAlphaBlending();
        writeFbo.end();
    }
}	