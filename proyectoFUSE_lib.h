/*
			LIBRERIA PROYECTO FUSE
Diseño de un FileSystem mediante FUSE.
En esta librería se implementa la estructura del sistema, en forma de árbol.

Diseñado por Rafael Ramírez Salas y Sofía Barril, Tercer Curso de Ingeniería de Computadores.
Asignatura de Diseño de Sistemas Operativos, Universidad de Málaga. Curso 2021/22.
*/

#ifndef __PROYECTO_FUSE__
#define __PROYECTO_FUSE__

#include <fuse.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

// Tamaño de los bloques.
#define block_size 1024

// Estructura Inodo, que contiene información y punteros para los datos almacenados.
typedef struct inodo{
	// Número de data block al que el inicio apunta.
	int databloques[16];
	// Número de link.
	int numero;
	// Número de bloques a los que apunta el inodo particular.
	int bloques;
	// Tamaño del file ó directory.	
	int size;
} inodo;

typedef struct superBloque{
	// Número total de data bloques.
	char databloques[block_size * 100];
	// Array de los números de data block disponibles.
	char data_bitmap[105];
	// Array de los números de inodo que hay disponibles.
	char inode_bitmap[105];
} superBloque;

superBloque spBloque;

typedef struct filetype{
	// PATH.
	char path[100];
	// Nombre del fichero dir.
	char nombre[100];

	// Puntero a la estrucutra hijo.
	struct filetype ** hijos;
	// Número de hijos.
	int numHijos;
	// Número de links.
	int numLinks;
	// Puntero a la estrucutra padre.
	struct filetype * padre;
	// Extensión del file.
	char type[20];
	// Tamaño del nodo.
	off_t size;	
	// Permisos.
	mode_t permissions;
	// ID del usuario.
	uid_t user_id;
	// ID del group.
	gid_t group_id;
	// Tiempo de aceso.
	time_t a_time;
	// Tiempo de modificación.
	time_t m_time;
	// Tiempo de cambio de Status.
	time_t c_time;
	// Tiempo de Creación.
	time_t b_time;

	int numero;
	int bloques;
	int databloques[16];

} filetype;

void initSuperbloque(){
	memset(spBloque.data_bitmap, '0', 100 * sizeof(char));
	memset(spBloque.inode_bitmap, '0', 100 * sizeof(char));
}

#endif
