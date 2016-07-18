/*
 main

 Copyright 2012 Thomas Dalling - http://tomdalling.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

//#include "platform.hpp"

// third-party libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <stdio.h>
#include "NetworkControl.h"
#include "Packetbuffer.h"

// tdogl classes
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include <thread>


#define WIDTH 256
#define HEIGHT 192
#define DIVISION 8
// constants
const glm::vec2 SCREEN_SIZE(800, 600);

// globals
GLFWwindow* gWindow = NULL;
tdogl::Texture* gTexture = NULL;
tdogl::Program* gProgram = NULL;
GLuint gVAO = 0;
GLuint gVBO = 0;

GLuint mytexture;
unsigned char *myimage;
NetworkControl *nc;
Packetbuffer *in;
struct mac_list{
    uint32_t x;
    uint32_t y;
    uint64_t mac;
    uint8_t active;
    uint8_t nr;
    struct mac_list *next;
    struct mac_list *prev;
};

struct mac_list *first_item = 0;
struct mac_list *last_item = 0;
uint8_t posx = 0;
uint8_t posy = 0;

double weight[4];

// loads the vertex shader and fragment shader, and links them to make the global gProgram
static void LoadShaders() {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile("../resources/vertex-shader.txt", GL_VERTEX_SHADER));
    shaders.push_back(tdogl::Shader::shaderFromFile("../resources/fragment-shader.txt", GL_FRAGMENT_SHADER));
    gProgram = new tdogl::Program(shaders);
}


// loads a triangle into the VAO global

static void LoadTriangle() {
    // make and bind the VAO
    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);

    // make and bind the VBO
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);

    GLfloat vertexData[] = {
        -1.0f, -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,

        -1.0f, 1.0f,
        1.0f, 1.0f,
        -1.0f, -1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof (vertexData), vertexData, GL_STATIC_DRAW);

    // connect the xyz to the "vert" attribute of the vertex shader
    glEnableVertexAttribArray(gProgram->attrib("vert"));
    glVertexAttribPointer(gProgram->attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);

    // connect the uv coords to the "vertTexCoord" attribute of the vertex shader
    //glEnableVertexAttribArray(gProgram->attrib("vertTexCoord"));
    //glVertexAttribPointer(gProgram->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,  5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    // unbind the VAO
    glBindVertexArray(0);
}


// loads the file "hazard.png" into gTexture
void LoadTexture() {
    //tdogl::Bitmap bmp = tdogl::Bitmap::bitmapFromFile(ResourcePath("hazard.png"));
    //printf("%d %d %d\n", bmp.format(), bmp.height(), bmp.width());
    //bmp.flipVertically();
    //gTexture = new tdogl::Texture(bmp);
    myimage = (unsigned char *) malloc(WIDTH*HEIGHT*1*sizeof(unsigned char));
    //for(int i = 0; i < (WIDTH*HEIGHT*3); i++){
    //    myimage[i] = 128;
    //}
    memset(myimage, 0, WIDTH*HEIGHT*1*sizeof(unsigned char));
    
    glGenTextures(1, &mytexture);
    glBindTexture(GL_TEXTURE_2D, mytexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //printf("test\n");
    glTexImage2D(GL_TEXTURE_2D,
                 0, 
                 GL_RED, //GL_RGB
                 WIDTH, 
                 HEIGHT,
                 0, 
                 GL_RED, //GL_RGB
                 GL_UNSIGNED_BYTE, 
                 myimage);
    //printf("test2\n");
    glBindTexture(GL_TEXTURE_2D, 0);
    //printf("test3\n");
}




void OnError(int errorCode, const char* msg) {
    throw std::runtime_error(msg);
}

void insertList(uint8_t x, uint8_t y, uint64_t mac, uint8_t nr){
    struct mac_list *item = (struct mac_list *)malloc(sizeof(struct mac_list));
    item->x = x;
    item->y = y;
    item->mac = mac;
    item->nr = nr;
    
    if(first_item == 0){
        first_item = item;
        last_item = item;
        first_item->prev = 0;
        first_item->next = 0;
    }else{
        last_item->next = item;
        item->prev = last_item;
        last_item = item;
        last_item->next = 0;
    }
}

struct mac_list * searchList(uint64_t mac, uint8_t del) {
    struct mac_list *item = first_item;
    while (item != 0) {
        if (item->mac == mac) {
            if (del == 1) {
                if (item->prev == 0 && item->next == 0) {
                    first_item = 0;
                    last_item = 0;
                } else if (item->prev == 0) {
                    item->next->prev = 0;
                    first_item = item->next;
                } else if (item->next == 0) {
                    item->prev->next = 0;
                    last_item = item->prev;
                } else {
                    item->prev->next = item->next;
                    item->next->prev = item->prev;
                }
            }
            return item;
        }
        item = item->next;
    }
    return 0;
}

void deleteList(){
    struct mac_list *item = first_item;
    while (item != 0) {
        struct mac_list *next = item->next;
        free(item);
        item = next;
    }
    first_item = 0;
}

void updateBuffer() {
    struct packet *pack = 0;
    struct topo_header* header = 0;
    while (in->get(&pack) == 0) {
        header = (struct topo_header*) pack->buffer;
        switch (header->port) {
            case TOPO_PACKET:
            {
                unsigned char* list = ((unsigned char *) pack->buffer) + sizeof (struct topo_header);
                printf("got topo packet: %d %d\n", header->sizex, header->sizey);
                if (posx != (header->sizex) || posy != (header->sizey)) {
                    posx = header->sizex;
                    posy = header->sizey;
                    printf("changing framebuffer to %d %d %d %d\n", HEIGHT*posx, WIDTH*posy, posx, posy);
                    free(myimage);
                    size_t size = WIDTH * HEIGHT * 1 * sizeof (unsigned char)*posx*posy + posy*posx*2*20;
                    myimage = (unsigned char *) malloc(size);
                    memset(myimage, 0, size);
                    glDeleteTextures(1, &mytexture);
                    glGenTextures(1, &mytexture);
                    glBindTexture(GL_TEXTURE_2D, mytexture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D,
                            0,
                            GL_RED, //GL_RGB
                            WIDTH*posy + (posy*2*20),
                            HEIGHT*posx + (posx*2*20),
                            0,
                            GL_RED, //GL_RGB
                            GL_UNSIGNED_BYTE,
                            myimage);
                    //glBindTexture(GL_TEXTURE_2D, 0);
                    
                    /*int resx = 0;
                    int resy = 0;
                    if(posy > posx){
                        resx = 800;
                        resy = (800/posy)*posx;
                    }else{
                        resy = 800;
                        resx = (800/posx)*posy;
                    }
                    glfwSetWindowSize(gWindow, resx, resy);
                    glViewport(0,0,WIDTH*posy,HEIGHT*posx);
                    printf("SETTING window to size: %d %d\n", HEIGHT*posx, WIDTH*posy);*/
                    deleteList();
                }

                for (int i = 0; i < (header->sizex * header->sizey); i++) {
                    struct topo_list *item = (struct topo_list *) (list + i * sizeof (struct topo_list));
                    if (item->mac != 0) {
                        if (searchList(item->mac, 0) != 0) {
                            struct mac_list * temp = searchList(item->mac, 1);
                            free(temp);
                        }
                        insertList(item->x, item->y, item->mac, i);
                        printf("inserting elem: %d %d %lx\n", item->x, item->y, item->mac);
                    }else{
                        /*unsigned int offsetx = 400 * item->x;
                        unsigned int offsety = 300 * item->y;
                        glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                offsetx,
                                offsety,
                                400,
                                300,
                                GL_RED, //GL_RGB
                                GL_UNSIGNED_BYTE,
                                myimage);*/
                    }            
                }
                break;
            }
            case IMAGE_PACKET:
            {
                if (posx != 0) {
                    struct low_res_header* low_header = (struct low_res_header*) pack->buffer;
                    //printf("image packet recv from %ld\n", low_header->mac);

                    struct mac_list * item = searchList(low_header->mac, 0);
                    if (item != 0) {
                        //printf("item: %d %d, pos %d\n", item->x, item->y, low_header->pos);
                        unsigned char * image_part = ((unsigned char *) pack->buffer) + sizeof (struct low_res_header);
                        //unsigned char * dest_part = myimage + 400 * 300 * 3 * (item->x + posy * item->y) + low_header->pos * 400 * 30 * 3;
                        //printf("inserting in offset: %d\n", 400 * 300 * 3 * (item->x + posy * item->y) + low_header->pos * 400 * 30 * 3);
                        //memcpy(dest_part, image_part, 400 * 30 * 3);
                        
                        /*uint8_t colr = 70*(item->x + item->y)+4*low_header->pos;
                        for(int i =0; i < 400*30; i++){
                            image_part[i] = colr;
                        }*/
                        
                        weight[item->nr] = low_header->weight;
                        
                        //flip image
                        unsigned char * temp_buf = (unsigned char *)malloc(WIDTH*(HEIGHT/DIVISION));
                        for(int i = 0; i < (HEIGHT/DIVISION); i++){
                            for(int j = 0; j < WIDTH; j++){
                                temp_buf[i*WIDTH + (WIDTH-1-j)] = image_part[i*WIDTH + j];
                            }
                        }
                        
                        
                        unsigned int offsety = HEIGHT * (posx - item->x - 1) + low_header->pos * DIVISION - 20*(item->x+1);
                        unsigned int offsetx = WIDTH * item->y + 20*(item->y+1);

                        //printf("%d %d %d offset: x: %d, y: %d %ld\n", item->x, item->y, low_header->pos, offsetx, offsety, low_header->mac);
                        //printf("setting rect %d %d to %d %d\n", offsetx, offsety, offsetx+400, offsety+30);
                            
                            glTexSubImage2D(GL_TEXTURE_2D,
                                    0,
                                    offsetx,
                                    offsety,
                                    WIDTH,
                                    HEIGHT/DIVISION,
                                    GL_RED, //GL_RGB
                                    GL_UNSIGNED_BYTE,
                                    temp_buf);
                            free(temp_buf);
                            
                    }
                }
                free(pack->buffer);
                free(pack);

                break;
            }
            default:
                printf("strange packet arrived\n");
                free(pack->buffer);
                free(pack);
        }
    }
}

