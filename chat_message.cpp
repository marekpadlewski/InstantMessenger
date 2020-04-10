#include "chat_message.h"

ChatMessage::ChatMessage() : body_length(0), data(header_length + max_body_length){}


std::vector<char> ChatMessage::get_data(){
    return data;
}

void ChatMessage::set_data(const std::vector<char>& fresh_data) {
    std::copy(fresh_data.begin(), fresh_data.end(), data.begin());
}

std::size_t ChatMessage::length() const{
    return header_length + body_length;
}

std::vector<char> ChatMessage::body(){
    return std::vector<char>(data.begin() + header_length, data.end());
}

void ChatMessage::set_body(const std::vector<char>& fresh_body) {
    std::copy(fresh_body.begin(), fresh_body.end(), data.begin() + header_length);
}

std::size_t ChatMessage::get_body_length() const{
    return body_length;
}

void ChatMessage::update_body_length(std::size_t new_length){
    body_length = new_length;
    if (body_length > max_body_length)
        body_length = max_body_length;
}

void ChatMessage::set_header(const std::vector<char> &fresh_header) {
    std::copy(fresh_header.begin(), fresh_header.end(), data.begin());
}

bool ChatMessage::decode_header(){
    std::vector<char> header(data.begin(), data.begin() + header_length);
    std::string h(header.begin() + type_length, header.end());
    body_length = std::atoi(h.c_str());
    if (body_length > max_body_length){
        body_length = 0;
        return false;
    }

    return true;
}

//  FILE0011example.txt0024ala ma kota a kot ma ale

//  mając jakiś int np 340 zrobić sobie vector{' ', '3', '4', '0'}

std::string number_in_header(std::size_t number){
    // we want to have block of 4 characters that store number
    // if number of digits is less than 4, we add space in front
    int digits = (int) log10(number) + 1;
    std::string s_num = std::to_string(number);
    while (s_num.size() < 4)
        s_num = ' ' + s_num;

    return s_num;
    //TODO add declaration
}

void ChatMessage::encode_header(const std::string& type){
    //4 bytes of type (MESS/FILE), 4 bytes of body_length
    std::vector<char> header(header_length + 1);
    std::copy(type.begin(), type.end(), header.begin());

    std::string body_length_conv = number_in_header(body_length);
    std::copy(body_length_conv.begin(), body_length_conv.end(), header.begin() + type_length);

    std::copy(header.begin(), header.end(), data.begin());
}
