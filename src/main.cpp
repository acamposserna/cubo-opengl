/**
 * @brief Cubo giratorio
 * 
 * @details
 * El objetivo principal es comprender el funcionamiento del pipeline de
 * renderizado, cómo se organiza la geometría en memoria de la GPU, cómo se
 * describen los atributos de vértice al driver y cómo los shaders los consumen
 * para producir píxeles en pantalla.
 * 
 * @author Antonio Campos (a.campos.serna@gmail.com)
 * @version 0.1
 * @date 2026-05-21
 * @copyright GNU General Public License v3 (see https://www.gnu.org/licenses/gpl-3.0.html)
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
 * Geometría del cubo
 * 
 * Los lados del cubo tienen longitud 2.0 y el origen de coordenadas está en el centro.
 * Los vértices están en los puntos [-1, 1]³.
 * Cada vértice almacena 6 floats:
 *   [ x       y      z  r    g    b ]
 *     └── posición ──┘  └─ color ─┘
 */
static constexpr float kVertices[] = {
    // Frontal (+Z) ─ Rojo
    -1.f,-1.f, 1.f,  1.f,0.f,0.f,   // 0: inferior-izquierda
     1.f,-1.f, 1.f,  1.f,0.f,0.f,   // 1: inferior-derecha
     1.f, 1.f, 1.f,  1.f,0.f,0.f,   // 2: superior-derecha
    -1.f, 1.f, 1.f,  1.f,0.f,0.f,   // 3: superior-izquierda
    // Trasera (-Z) ─ Verde
    // Visto desde fuera (desde -Z) el CCW invierte X respecto a la frontal
     1.f,-1.f,-1.f,  0.f,1.f,0.f,   // 4
    -1.f,-1.f,-1.f,  0.f,1.f,0.f,   // 5
    -1.f, 1.f,-1.f,  0.f,1.f,0.f,   // 6
     1.f, 1.f,-1.f,  0.f,1.f,0.f,   // 7
    // Izquierda (-X) ─ Azul
    -1.f,-1.f,-1.f,  0.f,0.f,1.f,   // 8
    -1.f,-1.f, 1.f,  0.f,0.f,1.f,   // 9
    -1.f, 1.f, 1.f,  0.f,0.f,1.f,   // 10
    -1.f, 1.f,-1.f,  0.f,0.f,1.f,   // 11
    // Derecha (+X) ─ Amarillo
     1.f,-1.f, 1.f,  1.f,1.f,0.f,   // 12
     1.f,-1.f,-1.f,  1.f,1.f,0.f,   // 13
     1.f, 1.f,-1.f,  1.f,1.f,0.f,   // 14
     1.f, 1.f, 1.f,  1.f,1.f,0.f,   // 15
    // Superior (+Y) ─ Cian
    -1.f, 1.f, 1.f,  0.f,1.f,1.f,   // 16
     1.f, 1.f, 1.f,  0.f,1.f,1.f,   // 17
     1.f, 1.f,-1.f,  0.f,1.f,1.f,   // 18
    -1.f, 1.f,-1.f,  0.f,1.f,1.f,   // 19
    // Inferior (-Y) ─ Magenta
    -1.f,-1.f,-1.f,  1.f,0.f,1.f,   // 20
     1.f,-1.f,-1.f,  1.f,0.f,1.f,   // 21
     1.f,-1.f, 1.f,  1.f,0.f,1.f,   // 22
    -1.f,-1.f, 1.f,  1.f,0.f,1.f,   // 23
};

// El EBO (Element Buffer Object) almacena índices en lugar de vértices.
// Cada cara se descompone en 2 triángulos → 6 índices por cara.
// Total: 6 caras × 6 índices = 36 índices.
// El patrón (i, i+1, i+2, i, i+2, i+3) genera los dos triángulos por cara.
static constexpr unsigned int kIndices[] = {
     0, 1, 2,  0, 2, 3,   // Frontal
     4, 5, 6,  4, 6, 7,   // Trasera
     8, 9,10,  8,10,11,   // Izquierda
    12,13,14, 12,14,15,   // Derecha
    16,17,18, 16,18,19,   // Superior
    20,21,22, 20,22,23,   // Inferior
};

/**
 * El Logger lo hacemos global a todo el programa.
 */
Logger console_log(LogLevel::TRACE, LogMode::CONSOLE, "");


