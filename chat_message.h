#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

class ChatMessage{
public:
    enum { type_length = 4};
    enum { header_length = 8 };
    enum { max_body_length = 512 };

    ChatMessage();

    const char* get_data() const;

    char* get_data();

    std::size_t length() const;

    const char* body() const;

    char* body();

    std::size_t get_body_length() const;

    void update_body_length(std::size_t new_length);

    bool decode_header();

    void encode_header(const std::string& type);

private:
    char data[header_length + max_body_length];
    std::size_t body_length;
};