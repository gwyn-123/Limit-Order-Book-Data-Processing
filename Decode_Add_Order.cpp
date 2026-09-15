//Test

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
using namespace std;

uint16_t read_u16(const uint8_t* x){
    uint16_t v = x[0] << 8 | x[1];
    return v;
};

uint32_t read_u32(const uint8_t* x){
    uint32_t v = 
    static_cast<uint32_t>(x[0]) << 24 | static_cast<uint32_t>(x[1]) << 16 
    | static_cast<uint32_t>(x[2]) << 8 | static_cast<uint32_t>(x[3]) ;
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
}

uint16_t expected_body_length[256] = {};

void body_length_table (){
    expected_body_length['A'] = 36; 
    expected_body_length['C'] = 36; 
    expected_body_length['D'] = 19; 
    expected_body_length['E'] = 31; 
    expected_body_length['F'] = 40; 
    expected_body_length['U'] = 35; 
    expected_body_length['X'] = 23; 
}

struct add_order {
    uint64_t order_reference_no;
    char side;
    uint32_t shares;
    char symbol[8];
    uint32_t price;
}; // A

add_order decode_add_order(const uint8_t* x){
    add_order decoded_order;
    decoded_order.order_reference_no = read_u64(x + 11);
    decoded_order.side = static_cast<char>(x[19]);
    decoded_order.shares = read_u32(x + 20);
    memcpy(decoded_order.symbol, x + 24, 8); //memcpy(to, from, size of bytes to be copied)
    decoded_order.price = read_u32(x +32);
    return decoded_order;
}

int main(){
    body_length_table();

    //Open limit order data book
    ifstream limit_order_data_book_data("SPY_20190730_PSX_ITCH50.bin", ios::binary);

    //Create a dummy byte
    uint8_t buf[2];    // Hold the 2 bytes
    uint8_t n_buf[64]; // Hold the message itself

    //Create dummy add order
    add_order temp_add_order;
    //Loop to test for decode_add_order
    while (true){
        limit_order_data_book_data.read(reinterpret_cast<char*>(buf), 2);
        if(!limit_order_data_book_data) break;

        //Calculate the n_ed
        uint16_t n_ed = buf[0] * 256 + buf[1];
        limit_order_data_book_data.read(reinterpret_cast<char*>(n_buf),n_ed);
        message_header temp_header = decode_header(n_buf);
        
        //Check if the header is 'A'
        if (temp_header.message_type == 'A'){
            temp_add_order = decode_add_order(n_buf);
            break;
        }
    }
    cout << "The first order 'A' detail: " << endl;
    cout << "Order reference number: " << temp_add_order.order_reference_no << endl;
    cout << "Side: " << temp_add_order.side << endl;
    cout << "Shares' size: " << temp_add_order.shares << endl;
    cout << "Price in total: " << temp_add_order.price << endl;
    cout << "Order symbol : ";
    cout.write(temp_add_order.symbol, 8) << endl;
    return 0;
}