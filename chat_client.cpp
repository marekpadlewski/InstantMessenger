#include "chat_client.h"

using boost::asio::ip::tcp;

ChatClient::ChatClient(boost::asio::io_context& io_ctx_, const tcp::resolver::results_type& endpoints)
    : io_ctx(io_ctx_), socket(io_ctx_){ connect(endpoints); }

void ChatClient::write(const ChatMessage& message){
    boost::asio::post(io_ctx, [this, message](){
        bool write_in_progress = !write_messages.empty();
        write_messages.push_back(message);
        if (!write_in_progress){
            do_write();
        }
    });
}

void ChatClient::close(){
    boost::asio::post(io_ctx, [this]() { socket.close(); });
}


void ChatClient::connect(const tcp::resolver::results_type& endpoints){
    boost::asio::async_connect(socket, endpoints,
            [this](boost::system::error_code ec, const tcp::endpoint&){
                if (!ec){
                    read_header();
                }
            });
}

void ChatClient::read_header(){
    boost::asio::async_read(socket,
            boost::asio::buffer(read_message.get_header_ref(), read_message.header_length),
            [this](boost::system::error_code ec, std::size_t){
                if (!ec && read_message.decode_header()){
                    read_body();
                }
                else{
                    socket.close();
                }
            });

}

void ChatClient::read_body(){
    boost::asio::async_read(socket,
            boost::asio::buffer(read_message.get_body_ref(), ChatMessage::max_body_length),
            [this](boost::system::error_code ec, std::size_t){
                if (!ec){
                    std::string msg_type = read_message.get_type_from_header();
                    std::vector<char> msg_body = read_message.get_body();

                    if (msg_type == "MESS"){
                        for (char c : msg_body)
                            std::cout << c;
                        std::cout << std::endl;

                        read_header();
                    }
                    else if (msg_type == "FILE"){
                        std::size_t filename_len = read_message.get_filename_len_from_header();
                        std::string filename(msg_body.begin(), msg_body.begin() + filename_len);

                        received_files.emplace_back(filename, std::vector<char>(msg_body.begin() + filename_len,
                                msg_body.begin() + read_message.get_body_length()));

                        read_header();
                    }
                }
                else{
                    socket.close();
                }
            });
}

void ChatClient::do_write(){
    std::vector<char> collect_msg;
    //concatenate header and body to one vector
    std::vector<char> h = write_messages.front().get_header();
    std::vector<char> b = write_messages.front().get_body();

    collect_msg.insert(collect_msg.begin(), h.begin(), h.end());
    collect_msg.insert(collect_msg.end(), b.begin(), b.end());

    boost::asio::async_write(socket,
            boost::asio::buffer(collect_msg),
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

std::vector<std::pair<std::string, std::vector<char>>> ChatClient::get_files_list() {
    return received_files;
}



bool file_exists(const std::string& name){
    std::ifstream fl(name);
    return fl.good();
}

std::string extract_filename(std::string& path){
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
    return base_filename;
}

std::vector<char> readFileBytes(const std::string& name) {
    std::ifstream fl(name);

    fl.seekg(0, std::ios::end );
    std::size_t len = fl.tellg();
    std::vector<char> ret(len);
    fl.seekg(0, std::ios::beg);
    if (len)
        fl.read(&ret[0], ChatMessage::max_body_length);
    fl.close();
    return std::move(ret);
}

void save_file(const std::pair<std::string, std::vector<char>>& f){
    std::string filename = f.first;
    std::cout << "Saving file to " << filename << std::endl;

    std::ofstream out_file(filename, std::ofstream::out);
    out_file.write((char*)&f.second[0], f.second.size());
    out_file.close();

}

void send_message_command(ChatClient& c, char* line, int l){
    ChatMessage message;
    std::cin.getline(line + l, ChatMessage::max_body_length + 1 - l);
    message.update_body_length(std::strlen(line));
    message.set_body(std::vector<char>(line, line + message.get_body_length()));
    message.encode_header("MESS", 0);
    c.write(message);
}

void send_file_command(ChatClient& c, char* line, int l){
    ChatMessage message;
    std::string file_path;
    std::getline(std::cin, file_path);
    file_path.erase(0, 1);
    if (!file_exists(file_path))
        std::cout << "There's no such file!" << std::endl;
    else{
        std::string extracted_name = extract_filename(file_path);
        line[l] = ' ';
        std::copy(extracted_name.begin(), extracted_name.end(), line + l + 1);

        message.update_body_length(l + extracted_name.size() + 1);
        message.set_body(std::vector<char>(line, line + message.get_body_length()));
        message.encode_header("MESS", 0);
        c.write(message);

        std::vector<char> file_bytes = readFileBytes(file_path);
        file_bytes.insert(file_bytes.begin(), extracted_name.begin(), extracted_name.end());

        message.update_body_length(file_bytes.size());
        message.set_body(std::vector<char>(file_bytes.begin(),
                                           file_bytes.begin() + message.get_body_length()));
        message.encode_header("FILE", extracted_name.size());
        c.write(message);
    }
}

void show_files_command(ChatClient& c){
    for (const auto& f : c.get_files_list())
        std::cout << f.first << " ; ";

    std::cout << std::endl;
}

void save_file_command(ChatClient& c){
    std::string filename;
    std::getline(std::cin, filename);
    filename.erase(0, 1);

    bool file_on_list = false;
    for (const auto& f : c.get_files_list()){
        if (f.first == filename){
            save_file(f);
            file_on_list = true;
        }
    }

    if (!file_on_list)
        std::cout << "There's no such file on list." << std::endl;
}

void input_manager(ChatClient& c, char* line, int l){
    //  COMMANDS
    //  /send-message <message>
    //  /send-file <path>
    //  /save-file <file_name>
    //  /show-files

    std::string command;
    while (std::cin >> command){
        if (command == "/send-message") send_message_command(c, line, l);
        else if (command == "/send-file") send_file_command(c, line, l);
        else if(command == "/show-files") show_files_command(c);
        else if(command == "/save-file") save_file_command(c);
        else{
            std::cin.getline(line + l, ChatMessage::max_body_length + 1);
            std::cout << "Wrong command! Commands list: /send-message ; /send-file ; /save-file ; /show-files" << std::endl;
        }
    }
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

        input_manager(c, line, l);

        c.close();
        t.join();
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
