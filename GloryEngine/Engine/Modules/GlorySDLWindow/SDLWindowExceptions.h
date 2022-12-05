#pragma once
#include <string>
#include <exception>

namespace Glory
{
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

    struct GetVulkanExtensionsException : public std::exception
    {
        const char* what() const throw ()
        {
            return "Could not get the names of required instance extensions from SDL.";
        }
    };
}
