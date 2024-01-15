#pragma once
#include <vector>
#include <memory>
#include <Helper/json.h>
#include <Helper/threading.h>
#include <d3d9.h>

/**
 * @file
 * @brief Descriptions and implementations of every pixel shader.
 * 
 * To implement a new shader:
 * - Create an HLSL file and add to the CMakeLists.txt
 * - Create a subclass of @ref PixelShader
 * - Implement the pure-virtual JSON and menu functionality, as well as assigning shader consts
 * - Add a shared instance of your new subclass to @ref Shader::PixelShader::all_types
 */

namespace Shader
{

/**
 * @brief A loaded pixel shader with a description, UI, and config implementation.
 * 
 * Subclasses are responsible for applying all the shader's constants before execution.
 */
class PixelShader : public Helper::JsonConfigurable
{
public:
    using Ptr = std::shared_ptr<PixelShader>;
    using ConstPtr = std::shared_ptr<const PixelShader>;
    virtual ~PixelShader();
    /// @brief Create a new instance of the same type
    /// @details This copies the base info (filename, displayname, desc, ptr)
    virtual std::shared_ptr<PixelShader> NewInstance() const = 0;
    /// @brief Set the shader variables in preparation for execution
    /// @param inv_projection The inverse of the 4x4 projection matrix
    /// @param depth A texture of the current depth data
    virtual void ApplyConstants(const float* inv_projection, IDirect3DTexture9* depth) = 0;
    /// @brief Display ImGui controls. Push an ID before calling.
    virtual void OnMenu() = 0;
    nlohmann::json ToJson() const override;

    const char* GetFileName() const { return m_filename; }
    const char* GetDisplayName() const { return m_displayname; }
    const char* GetDesc() const { return m_desc; }
    /// @brief The DirectX shader. May be `nullptr` if the shader was not loaded.
    IDirect3DPixelShader9* GetDxPtr() const { return m_ptr; };

    /// @brief The list of all loaded shaders. This will load all shaders for the first time.
    static Helper::LockedRef<const std::vector<ConstPtr>> GetLoadedShaders();
    /**
     * @brief Create a new instance from JSON.
     * @details This function will load all shaders if necessary and possible.
     * @param json The JSON object, as created by @ref ToJson. May be `nullptr`.
     * @return `nullptr` if the json object does not specify a valid filename
     */
    static std::shared_ptr<PixelShader> CreateFromJson(const nlohmann::json* json);

protected:
    /**
     * @brief Construct a pixel shader
     * @param filename The compiled shader file's name and extension. No directories included.
     * @param displayname A unique name to show in the UI
     * @param desc A user-friendly description of what the shader does
     */
    PixelShader(const char* filename, const char* displayname, const char* desc);
    PixelShader(const PixelShader& other);
    IDirect3DDevice9* GetDevice();
    /// @brief Convert properties to a JSON object without using the `m_filename` key.
    virtual nlohmann::json SubclassToJson() const = 0;

private:
    /// @brief The list of all shader types
    static const std::vector<std::shared_ptr<PixelShader>> all_types;
    /// @brief Load the shader. Only call once.
    bool Load(IDirect3DDevice9* device);

    const char* const m_filename;
    const char* const m_displayname;
    const char* const m_desc;
    /// @brief The loaded shader. If valid, it is released on destruction.
    IDirect3DPixelShader9* m_ptr = nullptr;
};

class DepthLinear : public PixelShader
{
public:
    DepthLinear() : PixelShader(
        "depth_linear.cso",
        "Depth (linear)",
        "Converts depth linearly to grayscale"
    ) {}

    std::shared_ptr<PixelShader> NewInstance() const override {
        return std::make_shared<DepthLinear>(*this);
    }
    void ApplyConstants(const float* inv_projection, IDirect3DTexture9* depth) override;
    void OnMenu() override;
    void FromJson(const nlohmann::json* json) override;

private:
    nlohmann::json SubclassToJson() const override;
    
    float m_near = 0;
    float m_far = 1000;
    bool m_spherical_correction = false;
};

class DepthLogarithm : public PixelShader
{
public:
    DepthLogarithm() : PixelShader(
        "depth_logarithm.cso",
        "Depth (logarithmic)",
        "Converts depth logarithmically grayscale"
    ) {}
    std::shared_ptr<PixelShader> NewInstance() const override {
        return std::make_shared<DepthLogarithm>(*this);
    }
    void ApplyConstants(const float* inv_projection, IDirect3DTexture9* depth) override;
    void OnMenu() override;
    void FromJson(const nlohmann::json* json) override;

private:
    nlohmann::json SubclassToJson() const override;

    float m_near = 0;
    float m_far = 1000;
    float m_near_precision = 1;
    bool m_spherical_correction = false;
};

class DepthToRgb : public PixelShader
{
public:
    DepthToRgb() : PixelShader(
        "depth_to_rgb.cso",
        "Depth (RGB)",
        "Converts depth to 24-bit RGB in the form of 0xRRGGBB"
    ) {}

    std::shared_ptr<PixelShader> NewInstance() const override {
        return std::make_shared<DepthToRgb>(*this);
    }
    void ApplyConstants(const float* inv_projection, IDirect3DTexture9* depth) override;
    void OnMenu() override;
    void FromJson(const nlohmann::json* json) override;

private:
    nlohmann::json SubclassToJson() const override;
};

class NormalToRgb : public PixelShader
{
public:
    NormalToRgb() : PixelShader(
        "normal_to_rgb.cso",
        "Normal map",
        "Reconstructs geometry from depth, creating an OpenGL-style normal map"
    ) {}

    std::shared_ptr<PixelShader> NewInstance() const override {
        return std::make_shared<NormalToRgb>(*this);
    }
    void ApplyConstants(const float* inv_projection, IDirect3DTexture9* depth) override;
    void OnMenu() override;
    void FromJson(const nlohmann::json* json) override;

private:
    nlohmann::json SubclassToJson() const override;
};

inline const std::vector<std::shared_ptr<PixelShader>> PixelShader::all_types = {
    std::make_unique<DepthLinear>(),
    std::make_unique<DepthLogarithm>(),
    std::make_unique<DepthToRgb>(),
    std::make_unique<NormalToRgb>(),
};

}