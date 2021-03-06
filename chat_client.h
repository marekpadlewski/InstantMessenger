#pragma once

#include <iostream>
#include <thread>
#include <fstream>
#include <deque>
#include <vector>

#include <boost/asio.hpp>

#include "chat_message.h"

class ChatClient{
public:
    ChatClient(boost::asio::io_context& io_ctx_, const boost::asio::ip::tcp::resolver::results_type& endpoints);

    void write(const ChatMessage& message);
    void close();
    std::vector<std::pair<std::string, std::vector<char>>> get_files_list();

private:
    void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);
    void read_header();
    void read_body();
    void do_write();

    boost::asio::io_context& io_ctx;
    boost::asio::ip::tcp::socket socket;
    ChatMessage read_message;
    std::deque<ChatMessage> write_messages;
    std::vector<std::pair<std::string, std::vector<char>>> received_files;
};

std::vector<char> readFileBytes(const std::string& name);

bool file_exists(const std::string& name);

std::string extract_filename(std::string& path);