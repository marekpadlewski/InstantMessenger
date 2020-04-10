#pragma once

#include <vector>
#include <string>
#include <cmath>

class ChatMessage{
public:
    enum { type_length = 4};
    enum { header_length = 8 };
    enum { max_body_length = 512 };

    ChatMessage();

    std::vector<char> get_data();

    void set_data(const std::vector<char>& fresh_data);

    std::size_t length() const;

    //std::vector<char> body() const;

    std::vector<char> body();

    void set_body(const std::vector<char>& fresh_body);

    std::size_t get_body_length() const;

    void update_body_length(std::size_t new_length);

    void set_header(const std::vector<char>& fresh_header);

    bool decode_header();

    void encode_header(const std::string& type);

private:
    //char data[header_length + max_body_length];
    std::vector<char> data;
    std::size_t body_length;
};