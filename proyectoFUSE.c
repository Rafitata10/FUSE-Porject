/*
			PROYECTO FUSE
Diseño de un FileSystem mediante FUSE.
En esta programa se implementan las funciones de:
	getattr, readdir, open, read, rename, mkdir, rmdir, create y rm.

Diseñado por Rafael Ramírez Salas, Tercer Curso de Ingeniería de Computadores.
Asignatura de Diseño de Sistemas Operativos, Universidad de Málaga. Curso 2021/22.
*/

#define FUSE_USE_VERSION 30

// Importo mi libería con la structura de mi File System.
#include "proyectoFUSE_lib.h"

// Declaración de la Raíz.
filetype * root;

void initRootDirectory(){
	spBloque.inode_bitmap[1] = 1;
	root = (filetype *) malloc (sizeof(filetype));

	strcpy(root->path, "/");
	strcpy(root->nombre, "/");

	root -> hijos = NULL;
	root -> numHijos = 0;
	root -> padre = NULL;
	root -> numLinks = 2;

	strcpy(root -> type, "directory");

	// Inicializo todo a NULL.
	root->c_time = time(NULL);
	root->a_time = time(NULL);
	root->m_time = time(NULL);
	root->b_time = time(NULL);

	// Doy los permisos de directorio.
	root -> permissions = S_IFDIR | 0777;

	root -> size = 0;
	root->group_id = getgid();
	root->user_id = getuid();

	root -> numero = 2;
	root -> bloques = 0;
}

/* Este método devuelve toda la información necesaria del PATH que se le mande por parámetro,
   devuelve una estrucutra del tipo filetype.
   Por ejemplo, estoy en /BIG.
*/
filetype * filetypeFromPath(char * path){
	// Array donde voy a almacenar el nombre de mi carpeta actual.
	char curr_folder[100];
	// Le sumo dos para  aumentar + 1 el puntero luego y el \0.
	char * path_name = malloc(strlen(path) + 2);
	// Copiamos path a path_name.
	strcpy(path_name, path);

	// Inicializamos un nodo como raíz.
	filetype * curr_node = root;

	fflush(stdin);
	// Estamos en nuestra RAÍZ.
	if(strcmp(path_name, "/") == 0){
		return curr_node;
	}

	// PATH incorrecto.
	if(path_name[0] != '/'){
		// Si no tiene "/" no es un PATH válido, se sale.
		exit(1);
	} else {  // Si es el PATH es correcto, movemos el puntero uno a la derecha.
		// Pasaría a estar en BIG en vez de en /BIG.
		path_name++;
	}

	// Si solo tengo "/" es que soy la raíz.
	if(path_name[strlen(path_name) - 1] == '/'){
		// El \0 indica  el final.
		path_name[strlen(path_name) - 1] = '\0';
	}

	char * index;
	int flag = 0;

	// Mientras que siga habiendo PATH... /BIG/Rafita/Doraemon.
	while(strlen(path_name) != 0){
		// Muevo index hasta el siguiente "/".
		index = strchr(path_name, '/');
		
		// Significa que sigue habiendo más PATH que investigar, tengo que llegar hasta /Doraemon.
		if(index != NULL){
			strncpy(curr_folder, path_name, index - path_name);
			curr_folder[index-path_name] = '\0';
			
			flag = 0;
			for(int i = 0; i < curr_node -> numHijos; i++){
				if(strcmp((curr_node -> hijos)[i] -> nombre, curr_folder) == 0){
					curr_node = (curr_node -> hijos)[i];
					flag = 1;
					break;
				}
			}
			if(flag == 0){
				return NULL;
			}
		// No tengo más PATH que visitar, he llegado hasta el final.
		} else {
			strcpy(curr_folder, path_name);
			flag = 0;
			for(int i = 0; i < curr_node -> numHijos; i++){
				if(strcmp((curr_node -> hijos)[i] -> nombre, curr_folder) == 0){
					curr_node = (curr_node -> hijos)[i];
					return curr_node;
				}
			}
			return NULL;
		}
		// Muevo mi index hacia la derecha.
		path_name = index + 1;
	}
}

// Busca un inodo vacío.
int findFreeInode(){
	for(int i = 2; i < 100; i++){
		if(spBloque.inode_bitmap[i] == '0'){
			spBloque.inode_bitmap[i] = '1';
		}
		return i;
	}
}

// Añade un hijo a la estructura.
void addHijo(filetype * padre, filetype * hijo){
	(padre -> numHijos)++;
	padre -> hijos = realloc(padre -> hijos, (padre -> numHijos) * sizeof(filetype *));
	(padre -> hijos)[padre -> numHijos - 1] = hijo;
}

