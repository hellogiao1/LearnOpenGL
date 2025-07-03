#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Utility/assertions.h"
#include "Utility/FileSystemUtils.h"
#include "Utility/logging.h"

class Shader
{
public:
    unsigned int ID = 0;

    bool fileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();  // 检查流状态
    }

    bool CheckFileExistAndReplaceRightPath(std::string& OutPath)
    {
        std::string extension = FileSystem::GetFileExtension(OutPath);
        std::string filenameWithoutExtension = FileSystem::GetFileNameWithoutExtension(OutPath);
        std::string directoryPath = FileSystem::GetDirectoryPath(OutPath);
        if (extension == "vs")
        {
            extension = ".vert";
        }
        else if (extension == "fs")
        {
            extension = ".frag";
        }
        else if (extension == "gs")
        {
            extension = ".geom";
        }

        std::string fullPath = directoryPath + filenameWithoutExtension + extension;

        if (fileExists(OutPath)) {
            // 文件存在
            return true;
        }

        std::string validPath = "Shader/";
        validPath.append(fullPath);
        if (!fileExists(validPath))
        {
            CHECK(0);
            return false;
        }
        OutPath = validPath;
        return true;
    }

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        std::string vertexPathStr = vertexPath ? std::string(vertexPath) : "";
        CheckFileExistAndReplaceRightPath(vertexPathStr);

        std::string fragmentPathStr = fragmentPath ? std::string(fragmentPath) : "";
        CheckFileExistAndReplaceRightPath(fragmentPathStr);

        std::string geometryPathStr = geometryPath ? std::string(geometryPath) : "";
        if (geometryPath)
        {
            CheckFileExistAndReplaceRightPath(geometryPathStr);
        }
        
        generateShaderProgram(vertexPathStr.data(), fragmentPathStr.data(), geometryPathStr.data());
    }

    void generateShaderProgram(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
    {
        internal_generateShaderProgram(vertexPath, GL_VERTEX_SHADER);
        internal_generateShaderProgram(fragmentPath, GL_FRAGMENT_SHADER);
        internal_generateShaderProgram(geometryPath, GL_GEOMETRY_SHADER);
    }

    void internal_generateShaderProgram(const char* vertexPath, GLenum type)
    {
        if (!vertexPath or std::string(vertexPath).empty())
            return;
        
         // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::ifstream vShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            std::stringstream vShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            CHECK(0);
        }
        const char* vShaderCode = vertexCode.c_str();
        // 2. compile shaders
        unsigned int vertex;
        // vertex shader
        vertex = glCreateShader(type);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // shader Program
        if (ID == 0)
            ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
    }

    // print error info
    void printError() const
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
            CHECK(0);
        }
    }
    
    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        printError();
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        printError();
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        printError();
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& trans = glm::mat4(1.f)) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(trans));
        printError();
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, float value1, float value2, float value3) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3);
        printError();
    }
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
        printError();
    }
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                CHECK(0);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                CHECK(0);
            }
        }
    }
};
#endif