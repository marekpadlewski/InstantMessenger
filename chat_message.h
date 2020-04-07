#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

class ChatMessage{
public:
    enum { type_length = 4};
    enum { header_length = 8 };
    enum { max_body_length = 512 };

    ChatMessage() : body_length(0){}

    const char* get_data() const{
        return data;
    }

    char* get_data(){
        return data;
    }

    std::size_t length() const{
        return header_length + body_length;
    }

    const char* body() const{
        return data + header_length;
    }

    char* body(){
        return data + header_length;
    }

    std::size_t get_body_length() const{
        return body_length;
    }

    void update_body_length(std::size_t new_length){
        body_length = new_length;
        if (body_length > max_body_length)
            body_length = max_body_length;
    }

    bool decode_header(){
        char header[header_length + 1] = "";
        std::strncat(header, data, header_length);
        body_length = std::atoi(header + type_length);
        if (body_length > max_body_length){
            body_length = 0;
            return false;
        }

        return true;
    }

    void encode_header(const std::string& type){
        //4 bytes of type (MESS/FILE), 4 bytes of body_length
        char header[header_length + 1] = "";
        std::strcpy(header, type.c_str());
        std::sprintf(header + type_length, "%4d", static_cast<int>(body_length));
        std::memcpy(data, header, header_length);
    }

private:
    char data[header_length + max_body_length];
    std::size_t body_length;
};