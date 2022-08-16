#pragma once
#include<iostream>
class Mesh
{
public:
    // OpenGL 对象
    GLuint vao, vbo, ebo;
    GLuint diffuseTexture;  // 漫反射纹理

    // 顶点属性
    std::vector<glm::vec3> vertexPosition;
    std::vector<glm::vec2> vertexTexcoord;
    std::vector<glm::vec3> vertexNormal;

    // glDrawElements 函数的绘制索引
    std::vector<int> index;

    Mesh() {}
    void bindData()
    {
        // 创建顶点数组对象
        glGenVertexArrays(1, &vao); // 分配1个顶点数组对象
        glBindVertexArray(vao);  	// 绑定顶点数组对象

        // 创建并初始化顶点缓存对象 这里填NULL 先不传数据
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            vertexPosition.size() * sizeof(glm::vec3) +
            vertexTexcoord.size() * sizeof(glm::vec2) +
            vertexNormal.size() * sizeof(glm::vec3),
            NULL, GL_STATIC_DRAW);

        // 传位置
        GLuint offset_position = 0;
        GLuint size_position = vertexPosition.size() * sizeof(glm::vec3);
        glBufferSubData(GL_ARRAY_BUFFER, offset_position, size_position, vertexPosition.data());
        glEnableVertexAttribArray(0);   // 着色器中 (layout = 0) 表示顶点位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset_position));
        // 传纹理坐标
        GLuint offset_texcoord = size_position;
        GLuint size_texcoord = vertexTexcoord.size() * sizeof(glm::vec2);
        glBufferSubData(GL_ARRAY_BUFFER, offset_texcoord, size_texcoord, vertexTexcoord.data());
        glEnableVertexAttribArray(1);   // 着色器中 (layout = 1) 表示纹理坐标
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset_texcoord));
        // 传法线
        GLuint offset_normal = size_position + size_texcoord;
        GLuint size_normal = vertexNormal.size() * sizeof(glm::vec3);
        glBufferSubData(GL_ARRAY_BUFFER, offset_normal, size_normal, vertexNormal.data());
        glEnableVertexAttribArray(2);   // 着色器中 (layout = 2) 表示法线
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset_normal));

        // 传索引到 ebo
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
    void draw(Shader& shader)
    {
        glBindVertexArray(vao);

        // 传纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        shader.setInt("texture1", 0);
        // 绘制
        glDrawElements(GL_TRIANGLES, this->index.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }
};

