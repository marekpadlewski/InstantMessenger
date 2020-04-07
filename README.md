# Instant Messenger

### Build and compilation

```
cd build
cmake ..
make

./server <port>
./client <host> <port> <username>
```

### Commands in application

```
/send-message <message_text>
/send-file <path_to_file>
```

Each file that you recieve from chat members is saved in `downloads` directory with name `fileX`, where X is a number.


