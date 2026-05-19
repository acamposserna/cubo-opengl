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
│   ├── main.cpp         # Código fuente principal (toda la lógica)
│   └── glad.c           # Código generado por GLAD (NO EDITAR)
│
├── shaders/
│   ├── cube.vert        # Vertex shader: transforma posiciones
│   └── cube.frag        # Fragment shader: determina el color final
│
└── include/
    ├── glad/
    │   └── glad.h       # Cabecera generada por GLAD
    └── KHR/
        └── khrplatform.h  # Tipos portables de Khronos
```

> **Nota:** GLM y GLFW se presuponen instalados en el sistema (`/usr/include`) y no se incluyen en el repositorio.

---
