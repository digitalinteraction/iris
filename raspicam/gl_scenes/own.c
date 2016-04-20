/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "own.h"
#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

unsigned char mapping[1458][1944][4];



RASPITEXUTIL_SHADER_PROGRAM_T own_shader = {
    .vertex_source = 0,
    .fragment_source = 0,
    .uniform_names = {"tex", "undist"},
    //.uniform_names = {},
    .attribute_names = {"vertex"},
    //.attribute_names = {},
};

RASPITEXUTIL_SHADER_PROGRAM_T own_shader2 = {
    .vertex_source = 0,
    .fragment_source = 0,
    .uniform_names = {"renderTexture", "tex_unit"},
    //.uniform_names = {},
    .attribute_names = {"vertex"},
    //.attribute_names = {},
};

int loadshader(char* filename, char* filename2, RASPITEXUTIL_SHADER_PROGRAM_T *p) {

    printf("load shader with name %s\n", filename);

    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("ERROR");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    p->vertex_source = malloc(sizeof (char)*(sz + 1));
    if (p->vertex_source == 0) return -1; // can't reserve memory
    fread(p->vertex_source, sz, 1, fp); /* Read the contents of the file in to the buffer */
    p->vertex_source[sz] = 0;
    fclose(fp);

    printf("load shader with name %s\n", filename2);
    fp = fopen(filename2, "rb");
    if (fp == NULL) {
        perror("ERROR");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    p->fragment_source = malloc(sizeof (char)*(sz + 1));
    if (p->fragment_source == 0) return -1; // can't reserve memory
    fread(p->fragment_source, sz, 1, fp); /* Read the contents of the file in to the buffer */
    p->fragment_source[sz] = 0;
    fclose(fp);

    return 0; // No Error
}

/**
 * Creates the OpenGL ES 2.X context and builds the shaders.
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
static int own_init(RASPITEX_STATE *state)
{
    printf("got to shader init\n");
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;
    loadshader("../gl_scenes/own_vertex.glsl", "../gl_scenes/own_fragment_undistort_2.glsl", &own_shader);
    loadshader("../gl_scenes/own_vertex.glsl", "../gl_scenes/own_fragment_3.glsl", &own_shader2);

    //GLint texLoc = glGetUniformLocation(own_shader.program, "undist");
    //glUniform1i(texLoc, 1);
    //glUniform1i(own_shader.uniform_locations[1], 1);
    rc = raspitexutil_build_shader_program(&own_shader);
    rc += raspitexutil_build_shader_program(&own_shader2);

    GLCHK(glGenFramebuffers(1, &state->framebuffer));
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer));
    
    GLCHK(glGenTextures(1, &state->renderTexture));
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->renderTexture));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 1944, 1458, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state->renderTexture,0);
    glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE){
        printf("Error Frame buffer\n");
    }
    
    GLCHK(glUseProgram(own_shader.program));
    //GLCHK(glUniform1i(own_shader.uniform_locations[0], 0)); // tex unit
    //GLCHK(glUniform1i(own_shader.uniform_locations[1], 1)); // tex unit
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->undist));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1944, 1458, 0, GL_RGBA, GL_UNSIGNED_BYTE, mapping));
    //GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1944, 1458, 0, GL_ALPHA, GL_FLOAT, mapping));




end:
    return rc;
}

static int own_redraw(RASPITEX_STATE *raspitex_state) {

    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->framebuffer));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader.program));

    glActiveTexture(GL_TEXTURE0);
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));
    GLCHK(glEnableVertexAttribArray(own_shader.attribute_locations[0]));
    GLfloat varray[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };
    GLCHK(glVertexAttribPointer(own_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->undist));
    
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
///////////////////////////////////////////////////////////////
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader2.program));
    GLCHK(glUniform2f(own_shader2.uniform_locations[1], 1.0 / (float) 1944.0, 1.0 / (float) 1458.0));
    
    glActiveTexture(GL_TEXTURE0);
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->renderTexture));
    GLCHK(glEnableVertexAttribArray(own_shader.attribute_locations[0]));
    GLfloat varray2[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };
    GLCHK(glVertexAttribPointer(own_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray2));
    
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
/////////////////////////////////////////////////////////////////////////////////
    
    GLCHK(glDisableVertexAttribArray(own_shader.attribute_locations[0]));
    GLCHK(glUseProgram(0));
    return 0;
}

int own_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = own_init;
   state->ops.redraw = own_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   

   unsigned int i;
   unsigned int j;
   unsigned int x=0;
   unsigned int y=0;
   float x_mul = 65536/1944;
   float y_mul = 65536/1458;
   
   for (i=0; i < 1944; i++) {
        for (j=0; j < 1458; j++) {

            /*x = i*x_mul;
            y = j*y_mul;
            mapping[j][i][0] = x>>8; 
            mapping[j][i][1] = x;
            mapping[j][i][2] = y>>8;
            mapping[j][i][3] = y;*/
            if((i / 972) == 0){
                if((j / 729) == 0){
                    x = (i+972)*x_mul;
                    y = (j+729)*y_mul;
                    mapping[j][i][0] = x>>8; 
                    mapping[j][i][1] = x;
                    mapping[j][i][2] = y>>8;
                    mapping[j][i][3] = y;
                }else{
                    x = (i+972)*x_mul;
                    y = (j-729)*y_mul;
                    mapping[j][i][0] = x>>8; 
                    mapping[j][i][1] = x;
                    mapping[j][i][2] = y>>8;
                    mapping[j][i][3] = y;
                }
            }else{
                if((j / 729) == 0){
                    x = (i-972)*x_mul;
                    y = (j+729)*y_mul;
                    mapping[j][i][0] = x>>8; 
                    mapping[j][i][1] = x;
                    mapping[j][i][2] = y>>8;
                    mapping[j][i][3] = y;
                }else{
                    x = (i-972)*x_mul;
                    y = (j-729)*y_mul;
                    mapping[j][i][0] = x>>8; 
                    mapping[j][i][1] = x;
                    mapping[j][i][2] = y>>8;
                    mapping[j][i][3] = y;
                }
            }
        }
    }
   
   FILE *fp;
   fp = fopen("out.tga", "w");
   write_tga(fp, 1944, 1458, mapping, sizeof(mapping));
   fclose(fp);
   return 0;
}
