
cmake ..
make
nasm -felf64 t.s -o t.o -g -dwarf
nasm -felf64 ../rt/texit.s -o texit.o
nasm -felf64 ../rt/start.s -o start.o
ar rcs libtix.a start.o texit.o
ld t.o libtix.a -o t
