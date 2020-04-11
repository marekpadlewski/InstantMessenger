#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../chat_message.h"

TEST(ChatMessageTest, InitDataTest){
    ChatMessage cm;
    EXPECT_EQ(ChatMessage::header_length, cm.get_header().size());
    EXPECT_EQ(ChatMessage::max_body_length, cm.get_body().size());
    EXPECT_EQ(0, cm.get_body_length());
}

TEST(ChatMessageTest, DataBodyTest){
    ChatMessage cm;
    std::string b_str = "Nickname: hello there";
    std::vector<char> body(b_str.begin(), b_str.end());

    cm.update_body_length(body.size());
    cm.set_body(body);

    std::vector<char> gb(cm.get_body());

    EXPECT_EQ(body.size(), cm.get_body_length());
    ASSERT_THAT(body,testing::ContainerEq(std::vector<char>(gb.begin(), gb.begin() + cm.get_body_length())));
}

TEST(ChatMessageTest, DataHeaderTest){
    ChatMessage cm;

    std::string type = "FILE";
    std::string filename = "test.txt";
    std::string b_str = "text.txtThis is the file content.\nSecond line of text.";
    std::string expected_header = "FILE  54   8";
    std::vector<char> body(b_str.begin(), b_str.end());

    cm.update_body_length(body.size());
    cm.set_body(body);
    cm.encode_header(type, filename.size());

    EXPECT_EQ(type, cm.get_type_from_header());
    EXPECT_EQ(filename.size(), cm.get_filename_len_from_header());
    EXPECT_TRUE(cm.decode_header());
    ASSERT_THAT(std::vector<char>(expected_header.begin(), expected_header.end()),
            testing::ContainerEq(cm.get_header()));
}
