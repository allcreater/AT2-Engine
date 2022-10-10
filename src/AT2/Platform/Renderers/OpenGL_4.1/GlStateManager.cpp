#include "GlStateManager.h"

#include <numeric>

#include "AT2lowlevel.h"
#include "GlBuffer.h"
#include "GlPipelineState.h"
#include "GlFrameBuffer.h"
#include "GlShaderProgram.h"
#include "GlTexture.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::OpenGL41;


GlStateManager::GlStateManager(IVisualizationSystem& renderer)
	: m_freeTextureSlots(renderer.GetRendererCapabilities().GetMaxNumberOfTextureUnits())
    , m_activeTextures(renderer.GetRendererCapabilities().GetMaxNumberOfTextureUnits())
{
    std::iota(m_freeTextureSlots.begin(), m_freeTextureSlots.end(), 0);
    std::reverse(m_freeTextureSlots.begin(), m_freeTextureSlots.end());
}

void GlStateManager::ApplyState(RenderState state)
{
    //TODO: remember current state or even state stack?
    std::visit(Utils::overloaded {
        [](const BlendColor& state){
            glBlendColor(state.Color.r, state.Color.g, state.Color.b, state.Color.a);
        },
        [](const FaceCullMode& state){
            const auto mode = Mappings::TranslateFaceCullMode(state);
            SetGlState(GL_CULL_FACE, mode != 0);
            glCullFace(mode);
        },
		[](const PolygonRasterizationMode& state){
            glPolygonMode(GL_FRONT_AND_BACK, Mappings::TranslatePolygonRasterizationMode(state));
        },
		[](const LineRasterizationMode& state){
			SetGlState(GL_LINE_SMOOTH, state == LineRasterizationMode::Smooth);
		}
    }, state);
}


void GlStateManager::Commit(const std::function<void(IUniformsWriter&)>& writeCommand)
{
    class ImmediateUniformWriter : public IUniformsWriter
    {
    public:
        explicit ImmediateUniformWriter(GlStateManager& stateManager)
    	: m_stateManager {stateManager}
    	, m_activeProgram {Utils::safe_dereference_cast<GlShaderProgram&>(m_stateManager.GetActiveShader()) }
    	{}

        void Write(std::string_view name, Uniform value) override { m_activeProgram.SetUniform(name, value); }
        void Write(std::string_view name, UniformArray value) override { m_activeProgram.SetUniformArray(name, value); }
        void Write(std::string_view name, std::shared_ptr<ITexture> texture) override
        {
	        m_activeProgram.SetUniform(name, static_cast<int>(m_stateManager.DoBind(std::move(texture))));
        }

        void Write(std::string_view name, std::shared_ptr<IBuffer> value) override
        {
            if (const auto location = m_activeProgram.GetUniformBufferLocation(name))
                m_stateManager.DoBind(*location, std::move(value));
        }

    private:
        GlStateManager& m_stateManager;
        GlShaderProgram& m_activeProgram;
    };

    ImmediateUniformWriter writer {*this};
    writeCommand(writer);
}

GlStateManager::TextureId GlStateManager::DoBind(std::shared_ptr<ITexture> texture)
{
    //TODO: use Strategy pattern
    //TODO: release textures with reference count == 1
    const auto texturesMapper = [this](const std::shared_ptr<ITexture>& texture) {
        assert(!m_freeTextureSlots.empty());

        const auto textureIndex = m_freeTextureSlots.back();

        const auto& glTexture = Utils::safe_dereference_cast<const GlTexture&>(texture);
        glActiveTexture(GL_TEXTURE0 + textureIndex);
        glBindTexture(glTexture.GetTarget(), glTexture.GetId());

        m_freeTextureSlots.pop_back();
        return std::tuple {textureIndex};
    };

    const auto textureUnmapper = [this](auto&& kv) { m_freeTextureSlots.push_back(kv.second); };

	return m_activeTextures.put(texture, texturesMapper, textureUnmapper).second;
}

