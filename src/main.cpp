/**
 * Cubo giratorio
 * 
 * El objetivo principal es comprender el funcionamiento del pipeline de
 * renderizado, cómo se organiza la geometría en memoria de la GPU, cómo se
 * describen los atributos de vértice al driver y cómo los shaders los consumen
 * para producir píxeles en pantalla.
 */

// GLAD debe incluirse ANTES que GLFW: carga las direcciones de las funciones
// de OpenGL en tiempo de ejecución. Sin él, glClear, glDrawElements, etc.
// serían símbolos sin resolver.
#include <glad/glad.h>

// GLFW gestiona la ventana del sistema operativo y el contexto OpenGL.
#include <GLFW/glfw3.h>

// GLM es una librería matemática pensada para gráficos (columna mayor, igual
// que GLSL). Usamos vec3, mat4 y las funciones perspective/lookAt/rotate.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>   // glm::value_ptr → puntero raw para uniform

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>

#include <Logger/Logger.hpp>

/**
 * Contantes
 */

// Tamaño de la ventana 
static constexpr int   kWidth    = 800;
static constexpr int   kHeight   = 600;

/**
 * @brief Programa principal.
 * 
 * @return int Código de salida del programa.
 */
int main() {
    Logger console_log(LogLevel::TRACE, LogMode::CONSOLE, "");

    console_log.log(LogLevel::INFO, "Inicio del programa.");

    // Inicializamos GLFW
    if (!glfwInit()) {
        console_log.log(LogLevel::ERROR, "Error inicializando GLFW.");
        return -1;
    }

    // Creamos el contexto OpenGL 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Creamos la ventana
    GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "Cubo Giratorio",
                                          nullptr, nullptr);   

    if (!window) {
        console_log.log(LogLevel::ERROR, "Error creando la ventana de GLFW.");
        glfwTerminate();
        return -1;
    }

    // Vinculamos las llamadas a OpenGL con la ventana
    glfwMakeContextCurrent(window);

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cerramos la ventana
    glfwDestroyWindow(window);
    glfwTerminate();

    console_log.log(LogLevel::INFO, "Fin del programa.");
    return 0;
}