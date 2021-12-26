#include "Renderer.h"

// Generating Metal-cpp implementation
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

//test
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

Renderer::Renderer(id nsWindow)
{
    //device.reset( MTL::CreateSystemDefaultDevice() );

}

void Renderer::Shutdown()
{
    //device.reset();
}

void Renderer::DispatchCompute(glm::uvec3 threadGroupSize)
{
    
}

void Renderer::Draw(Primitives::Primitive type, size_t first, long int count, int numInstances, int baseVertex)
{
    
}

void Renderer::SetViewport(const AABB2d& viewport)
{
    
}

void Renderer::ClearBuffer(const glm::vec4& color)
{
    
}

void Renderer::ClearDepth(float depth)
{
    
}

void Renderer::FinishFrame()
{
    
}

AT2::IFrameBuffer& Renderer::GetDefaultFramebuffer() const
{
    throw AT2::AT2Exception("");
}
