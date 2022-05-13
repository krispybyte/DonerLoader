#include "Utilities.hpp"

std::string Utilities::RandomString(const std::size_t Length)
{
    auto RandomCharacter = []() -> char
    {
        const char CharacterSet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        const std::size_t MaxIndex = (sizeof(CharacterSet) - 1);
        return CharacterSet[rand() % MaxIndex];
    };

    std::string String(Length, 0);
    std::generate_n(String.begin(), Length, RandomCharacter);
    return String;
}