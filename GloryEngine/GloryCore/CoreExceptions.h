#pragma once
#include <exception>
#include <string>

namespace Glory
{
    struct GameExistsException : public std::exception
    {
        const char* what() const throw ()
        {
            return "A GloryGame instance already exists! Please use GloryGame::GetGame() instead!";
        }
    };

    struct GameDoesNotExistException : public std::exception
    {
        const char* what() const throw ()
        {
            return "A GloryGame instance does not exist! Please call GloryGame::CreateGame() first!";
        }
    };
}