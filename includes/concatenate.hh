#define AC 4

using namespace std;

unsigned long long int concatenate(unsigned long long int x, unsigned long long int y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}