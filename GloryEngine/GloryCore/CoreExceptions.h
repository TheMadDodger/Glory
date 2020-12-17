#pragma once
#include <exception>

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

    struct SDLErrorException : public std::exception
    {
        SDLErrorException(const char* pSDLError) : m_SDLError("SDL encountered an error!\nSDL Error:\n" + std::string(pSDLError)) {}

        const char* what() const throw ()
        {
            return m_SDLError.c_str();
        }

    private:
        const std::string m_SDLError;
    };
}