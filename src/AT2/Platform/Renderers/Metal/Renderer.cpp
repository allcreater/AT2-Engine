#include "Renderer.h"

//test
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

using namespace std::literals;

// https://github.com/gzorin/sdl-metal-cpp-example

Renderer::Renderer(void* metalLayer)
{
    //device.reset( MTL::CreateSystemDefaultDevice() );
    
    //auto* nswin = static_cast<NSWindow*>(nsWindow);
    /*
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device.get();
    layer.pixelFormat = MTL::PixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;
     */
 
    swapchain = reinterpret_cast<CA::MetalLayer*>(metalLayer);
    device = swapchain->device();
    
    Log::Info() << "AT2 Metal renderer initialized"sv
                << "Device name: " << device->name()->cString(NS::ASCIIStringEncoding) << '\n';
    
    //library = device->librar
    commandQueue = device->newCommandQueue();
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
