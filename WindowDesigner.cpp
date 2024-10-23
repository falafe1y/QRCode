#include "WindowDesigner.h"
#include <imgui_internal.h>
#include <d3d11.h>
#include "ImFileDialog.h"

#define FILE_DIALOG_ID "FileDialog"
#define FILE_DIALOG_TITLE "Open CSV file"
#define FILE_DIALOG_LOGO_ID "FileDialogLogo"
#define FILE_DIALOG_LOGO_TITLE "Select logo"
#define MAIN_WND_TITLE "QR Codes"
#define DB_TAB_NAME "Database"

#define TABLE_TITLE_USER_NAME "Name"
#define TABLE_TITLE_TITLE "Title 1"
#define TABLE_TITLE_TITLE2 "Title 2"
#define TABLE_TITLE_QRCODE "QR code"
#define TABLE_TITLE_LOGO "logo"
#define TABLE_TITLE_BACKGROUND_COLOR "background color"
#define TABLE_TITLE_QR_CODE "QR code"
#define TABLE_TITLE_BUTTON "Operations"
#define TABLE_WIDTH_USER_NAME 0.35f
#define TABLE_WIDTH_TITLE 0.2f
#define TABLE_WIDTH_TITLE2 0.2f
#define TABLE_WIDTH_QRCODE 0.15f

#define LOGO_DIMENSION ImVec2(500, 500)
#define LOGO_BORDER_COLOR "#454545"

#define BUTTON_LOGO_LABEL "Logo" 
#define POPUP_LOGO_LABEL "Create logo##create_logo"

#define MENU_FILE_LABEL "File"
#define SUBMENU_FILE_OPEN_LABEL "Open"
#define SUBMENU_FILE_OPEN_DB_LABEL "Open DB"
#define SUBMENU_FILE_EXIT_LABEL "Quit"

#define BUTTON_BRANDING_LABEL "Branding"
#define POPUP_BRANDING_LABEL "Branding##branding"
#define BUTTON_BRANDING_CLOSE "Ok##close_branding"

#define BUTTON_RUN_SHOW_LABEL "Run Show"
#define POPUP_RUN_SHOW_LABEL "Run Show##runshow"
#define BUTTON_RUN_SHOW_CLOSE "Ok##close_runshow"

#define BUTTON_SOFTWARE_EXP_LABEL(x) ("Software Expires in " +  std::string(#x) +" Days").c_str()
#define POPUP_SOFTWARE_EXP_LABEL "License##license"
#define BUTTON_SOFTWARE_EXP_CLOSE "Ok##close_license"

#define BUTTON_HELP_LABEL "Help"
#define POPUP_HELP_LABEL "Help##help"
#define BUTTON_HELP_CLOSE "Ok##close_help"

#define ERROR_INCORRECT_FILE_NAME_WILDCHAR "Incorrect file name. Wild characters are not allowed.\n"\
"The file name consist hidden wildchar characters. To point problem more deeper please visit:\n"\
"https://stackoverflow.com/questions/73284766/stdsystemerror-exception-during-recursive-directory-iterator-unicode-trans"

#define SCAN_QRCODE_INFO "To scan QR code please push OK button,then your device should send ENTER key at the scanning end.\n"\
"At the same time QR code should be visible in input field.\n"\
"By the way you could cancel qr code scanning by pressing ESC button.\n"\
"(At the scan time the window will be frozen and not responding while not will be pushed the ESC or the ENTER keys.)"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

extern bool GlobalRunning;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;
D3D_FEATURE_LEVEL featureLevel;

//void CleanupD3D() {
//    if (renderTargetView) renderTargetView->Release();
//    if (swapChain) swapChain->Release();
//    if (context) context->Release();
//    if (device) device->Release();
//}

Row::Row(std::shared_ptr<DB_user> data, Tab* parent) : data(data), changed(false), bufer(new Bufer_user(data)), row_id(counter++), tab(parent) {}

Row::~Row() {
    delete bufer;
}

bool Row::is_changed() { update_data();  return changed; }

