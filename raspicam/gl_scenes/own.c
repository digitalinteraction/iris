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
#include "tga.h"
#include <stdlib.h>     
#include <dirent.h>


#ifdef EXTERNAL_IMAGES
unsigned char dirname[] = "/home/pi/gpu_main/cutting_board/external_images/";
uint8_t file_cnt=0;
unsigned char filenames[100][7];
#endif



RASPITEXUTIL_SHADER_PROGRAM_T own_shader = {
    .vertex_strings = 2,
    .fragment_strings = 2,
    .vertex_source = 0,
    .fragment_source = 0,
    .uniform_names = {"tex", "undist", "alt_tex"},
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

#ifdef EXTERNAL_IMAGES
uint8_t counter=0;

unsigned char * load_image(){
    unsigned char *name = malloc(strlen(dirname)+8);
    strcpy(name, dirname);
    strncat(name, filenames[counter], 7);
    
    struct tga_header tgatemp;
    unsigned char * image = load_tga(name, &tgatemp);
    if (image != 0) {
        counter++;
        printf("load image %s\n", name);
    }else if(counter != 0){
        //exit(EXIT_SUCCESS);
        //pthread_exit(EXIT_SUCCESS);
    }else{
        printf("Error in loading image\n");
        exit(EXIT_FAILURE);
    }
    free(name);
    return image;
}
#endif




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
    
#ifdef EXTERNAL_IMAGES
    char def_str[] = "#define B\n #define RES_X (1.0/1944.0)\n #define RES_Y (1.0/1458.0)\n";
#else
    char def_str[] = "#define A\n #define RES_X (1.0/1944.0)\n #define RES_Y (1.0/1458.0)\n";
#endif
    
    
    loadshader("../gl_scenes/own_vertex.glsl", &(own_shader.vertex_source), def_str);
    loadshader("../gl_scenes/own_fragment_undistort_2.glsl", &(own_shader.fragment_source), def_str);
    loadshader("../gl_scenes/own_vertex.glsl", &(own_shader2.vertex_source), def_str);
    loadshader("../gl_scenes/own_fragment_3.glsl", &(own_shader2.fragment_source), def_str);

    
    //glUniform1i(own_shader.uniform_locations[1], 1);
    printf("building shader units\n");
    rc = raspitexutil_build_shader_program(&own_shader);
    rc += raspitexutil_build_shader_program(&own_shader2);

    //GLint texLoc = glGetUniformLocation(own_shader.program, "undist");
    //glUniform1i(texLoc, 1);
    //texLoc = glGetUniformLocation(own_shader.program, "alt_tex");
    //glUniform1i(texLoc, 0);
    
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

    int i;
    for (i = 0; i < FRAMEBUFFER_CNT; i++) {
        printf("generating framebuffer %d\n", i);
        //generate framebuffer high
        GLCHK(glGenFramebuffers(1, &state->fb_high_end[i]));
        GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->fb_high_end[i]));

        printf("generating texture %d\n", i);
        //configure texture in framebuffer
        GLCHK(glGenTextures(1, &state->render_high_end[i]));
        GLCHK(glBindTexture(GL_TEXTURE_2D, state->render_high_end[i]));
        GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, HIGH_OUTPUT_X, HIGH_OUTPUT_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->render_high_end[i], 0));
    }
    
    
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

    for (i = 0; i < FRAMEBUFFER_CNT; i++) {
        printf("check framebuffer %d\n", i);
        GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, state->fb_high_end[i]));
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("Error Frame buffer %i\n", i);
        }
    }
    
    GLCHK(glUseProgram(own_shader.program));
    //GLCHK(glUniform1i(state->alt_tex, 0));
    //GLCHK(glUniform1i(state->undist, 1));

    GLCHK(glUniform1i(own_shader.uniform_locations[1], 0)); // tex unit
    GLCHK(glUniform1i(own_shader.uniform_locations[2], 1)); // tex unit
    
    
    printf("generate undistortion texture\n");
    GLCHK(glGenTextures(1, &state->undist));
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->undist));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, UNDISTORT_X, UNDISTORT_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, mapping));
    
    printf("generate alternative texture\n");
    GLCHK(glGenTextures(1, &state->alt_tex));
    GLCHK(glBindTexture(GL_TEXTURE_2D, state->alt_tex));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, UNDISTORT_X, UNDISTORT_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));

    //int i;
    printf("FB: %d (%d) ", state->framebuffer_low, state->renderTexture_low);
    printf(" %d (%d) ", state->framebuffer_high, state->renderTexture_high);
    printf(" (%d) (%d) ", state->undist, state->alt_tex);
    for(i=0;i<FRAMEBUFFER_CNT;i++){
        printf(" %d (%d) ", state->fb_high_end[i], state->render_high_end[i]);
    }
    printf("\n");
    
    state->curr_pos_fb = 0;
    printf("finished setting up OpenGL\n");

