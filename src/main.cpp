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

/**
 * Contantes
 */
static constexpr int   kWidth    = 800;
static constexpr int   kHeight   = 600;

/**
 * Sistema de log
 */

// Niveles de log
enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief Convierte un nivel de log a texto.
 * 
 * @param level Código del nivel de log.
 * @return const char* Texto del nivel de log.
 */
const char* logLevelToString(LogLevel level) {
    switch (level)
    {
    case TRACE: return "TRACE";
    case DEBUG: return "DEBUG";
    case INFO: return "INFO";
    case WARN: return "WARN";
    case ERROR: return "ERROR";
    case FATAL: return "FATAL";
    default: throw std::invalid_argument("Unknown log level");
    }
}

// Nivel de mínimo de log. Los mensajes de nivel inferior no se envian a la salida.
LogLevel minLogLevel = TRACE;

/**
 * @brief Escribe un mensaje de log.
 * 
 * Escribe por la salida de error <code>std::cerr</code> un mensaje de log.
 * Los mensajes de nivel inferior <code>minLogLevel</no se escribren.
 * 
 * @param level Nivel del mensaje de log.
 * @param text Texto del mensaje de log.
 */
static void log(LogLevel level, const char* text) {
    if (level >= minLogLevel) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        auto now = std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
    
        std::cerr << "[" << now << "] " << logLevelToString(level) << ": "
                  << text << std::endl;

    }
}

/**
 * @brief Programa principal.
 * 
 * @return int Código de salida del programa.
 */
int main() {
    log(TRACE, "Inicio del programa.");

    // Inicializamos GLFW
    if (!glfwInit()) {
        log(ERROR, "Error inicializando GLFW.");
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
        log(ERROR, "Error creando la ventana de GLFW.");
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

    log(TRACE, "Fin del programa.");
    return 0;
}