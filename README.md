Для сборки проекта используется cmake.

Последовательность команд при сборке на Windows:

mkdir build

cd build

cmake ..

cmake --build .

.\Debug\MyProgram.exe


Последовательность команд при сборке на Linux:

mkdir build

cd build

cmake ..

make

./MyProgram

ВАЖНО: при выполнении программы файлы document.txt и kits.txt должны находиться в папке, из которой происходит выполнение, в случае выше - из папки build. В корневой папке содержатся примеры файлов для использования.
