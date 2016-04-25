/*
 *  ContrastPass.h
 *
 *  Copyright (c) 2016, Patrick Fürst, http://www.patrickfuerst.at
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
 *  * Neither the name of Patrick Fürst nor the names of its contributors may be used 
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
#include "BrightnessFilterPass.h"
#include "ofMain.h"

namespace itg
{
    BrightnessFilterPass::BrightnessFilterPass(const ofVec2f& aspect, bool arb,  float brightnessThreshold) :
       brightnessThreshold(brightnessThreshold), RenderPass(aspect, arb, "contrast")
    {
		
		string vertShaderSrc = STRINGIFY(

			uniform mat4 modelViewProjectionMatrix;
		    uniform vec2 resolution;

			layout(location = 0) in vec4 position;
			layout(location = 3) in vec2 texcoord;

			out vec2 vUv;

			void main()
			{
				vUv = texcoord*resolution;
				gl_Position = modelViewProjectionMatrix * position;
			}
		);
		
		
        string fragShaderSrc = STRINGIFY(
					
			uniform SAMPLER_TYPE tex0;
			uniform float brightnessThreshold;
                                         
			in vec2 vUv;	
			out vec4 color;

			void main(){
				vec4 texColor = TEXTURE_FN(tex0, vUv);
				
				float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
				if (brightness > brightnessThreshold)
					color = texColor;
				else
					color.a = 0.0;


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
    

    void BrightnessFilterPass::render(ofFbo& readFbo, ofFbo& writeFbo)
    {
        writeFbo.begin();
        
        ofClear(0, 0, 0, 0);
        
        shader.begin();
        
        shader.setUniformTexture("tex0", readFbo, 0);
        shader.setUniform1f("brightnessThreshold", brightnessThreshold);
		if (arb) shader.setUniform2f("resolution", readFbo.getWidth(), readFbo.getHeight());
		else shader.setUniform2f("resolution", 1.f, 1.f);
        texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        
        shader.end();
        writeFbo.end();
    }
}