#pragma once
#include <vector>
#include <string>
#include <imgui.h>
#include "CSVHelper.h"
#include "DBHelper.h"
#include "common.h"
#include "Texture.h"


#define BUFER_SIZE 200

struct Bufer_user {
	Bufer_user(std::shared_ptr<DB_user> data) {
		strcpy_s(user_name, data->user_name.data());
		strcpy_s(title, data->title.data());
		strcpy_s(title2, data->title2.data());
		strcpy_s(qr_code, data->qr_code.data());
	};
	char user_name[BUFER_SIZE];
	char title[BUFER_SIZE];
	char title2[BUFER_SIZE];
	char qr_code[BUFER_SIZE];
};

struct Buffer_logo {
	Buffer_logo(std::shared_ptr<DB_logo> data) {
		background_color = Hex2ImVec4(data->background_color);
		strcpy_s(logo_name, data->logo_name.data());
	};
	char logo_name[BUFER_SIZE];
	ImVec4 background_color;
};

class MainWindow;
class Tab;

interface IRendered
{
public:
	virtual ~IRendered() {}
	virtual void render(int posx, int posy, int width, int height) = 0;

};

class Logo : public IRendered {
public:
	Logo() = delete;
	Logo(std::shared_ptr<DBHelper> db);	
	~Logo();	
	virtual void render(int posx, int posy, int width, int height);

private:
	void check_file_result();
	void save();	
	void update_data();

private:
	std::shared_ptr<DB_logo> data;
	std::shared_ptr<DBHelper> database;
	Buffer_logo* bufer;
	bool changed;
	Texture* texture;
};

class Row: public IRendered {
public:
	Row() = delete;
	Row(std::shared_ptr<DB_user> data, Tab* parent);
	~Row();
	virtual void render(int posx, int posy, int width, int height);
	void update_data();
	bool is_changed();
	std::shared_ptr<DB_user> get_data() { update_data();  return data; }

private:
	std::shared_ptr<DB_user> data;	
	Bufer_user* bufer;
	bool changed;
	int row_id;
	static int counter;
	Tab* tab;
};

class Tab: public IRendered {
public:
	Tab() = delete;
	Tab(std::shared_ptr<DBHelper> helper, MainWindow* window);
	Tab(std::string file_path, std::shared_ptr<DBHelper> helper, MainWindow* window);
	~Tab();
	static int change_filter_user_name_callback(ImGuiInputTextCallbackData* data);
	virtual void render(int posx, int posy, int width, int height);
	bool is_changed();
	std::vector<Row*> get_changed();
	//saves all CSV to database	
	virtual void delete_row(Row* row);
	virtual void init(bool select = false);
	std::string get_name() { return name; }	
	void update_search_name_and_filter(size_t new_size, char* buf);

protected:
	void filter();
	virtual void save();

private:
	void add_user();

protected:
	std::string name;
	std::string tab_id;
	std::string search_name;
	std::vector<Row*> rows_filtered;
	std::vector<Row*> rows;
	static size_t count;
	std::shared_ptr<DBHelper> db_helper;
	MainWindow* window;
	bool selected;

private:	
	CSVHelper* csv_helper;	
	bool opened;
};

// TODO: ������� DBTab ������� �������
class DBTab : public Tab {
public:
	DBTab(std::shared_ptr<DBHelper> helper, MainWindow* window);
	~DBTab();	
	//saves only changed data	
	void delete_row(Row* row) override;
	void init(bool select = false) override;

protected:
	void save() override;
};

class MainWindow: public IRendered {
public:
	MainWindow();
	~MainWindow();
	virtual void render(int posx, int posy, int width, int height) override;
	void set_texture(ID3D11ShaderResourceView* texture, int image_width, int image_height);
	void set_styles();
	void delete_tab(Tab* tab, bool refresh_db_tab);		

private:
	void check_file_result();
	void init_DB_tab();	
	Tab* get_DB_tab();

private:
	std::vector<Tab*> tabs;
	std::shared_ptr<DBHelper> db_helper;	
	Logo* logo;
	bool logo_popup_opened;
	//to show database tab in startup
	bool db_tab_initialized;

private:
	ID3D11ShaderResourceView* myTexture = nullptr;
	int image_width;
	int image_height;
};