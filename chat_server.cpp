#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;

class ChatMember{
public:
    virtual ~ChatMember(){}
    virtual void deliver(const ChatMessage& msg) = 0;
};

class ChatRoom{
private:
    std::set<std::shared_ptr<ChatMember>> members;
    std::deque<ChatMessage> recent_messages_deq;
    const unsigned int MAX_RECENT_MESSAGES = 100;
public:
    void deliver(const ChatMessage& message){
        recent_messages_deq.push_back(message);
        while (recent_messages_deq.size() > MAX_RECENT_MESSAGES)
            recent_messages_deq.pop_front();

        for (auto member : members){
            member->deliver(message);
        }
    }

    void join(std::shared_ptr<ChatMember> member){
        members.insert(member);
        for (auto message : recent_messages_deq)
            member->deliver(message);
    }

    void leave(std::shared_ptr<ChatMember> member){
        members.erase(member);
    }
};

class ChatSession : public ChatMember, public std::enable_shared_from_this<ChatSession>{
private:
    tcp::socket socket;
    ChatRoom& room;
    ChatMessage read_message;
    std::deque<ChatMessage> write_messages;

    void read_header(){
        auto self(shared_from_this());
        boost::asio::async_read(socket,
                boost::asio::buffer(read_message.get_data(), ChatMessage::header_length),
                [this, self](boost::system::error_code ec, std::size_t){
                    if (!ec && read_message.decode_header()){
                        read_body();
                    }
                    else{
                        room.leave(shared_from_this());
                    }
        });
    }

    void read_body(){
        auto self(shared_from_this());
        boost::asio::async_read(socket,
                boost::asio::buffer(read_message.body(), read_message.get_body_length()),
                [this, self](boost::system::error_code ec, std::size_t){
                    if (!ec){
                        room.deliver(read_message);
                        read_header();
                    }
                    else{
                        room.leave(shared_from_this());
                    }
        });
    }

    void write(){
        auto self(shared_from_this());
        boost::asio::async_write(socket,
                boost::asio::buffer(write_messages.front().get_data(), write_messages.front().length()),
                [this, self](boost::system::error_code ec, std::size_t){
                    if (!ec){
                        write_messages.pop_front();
                        if (!write_messages.empty()){
                            write();
                        }
                    }
                    else{
                        room.leave(shared_from_this());
                    }
        });
    }

public:
    ChatSession(tcp::socket socket_, ChatRoom& room_)
        :   socket(std::move(socket_)),
            room(room_)
    {}

    void start(){
        room.join(shared_from_this());
        read_header();
    }

    void deliver(const ChatMessage& message){
        bool write_in_progress = !write_messages.empty();
        write_messages.push_back(message);
        if (!write_in_progress){
            write();
        }
    }
};

class ChatServer{
private:
    tcp::acceptor acceptor;
    ChatRoom room;

    void accept(){
        acceptor.async_accept(
                [this](boost::system::error_code ec, tcp::socket socket){
                    if (!ec){
                        std::make_shared<ChatSession>(std::move(socket), room)->start();
                    }

                    accept();
                }
                );
    }

public:
    ChatServer(boost::asio::io_context& io_ctx, const tcp::endpoint& endpoint)
        :   acceptor(io_ctx, endpoint){
        accept();
    }
};

int main (int argc, char* argv[]){
    try {
        if (argc < 2){
            std::cerr << "Usage: ./server <port>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_ctx;

        std::list<ChatServer> servers;
        for (int i = 1 ; i < argc ; i++){
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_ctx, endpoint);
        }

        io_ctx.run();
    }
    catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}