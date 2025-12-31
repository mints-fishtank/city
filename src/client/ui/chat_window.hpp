#pragma once

#include "core/net/message.hpp"
#include <vector>
#include <string>

namespace city {

struct ChatLine {
    std::string sender;
    std::string content;
    net::ChatChannel channel;
    u64 timestamp;
};

class ChatWindow {
public:
    ChatWindow() = default;

    void add_message(const net::ChatPayload& payload);
    void add_system_message(const std::string& content);

    void set_visible(bool visible) { visible_ = visible; }
    bool is_visible() const { return visible_; }

    void toggle_input();
    bool is_input_active() const { return input_active_; }

    // Get current input text
    const std::string& input_text() const { return input_text_; }

    // Handle character input
    void on_char_input(char c);
    void on_backspace();

    // Submit current input
    std::string submit();

    // Get recent messages for rendering
    const std::vector<ChatLine>& messages() const { return messages_; }

private:
    std::vector<ChatLine> messages_;
    std::string input_text_;
    bool visible_{true};
    bool input_active_{false};
    static constexpr size_t MAX_MESSAGES = 100;
};

} // namespace city
