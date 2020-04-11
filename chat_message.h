#pragma once

#include <vector>
#include <string>
#include <cmath>

class ChatMessage{
public:
    enum { type_length = 4};
    enum { header_length = 12 };
    enum { max_body_length = 512 };

    ChatMessage();

    std::vector<char> get_header();
    std::vector<char>& get_header_ref();

    std::size_t data_length() const;

    std::vector<char> get_body();
    std::vector<char>& get_body_ref();
    void set_body(const std::vector<char>& fresh_body);
    std::size_t get_body_length() const;
    void update_body_length(std::size_t new_length);

    bool decode_header();
    void encode_header(const std::string& type, int filename_len);
    std::string get_type_from_header();
    std::size_t get_filename_len_from_header();

private:
    std::vector<char> header;
    std::vector<char> body;
    std::size_t body_length;
};