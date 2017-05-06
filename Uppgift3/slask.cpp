#include <tgmath.h>
#include <iostream>

//
// Created by totalorder on 5/4/17.
//
int smallestN(int n) {
    int sumOfSquares = 0;
    for (int i = 1; i < n+1; i++) {
        sumOfSquares += i*i;
    }
    return std::floor(std::sqrt(sumOfSquares));
}

int main(int argc, char* argv[]) {


    std::cout << smallestN(10) <<  std::endl;
}