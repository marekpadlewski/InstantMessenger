#include "chat_server.h"

using boost::asio::ip::tcp;

void ChatRoom::deliver(const ChatMessage& message){
    recent_messages_deq.push_back(message);
    while (recent_messages_deq.size() > MAX_RECENT_MESSAGES)
        recent_messages_deq.pop_front();

    for (const auto& member : members){
        member->deliver(message);
    }
}

void ChatRoom::join(const std::shared_ptr<ChatMember>& member){
    members.insert(member);
    for (const auto& message : recent_messages_deq)
        member->deliver(message);
}

void ChatRoom::leave(const std::shared_ptr<ChatMember>& member){
    members.erase(member);
}



void ChatSession::read_header(){
    auto self(shared_from_this());
    boost::asio::async_read(socket,
            boost::asio::buffer(read_message.get_header_ref(), read_message.header_length),
            [this, self](boost::system::error_code ec, std::size_t){
                if (!ec && read_message.decode_header()){
                    read_body();
                }
                else{
                    room.leave(shared_from_this());
                }
            });
}

void ChatSession::read_body(){
    auto self(shared_from_this());
    boost::asio::async_read(socket,
            boost::asio::buffer(read_message.get_body_ref(), ChatMessage::max_body_length),
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

void ChatSession::write(){
    auto self(shared_from_this());

    std::vector<char> collect_msg;
    //concatenate header and body to one vector
    std::vector<char> h = write_messages.front().get_header();
    std::vector<char> b = write_messages.front().get_body();

    collect_msg.insert(collect_msg.begin(), h.begin(), h.end());
    collect_msg.insert(collect_msg.end(), b.begin(), b.end());

    boost::asio::async_write(socket,
            boost::asio::buffer(collect_msg),
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

ChatSession::ChatSession(tcp::socket socket_, ChatRoom& room_)
    : socket(std::move(socket_)), room(room_){}

void ChatSession::start(){
    room.join(shared_from_this());
    read_header();
}

void ChatSession::deliver(const ChatMessage& message){
    bool write_in_progress = !write_messages.empty();
    write_messages.push_back(message);
    if (!write_in_progress){
        write();
    }
}



void ChatServer::accept(){
    acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket){
                if (!ec){
                    std::make_shared<ChatSession>(std::move(socket), room)->start();
                }

                accept();
            }
    );
}

ChatServer::ChatServer(boost::asio::io_context& io_ctx, const tcp::endpoint& endpoint)
    : acceptor(io_ctx, endpoint){ accept(); }

int main (int argc, char* argv[]){
    try {
        if (argc < 2){
            std::cerr << "Usage: ./server <port> [<port> ...]" << std::endl;
            return 1;
        }

        boost::asio::io_context io_ctx;

        std::list<ChatServer> servers;
        for (int i = 1 ; i < argc ; i++){
            if (auto port = std::atoi(argv[i])){
                tcp::endpoint endpoint(tcp::v4(), port);
                servers.emplace_back(io_ctx, endpoint);
            }
            else{
                std::cerr << "Error: argument (port) not valid" << std::endl;
            }

        }

        io_ctx.run();
    }
    catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}