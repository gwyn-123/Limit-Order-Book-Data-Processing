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
    return message_header{
        static_cast<char>(x[0]),
        read_u16(x + 1),
        read_u16(x + 3),
        read_u48(x + 5)
    };
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

struct delete_order{
    uint64_t order_reference_no;
};// D

delete_order decode_delete_order(const uint8_t* x){
    return delete_order{read_u64(x+11)};
}

struct cancel_order{
    uint64_t order_reference_no;
    uint32_t cancel_share_count;
};// X

cancel_order decode_cancel_order(const uint8_t* x){
    return cancel_order{
        read_u64(x + 11),
        read_u32(x + 19)
    };
}

struct replace_order{
    uint64_t old_order_reference_no;
    uint64_t new_order_reference_no;
    uint32_t shares;
    uint32_t price;
};//U

replace_order decode_replace_order(const uint8_t* x){
    return replace_order{
        read_u64(x + 11),
        read_u64(x + 19),
        read_u32(x + 27),
        read_u32(x + 31)
    };
}

struct add_order_MPID_attribution{
    add_order adjunct_add_order;
    char attribution[4];
};//F

add_order_MPID_attribution decode_add_order_MPID_attribution(const uint8_t* x){
    add_order_MPID_attribution temp_add_order_F;
    temp_add_order_F.adjunct_add_order = decode_add_order(x);
    memcpy(temp_add_order_F.attribution, x + 36, 4);
    return temp_add_order_F;
}

struct execute_order{
    uint64_t order_reference_no;
    uint32_t executed_shares;
    uint64_t match_no;
};//E

execute_order decode_execute_order(const uint8_t* x){
    return execute_order{
        read_u64(x + 11),
        read_u32(x + 19),
        read_u64(x + 23)  
    };
}

struct execute_order_with_price {
    execute_order adjunt_execute_order;
    char printable;
    uint32_t execution_price;
};// C