/**
 * Utilidades para la gestión de los shaders:
 * 
 * 1. readFile: Leer fichero de texto con el código del shader.
 * 2. compileShader: Compila el shader.
 * 3. createProgram: Enlaza los shaders compilados y crea el programa para la GPU.
 */

/**
 * @brief Leer fichero de texto.
 * 
 * @details
 * Lee el fichero de texto que se le pasa por parámetro y devuelve todo el contenido en
 * una cadena <code>std::string</code>.
 * 
 * Si se produce un error en la lectura del fichero devuelve un objeto vacío.
 * 
 * @param path Fichero de texto para leer.
 * @return std::string Contenido del fichero.
 */
static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f) {
        console_log.log(LogLevel::ERROR, "Error abriendo el fichero: " + std::string(path));
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

/**
 * @brief Compilación de un shader.
 * 
 * @details
 * Se lee el código del shader del fichero que se pasa por parámetro y se compila.
 * 
 * Se comprueba si ha habido algún error en la copilación con <code>glGetShaderiv</code>.
 * 
 * @param type Tipo del shader GL_VERTEX_SHADER o GL_FRAGMENT_SHADER.
 * @param path Fichero con el código del shader.
 * @return GLuint Identificador del shader compilado.
 */
static GLuint compileShader(GLenum type, const char* path) {
    std::string src  = readFile(path);
    const char* csrc = src.c_str();

    GLuint shader = glCreateShader(type);      // reserva objeto en la GPU
    glShaderSource(shader, 1, &csrc, nullptr); // carga el código fuente
    glCompileShader(shader);                   // compila en la GPU

    // Comprobamos errores de compilación
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        console_log.log(LogLevel::ERROR, "Error en el shader ("
            + std::string(path) + "): "
            + std::string(log)
        );
    }
    return shader;
}

/**
 * @brief Crea el programa para ejecutar en la GPU.
 * 
 * @details
 * Enlaza vertex shader + fragment shader en un programa ejecutable por la GPU.
 * 
 * Tras el enlazado, los objetos shader individuales ya no son necesarios.
 * 
 * @param vertPath Fichero con el código fuente del vertex shader.
 * @param fragPath Fichero con el código fuente del fragment shader.
 * @return GLuint ID del programa.
 */
static GLuint createProgram(const char* vertPath, const char* fragPath) {
    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertPath);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragPath);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);   // conecta salidas del vert con entradas del frag

    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        console_log.log(LogLevel::ERROR, "Error en el linkado: " + std::string(log));
    }

    // Los shaders ya están linkados en el programa; liberamos los objetos.
    glDeleteShader(vert);
    glDeleteShader(frag);

    return prog;
}

/**
 * @brief Programa principal.
 * 
 * @return int Código de salida del programa.
 */
int main() {

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

    // Hacemos que las llamadas OpenGL siguientes operen sobre esta ventana
    glfwMakeContextCurrent(window);

    // Inicializamos GLAD
    // OpenGL no expone sus funciones directamente: hay que consultar al driver
    // la dirección de cada una en tiempo de ejecución. GLAD automatiza esto
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        console_log.log(LogLevel::ERROR, "Error inicializando GLAD.");
        return -1;
    }

    // Área de dibujo para OpenGL
    glViewport(0, 0, kWidth, kHeight);

    // Activamos Z-Buffer para descartar los fragmentos que están tapados
    // por otra geometría más cercana a la cámara
    glEnable(GL_DEPTH_TEST);

    // Activamos Back-face culling para evitar que se envíen a renderizar
    // los triángulos cuya normal apunta en el sentido contrario a la cámara
    glEnable(GL_CULL_FACE);

    // Crear el programa para la GPU. Carga, compila y linka los shaders.
    GLuint program = createProgram("shaders/cube.vert", "shaders/cube.frag");

    // TODO: Crear y configurar VAO / VBO / EBO

    // TODO: Matrices de cámara (moodel, view, projection)

    // Bucle principal de renderizado
    while (!glfwWindowShouldClose(window)) {
        // TODO: Cálculo de matrices

        // TODO: Envío de matrices a los shaders

        // Intercambio de buffers de visualización
        glfwSwapBuffers(window);

        // Procesamiento de eventos del S.O.
        glfwPollEvents();
    }

    // TODO: Limpieza de recursos
    // Cerramos la ventana
    glfwDestroyWindow(window);
    glfwTerminate();

    console_log.log(LogLevel::INFO, "Fin del programa.");
    return 0;
}