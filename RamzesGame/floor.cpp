#include "shader_s.h"
#include "floor.h"

#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <iostream>

Floor::Floor(int x, int y, int z, int w): x1(x), z1(y), x2(z), z2(w) {
    // constructor implementation
}

Floor::Floor() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void Floor::render(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, glm::mat4& model) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    model = glm::mat4(1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindVertexArray(VAO);

    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 12);
}


Floor::~Floor() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}