end:
    return rc;
}


static int own_redraw(RASPITEX_STATE *raspitex_state) {

    
    
    if (raspitex_state->external_images_finished == 0) {
        raspitex_state->curr_pos_fb++;
    }
    
    if (raspitex_state->curr_pos_fb == FRAMEBUFFER_CNT) {
        raspitex_state->curr_pos_fb = 0;
    }
    
    if (raspitex_state->external_images_finished == 0) {
        raspitex_state->valid_token[raspitex_state->curr_pos_fb]++;
    }
    
    
    //printf("FRAMEBUFFER: %d %d\n", raspitex_state->valid_token[raspitex_state->curr_pos_fb], raspitex_state->curr_pos_fb);
    
    
    
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->framebuffer_low));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader.program));
    
    //GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));

    ///////////////////////////////////////////////////////////////////
#ifdef EXTERNAL_IMAGES
    //GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->alt_tex));
    unsigned char * image = load_image();
    if (image != 0) {
        //printf("loading image\n");
        GLCHK(glActiveTexture(GL_TEXTURE1));
        GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->alt_tex));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCHK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, UNDISTORT_X, UNDISTORT_Y, GL_RGBA, GL_UNSIGNED_BYTE, image));
    }else{
        //printf("finished loading image\n");
        raspitex_state->external_images_finished = 1;
    }
    usleep(50000);
    //GLCHK(glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, UNDISTORT_X, UNDISTORT_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
    
#endif
    ////////////////////////////////////////////////////////////////////

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
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->undist));
    GLCHK(glViewport(0,0,LOW_OUTPUT_X,LOW_OUTPUT_Y));
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    
////////////////////////////////////////////////////////////////////
    
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->framebuffer_high));
    GLCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHK(glUseProgram(own_shader.program));

    //GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_EXTERNAL_OES, raspitex_state->texture));
    
#ifdef EXTERNAL_IMAGES
    GLCHK(glActiveTexture(GL_TEXTURE1));
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->alt_tex));
    free(image);
#endif
    
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
    GLCHK(glActiveTexture(GL_TEXTURE0));
    GLCHK(glBindTexture(GL_TEXTURE_2D, raspitex_state->undist));
    
    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    
    
///////////////////////////////////////////////////////////////
    
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, raspitex_state->fb_high_end[raspitex_state->curr_pos_fb]));
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
   
   uint8_t step = 256/FRAMEBUFFER_CNT;
   uint8_t p;
   for(p=0;p<FRAMEBUFFER_CNT;p++){
        state->valid_token[p] = step*p;
   }
   
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
    write_tga(fp, UNDISTORT_X, UNDISTORT_Y, mapping, sizeof (mapping));
    fclose(fp);

#ifdef EXTERNAL_IMAGES
    DIR *d;
    file_cnt = 0;
    struct dirent *dir;
    d = opendir(dirname);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(strlen(dir->d_name) == 7 && strcmp((dir->d_name+3), ".tga") == 0){
                strncpy(filenames[file_cnt], dir->d_name, 7);
                file_cnt++;
            }
        }
        closedir(d);
    }
#endif
    
    return 0;
}
