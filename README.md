# Cubo Giratorio

## Introducción

Este proyecto es una aplicación básica, con fines didácticos, de renderizado de imagen. Renderiza un cubo, con cada cara de un color distinto, que gira continuamente. Utilizamos la API gráfica OpenGL 4.6 con el pipeline programable (shaders).

El objetivo principal es comprender el funcionamiento del pipeline de renderizado, cómo se organiza la geometría en memoria de la GPU, cómo se describen los atributos de vértice al driver y cómo los shaders los consumen para producir píxeles en pantalla.

No se usa iluminación ni texturizado. El color de cada cara proviene directamente de los datos de vértice, lo que permite centrarse exclusivamente en la mecánica del pipeline gráfico.

---

## Stack Tecnológico

| Componente          | Tecnología       | Rol en el proyecto                                              |
|---------------------|------------------|-----------------------------------------------------------------|
| Lenguaje            | C++23            | Lógica de la aplicación, gestión de recursos                    |
| API gráfica         | OpenGL 4.6       | Comunicación con la GPU                                         |
| Lenguaje shaders    | GLSL 4.60        | Programas que se ejecutan en la GPU (vertex y fragment shader)  |
| Ventana y contexto  | GLFW             | Creación de ventana multiplataforma y contexto OpenGL           |
| Carga de extensiones| GLAD             | Resolución en tiempo de ejecución de las funciones de OpenGL    |
| Matemáticas         | GLM              | Vectores y matrices compatibles con GLSL (column-major)         |
| Sistema de build    |  Make            | Configuración y compilación                                     |

### ¿Por qué GLAD?

OpenGL no es una librería enlazable directamente: sus funciones las exporta el driver gráfico instalado en el sistema. GLAD genera código C que consulta al driver la dirección de cada función en tiempo de ejecución y la almacena en un puntero, de modo que el resto del código puede llamarlas.

---

## Estructura de Carpetas

```
cubo-opengl/
├── Makefike             # Fichero para compilación con make
├── README.md            # Este fichero
│
├── src/
│   ├── Logger/
│   │   └── Logger.cpp   # Código de la clase Logger para escritura de logs
│   ├── main.cpp         # Código fuente principal (toda la lógica)
│   └── glad.c           # Código generado por GLAD (NO INCLUIDO EN ESTE REPOSITORIO)
│
├── shaders/
│   ├── cube.vert        # Vertex shader: transforma posiciones
│   └── cube.frag        # Fragment shader: determina el color final
│
└── include/
    ├── Logger/
    │   └── Logger.hpp   # Cabecera de la clase Logger
    ├── glad/
    │   └── glad.h       # Cabecera generada por GLAD (NO INCLUIDA EN ESTE REPOSITORIO)
    └── KHR/
        └── khrplatform.h  # Tipos portables de Khronos
```

> **Nota:** GLM y GLFW se presuponen instalados en el sistema (`/usr/include`) y no se incluyen en el repositorio.

> **Nota2:** El fichero glad.c no se incluye en este repositorio, debe descargarse desde https://glad.dav1d.de/

---

## Descripción del Código (`main.cpp`)

### 1. Geometría del cubo

El cubo tiene lados de longitud 2.0, y está centrado en el origen. Sus vértices están en el rango [-1, 1]³.

**¿Por qué 24 vértices y no 8?**

Un cubo tiene 8 esquinas, pero cada esquina pertenece a 3 caras de colores distintos. Como el color es un atributo de vértice, esa esquina necesita tres entradas independientes en el array (una por cara). Por tanto: **4 vértices × 6 caras = 24 vértices**.

Cada vértice ocupa 6 `float` en memoria:

```
[ x       y      z  r    g    b ]
  └── posición ──┘  └─ color ─┘
```

**Winding order (orden de vínculo)**

Los vértices de cada cara se definen en sentido **antihorario (CCW)** vistos desde el exterior del cubo. Esto permite activar `GL_CULL_FACE`: la GPU identifica automáticamente las caras traseras (cuya normal apunta en sentido contrario a la cámara) y las descarta sin procesarlas.

```
  3 ──── 2      Triángulo 1: (0, 1, 2)
  │   /  │      Triángulo 2: (0, 2, 3)
  │ /    │
  0 ──── 1
```

#### 1.1. Objetos de geometría en OpenGL

En **OpenGL** se utilizan los siguientes objetos para almacenar la geometría de los modelos:

**VBO (Vertex Buffer Object)**

En este bloque de memoria VRAM de la GPU se almacenan los datos de todos los vértices del cubo, tanto la posición como el color.

