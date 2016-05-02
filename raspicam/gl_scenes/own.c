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
#include <stdarg.h>

unsigned char mapping[UNDISTORT_Y][UNDISTORT_X][4];


RASPITEXUTIL_SHADER_PROGRAM_T own_shader = {
    .vertex_strings = 2,
    .fragment_strings = 2,
    .vertex_source = 0,
    .fragment_source = 0,
    .uniform_names = {"tex", "undist"},
    //.uniform_names = {},
    .attribute_names = {"vertex"},
    //.attribute_names = {},
};

RASPITEXUTIL_SHADER_PROGRAM_T own_shader2 = {
    .vertex_strings = 2,
    .fragment_strings = 2,
    .vertex_source = 0,
    .fragment_source = 0,
    .uniform_names = {"renderTexture", "tex_unit"},
    //.uniform_names = {},
    .attribute_names = {"vertex"},
    //.attribute_names = {},
};

int loadshader(char* filename, char*** p, char* addition){
    *p = malloc(sizeof(char*)*2);
    if (*p == 0){
        perror("ERROR_A");
        return -1;
    }
    **p = addition;

    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("ERROR_B");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    *(*p+1) = malloc(sizeof (char)*(sz + 1));
    if (*(*p+1)== 0){
        perror("ERROR_C");
        return -1;
    } 
    fread(*(*p+1), sz, 1, fp); 
    fclose(fp);
    return 0;
}

/*int free_shader_mem(RASPITEXUTIL_SHADER_PROGRAM_T *p){
    int i;
    for(i=0;i < p->vertex_strings; i++){
        free(p->vertex_source[i]);
    }
    for(i=0;i < p->fragment_strings; i++){
        free(p->fragment_source[i]);
    }
    free(p->vertex_source);
    free(p->fragment_source);
}*/

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
    //loadshader("../gl_scenes/own_vertex.glsl", "../gl_scenes/own_fragment_undistort_2.glsl", &own_shader, 'A', 'A');
    //loadshader("../gl_scenes/own_vertex.glsl", "../gl_scenes/own_fragment_3.glsl", &own_shader2, 'A', 'A');
    char def_str[] = "#define A\n #define RES_X (1.0/1944.0)\n #define RES_Y (1.0/1458.0)\n";
    loadshader("../gl_scenes/own_vertex.glsl", &(own_shader.vertex_source), def_str);
    loadshader("../gl_scenes/own_fragment_undistort_2.glsl", &(own_shader.fragment_source), def_str);
    loadshader("../gl_scenes/own_vertex.glsl", &(own_shader2.vertex_source), def_str);
    loadshader("../gl_scenes/own_fragment_3.glsl", &(own_shader2.fragment_source), def_str);

    //GLint texLoc = glGetUniformLocation(own_shader.program, "undist");
    //glUniform1i(texLoc, 1);
    //glUniform1i(own_shader.uniform_locations[1], 1);
    printf("building shader units\n");
    rc = raspitexutil_build_shader_program(&own_shader);
    rc += raspitexutil_build_shader_program(&own_shader2);

    printf("generating framebuffer high\n");
    //generate framebuffer high
    GLCHK(glGenFramebuffers(1, &state->framebuffer_high));
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer_high));
    
    printf("generating texture high\n");
    //configure texture in framebuffer
    GLCHK(glGenTextures(1, &state->renderTexture_high));
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->renderTexture_high));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, HIGH_OUTPUT_X, HIGH_OUTPUT_Y, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state->renderTexture_high,0));

    printf("generating framebuffer low\n");
    //generate framebuffer low
    GLCHK(glGenFramebuffers(1, &state->framebuffer_low));
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer_low));
    
    printf("generating texture low\n");
    GLCHK(glGenTextures(1, &state->renderTexture_low));
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->renderTexture_low));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, LOW_OUTPUT_X, LOW_OUTPUT_Y, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state->renderTexture_low,0));

    printf("check framebuffer low\n");
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer_low));
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE){
        printf("Error Frame buffer Low\n");
    }
    
    printf("check framebuffer high\n");
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->framebuffer_high));
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE){
        printf("Error Frame buffer High\n");
    }
    
    
    
    GLCHK(glUseProgram(own_shader.program));
    //GLCHK(glUniform1i(own_shader.uniform_locations[0], 0)); // tex unit
    //GLCHK(glUniform1i(own_shader.uniform_locations[1], 1)); // tex unit
    printf("generate undistortion texture\n");
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->undist));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, UNDISTORT_X, UNDISTORT_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, mapping));
    //GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1944, 1458, 0, GL_ALPHA, GL_FLOAT, mapping));




end:
    return rc;
}


static int own_redraw(RASPITEX_STATE *raspitex_state) {
///////////////////////////////////////////////////////////////////
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->framebuffer_low));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader.program));

    GLCHK(glActiveTexture(GL_TEXTURE0));
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
    GLCHK(glViewport(0,0,LOW_OUTPUT_X,LOW_OUTPUT_Y));
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    
////////////////////////////////////////////////////////////////////
    
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->framebuffer_high));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader.program));

    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));
    GLCHK(glViewport(0,0,UNDISTORT_X,UNDISTORT_Y));

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
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->undist));
    
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    
    
///////////////////////////////////////////////////////////////
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader2.program));
    GLCHK(glUniform2f(own_shader2.uniform_locations[1], 1.0 / (float) HIGH_OUTPUT_X, 1.0 / (float) HIGH_OUTPUT_Y));
    
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->renderTexture_high));
    GLCHK(glViewport(0,0,HIGH_OUTPUT_X,HIGH_OUTPUT_Y));
    GLCHK(glEnableVertexAttribArray(own_shader.attribute_locations[0]));
    /*GLfloat varray2[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };*/
    GLCHK(glVertexAttribPointer(own_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    
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
   //float x_mul = 65536/1944;
   //float y_mul = 65536/1458;
   printf("start reading out matrix\n"); fflush(stdout);
   
   FILE *matrix = fopen("../gl_scenes/matrix.txt", "rb");
   if(matrix != 0){
       for(i=0;i<UNDISTORT_X;i++){
           for(j=0;j<UNDISTORT_Y;j++){
               mapping[j][i][0] = fgetc(matrix);
               mapping[j][i][1] = fgetc(matrix);
               mapping[j][i][2] = fgetc(matrix);
               mapping[j][i][3] = fgetc(matrix);
           }
       }
       fclose(matrix);
   }else{
       printf("Could not open matrix file\n");
   }
   
  
   FILE *fp;
   fp = fopen("out.tga", "w");
   write_tga(fp, UNDISTORT_X, UNDISTORT_Y, mapping, sizeof(mapping));
   fclose(fp);
   return 0;
}
