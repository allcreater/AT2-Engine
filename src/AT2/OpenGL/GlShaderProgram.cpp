#include "GlShaderProgram.h"
#include "GlUniformContainer.h"

using namespace AT2;

static const GLuint s_shaderGlType[] = { GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER };

static GLuint GetGlShaderType(GlShaderType _type)
{
	int typeIndex = (int)_type;
	assert(typeIndex >= 0 && typeIndex <= 5);
	return s_shaderGlType[typeIndex];
}

GlShaderProgram::GlShaderProgram()
{
	m_programId = glCreateProgram();
}

GlShaderProgram::GlShaderProgram(const str& _vs, const str& _tcs, const str& _tes, const str& _gs, const str& _fs) : GlShaderProgram()
{
	AttachShader(_vs, GlShaderType::Vertex);
	AttachShader(_tcs, GlShaderType::TesselationControl);
	AttachShader(_tes, GlShaderType::TesselationEvaluation);
	AttachShader(_gs, GlShaderType::Geometry);
	AttachShader(_fs, GlShaderType::Fragment);
	Compile();
}

GlShaderProgram::~GlShaderProgram()
{
	CleanUp();
	glDeleteProgram(m_programId);
	
}

void GlShaderProgram::AttachShader(const str& _code, GlShaderType _type)
{
	if (_code.empty())
		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, "GlShaderProgram: trying to attach empty shader");
	
	GLuint shaderId = LoadShader(GetGlShaderType(_type), _code);
	glAttachShader(m_programId, shaderId);
	
	m_shaderIds.push_back(shaderId);
}

bool GlShaderProgram::Compile()
{
	glLinkProgram(m_programId);
	GLint isLinked = 0;
	glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);

	//log
	GLchar infoLogBuffer[2048];
	GLint infoLogLength;
	glGetProgramInfoLog(m_programId, 2048, &infoLogLength, infoLogBuffer);
	if (infoLogLength > 0)
		Log::Debug() << "Shader program log: " << std::endl << infoLogBuffer;

	
	return isLinked;
	/*
	if (!isLinked)
		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, "GlShaderProgram: program not linked");
		*/
}

std::shared_ptr<IUniformContainer> GlShaderProgram::CreateAssociatedUniformStorage()
{
	return std::make_shared<AT2::GlUniformContainer>(shared_from_this());
}

GLuint GlShaderProgram::LoadShader(GLenum _shaderType, const str& _text)
{
	GLuint shader = glCreateShader(_shaderType);

	const GLchar* data = _text.c_str();
	auto dataLength = static_cast<GLint>(_text.length());
	glShaderSource(shader, 1, &data, &dataLength);

	glCompileShader(shader);


	
	GLint infoLogLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	std::string infoLog (infoLogLength, '\0');

	glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), &infoLogLength, infoLog.data());
	if (infoLogLength > 0)
		Log::Debug() << "Shader \"" << GetName() << "\" log: " << std::endl << infoLog;

	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::string shaderType = (_shaderType == GL_VERTEX_SHADER)? std::string("vertex") : 
			(_shaderType == GL_TESS_CONTROL_SHADER)? std::string("tesselation control") :
			(_shaderType == GL_TESS_EVALUATION_SHADER)? std::string("tesselation evaluation") :
			(_shaderType == GL_GEOMETRY_SHADER)? std::string("geometry") :
			(_shaderType == GL_FRAGMENT_SHADER)? std::string("fragment") :
			std::string("unknown");

		std::stringstream ss;
		ss << "GlShaderProgram:" << shaderType << " shader \"" << GetName() <<"\" compilation failed!" << std::endl << infoLog;

		throw AT2Exception(AT2::AT2Exception::ErrorCase::Shader, ss.str());
	}

	return shader;
}

void GlShaderProgram::CleanUp()
{
	for (const GLuint shaderId : m_shaderIds)
	{
		glDetachShader(m_programId, shaderId);
		glDeleteShader(shaderId);
	}

	m_shaderIds.clear();
}

