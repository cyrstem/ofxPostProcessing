/*
 *  ContrastPass.h
 *
 *  Copyright (c) 2013, satcy, http://satcy.net
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
#include "ContrastPass.h"
#include "ofMain.h"

namespace itg
{
    ContrastPass::ContrastPass(const ofVec2f& aspect, bool arb, float contrast, float brightness) :
        contrast(contrast), brightness(brightness), RenderPass(aspect, arb, "contrast")
    {
		
		string vertShaderSrc = STRINGIFY(

			uniform mat4 modelViewProjectionMatrix;
			layout(location = 0) in vec4 position;
			layout(location = 2) in vec4 color;
			layout(location = 3) in vec2 texcoord;

			out vec2 vUv;

			void main()
			{
				vUv = texcoord;
				gl_Position = modelViewProjectionMatrix * position;
			}
		);
		
		
		
		multiple = 1.0f;
        string fragShaderSrc = STRINGIFY(
					
			uniform SAMPLER_TYPE tex0;
			uniform float contrast;
			uniform float brightness;
			uniform float multiple;
                                         
			in vec2 vUv;	
			out vec4 color;

			void main(){
				vec4 texColor = TEXTURE_FN(tex0, vUv);
                                             
				float p = 0.3 *texColor.g + 0.59*texColor.r + 0.11*texColor.b;
				p = p * brightness;
				vec4 color2 = vec4(p,p,p,1.0);
				texColor *= color2;
				texColor *= vec4(multiple,multiple,multiple,1.0);
				texColor = mix( vec4(1.0,1.0,1.0,1.0), texColor,contrast);
                                             
				color =  vec4(texColor.r , texColor.g, texColor.b, 1.0);
			}
		);
        
		ostringstream oss;
		oss << "#version 330"<< endl << vertShaderSrc;
		shader.setupShaderFromSource(GL_VERTEX_SHADER, oss.str());

		oss.str("");
		oss << "#version 330" << endl;
		if (arb)
		{
			oss << "#define SAMPLER_TYPE sampler2DRect" << endl;
			oss << "#define TEXTURE_FN texture2DRect" << endl;
		}
		else
		{
			oss << "#define SAMPLER_TYPE sampler2D" << endl;
			oss << "#define TEXTURE_FN texture2D" << endl;
		}
		oss << fragShaderSrc;
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, oss.str());
		shader.linkProgram();
    }
    

    void ContrastPass::render(ofFbo& readFbo, ofFbo& writeFbo)
    {
        writeFbo.begin();
        
        ofClear(0, 0, 0, 255);
        
        shader.begin();
        
        shader.setUniformTexture("tex0", readFbo, 0);
        shader.setUniform1f("contrast", contrast);
        shader.setUniform1f("brightness", brightness);
        shader.setUniform1f("multiple", multiple);
        
        texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        
        shader.end();
        writeFbo.end();
    }
}