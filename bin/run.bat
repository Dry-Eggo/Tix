echo Compiling...

clear &&
make &&
clear &&
gcc -c ../Eggo/std/backend.c -o ../Eggo/std/tixclib.o &&
echo Runnning &&
./tix -s ../examples/main.tix -o main &&
gcc main.o ../Eggo/std/std_h.o ../Eggo/std/tixclib.o -no-pie -o exit
