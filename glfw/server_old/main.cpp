#include <iostream>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other includes
#include "MyShader.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Imaging Surface Device Visual Debugging", nullptr, nullptr);
    //glfwMakeContextCurrent(window);
printf("A\n");
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    //glewExperimental = GL_TRUE;
    //GLenum err = glewInit();
    //if (err != GLEW_OK) {
        //Problem: glewInit failed, something is seriously wrong.
    //    printf("glewInit failed, aborting.");
    //}
printf("B\n");
    // Define the viewport dimensions
    GLCHK(glViewport(0, 0, WIDTH, HEIGHT));
    GLuint picture;
    printf("C\n");
    glGenTextures(1, &picture);
    printf("D\n");
    glBindTexture(GL_TEXTURE_2D, picture);
    printf("E\n");
   // GLCHK(glGenFramebuffers(1, &fb));
    //GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, fb));
    //GLCHK(glGenTextures(1, &picture));

    // Build and compile our shader program
    MyShader ourShader("../vertex.vs", "../fragment.frag");
    printf("F\n");
    ourShader.Use();
    printf("G\n");
    GLint pic_loc = glGetUniformLocation(ourShader.Program, "picture");
    GLint vertex = glGetAttribLocation(ourShader.Program, "vertex");
    printf("H\n");
    
    GLCHK(glEnableVertexAttribArray(vertex));
        GLfloat varray[] = {
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f,

            -1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, -1.0f,
        };
        GLCHK(glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, varray));
    //GLint texcoord = glGetAttribLocation(ourShader.Program, "texcoord");


    // Set up vertex data (and buffer(s)) and attribute pointers
    /*GLfloat vertices[] = {
        // Positions         // Colors
        1.0f, -1.0f, 0.0f, 
       -1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);*/

    // Position attribute
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    //glEnableVertexAttribArray(0);
    // Color attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(1);

    //glBindVertexArray(0); // Unbind VAO
    
    unsigned char *image = (unsigned char *) malloc(WIDTH*HEIGHT*3);
    for(int i = 0; i < (WIDTH*HEIGHT*3); i++){
        image[i] = 128;
    }
    printf("locations: %d %d\n", pic_loc, vertex);
    
    GLCHK(glUniform1i(pic_loc, 0));

    GLCHK(glBindTexture(GL_TEXTURE_2D, picture));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCHK(glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WIDTH, HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, image));

    GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,picture,0));
    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        GLCHK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GLCHK(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
        


        // Draw the triangle
        ourShader.Use();
        //GLCHK(glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WIDTH, HEIGHT, 0,GL_RGB, GL_UNSIGNED_BYTE, 0));
        GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, image));

        //glBindVertexArray(VAO);
        GLCHK(glEnableVertexAttribArray(vertex));
        GLfloat varray[] = {
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f,

            -1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, -1.0f,
        };
        GLCHK(glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, varray));
        GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        GLCHK(glBindVertexArray(0));

        // Swap the screen buffers
        GLCHK(glfwSwapBuffers(window));
    }
    // Properly de-allocate all resources once they've outlived their purpose
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
