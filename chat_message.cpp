#include "chat_message.h"

ChatMessage::ChatMessage() : body_length(0), header(header_length), body(max_body_length){}

std::vector<char> ChatMessage::get_header(){
    return header;
}

std::vector<char>& ChatMessage::get_header_ref() {
    return header;
}

void ChatMessage::set_header(const std::vector<char>& fresh_header) {
    std::copy(fresh_header.begin(), fresh_header.end(), header.begin());
}

std::size_t ChatMessage::data_length() const{
    return header_length + body_length;
}

std::vector<char> ChatMessage::get_body(){
    return body;
}

std::vector<char>& ChatMessage::get_body_ref(){
    return body;
}

void ChatMessage::set_body(const std::vector<char>& fresh_body){
    std::copy(fresh_body.begin(), fresh_body.end(), body.begin());
}

std::size_t ChatMessage::get_body_length() const{
    return body_length;
}

void ChatMessage::update_body_length(std::size_t new_length){
    body_length = new_length;
    if (body_length > max_body_length)
        body_length = max_body_length;
}

bool ChatMessage::decode_header(){
    std::string h(header.begin() + type_length, header.end());
    body_length = std::atoi(h.c_str());
    if (body_length > max_body_length){
        body_length = 0;
        return false;
    }

    return true;
}


std::string number_in_header(std::size_t number){
    // we want to have block of 4 characters that store number
    // if number of digits is less than 4, we add space in front
    int digits = (int) log10(number) + 1;
    std::string s_num = std::to_string(number);
    while (s_num.size() < 4)
        s_num = ' ' + s_num;

    return s_num;
}

// last version: [TYPE][BODY_LEN]

//  FILE 0024 0011
// [TYPE][BODY_LEN][FILENAME_LEN]
// [FILENAME] store in body
void ChatMessage::encode_header(const std::string& type, int filename_len){
    //4 bytes of type (MESS/FILE), 4 bytes of body_length, 4 bytes of filename_length

    std::string body_length_conv = number_in_header(body_length);
    std::string fn_length_conv = number_in_header(filename_len);
    std::string h = type + body_length_conv + fn_length_conv;
    std::copy(h.begin(), h.end(), header.begin());
}

std::string ChatMessage::get_type_from_header() {
    return std::string(header.begin(), header.begin() + type_length);
}

std::size_t ChatMessage::get_filenamelen_from_header(){
    std::string s(header.begin() + 8, header.end());
    return std::atoi(s.c_str());
}