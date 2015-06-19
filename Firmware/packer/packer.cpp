// arrays example
#include <iostream>
#include <bitset>
using namespace std;

char input [] = { 0b01101000, 0b01100101, 0b01101100, 0b01101100, 0b01101111 };
char msg[128];
char decode[128] = {};
int len;

unsigned char c = 0;
unsigned char w  = 0;
int n = 0;
int shift = 0;
int x = 0;

int main () {
  for ( n=0 ; n<5 ; ++n ) {
    c = input[n] & 0b01111111;
    c >>= shift;
    w = input[n+1] & 0b01111111;
    w <<= (7-shift);
    shift +=1;
    c = c | w;
    if (shift == 7) {
        shift = 0x00;
        n++;
    }
    x = strlen(decode);
    decode[x] = c;
    decode[x+1] = 0;
 //   std::cout << std::bitset<8>(c);

    std::cout << std::bitset<8>(decode[n]);
    cout << '\n';
  }
  return 0;
}