static int mygetattr(const char *path, struct stat *statit){
	char *pathname;
	pathname = (char *) malloc(strlen(path) + 2);
	strcpy(pathname, path);

	filetype * file_node = filetypeFromPath(pathname);
	if(file_node == NULL){
		return -ENOENT;
	}

	// El dueño del arhivo ó del directorio es el usuario que monta el filesystem.
	statit->st_uid = file_node -> user_id;
	// El grupo del archivo ó del directorio es el mismo que el grupo del usuario que montó el sistema de archivos.
	statit->st_gid = file_node -> group_id;
	// El último acceso al arhivo ó directorio está aquí.
	statit->st_atime = file_node -> a_time;
	// La última modificación del arhivo ó directorio está aquí.
	statit->st_mtime = file_node -> m_time;
	statit->st_ctime = file_node -> c_time;
	statit->st_mode = file_node -> permissions;
	statit->st_nlink = file_node -> numLinks + file_node -> numHijos;
	statit->st_size = file_node -> size;
	statit->st_blocks = file_node -> bloques;

	return 0;
}

static int myreaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
	// Creo primero "." y "..".
	filler(buffer, ".", NULL, 0 );
	filler(buffer, "..", NULL, 0 );

	char * pathname = malloc(strlen(path) + 2);
	strcpy(pathname, path);
	filetype * dir_node = filetypeFromPath(pathname);

	if(dir_node == NULL){
		return -ENOENT;
	} else {
		dir_node->a_time = time(NULL);
		for(int i = 0; i < dir_node->numHijos; i++){
			filler(buffer, dir_node->hijos[i]->nombre, NULL, 0);
		}
	}

	return 0;
}

static int myopen(const char *path, struct fuse_file_info *fi){
	// Pido memoria.
	char * pathname = malloc(sizeof(path) + 1);
	// Copio el "path" que me llega por parámetro, el que quiero abrir, en "pathname".
	strcpy(pathname, path);
	// Recupero todos los datos de ese "pathname".
	filetype * file = filetypeFromPath(pathname);

	return 0;
}

static int myread(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
	char * pathname = malloc(sizeof(path) + 1);
	strcpy(pathname, path);

	filetype * file = filetypeFromPath(pathname);
	if(file == NULL){
		return -ENOENT;
	} else {
		// Como no tenemos implementado la función de write, no podemos leer na'.
	}
	return file->size;
}

static int myrename(const char* from, const char* to) {
	char * pathname = malloc(strlen(from) + 2);
	strcpy(pathname, from);

	char * rindex1 = strrchr(pathname, '/');

	// Directorio ó archivo que vamos a renombrar.
	filetype * file = filetypeFromPath(pathname);

	*rindex1 = '\0';

	char * pathname2 = malloc(strlen(to) + 2);
	strcpy(pathname2, to);

	char * rindex2 = strrchr(pathname2, '/');

	if(file == NULL){
		return -ENOENT;
	}

	// Le sumamos uno al index para saltarnos la "/".
	strcpy(file -> nombre, rindex2 + 1);
	// Copiamos hacia el nuevo PATH.
	strcpy(file -> path, to);

	return 0;
}

static int mymkdir(const char *path, mode_t mode){
	// Busca un nodo libre.
	int index = findFreeInode();

	filetype * new_folder = malloc(sizeof(filetype));

	char * pathname = malloc(strlen(path) + 2);
	strcpy(pathname, path);
	char * rindex = strrchr(pathname, '/');

	strcpy(new_folder -> nombre, rindex + 1);
	strcpy(new_folder -> path, pathname);

	// Obviamos todo los posterior a la última "/".
	*rindex = '\0';

	if(strlen(pathname) == 0){
		strcpy(pathname, "/");
	}
	
	new_folder -> hijos = NULL;
	new_folder -> numHijos = 0;
	
	// Buscamos en nuestra estructura si está el padre.
	new_folder -> padre = filetypeFromPath(pathname);
	new_folder -> numLinks = 2;

	if(new_folder -> padre == NULL){
		return -ENOENT;
	}

	// Añadimos un hijo.
	addHijo(new_folder->padre, new_folder);

	strcpy(new_folder -> type, "directory");

	new_folder->c_time = time(NULL);
	new_folder->a_time = time(NULL);
	new_folder->m_time = time(NULL);
	new_folder->b_time = time(NULL);
	new_folder -> permissions = S_IFDIR | 0777;
	new_folder -> size = 0;
	new_folder->group_id = getgid();
	new_folder->user_id = getuid();
	new_folder -> numero = index;
	new_folder -> bloques = 0;

	return 0;
}

