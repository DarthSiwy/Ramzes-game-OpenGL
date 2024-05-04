#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb.cpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include <ft2build.h>

#include "camera.h"
#include "axis.h"
#include "pyramid.h"
#include "border.h"
#include "functions.h"
#include "player.h"
#include "keyboard.h"

#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <iostream>

#include FT_FREETYPE_H

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

int find_value(std::vector<std::vector<int>>& board, int direction, const Empty_Space& empty_space) {
    int result = 0;
    if (direction == 1) result = board[empty_space.posX-1][empty_space.posY];
    if (direction == 2) result = board[empty_space.posX][empty_space.posY+1];
    if (direction == 3) result = board[empty_space.posX+1][empty_space.posY];
    if (direction == 4) result = board[empty_space.posX][empty_space.posY-1];
    return result;
}

void swap_value(std::vector<std::vector<int>>& board, int direction, Empty_Space& empty_space) {
    int value = 0;
    if (direction == 1) {
        value = board[empty_space.posX - 1][empty_space.posY];
        board[empty_space.posX - 1][empty_space.posY] = 77;
        board[empty_space.posX][empty_space.posY] = value;
        empty_space.posX += -1;
    }
    if (direction == 2) {
        value = board[empty_space.posX][empty_space.posY + 1];
        board[empty_space.posX][empty_space.posY + 1] = 77;
        board[empty_space.posX][empty_space.posY] = value;
        empty_space.posY += 1;
    }
    if (direction == 3) {
        value = board[empty_space.posX + 1][empty_space.posY];
        board[empty_space.posX + 1][empty_space.posY] = 77;
        board[empty_space.posX][empty_space.posY] = value;
        empty_space.posX += 1;
    }
    if (direction == 4) {
        value = board[empty_space.posX][empty_space.posY - 1];
        board[empty_space.posX][empty_space.posY - 1] = 77;
        board[empty_space.posX][empty_space.posY] = value;
        empty_space.posY -= 1;
    }
}

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RamzesGame", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader_main("shader_vertex", "shader_fragment");
    
    Axis axis;
    Border border;

    Pyramid pyramids[47];
    Empty_Space empty_space_on_board(8,6);

    std::vector<std::vector<int>> board;
    make_board(board);

    // TEXTURES
    unsigned int texture1;
    unsigned char* data;
    int width, height, nrChannels;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("resources/textures/wall.jpg", &width, &height, &nrChannels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;
    stbi_image_free(data);  

    int x = 0, z = 0, step = 2, index = 0;    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 6; j++) {
            pyramids[index].move(x, 0, z);
            x += step;
            index++;
            if (index == 47) j = 6;
        }
        z += step;
        x = 0;
    }
    for (int i = 0; i < 47; i++) pyramids[i].move(0, -1.0f, 0);

    // CAMERA POSITION
    camera.Position.x = 14.0f;
    camera.Position.y = 10.0f;
    camera.Position.z = 20.0f;
    camera.Pitch += -40.0f;
    camera.Yaw += -40.0f;
    camera.updateCameraVectors();

    int move_direction = 0;
    int animation = 0;

    int previousKeyState[10], currentKeyState[10];
    for (int i = 0; i < 10; i++) previousKeyState[i] = GLFW_RELEASE;



  // RENDER LOOP      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // RENDER LOOP      ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        shader_main.use();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TIME 
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //camera.DisplayPosition();

        // MATRIX
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // SHADER BIND
        shader_main.setMat4("projection", projection);
        shader_main.setMat4("view", view);
        shader_main.setMat4("model", model);
      
        // RENDER 
        axis.render(shader_main, view, projection);
        border.render(shader_main, view, projection);

        for (int i = 0; i < 47; i++) pyramids[i].render(shader_main, view, projection, model, animation);   

        // KEYBOARD
        updateKeyboardState(window, currentKeyState);

        if (animation == 0){
            if (currentKeyState[1] == GLFW_PRESS && previousKeyState[1] == GLFW_RELEASE) move_direction = 10;
            if (currentKeyState[2] == GLFW_PRESS && previousKeyState[2] == GLFW_RELEASE) move_direction = 30;
            if (currentKeyState[3] == GLFW_PRESS && previousKeyState[3] == GLFW_RELEASE) move_direction = 20;
            if (currentKeyState[4] == GLFW_PRESS && previousKeyState[4] == GLFW_RELEASE) move_direction = 40;
        }

        if (move_direction > 9) {
            move_direction /= 10;
            if (find_value(board, move_direction, empty_space_on_board) > -1) {
                pyramids[find_value(board, move_direction, empty_space_on_board)].move_direction(move_direction, animation);
                swap_value(board, move_direction, empty_space_on_board);
                show_board(board);
            }
            move_direction = 0;
        }

        for (int i = 0; i < 10; i++) previousKeyState[i] = currentKeyState[i];
        
        
        // SWAP BUFFERS
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CAMERA MOVEMENT
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// FRAME BUFFER
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// MOUSE MOVEMENT
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// SCROOL MOUSE ZOOM
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
