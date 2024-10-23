#pragma once

#include <string>
#include <vector>
#include <sqlite3.h>
#include <functional>
#include <memory>

#define TABLE_NAME_USER "ATTENDEE"
#define TABLE_NAME_LABEL "LABEL"
#define DEFAULT_BACKGROUND_COLOR "FFFFFF"

typedef int (*db_exec_callback)(void*, int, char**, char**);
typedef void (*fetch_data_completed_callback)();

/*
Data will be saved in format:
user_name|title|title2
*/
struct DB_user {
	DB_user(int id, std::string user_name, std::string title, std::string title2, std::string qr_code) :
		id(id), user_name(user_name), title(title), title2(title2), qr_code(qr_code){}
	DB_user(std::string user_name, std::string title, std::string title2, std::string qr_code) : id(-1), user_name(user_name), title(title), title2(title2), qr_code(qr_code) {}
	int id;
	std::string user_name;
	std::string title;
	std::string title2;
	std::string qr_code;
	std::string operator[](int i) { switch (i) { case 0: return user_name; case 1: return title; case 2: return title2; case 3: return qr_code; } }
};

struct DB_logo {
	DB_logo(int id, std::string background_color, std::string logo_name, std::vector<unsigned char> logo, int width, int height, int numcomponents):
		id(id), background_color(background_color), logo_name(logo_name), logo(logo), width(width), height(height), numcomponents(numcomponents){}
	DB_logo(std::string background_color, std::string logo_name, std::vector<unsigned char> logo, int width, int height, int numcomponents) :
		id(-1), background_color(background_color), logo_name(logo_name), logo(logo), width(width), height(height), numcomponents(numcomponents){}
	DB_logo() : id(-1), background_color(DEFAULT_BACKGROUND_COLOR), logo_name(""), logo({}), width(0), height(0), numcomponents(0){}	
	~DB_logo() {}
	int id;
	std::string background_color;
	std::string logo_name;
	std::vector<unsigned char> logo;
	// numcomponents= components per pixel
	// 1 grey
	// 2 grey, alpha
	// 3 red, green, blue
	// 4 red, green, blue, alpha
	int width, height, numcomponents;
};

class DBHelper {
public:
	DBHelper();
	~DBHelper();
	// sql commands user
	void insert_scope_user(std::vector<std::shared_ptr<DB_user>> data);
	void delete_user(std::shared_ptr<DB_user> data);
	void update_scope_user(std::vector<std::shared_ptr<DB_user>> cont);	
		
	void refresh_user();
	std::vector<std::shared_ptr<DB_user>> get_data_user();

	// sql commands logo
	void insert_logo(std::shared_ptr<DB_logo> data);
	void delete_logo();
	std::shared_ptr<DB_logo> get_logo();

	// called in callbacks
	int get_rows_count_user();
	void set_rows_count_user(int count);	
	void push_back_user(std::shared_ptr<DB_user> d);			
	// callbacks
	void add_callback(std::function<void()> clbck);
	static int get_data_user_callback(void* data, int argc, char** argv, char** azColName);
	static int calculate_rows_user_callback(void* data, int argc, char** argv, char** azColName);

private:
	sqlite3* database;
	char* zErrMsg;
	int rc;
	int rows_count;
	std::vector<std::shared_ptr<DB_user>> database_data;
	std::vector<std::function<void()>> callbacks;
	
private:
	int run_sql_no_callback(std::string sql);
	void calculate_rows_count_user();
	void check_DB_errors();
	void execute(std::string command, db_exec_callback clbck = nullptr, void* ref = nullptr);
	void get(const char* table_name, int id, db_exec_callback clbck = nullptr, void* ref = nullptr);
	void get_all(const char* table_name, db_exec_callback clbck = DBHelper::get_data_user_callback, void* ref = nullptr);
	void del(const char* table_name, int id);
	void update_user(std::shared_ptr<DB_user> data);
	void create_table_user();
	void create_table_logo();
	void init();
	void insert_user(std::shared_ptr<DB_user> data);		
};