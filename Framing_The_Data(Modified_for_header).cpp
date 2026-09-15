#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
using namespace std;

uint16_t read_u16(const uint8_t* x){
    uint16_t v = x[0] << 8 | x[1];
    return v;
};

uint32_t read_u32(const uint8_t* x){
    uint32_t v = 
    static_cast<uint32_t>(x[0]) << 24 | static_cast<uint32_t>(x[1]) << 16 | 
    static_cast<uint32_t>(x[2]) << 8 | static_cast<uint32_t>(x[3]) ;
    return v;
};

uint64_t read_u48(const uint8_t* x){
    uint64_t v =  
    static_cast<uint64_t>(x[0]) << 40 | static_cast<uint64_t>(x[1]) << 32 | 
    static_cast<uint64_t>(x[2]) << 24 | static_cast<uint64_t>(x[3]) << 16 | 
    static_cast<uint64_t>(x[4]) << 8 | static_cast<uint64_t>(x[5]) ;
    return v;
};

uint64_t read_u64(const uint8_t* x){
    uint64_t v =
    static_cast<uint64_t>(x[0]) << 56 | static_cast<uint64_t>(x[1]) << 48|
    static_cast<uint64_t>(x[2]) << 40 | static_cast<uint64_t>(x[3]) << 32|
    static_cast<uint64_t>(x[4]) << 24 | static_cast<uint64_t>(x[5]) << 16|
    static_cast<uint64_t>(x[6]) << 8  | static_cast<uint64_t>(x[7])
    ;
    return v;
};

struct message_header{
    char message_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint64_t time_stamp;
};

message_header decode_header(const uint8_t* x){
    message_header decoded_header;
    decoded_header.message_type = static_cast<char>(x[0]);
    decoded_header.stock_locate = read_u16(x + 1);
    decoded_header.tracking_number = read_u16(x + 3);
    decoded_header.time_stamp = read_u48(x + 5);
    return decoded_header;
};

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

    //Counter for header
    uint64_t header_counter = 0;

    //Error counter for time stamp
    uint64_t error_counter = 0;

    //Dummy timestamp variable
    uint64_t dummy_timestamp = 0;

    //Stock locate track (Should be only either 0 or 7397)
    uint64_t stock_locate_track[65536] = {};
    /*
    Modified to decode header in the loop
    */
   while (true)
   {
    limit_order_book_data.read(reinterpret_cast<char*>(buf),2);
    if(!limit_order_book_data) break;
    //Calculate the n_ed number for the file
    uint16_t n_ed = buf[0] * 256 + buf[1];
    limit_order_book_data.read(reinterpret_cast<char*>(n_buf),n_ed);
    message_header temp_header = decode_header(n_buf);
    if (header_counter < 10){
        cout << temp_header.message_type << " " << temp_header.stock_locate
             << " " << temp_header.time_stamp << endl;
    };
    if (temp_header.time_stamp < dummy_timestamp){
        error_counter += 1;
    }
    dummy_timestamp = temp_header.time_stamp;

    header_counter += 1;
    counts[n_buf[0]] += 1;
    stock_locate_track[temp_header.stock_locate] += 1;
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
   cout << "Total error for timestamp that decrease instead of increase: " << error_counter << endl;
   
   //Check if stock locate is only either 0 or 7398
   if (stock_locate_track[0] + stock_locate_track[7397] == total){
    cout << "stock_locate takes only the value 0 and 7397 across " 
    << total << " messages" << endl; 
   } else {
    cout << "stock_locate takes some other value beside 0 and 7397" << endl;
   };
   return 0;
}