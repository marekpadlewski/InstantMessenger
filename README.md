# Instant Messenger

### Build and compilation

```bash
mkdir downloads
mkdir build
cd build
cmake ..
make

# to run application
./server <port> [<port> ...]
./client <host> <port> <username>

# to run unit tests
cd messenger_tests
./messenger_tests_run
```

### Commands in application

```
/send-message <message_text>	| sends text message to chat participants
/send-file <path_to_file>	| sends file to chat participants
/save-file <file_name>		| saves file, you need to put extension in <file_name> 
/show-files			| shows list of file that you received from chat participants
```

Each file that you receive from chat praticipants is not saved immediately.  
You have to save specific file using `/save-file` command.  
File will be saved at `downloads` directory with the same name.  