void Row::update_data() {
    std::string un = std::string(bufer->user_name);
    if (un != data->user_name) {
        data->user_name = un;
        changed = true;
    }
    std::string t = std::string(bufer->title);
    if (t != data->title) {
        data->title = t;
        changed = true;
    }
    std::string t2 = std::string(bufer->title2);
    if (t2 != data->title2) {
        data->title2 = t2;
        changed = true;
    }
    std::string qr = std::string(bufer->qr_code);
    if (qr != data->qr_code) {
        data->qr_code = qr;
        changed = true;
    }
}
#pragma optimize("", off)
void Row::render(int posx, int posy, int width, int height) {
    update_data();
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(width * TABLE_WIDTH_USER_NAME);
    ImGui::InputText(("##user_name" + std::to_string(row_id)).c_str(), bufer->user_name, BUFER_SIZE);
    ImGui::PopItemWidth();
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(width * TABLE_WIDTH_TITLE);
    ImGui::InputText(("##title" + std::to_string(row_id)).c_str(), bufer->title, BUFER_SIZE);
    ImGui::PopItemWidth();
    ImGui::TableSetColumnIndex(2);
    ImGui::PushItemWidth(width * TABLE_WIDTH_TITLE2);
    ImGui::InputText(("##title2" + std::to_string(row_id)).c_str(), bufer->title2, BUFER_SIZE);
    ImGui::PopItemWidth();
    ImGui::TableSetColumnIndex(3);
    ImGui::PushItemWidth(width * TABLE_WIDTH_QRCODE);
    // TODO: ������������ � �������� ������������ (����� �������� ��� ������ �������� ������ � ���� �����.)
    ImGui::InputText(("##qr_code" + std::to_string(row_id)).c_str(), bufer->qr_code, BUFER_SIZE/*, ImGuiInputTextFlags_ReadOnly*/);
    ImGui::PopItemWidth();
    ImGui::TableSetColumnIndex(4);
    if (ImGui::Button(("Delete##delete" + std::to_string(row_id)).c_str())) {
        tab->delete_row(this);
    }
    ImGui::SameLine();
    if (ImGui::Button(("Scan QR Code (keyloger)##scan" + std::to_string(row_id)).c_str())) {
        ImGui::OpenPopup(("Scan QR code##scan_qr_code" + std::to_string(row_id)).c_str());
    }
    if (ImGui::BeginPopupModal(("Scan QR code##scan_qr_code" + std::to_string(row_id)).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(SCAN_QRCODE_INFO);
        if (ImGui::Button("Ok##scan_qr_code_ok")) {
            ImGui::CloseCurrentPopup();
            std::string qr_code = "";
            keylogger(qr_code);
            strcpy_s(bufer->qr_code, qr_code.data());
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##scan_qr_code_cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
#pragma optimize("", on)

int Row::counter = 0;

//////////////////////////////////////////////////////////////////

Tab::Tab(std::shared_ptr<DBHelper> helper, MainWindow* window) : db_helper(helper), csv_helper(nullptr), opened(true), window(window), selected(false) {}

Tab::Tab(std::string path, std::shared_ptr<DBHelper> helper, MainWindow* window) : db_helper(helper), opened(true), window(window), selected(false) {
    csv_helper = new CSVHelper();
    csv_helper->read_csv_file(path);
    name = get_file_name(path);
    tab_id = name + "##" + std::to_string(count++);
    std::string tab_id = name + std::to_string(count++);
    init();
}

Tab::~Tab() {
    if (csv_helper != nullptr) {
        delete csv_helper;
    }
    for (size_t i = 0; i < rows.size(); i++) { delete rows[i]; }
    rows.clear();
}

void Tab::init(bool select) {
    selected = select;
    for (size_t i = 0; i < rows.size(); i++) { delete rows[i]; }
    rows.clear();
    auto data = csv_helper->get_all_data<std::string>();
    for (auto row : data) {
        std::shared_ptr<DB_user> data_user;
        //read incomplete data: only user_name and title
        if (row.size() < 4) {
            data_user = std::shared_ptr<DB_user>(new DB_user(-1, row[0], row[1], row[2], ""));
        }
        else {
            data_user = std::shared_ptr<DB_user>(new DB_user(-1, row[0], row[1], row[2], row[3]));
        }

        rows.push_back(new Row(data_user, this));
    }
    filter();
}

void Tab::update_search_name_and_filter(size_t new_size, char* bufer)
{
    search_name.resize(new_size);
    search_name = std::string(bufer);
    filter();
}

void Tab::filter()
{
    if (search_name.length() > 0) {
        rows_filtered.clear();
        std::copy_if(rows.begin(), rows.end(), std::back_inserter(rows_filtered), [&](Row* row) { return row->get_data()->user_name.find(search_name) != std::string::npos; });
    }
    else {
        rows_filtered = rows;
    }
}

bool Tab::is_changed() {
    return std::find_if(rows.begin(), rows.end(), [](Row* row) { return row->is_changed();}) != rows.end();
}

std::vector<Row*> Tab::get_changed() {
    std::vector<Row*> changed_rows;
    std::copy_if(rows.begin(), rows.end(), std::back_inserter(changed_rows), [&](Row* row) { return row->is_changed();});
    return changed_rows;
}

size_t Tab::count = 0;

int Tab::change_filter_user_name_callback(ImGuiInputTextCallbackData* data) {
    auto self = reinterpret_cast<Tab*>(data->UserData);
    self->update_search_name_and_filter(data->BufTextLen, data->Buf);
    return 0;
}

void Tab::render(int posx, int posy, int width, int height) {
    ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
    if (selected) {
        flags |= ImGuiTabItemFlags_SetSelected;
        selected = false;
    }
    if (ImGui::BeginTabItem(tab_id.c_str(), &opened, flags)) {

        // Вызов InputTextWithHint с placeholder "Search"
        ImGui::InputTextWithHint(("##search_user_name" + tab_id).c_str(), "Search", search_name.data(), BUFER_SIZE, ImGuiInputTextFlags_CallbackEdit, Tab::change_filter_user_name_callback, this);
        ImGuiTableFlags flags1 = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;// | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_Resizable;
        auto inner_width = width - 200;
        if (ImGui::BeginTable("table1", 5, flags1, ImVec2(0.0f, 0.0f), inner_width))
        {
            // We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
            ImGui::TableSetupColumn(TABLE_TITLE_USER_NAME, ImGuiTableColumnFlags_WidthFixed, TABLE_WIDTH_USER_NAME * inner_width);
            ImGui::TableSetupColumn(TABLE_TITLE_TITLE, ImGuiTableColumnFlags_WidthFixed, TABLE_WIDTH_TITLE * inner_width);
            ImGui::TableSetupColumn(TABLE_TITLE_TITLE2, ImGuiTableColumnFlags_WidthFixed, TABLE_WIDTH_TITLE2 * inner_width);
            ImGui::TableSetupColumn(TABLE_TITLE_QRCODE, ImGuiTableColumnFlags_WidthFixed, TABLE_WIDTH_QRCODE * inner_width);
            ImGui::TableSetupColumn(TABLE_TITLE_BUTTON, ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (size_t i = 0;i < rows_filtered.size();i++) {
                rows_filtered[i]->render(posx, posy, inner_width, height);
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        if (ImGui::Button("Save")) {
            save();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add user")) {
            add_user();
        }
        ImGui::EndTabItem();
    }
    if (!opened) {
        window->delete_tab(this, false);
    }
}

//save all data in rows to db
void Tab::save() {
    // ����� �� ������� ������������ � ����������� � ��� ��� ���������� ������ �������������� ����������.
    search_name = "";
    std::vector<std::shared_ptr<DB_user>> cont;
    std::transform(rows.begin(), rows.end(), std::back_inserter(cont), [](Row* row) { return row->get_data(); });
    db_helper->insert_scope_user(cont);
    window->delete_tab(this, true);
}

void Tab::add_user()
{
    // ����� �� ������� ������������ � ����������� � ��� ��� ���������� ������ �������������� ����������.
    search_name = "";
    std::shared_ptr<DB_user> data_user = std::shared_ptr<DB_user>(new DB_user(-1, "", "", "", ""));
    rows.push_back(new Row(data_user, this));
    filter();
}

void Tab::delete_row(Row* row)
{
    rows.erase(std::remove(rows.begin(), rows.end(), row), rows.end());
    filter();
    if (rows.size() == 0) {
        window->delete_tab(this, false);
    }
}

/////////////////////////////////////////////////////////////////


DBTab::DBTab(std::shared_ptr<DBHelper> helper, MainWindow* window) :Tab(helper, window) {
    name = DB_TAB_NAME;
    tab_id = name + "##1";
    init();
}
DBTab::~DBTab() {}
//reload all data from db
void DBTab::init(bool select) {
    selected = select;
    for (size_t i = 0;i < rows.size();i++) {
        delete rows[i];
    }
    rows.clear();
    if (db_helper->get_rows_count_user() > 0) {
        for (auto d : db_helper->get_data_user()) {
            rows.push_back(new Row(d, this));
        }
        filter();
    }
}

//refresh only changed rows in db
void DBTab::save() {
    std::vector<std::shared_ptr<DB_user>> cont;
    std::vector<Row*> changed = get_changed();
    std::transform(changed.begin(), changed.end(), std::back_inserter(cont), [](Row* row) { return row->get_data(); });
    db_helper->add_callback(std::bind(&DBTab::init, this, false));
    db_helper->update_scope_user(cont);
}

void DBTab::delete_row(Row* row)
{
    auto data = row->get_data();
    if (data->id == -1) {
        return Tab::delete_row(row);
    }
    db_helper->add_callback(std::bind(&DBTab::init, this, false));
    db_helper->delete_user(data);
    db_helper->refresh_user();
}

/////////////////////////////////////////////////////////////////

MainWindow::MainWindow() {
    // Инициализация лямбда-функции для создания текстуры
    ifd::FileDialog::Instance().CreateTexture = [this](uint8_t* data, int w, int h, char fmt) -> void* {
        ID3D11Texture2D* texture = nullptr;
        ID3D11ShaderResourceView* textureView = nullptr;
        ID3D11SamplerState* samplerState = nullptr; // Локальный сэмплер

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = w;
        textureDesc.Height = h;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;
        initData.SysMemPitch = w * 4;

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &device,
            nullptr,
            &context
        );

        // Создаем текстуру
        device->CreateTexture2D(&textureDesc, &initData, &texture);
        if (FAILED(hr)) {
            // Обработка ошибки
            return nullptr;
        }

        // Создаем представление для текстуры
        hr = device->CreateShaderResourceView(texture, nullptr, &textureView);
        if (FAILED(hr)) {
            if (texture) texture->Release();
            return nullptr;
        }

        // Привязываем текстуру к пиксельному шейдеру
        context->PSSetShaderResources(0, 1, &textureView);

        // Создаем сэмплер для текстуры
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

        hr = device->CreateSamplerState(&samplerDesc, &samplerState);
        if (FAILED(hr)) {
            textureView->Release();
            if (texture) texture->Release();
            return nullptr;
        }

        // Устанавливаем сэмплер в пиксельный шейдер
        context->PSSetSamplers(0, 1, &samplerState);

        return (void*)textureView;  // Возвращаем представление текстуры
        };

    // Лямбда-функция для удаления текстуры
    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        ID3D11ShaderResourceView* textureView = (ID3D11ShaderResourceView*)tex;
        if (textureView) {
            textureView->Release();
        }
        // Убедитесь, что вы сохраняете ссылки на сэмплер и текстуру в другом месте, чтобы освободить их
        };

    db_helper = std::shared_ptr<DBHelper>(new DBHelper());
    db_tab_initialized = false;
    logo = nullptr;
    logo_popup_opened = true;
}


void MainWindow::init_DB_tab() {
    tabs.push_back(new DBTab(db_helper, this));
}

void MainWindow::delete_tab(Tab* tab, bool refresh_db_tab)
{
    tabs.erase(std::remove(tabs.begin(), tabs.end(), tab), tabs.end());
    if (refresh_db_tab) {
        auto db_tab = get_DB_tab();
        if (db_tab) {
            db_tab->init(true);
        }
    }
}

Tab* MainWindow::get_DB_tab()
{
    auto db_tab = std::find_if(tabs.begin(), tabs.end(), [&](Tab* tab) { return tab->get_name() == DB_TAB_NAME; });
    if (db_tab != tabs.end()) {
        return *db_tab;
    }
    return nullptr;
}

MainWindow::~MainWindow() {
    delete logo;
    for (size_t i = 0; i < tabs.size(); i++) {
        delete tabs[i];
    }
    tabs.clear();
    ifd::FileDialog::Instance().DeleteTexture = nullptr;
    ifd::FileDialog::Instance().CreateTexture = nullptr;
    ifd::FileDialog::Instance().Close();
}

void MainWindow::check_file_result() {
    // file dialogs
    if (ifd::FileDialog::Instance().IsDone(FILE_DIALOG_ID)) {
        if (ifd::FileDialog::Instance().HasResult()) {
            const std::vector<std::filesystem::path>& res = ifd::FileDialog::Instance().GetResults();
            for (const auto& r : res) {
                // ����� ���� ������ ���� � ����� ����� ���������� ��������� ����������
                // (��. https://stackoverflow.com/questions/73284766/stdsystemerror-exception-during-recursive-directory-iterator-unicode-trans)
                const std::wstring currentPath = r.c_str();
                std::string path = wide_string_to_string(currentPath);
                //���� ���������� ���������� ��������� �� ���� �� ���������� (���� �� ����������)
                if (!file_exists(path)) {
                    ImGui::OpenPopup("Alert##csv_incorrect_file_name");
                    if (ImGui::BeginPopupModal("Alert##csv_incorrect_file_name", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text(ERROR_INCORRECT_FILE_NAME_WILDCHAR);
                        if (ImGui::Button("Ok##close_csv_incorrect_file_name")) {
                            ImGui::CloseCurrentPopup();
                            ifd::FileDialog::Instance().Close();
                        }
                        ImGui::EndPopup();
                    }
                    return;
                }
                auto tab = new Tab(path, db_helper, this);
                tabs.push_back(tab);
            }
        }
        ifd::FileDialog::Instance().Close();
    }
}

void MainWindow::set_texture(ID3D11ShaderResourceView* texture, int image_width, int image_height) {
    myTexture = texture;
    this->image_width = image_width;
    this->image_height = image_height;
}

void MainWindow::render(int posx, int posy, int width, int height) {
    static bool initiated = false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGui::SetNextWindowSize(ImVec2{ (float)width, (float)height });

    ID3D11ShaderResourceView* Logotype = nullptr;

    ImGuiWindowFlags flags_wnd = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    bool open = true;
    
    if (ImGui::Begin(MAIN_WND_TITLE, &open, flags_wnd)) {
        ImGui::Image((void*)myTexture, ImVec2(image_width, image_height));
        if (ImGui::Button(MENU_FILE_LABEL)) {
            ImGui::OpenPopup("FileMenuPopup");
        }

        if (ImGui::BeginPopup("FileMenuPopup")) {
            if (ImGui::MenuItem(SUBMENU_FILE_OPEN_LABEL)) {
                ifd::FileDialog::Instance().Open(FILE_DIALOG_ID, FILE_DIALOG_TITLE, "Image file (*.csv){.csv},.*", false);
            }
            if (ImGui::MenuItem(SUBMENU_FILE_OPEN_DB_LABEL)) {
                auto tab = get_DB_tab();
                if (tab) {
                    tab->init(true);
                }
                else {
                    init_DB_tab();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(SUBMENU_FILE_EXIT_LABEL)) {
                GlobalRunning = false;
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(BUTTON_BRANDING_LABEL)) {
            ImGuiPopupFlags flag = ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonLeft;
            ImGui::OpenPopup(POPUP_BRANDING_LABEL, flag);
        }

        ImGui::SameLine();

        if (ImGui::BeginPopupModal(POPUP_BRANDING_LABEL, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Element is under construction...");
            if (ImGui::Button(BUTTON_BRANDING_CLOSE)) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(BUTTON_RUN_SHOW_LABEL)) {
            ImGuiPopupFlags flag = ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonLeft;
            ImGui::OpenPopup(POPUP_RUN_SHOW_LABEL, flag);
        }

        ImGui::SameLine();

        if (ImGui::BeginPopupModal(POPUP_RUN_SHOW_LABEL, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Element is under construction...");
            if (ImGui::Button(BUTTON_RUN_SHOW_CLOSE)) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(BUTTON_SOFTWARE_EXP_LABEL(100))) {
            ImGuiPopupFlags flag = ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonLeft;
            ImGui::OpenPopup(POPUP_SOFTWARE_EXP_LABEL, flag);
        }

        ImGui::SameLine();

        if (ImGui::BeginPopupModal(POPUP_SOFTWARE_EXP_LABEL, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Element is under construction...");
            if (ImGui::Button(BUTTON_SOFTWARE_EXP_CLOSE)) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(BUTTON_HELP_LABEL)) {
            ImGuiPopupFlags flag = ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonLeft;
            ImGui::OpenPopup(POPUP_HELP_LABEL, flag);
        }

        if (ImGui::BeginPopupModal(POPUP_HELP_LABEL, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Element is under construction...");
                if (ImGui::Button(BUTTON_HELP_CLOSE)) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
        }

        check_file_result();
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll;;
        if (ImGui::BeginTabBar("Tabs", tab_bar_flags)) {
            if (!db_tab_initialized) {
                init_DB_tab();
                db_tab_initialized = true;
            }
            for (size_t i = 0; i < tabs.size(); i++) {
                tabs[i]->render(posx, posy, width, height);
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    if (!open) {
        GlobalRunning = false;
    }
}

void MainWindow::set_styles() {
    ImGuiStyle* style = &ImGui::GetStyle();
    int hspacing = 8;
    int vspacing = 6;
    style->DisplaySafeAreaPadding = ImVec2(0, 0);
    style->WindowPadding = ImVec2(hspacing / 2, vspacing);
    style->FramePadding = ImVec2(hspacing, vspacing);
    style->ItemSpacing = ImVec2(hspacing, vspacing);
    style->ItemInnerSpacing = ImVec2(hspacing, vspacing);
    style->IndentSpacing = 20.0f;

    style->WindowRounding = 0.0f;
    style->FrameRounding = 0.0f;

    style->WindowBorderSize = 0.0f;
    style->FrameBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;

    style->ScrollbarSize = 20.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 0.0f;

    ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    ImVec4 transparent = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    ImVec4 dark = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
    ImVec4 darker = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

    ImVec4 background = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    ImVec4 text = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    ImVec4 border = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    ImVec4 grab = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    ImVec4 header = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 active = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
    ImVec4 hover = ImVec4(0.00f, 0.47f, 0.84f, 0.20f);

    style->Colors[ImGuiCol_Text] = text;
    style->Colors[ImGuiCol_WindowBg] = background;
    style->Colors[ImGuiCol_ChildBg] = background;
    style->Colors[ImGuiCol_PopupBg] = white;

    style->Colors[ImGuiCol_Border] = border;
    style->Colors[ImGuiCol_BorderShadow] = transparent;

    style->Colors[ImGuiCol_Button] = header;
    style->Colors[ImGuiCol_ButtonHovered] = hover;
    style->Colors[ImGuiCol_ButtonActive] = active;

    style->Colors[ImGuiCol_FrameBg] = white;
    style->Colors[ImGuiCol_FrameBgHovered] = hover;
    style->Colors[ImGuiCol_FrameBgActive] = active;

    style->Colors[ImGuiCol_MenuBarBg] = header;
    style->Colors[ImGuiCol_Header] = header;
    style->Colors[ImGuiCol_HeaderHovered] = hover;
    style->Colors[ImGuiCol_HeaderActive] = active;

    style->Colors[ImGuiCol_CheckMark] = text;
    style->Colors[ImGuiCol_SliderGrab] = grab;
    style->Colors[ImGuiCol_SliderGrabActive] = darker;

    style->Colors[ImGuiCol_ScrollbarBg] = header;
    style->Colors[ImGuiCol_ScrollbarGrab] = grab;
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = dark;
    style->Colors[ImGuiCol_ScrollbarGrabActive] = darker;

    style->Colors[ImGuiCol_ChildBg] = white;
}

/////////////////////////////////////////////////////////////////

Logo::Logo(std::shared_ptr<DBHelper> database) : database(database), changed(false) {
    // ImFileDialog requires you to set the CreateTexture and DeleteTexture
    ifd::FileDialog::Instance().CreateTexture = [this](uint8_t* data, int w, int h, char fmt) -> void* {
        ID3D11Texture2D* texture = nullptr;
        ID3D11ShaderResourceView* textureView = nullptr;
        ID3D11SamplerState* samplerState = nullptr; // Локальный сэмплер

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = w;
        textureDesc.Height = h;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;
        initData.SysMemPitch = w * 4;

        // Создаем текстуру
        device->CreateTexture2D(&textureDesc, &initData, &texture);

        // Создаем представление для текстуры
        device->CreateShaderResourceView(texture, nullptr, &textureView);

        // Привязываем текстуру к пиксельному шейдеру
        context->PSSetShaderResources(0, 1, &textureView);

        // Создаем сэмплер для текстуры
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // Линейная фильтрация
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

        device->CreateSamplerState(&samplerDesc, &samplerState);
        context->PSSetSamplers(0, 1, &samplerState);

        return (void*)textureView;  // Возвращаем представление текстуры
        };

    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        ID3D11ShaderResourceView* textureView = (ID3D11ShaderResourceView*)tex;
        if (textureView) {
            textureView->Release();
        }
        };

    data = database->get_logo();
    bufer = new Buffer_logo(data);
    texture = new Texture(data, device, context); // Убедитесь, что передаете device и context
}

void Logo::update_data()
{
    std::string bc = ImVec42Hex(bufer->background_color);
    if (bc != data->background_color) {
        data->background_color = bc;
        changed = true;
    }
    std::string ln = std::string(bufer->logo_name);
    if (ln != data->logo_name) {
        data->logo_name = ln;
        changed = true;
    }
    //TODO: �����������
    std::vector<unsigned char> l = texture->get_data();
    if (l != data->logo) {
        data->logo = l;
        changed = true;
    }
    ImVec2 dimension = texture->get_size();
    if (data->width != dimension.x) {
        data->width = dimension.x;
        changed = true;
    }
    if (data->height != dimension.y) {
        data->height = dimension.y;
        changed = true;
    }
    int c = texture->get_components();
    if (data->numcomponents != c) {
        data->numcomponents = c;
        changed = true;
    }
}

Logo::~Logo()
{
    delete bufer;
    ifd::FileDialog::Instance().DeleteTexture = nullptr;
    ifd::FileDialog::Instance().CreateTexture = nullptr;
    ifd::FileDialog::Instance().Close();
}

void Logo::check_file_result() {
    // file dialogs
    if (ifd::FileDialog::Instance().IsDone(FILE_DIALOG_LOGO_ID)) {
        if (ifd::FileDialog::Instance().HasResult()) {
            const std::vector<std::filesystem::path>& res = ifd::FileDialog::Instance().GetResults();
            for (const auto& r : res) {
                // ����� ���� ������ ���� � ����� ����� ���������� ��������� ����������
                // (��. https://stackoverflow.com/questions/73284766/stdsystemerror-exception-during-recursive-directory-iterator-unicode-trans)
                const std::wstring currentPath = r.c_str();
                std::string path = wide_string_to_string(currentPath);
                //���� ���������� ���������� ��������� �� ���� �� ���������� (���� �� ����������)
                if (!file_exists(path)) {
                    ImGui::OpenPopup("Alert##logo_incorrect_file_name");
                    if (ImGui::BeginPopupModal("Alert##logo_incorrect_file_name", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text(ERROR_INCORRECT_FILE_NAME_WILDCHAR);
                        if (ImGui::Button("Ok##close_logo_incorrect_file_name")) {
                            ImGui::CloseCurrentPopup();
                            ifd::FileDialog::Instance().Close();
                        }
                        ImGui::EndPopup();
                    }
                    return;
                }
                delete texture;
                texture = new Texture(path, device, context);
            }
        }
        ifd::FileDialog::Instance().Close();
    }
}

void Logo::render(int posx, int posy, int width, int height)
{
    ImGui::Image((ImTextureID)texture->get_textureID(), /*ImGui::GetContentRegionAvail()*/LOGO_DIMENSION, ImVec2(0, 0), ImVec2(1, 1), bufer->background_color, Hex2ImVec4(LOGO_BORDER_COLOR));
    ImGui::ColorEdit4("##background_color", (float*)&bufer->background_color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
    ImGui::LabelText("##logo_name", bufer->logo_name);
    if (ImGui::Button("Select##select_logo")) {
        ifd::FileDialog::Instance().Open(FILE_DIALOG_LOGO_ID, FILE_DIALOG_LOGO_TITLE, "Image file (*.png,*jpg,*jpeg){.png,.jpg,.jpeg},.*", false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save##save_logo")) {
        save();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Close##close_logo")) {
        ImGui::CloseCurrentPopup();
    }
    check_file_result();
}

void Logo::save()
{
    update_data();
    if (changed) {
        database->insert_logo(data);
    }
}