void GlStateManager::DoBind(unsigned int index, const std::shared_ptr<IBuffer>& buffer) 
{
    const auto& glBuffer = Utils::safe_dereference_cast<const GlBuffer&>(buffer);
    
    //TODO: support for GL_SHADER_STORAGE_BUFFER, GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER ?
    //TODO: automatically decide which buffer type should be used
    assert(glBuffer.GetType() == VertexBufferFlags::UniformBuffer);
    glBindBufferBase(Mappings::TranslateBufferType(glBuffer.GetType()), index, glBuffer.GetId());
    //TODO: track buffer state, it's OpenGL with global state...
}

void GlStateManager::DoBind( IShaderProgram& shader )
{
    auto& glProgram = Utils::safe_dereference_cast<GlShaderProgram&>(&shader);
    glProgram.Bind();
}

void GlStateManager::DoBind( IVertexArray& vertexArray )
{
    glBindVertexArray(vertexArray.GetId());
    if (const auto indexBuffer = vertexArray.GetIndexBuffer())
    {
        const auto& glIndexBuffer = Utils::safe_dereference_cast<GlBuffer&>(indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer.GetId());
    }
}

// TODO: track states
void GlStateManager::ApplyPipelineState(const std::shared_ptr<IPipelineState>& state)
{
    auto& stateDescriptor = Utils::safe_dereference_cast<PipelineState&>(state).GetDescriptor();

    if (m_activeShader != stateDescriptor.GetShader())
    {
        DoBind(*stateDescriptor.GetShader());
        m_activeShader = stateDescriptor.GetShader();
    }

    [state = stateDescriptor.GetDepthState()](){
        SetGlState(GL_DEPTH_TEST, state.TestEnabled);
        glDepthMask(state.WriteEnabled);
        glDepthFunc(AT2::Mappings::TranslateCompareFunction(state.CompareFunc));
    }();
    
    if (stateDescriptor.AttachmentDescriptorsAreSame()) //TODO: OR OpenGL ES?
    {
        [desc = stateDescriptor.GetAttachmentDescriptor(0)](){
            auto colorMask = desc.Mask;
            glColorMask(colorMask.Contains(ColorWriteFlags::Red), colorMask.Contains(ColorWriteFlags::Green), colorMask.Contains(ColorWriteFlags::Blue), colorMask.Contains(ColorWriteFlags::Alpha));

            SetGlState(GL_BLEND, desc.BlendMode.Enabled);
            if (!desc.BlendMode.Enabled)
                return;

            glBlendFunc(Mappings::TranslateBlendFactor(desc.BlendMode.SourceFactor),
                    Mappings::TranslateBlendFactor(desc.BlendMode.DestinationFactor));
        }();
    }
    else
    {
        bool isBlendPresent = false;
        for (size_t fbIndex = 0; fbIndex < stateDescriptor.GetVertexArrayDescriptor().GetVertexAttributeLayouts().size(); ++fbIndex)
        {
            auto colorDesc = stateDescriptor.GetAttachmentDescriptor(fbIndex);
            if (colorDesc.BlendMode.Enabled)
            {
                isBlendPresent = true;
                glBlendFunci(fbIndex, Mappings::TranslateBlendFactor(colorDesc.BlendMode.SourceFactor), Mappings::TranslateBlendFactor(colorDesc.BlendMode.DestinationFactor));
            }
            else
                glBlendFunci(fbIndex, GL_ZERO, GL_ONE);

            glColorMaski(fbIndex, colorDesc.Mask.Contains(ColorWriteFlags::Red), colorDesc.Mask.Contains(ColorWriteFlags::Green), colorDesc.Mask.Contains(ColorWriteFlags::Blue), colorDesc.Mask.Contains(ColorWriteFlags::Alpha));
        }

        SetGlState(GL_BLEND, isBlendPresent);
    }

}

void GlStateManager::BindVertexArray(const std::shared_ptr<IVertexArray>& _vertexArray)
{
    if (m_activeVertexArray && m_activeVertexArray == _vertexArray)
        return;

    if (_vertexArray)
    {
        DoBind(*_vertexArray);

        m_activeIndexBufferType = _vertexArray->GetIndexBufferType();
    }

    m_activeVertexArray = _vertexArray;
}

std::optional<unsigned> GlStateManager::GetActiveTextureIndex(std::shared_ptr<ITexture> texture) const noexcept
{
    return m_activeTextures.find(texture);
}
