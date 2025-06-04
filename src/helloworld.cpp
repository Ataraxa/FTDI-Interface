#include <iostream>
#include <random>

int randomInt() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(-6, 6);
    return dist(gen);
}

int main() {
    for (int i = 0; i < 10; i++) {
        std::cout << randomInt()/2 << std::endl;
    }
    return 0;
}