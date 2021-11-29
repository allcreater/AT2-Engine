#pragma once

#include <AT2/AT2_types.hpp>

namespace AT2
{
	class Application
    {
    public:
        ~Application() = default;

    protected:
        virtual void OnInitialize() {};
        virtual void OnRender(AT2::Seconds dt) = 0;
    };
}