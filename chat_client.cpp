#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;

class ChatClient{
public:
    ChatClient(boost::asio::io_context& io_ctx_,
            const tcp::resolver::results_type& endpoints)
            : io_ctx(io_ctx_), socket(io_ctx_){
        connect(endpoints);
    }

    void write(const ChatMessage& message){
        boost::asio::post(io_ctx, [this, message](){
            bool write_in_progress = !write_messages.empty();
            write_messages.push_back(message);
            if (!write_in_progress){
                do_write();
            }
        });
    }

    void close(){
        boost::asio::post(io_ctx, [this]() { socket.close(); });
    }

private:
    unsigned int file_count = 1;

    void connect(const tcp::resolver::results_type& endpoints){
        boost::asio::async_connect(socket, endpoints,
                [this](boost::system::error_code ec, tcp::endpoint){
                    if (!ec){
                        read_header();
                    }
                });
    }

    void read_header(){
        boost::asio::async_read(socket,
                boost::asio::buffer(read_message.get_data(), ChatMessage::header_length),
                [this](boost::system::error_code ec, std::size_t){
                    if (!ec && read_message.decode_header()){
                        read_body();
                    }
                    else{
                        socket.close();
                    }
                });
    }

    void read_body(){
        boost::asio::async_read(socket,
                boost::asio::buffer(read_message.body(), read_message.get_body_length()),
                [this](boost::system::error_code ec, std::size_t){
                    if (!ec){
                        std::string type(read_message.get_data(), read_message.type_length);

                        if (type == "MESS"){
                            std::cout.write(read_message.body(), read_message.get_body_length());
                            std::cout << std::endl;
                            read_header();
                        }
                        else if (type == "FILE"){
                            std::string filename = "../downloads/file";
                            filename += std::to_string(file_count);
                            std::cout << "Writing file content to " << filename << std::endl;
                            std::ofstream out_file(filename, std::ofstream::out);
                            out_file.write(read_message.body(), read_message.get_body_length());
                            out_file.close();

                            file_count++;
                            read_header();
                        }
                    }
                    else{
                        socket.close();
                    }
                });
    }

    void do_write(){
        boost::asio::async_write(socket,
                boost::asio::buffer(write_messages.front().get_data(), write_messages.front().length()),
                [this](boost::system::error_code ec, std::size_t){
                    if (!ec){
                        write_messages.pop_front();
                        if (!write_messages.empty()){
                            do_write();
                        }
                    }
                    else{
                        socket.close();
                    }
                });
    }

    boost::asio::io_context& io_ctx;
    tcp::socket socket;
    ChatMessage read_message;
    std::deque<ChatMessage> write_messages;
    std::queue<ChatMessage> received_files;
};

std::vector<char> readFileBytes(const std::string& name) {
    std::ifstream fl(name);

    fl.seekg(0, std::ios::end );
    std::size_t len = fl.tellg();
    std::vector<char> ret(len);
    fl.seekg(0, std::ios::beg);
    if (len)
        fl.read(&ret[0], len);
    fl.close();
    return move(ret);
}

bool file_exists(const std::string& name){
    std::ifstream fl(name);
    return fl.good();
}

std::string extract_filename(std::string& path){
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
    return base_filename;
}

int main(int argc, char* argv[]){
    try{
        if (argc != 4){
            std::cerr << "Usage: ./client <host> <port> <username>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_ctx;

        tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        ChatClient c(io_ctx, endpoints);

        std::thread t([&io_ctx](){io_ctx.run();});

        std::string username = argv[3];
        username += ":";
        char line[ChatMessage::max_body_length + 1];
        int l = username.size();
        std::memcpy(line, username.c_str(), username.size());

        //  COMMANDS
        //  /send-message <message>
        //  /send-file <path>

        std::string command;
        while (std::cin >> command){
            ChatMessage message;

            if (command == "/send-message"){
                std::cin.getline(line + l, ChatMessage::max_body_length + 1 - username.size());
                message.update_body_length(std::strlen(line));
                std::memcpy(message.body(), line, message.get_body_length());
                message.encode_header("MESS");
                c.write(message);
            }
            else if (command == "/send-file"){
                std::string file_path;
                std::cin >> file_path;
                if (!file_exists(file_path))
                    std::cout << "There's no such file!" << std::endl;
                else{
                    std::string extracted_name = extract_filename(file_path);
                    line[l] = ' ';
                    std::memcpy(line + l + 1, extracted_name.c_str(), extracted_name.size());

                    message.update_body_length(l + extracted_name.size() + 1);
                    std::memcpy(message.body(), line, message.get_body_length());
                    message.encode_header("MESS");
                    c.write(message);

                    std::vector<char> file_bytes = readFileBytes(file_path);
                    unsigned int s = file_bytes.size() > message.max_body_length ? message.max_body_length : file_bytes.size();
                    message.update_body_length(file_bytes.size());
                    std::memcpy(message.body(), file_bytes.data(), s);
                    message.encode_header("FILE");
                    c.write(message);
                }

            }
            else{
                std::cin.getline(line + l, ChatMessage::max_body_length + 1);
                std::cout << "Wrong command! Commands list: /send-message ; /send-file ; /save-file" << std::endl;
            }
        }

        c.close();
        t.join();
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
