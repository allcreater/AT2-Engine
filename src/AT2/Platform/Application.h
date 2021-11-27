#pragma once

namespace AT2
{
	class Application
    {
    public:

    private:
        virtual void OnInitialize() {};
        virtual void OnRender(double dt) = 0;
    };
}