// Es común para archivos y directorios, es rm y rmdir a la vez.
static int myrmTotal(const char * path){
	char * pathname = malloc(strlen(path) + 2);
	strcpy(pathname, path);

	char * rindex = strrchr(pathname, '/');
	char * folder_delete = malloc(strlen(rindex + 1) + 2);

	// Copiamos el PATH entero excepto la "/" primera.
	strcpy(folder_delete, rindex + 1);

	// Obviamos todo los posterior a la última "/".
	*rindex = '\0';

	if(strlen(pathname) == 0){
		strcpy(pathname, "/");
	}

	filetype * padre = filetypeFromPath(pathname);

	if(padre == NULL){
		return -ENOENT;
	}

	// Cuando no tienes hijos, (es una hoja).
	if(padre -> numHijos == 0){
		return -ENOENT;
	}

	filetype * curr_child = (padre -> hijos)[0];

	int index = 0;
	// Hay varios nodos hijos, se va buscando entre ellos al que queremos borrar.
	while(index < (padre -> numHijos)){
		if(strcmp(curr_child -> nombre, folder_delete) == 0){
			break;
		}
		index++;
		curr_child = (padre -> hijos)[index];
	}

	// Si es válido:
	if(index < (padre -> numHijos)){
		if(((padre -> hijos)[index] -> numHijos) != 0){
			// Aún tiene hijos, no hemos llegado al final del PATH.
			return -ENOTEMPTY;
		}
		for(int i = index + 1; i < (padre -> numHijos); i++){
			(padre -> hijos)[i - 1] = (padre -> hijos)[i];
		}
		(padre -> numHijos)--;
	} else {
		return -ENOENT;
	}

	return 0;
}

int mycreate(const char * path, mode_t mode, struct fuse_file_info *fi) {
	// Se busca un inodo libre.
	int index = findFreeInode();

	filetype * new_file = malloc(sizeof(filetype));

	char * pathname = malloc(strlen(path) + 2);
	strcpy(pathname, path);

	char * rindex = strrchr(pathname, '/');

	// Se guarda el nombre del nuevo fichero en nuestro filesystem.
	strcpy(new_file -> path, pathname);
	// (Guardamos rindex + 1 para obviar la primera "/").
	strcpy(new_file -> nombre, rindex + 1);

	// Obviamos todo los posterior a la última "/".
	*rindex = '\0';

	if(strlen(pathname) == 0){
		strcpy(pathname, "/");
	}	

	new_file -> numHijos = 0;
	new_file -> hijos = NULL;

	new_file -> numLinks = 0;
	// Buscamos en nuestra estructura si está el padre.
	new_file -> padre = filetypeFromPath(pathname);

	if(new_file -> padre == NULL){
		return -ENOENT;
	}

	// Añadimos al hijo a nuestra estructura.
	addHijo(new_file->padre, new_file);

	strcpy(new_file -> type, "file");

	new_file -> c_time = time(NULL);
	new_file -> a_time = time(NULL);
	new_file -> m_time = time(NULL);
	new_file -> b_time = time(NULL);

	// Doy los permisos necesarios para un archivo.
	new_file -> permissions = S_IFREG | 0777;

	new_file -> size = 0;
	new_file -> group_id = getgid();
	new_file -> user_id = getuid();
	new_file  -> numLinks = 1;
	new_file -> numero = index;
	
	return 0;
}

static struct fuse_operations operations = {
	.getattr =   mygetattr,
	.readdir =   myreaddir,
	.open    =   myopen,
	.read    =   myread,
	.rename  =	 myrename,

	// DIRECOTORIOS
	.mkdir   =	 mymkdir,
	.rmdir   =   myrmTotal,

	// ARCHIVOS
	.create  = 	 mycreate,
	.unlink  = 	 myrmTotal,
};

/*
Nos hubiera gustado implementar más funcionalidades como el write y el save (para que al hacer umount
no perdiéramos los datos); la idea la tenemos en mente, pero los medios y nuestros conocimientos, 
al igual que el tiempo, no estaban a nuestro favor; no obstante nuestra satisfacción por ver a nuestro 
VFS funcionando es máxima.		: )
*/

int main(int argc, char *argv[]){	
	initSuperbloque();
	initRootDirectory();
	return fuse_main(argc, argv, &operations, NULL);
}
