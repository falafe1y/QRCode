#pragma once
#include "DBHelper.h"
#include <memory>
#include <string>
#include <imgui.h>
#include <d3d11.h> // Добавьте заголовок для DirectX 11

class Texture
{
public:
    Texture(std::string filepath, ID3D11Device* device, ID3D11DeviceContext* context);
    Texture(std::shared_ptr<DB_logo> data, ID3D11Device* device, ID3D11DeviceContext* context);

    ~Texture();

    ID3D11ShaderResourceView* get_textureID(); // Замените на указатель на представление текстуры
    std::vector<unsigned char> get_data();
    ImVec2 get_size();
    int get_components();

private:
    void create_texture(ID3D11Device* device, ID3D11DeviceContext* context);
    DXGI_FORMAT image_format(); // Изменено на DXGI_FORMAT для DirectX

private:
    int width, height, numcomponents;
    unsigned char* data;
    ID3D11Texture2D* texture; // Замена на ID3D11Texture2D
    ID3D11ShaderResourceView* textureView; // Представление текстуры
    ID3D11SamplerState* samplerState; // Сэмплер для текстуры
};
