#pragma once

#include <stdexcept>
#include <string>

namespace AT2
{
    //AT2 basic exception class
    //Uses dynamic allocations. We must admit to ourselves that we will never be able to normally experience the conditions of a lack of memory
    //As we Russians say, "a barn is on fire - burn a hat too" :) 
    class AT2Exception : public std::runtime_error
    {
    public:
        AT2Exception(std::string errorMessage) : std::runtime_error("AT2Exception"), errorMessage(std::move(errorMessage)) {}

        [[nodiscard]] const char* what() const noexcept override { return errorMessage.c_str(); }

    private:
        std::string errorMessage;
    };

    class AT2NotImplementedException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

    class AT2RendererException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

    class AT2ShaderException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

    class AT2BufferException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

    class AT2TextureException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

    class AT2IOException : public AT2Exception
    {
    public:
        using AT2Exception::AT2Exception;
    };

} // namespace AT2
