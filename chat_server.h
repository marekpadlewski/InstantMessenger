#pragma once

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

#include <boost/asio.hpp>

#include "chat_message.h"

class ChatMember{
public:
    virtual ~ChatMember()= default;
    virtual void deliver(const ChatMessage& msg) = 0;
};

class ChatRoom{
private:
    std::set<std::shared_ptr<ChatMember>> members;
    std::deque<ChatMessage> recent_messages_deq;
    const unsigned int MAX_RECENT_MESSAGES = 100;
public:
    void deliver(const ChatMessage& message);

    void join(std::shared_ptr<ChatMember> member);

    void leave(std::shared_ptr<ChatMember> member);
};

class ChatSession : public ChatMember, public std::enable_shared_from_this<ChatSession>{
private:
    boost::asio::ip::tcp::socket socket;
    ChatRoom& room;
    ChatMessage read_message;
    std::deque<ChatMessage> write_messages;

    void read_header();

    void read_body();

    void write();

public:
    ChatSession(boost::asio::ip::tcp::socket socket_, ChatRoom& room_);

    void start();

    void deliver(const ChatMessage& message) override;
};

class ChatServer{
private:
    boost::asio::ip::tcp::acceptor acceptor;
    ChatRoom room;

    void accept();

public:
    ChatServer(boost::asio::io_context& io_ctx, const boost::asio::ip::tcp::endpoint& endpoint);
};