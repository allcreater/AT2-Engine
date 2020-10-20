#include "ShaderResource.h"

using namespace std::literals;

namespace
{
    //TODO: to utilities
    constexpr bool endsWith(std::string_view string, std::string_view suffix)
    {
        return string.length() >= suffix.length() && string.substr(string.length() - suffix.length()) == suffix;
    }

    std::optional<AT2::ShaderType> ShaderTypeByExtension(std::string_view filename)
    {
        constexpr std::array extensions = {
           std::pair {".vs.glsl"sv, AT2::ShaderType::Vertex},
           std::pair {".tcs.glsl"sv, AT2::ShaderType::TesselationControl},
           std::pair {".tes.glsl"sv, AT2::ShaderType::TesselationEvaluation},
           std::pair {".gs.glsl"sv, AT2::ShaderType::Geometry},
           std::pair {".fs.glsl"sv, AT2::ShaderType::Fragment},
           std::pair {".cs.glsl"sv, AT2::ShaderType::Computational}
        };

        for (const auto& [extension, type] : extensions)
            if (endsWith(filename, extension))
                return type;

        return {};
    }

    //std::optional<AT2::ShaderType> ShaderTypeByName(std::string_view filename)
    //{
    //    constexpr std::array extensions = {std::pair {".vs"sv, AT2::ShaderType::Vertex},
    //                                       std::pair {".tcs"sv, AT2::ShaderType::TesselationControl},
    //                                       std::pair {".tes"sv, AT2::ShaderType::TesselationEvaluation},
    //                                       std::pair {".gs"sv, AT2::ShaderType::Geometry},
    //                                       std::pair {".fs"sv, AT2::ShaderType::Fragment},
    //                                       std::pair {".cs"sv, AT2::ShaderType::Computational}};

    //    for (const auto& [extension, type] : extensions)
    //        if (filename.find(extension) != std::string_view::npos)
    //            return type;

    //    return {};
    //}

}

using namespace AT2;


std::shared_ptr<void> AT2::Resources::ShaderResource::Load(IResourceFactory& resourceFactory)
{
    if (std::any_of(m_sources.begin(), m_sources.end(),
                    [](const std::unique_ptr<IDataSource>& source) { return !source->IsReloadable(); }))
        return nullptr;

    auto newProgram = resourceFactory.CreateShaderProgram();
    for (auto& source : m_sources)
    {
        auto stream = source->Load();

        //TODO: check source names, not exactly FILEname
        if (auto* fileSource = dynamic_cast<FileDataSource*>(source.get()))
        {
            newProgram->AttachShader(
                std::string {std::istreambuf_iterator<char>(*stream), std::istreambuf_iterator<char>()},
                ShaderTypeByExtension(fileSource->getFilename().filename().u8string()).value()
            );
        }
    }

    return newProgram;
}