execute_order_with_price decode_execute_order_with_price (const uint8_t* x){
    execute_order_with_price temp_ex_ord_with_price;
    temp_ex_ord_with_price.adjunt_execute_order = decode_execute_order(x);
    temp_ex_ord_with_price.printable = static_cast<char>(x[31]);
    temp_ex_ord_with_price.execution_price = read_u32(x + 32);
    return temp_ex_ord_with_price;
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

    //Create dummy delete order
    delete_order temp_delete_order;

    //Create dummy cancel order
    cancel_order temp_cancel_order;

    //Create dummy replace order
    replace_order temp_replace_order;

    //Create dummy execute order
    execute_order temp_execute_order;

    //Create dummy execute_order_with_price
    execute_order_with_price temp_execute_order_with_price;

    //Add order flag , is this the first header we have?
    bool add_order_flag = false;

    //Delete order flag , is this the first header we have?
    bool delete_order_flag= false;

    //Cancel order flag , is this the first header we have?
    bool cancel_order_flag = false;

    //Replace order flag, is this the first header we have?
    bool replace_order_flag = false;

    //Replace execute flag, is this the first header we have?
    bool execute_order_flag = false;

    //Replace execute order with price flag, is this the first header we have:
    bool execute_order_with_price_flag = false;

    //F order counter
    int f_counter = 0;
    //C order counter
    int c_counter = 0;
    //Loop to test for decoder
    while (true){
        limit_order_data_book_data.read(reinterpret_cast<char*>(buf), 2);
        if(!limit_order_data_book_data) break;

        //Calculate the n_ed
        uint16_t n_ed = buf[0] * 256 + buf[1];
        limit_order_data_book_data.read(reinterpret_cast<char*>(n_buf),n_ed);
        message_header temp_header = decode_header(n_buf);
        
        //Check if the header is 'A'
        if (temp_header.message_type == 'A' && !add_order_flag){
            temp_add_order = decode_add_order(n_buf);
            add_order_flag = true;
        } else if (temp_header.message_type == 'D' && !delete_order_flag){
            // Check if the header is 'D'
            temp_delete_order = decode_delete_order(n_buf);
            delete_order_flag = true;
        } else if (temp_header.message_type == 'X' && !cancel_order_flag){
            // Check if the header is 'X'
            temp_cancel_order = decode_cancel_order(n_buf);
            cancel_order_flag = true;
        } else if (temp_header.message_type == 'U' && !replace_order_flag){
            // Check if the header is 'U'
            temp_replace_order = decode_replace_order(n_buf);
            replace_order_flag = true;
        } else if (temp_header.message_type == 'F'){
            // Check if the header is F
            add_order_MPID_attribution temp_add_order_F = decode_add_order_MPID_attribution(n_buf);
            add_order adjunct_add_order_F = temp_add_order_F.adjunct_add_order;
            cout << adjunct_add_order_F.order_reference_no << " " << adjunct_add_order_F.side
                 << " " << adjunct_add_order_F.shares << " " << "[";
            cout.write (adjunct_add_order_F.symbol, 8); 
            cout << "]"
                 << " " << adjunct_add_order_F.price << " " << "[";
            cout.write(temp_add_order_F.attribution, 4);
            cout << "]"
                 << endl;
            f_counter += 1;
        } else if (temp_header.message_type == 'E' && !execute_order_flag){
            // Check if the header is E
            temp_execute_order = decode_execute_order(n_buf);
            execute_order_flag = true;
        } else if (temp_header.message_type == 'C'){
            //Check if the header is C
            temp_execute_order_with_price = decode_execute_order_with_price(n_buf);
            execute_order_with_price_flag = true;
            execute_order temp_adj =  temp_execute_order_with_price.adjunt_execute_order;
            c_counter += 1;

            cout << temp_adj.order_reference_no << " " << temp_adj.executed_shares
                 << " " << temp_adj.match_no << " " << temp_execute_order_with_price.printable
                 << " " << temp_execute_order_with_price.execution_price << endl;
        }
    }

    cout << "\nThe number of F order is: " << f_counter << endl;
    cout << "The number of C order is: " << c_counter << endl;
    //Checking if the order reference number are equal
    if (temp_add_order.order_reference_no == temp_delete_order.order_reference_no){
        cout << "The order reference numbers of both first add and delete order are equal" << endl
             << "The match order reference number of both is: " << temp_add_order.order_reference_no
             << endl;
    } else {
        cout << "The order reference numbers of both first add and delete order aren't equal" << endl
             << "The order reference number of the add order is: " << temp_add_order.order_reference_no 
             << endl << "The order reference number of the delete order is: " << temp_delete_order.order_reference_no
             << endl;
    }

    cout << "\nThe first order 'A' detail: " << endl 
         << "Order reference number: " << temp_add_order.order_reference_no << endl 
         << "Side: " << temp_add_order.side << endl 
         << "Shares' size: " << temp_add_order.shares << endl 
         << "Price in total: " << temp_add_order.price << endl 
         << "Order symbol : ";
    cout.write(temp_add_order.symbol, 8) << endl;
    cout << "\nThe first cancel order has reference number: " << temp_cancel_order.order_reference_no
         << endl << "The first cancel order cancelled: " << temp_cancel_order.cancel_share_count << " shares"
         << endl;

    cout << "\nThe first replace order's old reference number is: " << temp_replace_order.old_order_reference_no
         << endl << "The first replace order's new reference number is: " << temp_replace_order.new_order_reference_no
         << endl << "The amount of shares: " << temp_replace_order.shares
         << endl << "The price of shares: " << temp_replace_order.price
         << endl;

    cout << "\nThe first executed's order reference number is: " << temp_execute_order.order_reference_no
         << endl << "The executed shares are: " << temp_execute_order.executed_shares << endl
         << "The match number is: " << temp_execute_order.match_no << endl;
    return 0;
}