static void Render() {
    // clear everything
    glClearColor(0, 0, 0, 1); // black
    glClear(GL_COLOR_BUFFER_BIT);
    
    // bind the program (the shaders)
    gProgram->use();
        
    // bind the texture and set the "tex" uniform in the fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mytexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //printf("test\n");
    /*glTexImage2D(GL_TEXTURE_2D,
                 0, 
                 GL_RGB,
                 WIDTH, 
                 HEIGHT,
                 0, 
                 GL_RGB, 
                 GL_UNSIGNED_BYTE, 
                 myimage);*/
    updateBuffer();
    gProgram->setUniform("tex", 0); //set to 0 because the texture is bound to GL_TEXTURE0

    // bind the VAO (the triangle)
    glBindVertexArray(gVAO);

    // draw the VAO
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // unbind the VAO, the program and the texture
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    gProgram->stopUsing();

    // swap the display buffers (displays what was just drawn)
    glfwSwapBuffers(gWindow);

}
// the program starts here
void AppMain() {
    // initialise GLFW
    glfwSetErrorCallback(OnError);
    if(!glfwInit())
        throw std::runtime_error("glfwInit failed");
    
    // open a window with GLFW
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "OpenGL Tutorial", NULL, NULL);
    if(!gWindow)
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");

    // GLFW settings
    glfwMakeContextCurrent(gWindow);
    
    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    // OpenGL settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load vertex and fragment shaders into opengl
    LoadShaders();

    // load the texture
    LoadTexture();

    // create buffer and fill it with the points of the triangle
    LoadTriangle();
    nc = new NetworkControl();
    in = nc->unrel_out;

    //std::thread net_work(&NetworkControl::run, nc);

    // run while the window is open
    while(!glfwWindowShouldClose(gWindow)){
        // process pending events

        glfwPollEvents();
        nc->run();
        //updateBuffer();
        // draw one frame

        Render();
        //printf("Weight: %04f %04f %04f %04f\n", weight[0],weight[1],weight[2],weight[3]);

    }

    // clean up and exit
    glfwTerminate();
}


int main(int argc, char *argv[]) {
    try {
        AppMain();
    } catch (const std::exception& e){
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
