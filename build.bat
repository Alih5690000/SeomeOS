g++ -g main.cpp -o main
g++ -c api.cpp -o api.o
g++ --shared test.cpp api.o -o test.dll