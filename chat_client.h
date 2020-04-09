#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <queue>

#include <boost/asio.hpp>

#include "chat_message.h"

class ChatClient{
public:
    ChatClient(boost::asio::io_context& io_ctx_, const boost::asio::ip::tcp::resolver::results_type& endpoints);

    void write(const ChatMessage& message);

    void close();

private:
    unsigned int file_count = 1;

    void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);

    void read_header();

    void read_body();

    void do_write();

    boost::asio::io_context& io_ctx;
    boost::asio::ip::tcp::socket socket;
    ChatMessage read_message;
    std::deque<ChatMessage> write_messages;
};

std::vector<char> readFileBytes(const std::string& name);

bool file_exists(const std::string& name);

std::string extract_filename(std::string& path);