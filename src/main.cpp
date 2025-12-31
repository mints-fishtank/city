#include <iostream>
#include <format>
#include <vector>
#include <ranges>

int main() {
    std::cout << std::format("Hello from {}!\n", "city");

    // C++20 ranges example
    std::vector<int> numbers{1, 2, 3, 4, 5};

    auto even_doubled = numbers
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * 2; });

    std::cout << "Even numbers doubled: ";
    for (int n : even_doubled) {
        std::cout << n << " ";
    }
    std::cout << "\n";

    return 0;
}
