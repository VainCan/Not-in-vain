#include <iostream>
#include "../allCRC.h"

using namespace std;

int main()
{
    string strTest = "1.2.392.200036.9116.4.1.6578.135052.1001";
    uint16_t crc = CRC::crc16_x25(strTest.data(), strTest.length());
    printf("%d\n", crc);
    return 0;
}

