#include "chat_message.h"

ChatMessage::ChatMessage() : body_length(0){}

const char* ChatMessage::get_data() const{
    return data;
}

char* ChatMessage::get_data(){
    return data;
}

std::size_t ChatMessage::length() const{
    return header_length + body_length;
}

const char* ChatMessage::body() const{
    return data + header_length;
}

char* ChatMessage::body(){
    return data + header_length;
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
    char header[header_length + 1] = "";
    std::strncat(header, data, header_length);
    body_length = std::atoi(header + type_length);
    if (body_length > max_body_length){
        body_length = 0;
        return false;
    }

    return true;
}

void ChatMessage::encode_header(const std::string& type){
    //4 bytes of type (MESS/FILE), 4 bytes of body_length
    char header[header_length + 1] = "";
    std::strcpy(header, type.c_str());
    std::sprintf(header + type_length, "%4d", static_cast<int>(body_length));
    std::memcpy(data, header, header_length);
}