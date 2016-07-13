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

#define WIDTH 400
#define HEIGHT 300
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
    uint8_t x;
    uint8_t y;
    uint64_t mac;
    uint8_t active;
    struct mac_list *next;
    struct mac_list *prev;
};

struct mac_list *first_item = 0;
struct mac_list *last_item = 0;
uint8_t posx = 0;
uint8_t posy = 0;


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
    myimage = (unsigned char *) malloc(WIDTH*HEIGHT*3*sizeof(unsigned char));
    //for(int i = 0; i < (WIDTH*HEIGHT*3); i++){
    //    myimage[i] = 128;
    //}
    
    glGenTextures(1, &mytexture);
    glBindTexture(GL_TEXTURE_2D, mytexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //printf("test\n");
    glTexImage2D(GL_TEXTURE_2D,
                 0, 
                 GL_RGB,
                 WIDTH, 
                 HEIGHT,
                 0, 
                 GL_RGB, 
                 GL_UNSIGNED_BYTE, 
                 myimage);
    //printf("test2\n");
    glBindTexture(GL_TEXTURE_2D, 0);
    //printf("test3\n");
}




void OnError(int errorCode, const char* msg) {
    throw std::runtime_error(msg);
}

void insertList(uint8_t x, uint8_t y, uint64_t mac){
    struct mac_list *item = (struct mac_list *)malloc(sizeof(struct mac_list));
    item->x = x;
    item->y = y;
    item->mac = mac;
    
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
}

void updateBuffer() {
    //printf("aa");
    struct packet *pack = 0;
    struct topo_header* header = 0;
    while (in->get(&pack) == 0) {
        printf("packet received\n");
        header = (struct topo_header*) pack->buffer;
        switch (header->port) {
            case TOPO_PACKET:
            {
                unsigned char* list = ((unsigned char *) pack->buffer) + sizeof (struct topo_header);
                printf("got topo packet: %d %d\n", header->sizex, header->sizey);
                if (posx != (header->sizex) || posy != (header->sizey)) {
                    posx = header->sizex;
                    posy = header->sizey;
                    printf("changing framebuffer to %d %d %d %d\n", WIDTH*posy, HEIGHT*posx, posx, posy);

                    myimage = (unsigned char *) malloc(WIDTH * HEIGHT * 3 * sizeof (unsigned char)*posx*posy);
                    glDeleteTextures(1, &mytexture);
                    glGenTextures(1, &mytexture);
                    glBindTexture(GL_TEXTURE_2D, mytexture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D,
                            0,
                            GL_RGB,
                            WIDTH*posy,
                            HEIGHT*posx,
                            0,
                            GL_RGB,
                            GL_UNSIGNED_BYTE,
                            myimage);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glfwSetWindowSize(gWindow, WIDTH*posy, HEIGHT*posx);
                    deleteList();
                }

                for (int i = 0; i < (header->sizex * header->sizey); i++) {
                    struct topo_list *item = (struct topo_list *) (list + i * sizeof (struct topo_list));
                    if (searchList(item->mac, 0) != 0) {
                        struct mac_list * temp = searchList(item->mac, 1);
                        free(temp);
                    }
                    insertList(item->x, item->y, item->mac);
                    printf("inserting elem: %d %d\n", item->x, item->y);
                }
                break;
            }
            case IMAGE_PACKET:
            {
                if(posx != 0){
                struct low_res_header* low_header = (struct low_res_header*) pack->buffer;
                printf("image packet recv from %ld\n", low_header->mac);

                struct mac_list * item = searchList(low_header->mac, 0);
                if (item != 0) {
                    printf("item: %d %d, pos %d\n", item->x, item->y, low_header->pos);
                    unsigned char * image_part = ((unsigned char *) pack->buffer) + sizeof (struct low_res_header);
                    //unsigned char * dest_part = myimage + 400 * 300 * 3 * (item->x + posy * item->y) + low_header->pos * 400 * 30 * 3;
                    //printf("inserting in offset: %d\n", 400 * 300 * 3 * (item->x + posy * item->y) + low_header->pos * 400 * 30 * 3);
                    //memcpy(dest_part, image_part, 400 * 30 * 3);
                    unsigned int offsetx = 400*item->y;
                    unsigned int offsety = 300*item->x + low_header->pos*30;
                    printf("offset: x: %d, y: %d %ld\n", offsetx, offsety, low_header->mac);
                    glTexSubImage2D(GL_TEXTURE_2D,
                            0,
                            offsetx,
                            offsety,
                            400,
                            30,
                            GL_RGB,
                            GL_UNSIGNED_BYTE,
                            image_part);
                }
                }
                free(pack->buffer);
                free(pack);

                break;
            }
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
