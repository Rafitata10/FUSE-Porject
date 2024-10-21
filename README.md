# Proyecto FUSE - Diseño de un Sistema de Archivos

Este proyecto implementa un sistema de archivos utilizando la biblioteca FUSE (Filesystem in Userspace). Las funcionalidades incluyen operaciones básicas de un sistema de archivos, como obtener atributos de archivos, listar directorios, crear, eliminar y leer archivos y directorios.

## Descripción

Este sistema de archivos fue desarrollado como parte de la asignatura de **Diseño de Sistemas Operativos** en la **Universidad de Málaga** durante el curso 2021/2022. Implementa las siguientes operaciones:
- `getattr`: Obtener atributos de archivos/directorios.
- `readdir`: Listar contenido de un directorio.
- `open`: Abrir un archivo.
- `read`: Leer datos de un archivo.
- `rename`: Renombrar archivos o directorios.
- `mkdir`: Crear un directorio.
- `rmdir`: Eliminar un directorio.
- `create`: Crear un archivo.
- `rm`: Eliminar un archivo.

### Estructura de Archivos
El proyecto incluye los siguientes archivos:
- **proyectoFUSE.c**: Implementación de las operaciones del sistema de archivos.
- **proyectoFUSE_lib.h**: Definición de las estructuras y constantes necesarias.
- **Makefile**: Instrucciones para compilar y montar el sistema de archivos.

## Requisitos

- `libfuse` (Biblioteca FUSE)
- `gcc` (Compilador de C)

## Compilación y Ejecución

1. Clonar el repositorio en tu máquina local:
    ```bash
    git clone <url-del-repositorio>
    cd <nombre-del-repositorio>
    ```
2. Compilar el proyecto usando `make`:
    ```bash
    make
    ```
3. Montar el sistema de archivos en el punto de montaje:
    ```bash
    make mount
    ```
4. Para desmontar el sistema de archivos:
    ```bash
    make umount
    ```

## Estructuras Principales

- **inodo**: Representa los inodos del sistema de archivos, incluyendo punteros a bloques de datos y metadatos de archivos.
- **superBloque**: Contiene la información general del sistema de archivos, como el mapa de bits para bloques e inodos.
- **filetype**: Define las propiedades de cada archivo o directorio, incluyendo su ruta, permisos y tiempo de creación.

## Autores

- Rafael Ramírez Salas

Desarrollado para la asignatura de Diseño de Sistemas Operativos en la Universidad de Málaga.
