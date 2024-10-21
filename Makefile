montaje = puntodemontaje

fuse_flags= -D_FILE_OFFSET_BITS=64 -lfuse -pthread

proyectoFUSE: proyectoFUSE.o
	gcc -g -o $@  $^ ${fuse_flags}
	mkdir -p $(montaje)
	
proyectoFUSE.o : proyectoFUSE.c 
	gcc -g -c -o $@  $< ${fuse_flags}

mount: proyectoFUSE
	./proyectoFUSE  $(montaje)

umount:
	fusermount -u $(montaje)