```
// Frontal (+Z) ─ Rojo
-1.f,-1.f, 1.f,  1.f,0.f,0.f,   // 0: inferior-izquierda
 1.f,-1.f, 1.f,  1.f,0.f,0.f,   // 1: inferior-derecha
 1.f, 1.f, 1.f,  1.f,0.f,0.f,   // 2: superior-derecha
-1.f, 1.f, 1.f,  1.f,0.f,0.f,   // 3: superior-izquierda
...
```

**EBO (Element Buffer Object)**

En lugar de repetir vértices, el EBO almacena 36 índices que indican el orden en que la GPU debe leer el VBO para formar los 12 triángulos (2 por cara):

```
 0, 1, 2,  0, 2, 3,   // Frontal
 4, 5, 6,  4, 6, 7,   // Trasera
 ...
```

**VAO (Vertex Array Object)**

Objeto que almacena que VBO/EBO están activos y cómo se deben interpretar sus datos, con solo llamar a `glBindVertexArray` no en necesario configurar nada más.

### 2. Pipeline de inicialización

```
glfwInit()
  │
  ├─ glfwCreateWindow()       → ventana del SO + contexto OpenGL
  │
  ├─ gladLoadGLLoader()       → resuelve funciones OpenGL en el driver
  │
  ├─ glEnable(DEPTH_TEST)     → z-buffer activo
  ├─ glEnable(CULL_FACE)      → descartar caras traseras
  │
  ├─ createProgram()          → compila y enlaza los shaders en la GPU
  │
  └─ VAO/VBO/EBO setup        → sube geometría a VRAM y describe su layout
```

### 3. VAO / VBO / EBO

Como hemos visto en el punto **1.1**, estos tres objetos conforman la pieza central del pipeline de OpenGL:

| Objeto | Qué almacena |
|--------|-------------|
| **VBO** | Los datos de vértice (posición + color) en VRAM |
| **EBO** | Los índices que definen los triángulos |
| **VAO** | La configuración: qué VBO/EBO usar y cómo interpretar cada atributo |

`glVertexAttribPointer` es la llamada clave: describe a la GPU cómo leer un atributo del VBO indicando su índice, número de componentes, tipo, stride (tamaño total del vértice en bytes) y offset desde el inicio del vértice:

```
Vértice en memoria (stride = 24 bytes):
┌──────────────┬──────────────┐
│  aPos (12 B) │ aColor (12 B)│
│  offset = 0  │ offset = 12  │
└──────────────┴──────────────┘
```

### 4. Transformación MVP

La posición de cada vértice pasa por tres transformaciones encadenadas:

```
gl_Position = Proyección × Vista × Modelo × posición_local
```

| Matriz | Función | En este proyecto |
|--------|---------|-----------------|
| **Modelo** | Espacio local → espacio mundo | Rotación compuesta sobre Y (45°/s) y X (22.5°/s), varía cada fotograma |
| **Vista** | Espacio mundo → espacio cámara | Cámara fija en (0, 1.5, 5) mirando al origen |
| **Proyección** | Espacio cámara → clip space | Perspectiva 45°, aspecto 4:3 |

Las matrices se envían al shader como **uniforms**: variables globales que la CPU escribe y todos los vértices/fragmentos leen.

### 5. Bucle de renderizado

```
while (ventana abierta) {
    calcular dt (delta time)
    angle += 45°/s × dt          // rotación independiente de los FPS
    limpiar color buffer + depth buffer
    calcular matriz modelo con el ángulo actual
    enviar matrices MVP al shader
    glDrawElements(36 índices)   // dibuja el cubo
    glfwSwapBuffers()            // muestra el frame (double buffering)
    glfwPollEvents()             // procesa input y eventos del SO
}
```

**Delta time**: calcular el ángulo en función del tiempo transcurrido (no del número de fotogramas) garantiza que el cubo gire a exactamente 45°/s con independencia de los FPS de la máquina.

**Double buffering**: mientras la GPU dibuja en el *back buffer*, el *front buffer* se muestra en pantalla. `glfwSwapBuffers` los intercambia al final de cada frame, evitando el tearing.

### 6. Limpieza

Al salir del bucle se liberan explícitamente todos los recursos de la GPU (`glDelete*`) y se destruye la ventana. Aunque el sistema operativo los recuperaría al terminar el proceso, liberarlos en orden correcto es una buena práctica que facilita la depuración.

---

## Compilación

La compilación se realiza con la herramienta `make` y el fichero `Makefile`.

### Compilar

Desde la raiz del proyecto:

```bash
make clean
make
```

El ejecutable se genera en `build/cubo-opengl`.

### Ejecutar

```bash
./build/cubo-opengl
```

> El ejecutable debe lanzarse desde la raíz del proyecto (o desde `build/`), ya que busca los shaders en la ruta relativa `shaders/`. El `Makefile` copia automáticamente la carpeta `shaders/` al directorio de build durante la compilación.


