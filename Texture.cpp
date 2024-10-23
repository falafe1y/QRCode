#include "Texture.h"
#include "easylogging++.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
#include <d3d11.h>

Texture::Texture(std::string filepath, ID3D11Device* device, ID3D11DeviceContext* context)
{
    data = stbi_load(filepath.c_str(), &width, &height, &numcomponents, 0);
    create_texture(device, context);
}

Texture::Texture(std::shared_ptr<DB_logo> data, ID3D11Device* device, ID3D11DeviceContext* context)
    : width(data->width), height(data->height), numcomponents(data->numcomponents), data(data->logo.data())
{
    create_texture(device, context);
}

Texture::~Texture()
{
    if (textureView) {
        textureView->Release();
    }
    if (texture) {
        texture->Release();
    }
    if (samplerState) {
        samplerState->Release();
    }
    // Если data было выделено с помощью stbi_load, освобождаем память
    if (data) {
        stbi_image_free(data);
    }
}

DXGI_FORMAT Texture::image_format()
{
    switch (numcomponents)
    {
    case 1:
        return DXGI_FORMAT_R8_UNORM; // Для одного компонента
    case 2:
        return DXGI_FORMAT_R8G8_UNORM; // Для двух компонентов
    case 3:
        return DXGI_FORMAT_R8G8B8A8_UNORM; // Для трех компонентов мы используем RGBA (чтобы избежать проблем с DirectX)
    default:
        return DXGI_FORMAT_R8G8B8A8_UNORM; // По умолчанию RGBA
    }
}

ID3D11ShaderResourceView* Texture::get_textureID()
{
    return textureView;
}

std::vector<unsigned char> Texture::get_data()
{
    int size = width * height * numcomponents;
    return std::vector<unsigned char>(data, data + size);
}

ImVec2 Texture::get_size()
{
    return ImVec2(static_cast<float>(width), static_cast<float>(height));
}

int Texture::get_components()
{
    return numcomponents;
}

void Texture::create_texture(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (data)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = image_format();
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;
        initData.SysMemPitch = width * numcomponents;

        // Создаем текстуру
        if (FAILED(device->CreateTexture2D(&textureDesc, &initData, &texture)))
        {
            LOG(ERROR) << "Failed to create texture";
            return;
        }

        // Создаем представление текстуры
        if (FAILED(device->CreateShaderResourceView(texture, nullptr, &textureView)))
        {
            LOG(ERROR) << "Failed to create shader resource view";
            return;
        }

        // Создаем сэмплер
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

        if (FAILED(device->CreateSamplerState(&samplerDesc, &samplerState)))
        {
            LOG(ERROR) << "Failed to create sampler state";
        }

        // Установка сэмплера в контекст
        context->PSSetSamplers(0, 1, &samplerState);
    }
    else
    {
        LOG(ERROR) << "The data for texture is NULL";
    }
}