void GlShaderProgram::Bind()
{
	glUseProgram(m_programId);
}

bool GlShaderProgram::IsActive() const
{
	GLint activeProgramId;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgramId);

	return m_programId == activeProgramId;
}

void GlShaderProgram::SetUBO(const str& blockName, unsigned int index)
{
	GLuint blockIndex = glGetUniformBlockIndex(m_programId, blockName.c_str());
	if (blockIndex != GL_INVALID_INDEX)
		glUniformBlockBinding(m_programId, blockIndex, index);
	else
		Log::Warning() << "Uniform block " << blockName << "not found" << std::endl;
}

std::shared_ptr<GlShaderProgram::UniformBufferInfo> GlShaderProgram::GetUniformBlockInfo(const str& blockName) const
{
	GLuint blockIndex = glGetUniformBlockIndex(m_programId, blockName.c_str());

	if (blockIndex == GL_INVALID_INDEX)
		return std::shared_ptr<GlShaderProgram::UniformBufferInfo>();

	auto ubi = std::make_shared<GlShaderProgram::UniformBufferInfo>();
	ubi->m_blockIndex = blockIndex;
	glGetActiveUniformBlockiv (m_programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE,       &ubi->m_blockSize );
	glGetActiveUniformBlockiv (m_programId, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &ubi->m_numActiveUniforms);

	//get active uniforms indices
	std::vector<GLuint> activeUniformIndices;
	activeUniformIndices.resize(ubi->m_numActiveUniforms);
	glGetActiveUniformBlockiv (m_programId, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast<GLint*>(activeUniformIndices.data()));

	const auto uniformCount = static_cast<GLsizei>(activeUniformIndices.size());

	//read uniform offsets
	std::vector<GLint> activeUniformOffsets;
	activeUniformOffsets.resize(ubi->m_numActiveUniforms);
	glGetActiveUniformsiv (m_programId, uniformCount, activeUniformIndices.data(), GL_UNIFORM_OFFSET, activeUniformOffsets.data());
		
	//read uniform types
	std::vector<GLint> activeUniformTypes;
	activeUniformTypes.resize(ubi->m_numActiveUniforms);
	glGetActiveUniformsiv (m_programId, uniformCount, activeUniformIndices.data(), GL_UNIFORM_TYPE, activeUniformTypes.data());
	
	//read array strides
	std::vector<GLint> activeUniformArrayStrides;
	activeUniformArrayStrides.resize(ubi->m_numActiveUniforms);
	glGetActiveUniformsiv(m_programId, uniformCount, activeUniformIndices.data(), GL_UNIFORM_ARRAY_STRIDE, activeUniformArrayStrides.data());

	//read matrix strides
	std::vector<GLint> activeUniformMatrixStrides;
	activeUniformMatrixStrides.resize(ubi->m_numActiveUniforms);
	glGetActiveUniformsiv(m_programId, uniformCount, activeUniformIndices.data(), GL_UNIFORM_MATRIX_STRIDE, activeUniformMatrixStrides.data());

	//read uniform names and collect all data into map
	GLint bufferLength = 512;
	glGetProgramiv(m_programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufferLength);
    std::string buffer (bufferLength, '\0');
	for (int i = 0; i < ubi->m_numActiveUniforms; ++i)
	{
		GLsizei actualLength = 0;
		glGetActiveUniformName(m_programId, activeUniformIndices[i], static_cast<GLsizei>(buffer.size()), &actualLength, buffer.data());

		//TODO: investigate, WTF
		//.data() call seems so peacful, safe, even redundant, but without it all brokes out by some strange reason
		ubi->m_uniforms[buffer.data()] = UniformInfo(activeUniformIndices[i], activeUniformOffsets[i], activeUniformTypes[i], activeUniformArrayStrides[i], activeUniformMatrixStrides[i]);
	}
	

	return ubi;
}
