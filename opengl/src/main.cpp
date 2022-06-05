#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

/*read shader source code from shader file*/
static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream.good())
        std::cout << "Shader file is not exist,will be use default shader." << std::endl;

    enum class ShaderType{
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2]; // create string stream to store shaders
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        // judge shader type
        if (line.find("#shader") != std::string::npos) // npos present invalid char
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        // fill spcify string stream
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int  id = glCreateShader(type);
    const char* src = source.c_str(); // convert string type to char*
    /*shader, count, string, length(each string is assumed to be null terminated)*/
    glShaderSource(id, 1, &src, nullptr);
    /*Compiles a shader object*/
    glCompileShader(id);
    
    /*retrieve shader object parameters*/
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) 
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length*sizeof(char)); // alloca() get void pointer
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

/*use single shader progam, return unique identifier */
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
    unsigned int program = glCreateProgram();
    // create shader object
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    // Attaches a shader object to a program object
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    /*Links a program object*/
    glLinkProgram(program);
    /*Validates a program object*/
    glValidateProgram(program);

    // delete intermediate
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "glew init error." << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    /*save every triangle's vertex position*/
    float positions[] = {
        -0.5f, -0.5f,
        -0.5f, 0.5f,
        0.5f, -0.5f,

        0.5f, 0.5f,
        //-0.5f, 0.5f, //duplicated vertex
        //0.5f, -0.5f, 
    };

    /*save vertex index*/
    unsigned int indices[] = {
        0, 1, 2,
        1, 2, 3
    };

    /*define vertex buffer*/
    unsigned int buffer; // generated buffer id
    glGenBuffers(1, &buffer); // create buffer, size 1
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // select buffer
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW); // specific data for buffer

    /*define vertex index buffer, specify every triangle's vertex*/
    unsigned int ibo; // generated buffer id
    glGenBuffers(1, &ibo); // create buffer, size 1
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // select buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW); // specific data for buffer

    /*To enable and disable a generic vertex attribute array*/
    glEnableVertexAttribArray(0);

	/*specify buffer layout,  size, type, normalized, stride, and pointer*/
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);// pointer can be specified as 0, present nullptr

    /*Create shader code*/
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    
    //std::cout << source.VertexSource << std::endl;
    //std::cout << source.FragmentSource << std::endl;

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    glDeleteProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        //glDrawElements(GL_TRIANGLES, 3, NULL);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}