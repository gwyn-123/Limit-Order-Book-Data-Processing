/*
Task 1: Framing and Histogram
Objectives:

*/
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
using namespace std;

int main(){
    //Open limit order book data file which is stored in binary form
    ifstream limit_order_book_data("SPY_20190730_PSX_ITCH50.bin", ios::binary);

    //Create fixed count
    uint64_t counts[256] = {};
    //Create a dummy byte
    uint8_t buf[2];    // Hold the 2 bytes
    uint8_t n_buf[64]; // Hold the message itself
    //Create a total
    uint64_t total = 0;
    
    /*
    Leap through the file, for each 2 bytes, we found the N number in 256
    base system from those 2 bytes. Then in the next N bytes, we find position 
    0, which let us know the name of the orders, i.e A, C, D, ...
    */
   while (true)
   {
    limit_order_book_data.read(reinterpret_cast<char*>(buf),2);
    if(!limit_order_book_data) break;
    //Calculate the n number for the file
    uint16_t n = buf[0] * 256 + buf[1];
    limit_order_book_data.read(reinterpret_cast<char*>(n_buf),n);
    counts[n_buf[0]] += 1;
   };
   
   //Output the number of orders for each type
   int n = 0;
   for (int i = 0; i < 256; i += 1){
    if (counts[i] != 0){
        if (n > 0 && n%5 == 0) {cout << "\n";}
        cout << char(i) << " " << counts[i] << "  ";
        total += counts[i];
        n += 1;
    };
   };
   cout << "\nTotal: " << total << endl;
    return 0;
}