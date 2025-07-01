#include <iostream>
#include <cstdint>
#include <bitset>

int main() 
{
    uint8_t buf1 = 0xF6;
    uint8_t buf2 = 0x80;
    uint8_t buf3 = 0xA0;
    uint16_t data = 0x0000;
    std::cout << std::bitset<8>(buf1) << "|" << std::bitset<8>(buf2) << "|" << std::bitset<8>(buf3) << std::endl;

    data = data | (buf3 >> 6);
    data = data | (buf2 << 2);
    data = data | (buf1 << 10);

    std::cout << std::bitset<16>(data) << std::endl;
    return 0;
}