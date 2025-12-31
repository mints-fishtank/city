#include "chat_window.hpp"
#include <chrono>

namespace city {

void ChatWindow::add_message(const net::ChatPayload& payload) {
    ChatLine line{
        .sender = payload.sender,
        .content = payload.content,
        .channel = payload.channel,
        .timestamp = static_cast<u64>(
            std::chrono::system_clock::now().time_since_epoch().count()
        )
    };

    messages_.push_back(std::move(line));

    // Trim old messages
    while (messages_.size() > MAX_MESSAGES) {
        messages_.erase(messages_.begin());
    }
}

void ChatWindow::add_system_message(const std::string& content) {
    net::ChatPayload payload{
        .channel = net::ChatChannel::System,
        .sender = "",
        .target = "",
        .content = content
    };
    add_message(payload);
}

void ChatWindow::toggle_input() {
    input_active_ = !input_active_;
    if (!input_active_) {
        input_text_.clear();
    }
}

void ChatWindow::on_char_input(char c) {
    if (!input_active_) return;
    if (input_text_.size() < 256) {
        input_text_ += c;
    }
}

void ChatWindow::on_backspace() {
    if (!input_active_) return;
    if (!input_text_.empty()) {
        input_text_.pop_back();
    }
}

std::string ChatWindow::submit() {
    if (!input_active_ || input_text_.empty()) {
        return "";
    }

    std::string result = std::move(input_text_);
    input_text_.clear();
    input_active_ = false;
    return result;
}

} // namespace city
