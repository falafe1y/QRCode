#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "DBHelper.h"
#include <easylogging++.h>

#define SQLITE_FILE_NAME "data.db"

DBHelper::DBHelper()
{
	rows_count = 0;
	rc = sqlite3_open(SQLITE_FILE_NAME, &database);
	check_DB_errors();
	init();
	refresh_user();
}

DBHelper::~DBHelper()
{
	sqlite3_close(database);
	for (size_t i = 0; i <  database_data.size(); i++) {
		database_data[i].reset();
	}
	database_data.clear();
}

void DBHelper::execute(std::string command, db_exec_callback clbck, void* ref) {
	LOG(DEBUG) << "Execite sql command: " << command;
	rc = sqlite3_exec(database, command.c_str(), clbck, ref, &zErrMsg);
	check_DB_errors();
}

void DBHelper::init()
{
	create_table_user();
	create_table_logo();
}

void DBHelper::get(const char* table_name, int id, db_exec_callback clbck, void* ref)
{
	std::string sql = "SELECT * FROM " + std::string(table_name) + " WHERE id=" + std::to_string(id) + ";";
	execute(sql, clbck, ref);
}

void DBHelper::get_all(const char* table_name, db_exec_callback clbck, void* ref)
{
	std::string sql = "SELECT * FROM " + std::string(table_name) + ";";
	execute(sql, clbck, ref);
}

void DBHelper::del(const char* table_name, int id)
{
	std::string sql = "DELETE FROM " + std::string(table_name) + " WHERE id=" + std::to_string(id) + ";";
	execute(sql);
}

void DBHelper::insert_user(std::shared_ptr<DB_user> data)
{
	std::stringstream sql;
	sql << "INSERT INTO " << std::string(TABLE_NAME_USER) << " (user_name, title, title2, qr_code) " << \
		"VALUES (\"" << data->user_name << "\",\"" << data->title << "\",\"" << data->title2 << "\",\"" << data->qr_code << "\");";
	execute(sql.str());
}

void DBHelper::insert_scope_user(std::vector<std::shared_ptr<DB_user>> cont) {
	for (auto data : cont) {
		insert_user(data);
	}
	refresh_user();
}

void DBHelper::delete_user(std::shared_ptr<DB_user> data)
{
	del(TABLE_NAME_USER, data->id);
	refresh_user();
}

void DBHelper::update_user(std::shared_ptr<DB_user> data) {
	if (data->id == -1) {		
		return insert_user(data);
	}
	std::stringstream sql;
	sql << "UPDATE " << std::string(TABLE_NAME_USER) << " SET user_name = \"" << data->user_name << "\", title=\"" << data->title << "\", title2=\"" << data->title2  <<"\", qr_code=\"" << data->qr_code << "\" WHERE id=" << data->id << "; ";
	execute(sql.str());
}

void DBHelper::create_table_user()
{
	std::string sql = "CREATE TABLE IF NOT EXISTS " + std::string(TABLE_NAME_USER) + " ("\
		"id					 INTEGER		PRIMARY KEY	AUTOINCREMENT," \
		"user_name           TEXT			NOT NULL," \
		"title			     TEXT			NOT NULL,"\
		"title2				 TEXT			NOT NULL DEFAULT \"\","\
		"qr_code		     VARCHAR(255)	NOT NULL DEFAULT \"\");";
	execute(sql);
}

void DBHelper::create_table_logo()
{
	std::string sql = "CREATE TABLE IF NOT EXISTS " + std::string(TABLE_NAME_LABEL) + " ("\
		"id					 INTEGER		PRIMARY KEY	AUTOINCREMENT,"\
		"logo_name			 TEXT			DEFAULT NULL," \
		"logo			     BLOB			DEFAULT NULL," \
		"width			     INTEGER		DEFAULT NULL," \
		"height			     INTEGER		DEFAULT NULL," \
		"numcomponents		 INTEGER		DEFAULT NULL," \
		"background_color    TEXT  NOT NULL DEFAULT \"" + std::string(DEFAULT_BACKGROUND_COLOR) + "\");";
	execute(sql);
}

void DBHelper::update_scope_user(std::vector<std::shared_ptr<DB_user>> cont) {
	for (auto data : cont) {
		update_user(data);
	}
	refresh_user();
}

void DBHelper::check_DB_errors()
{
	if (rc) {
		// Show an error message
		LOG(ERROR) << "DB Error: " << sqlite3_errmsg(database);
		sqlite3_close(database);
	}
}

void DBHelper::calculate_rows_count_user() {
	std::string sql = "SELECT COUNT(*) FROM " + std::string(TABLE_NAME_USER) + ";";
	execute(sql, DBHelper::calculate_rows_user_callback, this);
}

int DBHelper::get_rows_count_user() {
	return rows_count;
}

void DBHelper::set_rows_count_user(int count) {
	rows_count = count;
}

void DBHelper::push_back_user(std::shared_ptr<DB_user> d) {
	database_data.push_back(d);	
	if (database_data.size() == get_rows_count_user()) {
		if (callbacks.size() > 0) {
			for (size_t i = 0; i < callbacks.size(); i++) {
				callbacks[i]();
			}
			callbacks.clear();
		}
	}
}

std::vector<std::shared_ptr<DB_user>> DBHelper::get_data_user() {
	return database_data;
}

int DBHelper::run_sql_no_callback(std::string sql)
{
	sqlite3_stmt* stmt = NULL;
	int rc = sqlite3_prepare_v2(database, sql.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		return rc;
	int rowCount = 0;
	rc = sqlite3_step(stmt);
	while (rc != SQLITE_DONE && rc != SQLITE_OK)
	{
		rowCount++;
		int colCount = sqlite3_column_count(stmt);
		for (int colIndex = 0; colIndex < colCount; colIndex++)
		{
			int type = sqlite3_column_type(stmt, colIndex);
			const char* columnName = sqlite3_column_name(stmt, colIndex);
			if (type == SQLITE_INTEGER)
			{
				int valInt = sqlite3_column_int(stmt, colIndex);
				LOG(INFO) << "columnName = " << columnName << ", Integer val = " << valInt;
			}
			else if (type == SQLITE_FLOAT)
			{
				double valDouble = sqlite3_column_double(stmt, colIndex);
				LOG(INFO) << "columnName = " << columnName << ",Double val =" << valDouble;
			}
			else if (type == SQLITE_TEXT)
			{
				const unsigned char* valChar = sqlite3_column_text(stmt, colIndex);
				LOG(INFO) << "columnName = " << columnName << ",Text val = " << valChar;
				free((void*)valChar);
			}
			else if (type == SQLITE_BLOB)
			{
				std::vector<float> logo;
				// retrieve blob data
				const float* pdata = reinterpret_cast<const float*>(sqlite3_column_blob(stmt, colIndex));
				// query blob data size
				int size = sqlite3_column_bytes(stmt, colIndex) / static_cast<int>(sizeof(float));
				logo.resize(size);
				// copy to data vector
				std::copy(pdata, pdata + size, logo.data());
			}
			else if (type == SQLITE_NULL)
			{
				LOG(INFO) << "columnName = " << columnName << ", NULL";
			}
		}
		LOG(INFO) << "Line " << rowCount << ", rowCount = %d" << colCount;
		rc = sqlite3_step(stmt);
	}
	rc = sqlite3_finalize(stmt);
	return rc;
}

void DBHelper::insert_logo(std::shared_ptr<DB_logo> data)
{
	delete_logo();
	std::stringstream sql;
	sql << "INSERT INTO " << std::string(TABLE_NAME_LABEL) << " (logo_name, logo, background_color, width, height, numcomponents) " <<
		"VALUES (\"" << data->logo_name << "\",?,\"" << data->background_color <<  "\",\"" << data->width << "\",\"" << data->height << "\", \"" << data->numcomponents << "\");";

	sqlite3_stmt* stmtInsert = NULL;
	rc = sqlite3_prepare_v2(database, sql.str().c_str(), -1, &stmtInsert, nullptr);
	if (rc == SQLITE_OK) {
		sqlite3_bind_blob(stmtInsert, 1, data->logo.data(), static_cast<int>(data->logo.size() * sizeof(unsigned char)), SQLITE_STATIC);
		if (rc == SQLITE_OK) {
			rc = sqlite3_step(stmtInsert);
			if (rc != SQLITE_DONE) {
				throw std::runtime_error("Failed to insert logo");
				LOG(ERROR) << "DB Error: " << sqlite3_errmsg(database);
			}			
		}
		else {
			LOG(ERROR) << "DB Error: " << sqlite3_errmsg(database);
		}
	}
	else {
		LOG(ERROR) << "DB Error: " << sqlite3_errmsg(database);
	};	
	rc = sqlite3_finalize(stmtInsert);
	check_DB_errors();
}

void DBHelper::delete_logo()
{
	run_sql_no_callback("DELETE FROM " + std::string(TABLE_NAME_LABEL) + ";");
}

std::shared_ptr<DB_logo> DBHelper::get_logo()
{
	std::string sql = "SELECT id, logo_name, logo, background_color, width, height, numcomponents FROM " + std::string(TABLE_NAME_LABEL) + " LIMIT 1;";
	sqlite3_stmt* stmtRetrieve = nullptr;
	sqlite3_prepare_v2(database, sql.c_str(), -1, &stmtRetrieve, nullptr);

	std::vector<unsigned char> logo;
	std::shared_ptr<DB_logo> data;
	if (sqlite3_step(stmtRetrieve) == SQLITE_ROW)
	{		
		int id = sqlite3_column_int(stmtRetrieve, 0);
		const unsigned char* ln = sqlite3_column_text(stmtRetrieve, 1);
		std::string logo_name(reinterpret_cast<const char*>(ln));
		// retrieve blob data
		const unsigned char* pdata = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmtRetrieve, 2));
		// query blob data size
		int size = sqlite3_column_bytes(stmtRetrieve, 2) / static_cast<int>(sizeof(unsigned char));
		logo.resize(size);
		// copy to data vector
		std::copy(pdata, pdata + size, logo.data());
		const unsigned char* bc = sqlite3_column_text(stmtRetrieve, 3);
		std::string background_color(reinterpret_cast<const char*>(bc));
		int width = sqlite3_column_int(stmtRetrieve, 4);
		int height = sqlite3_column_int(stmtRetrieve, 5);		
		int numcomponents = sqlite3_column_int(stmtRetrieve, 6);
		data = std::shared_ptr<DB_logo>(new DB_logo( id, background_color, logo_name, logo,  width,  height, numcomponents));
	}
	else {
		data = std::shared_ptr<DB_logo>(new DB_logo());
	}

	sqlite3_finalize(stmtRetrieve);
	return data;
}

void DBHelper::refresh_user() {	
	database_data.clear();
	calculate_rows_count_user();
	get_all(TABLE_NAME_USER, DBHelper::get_data_user_callback, this);
}

void DBHelper::add_callback(std::function<void()> clbck) {
	callbacks.push_back(clbck);
}

// int argc: holds the number of results
// (array) azColName: holds each column returned
// (array) argv: holds each value
int DBHelper::get_data_user_callback(void* self, int argc, char** argv, char** azColName) {
	if (argc != 5) {
		throw std::invalid_argument("Incorrect number columns in database query.");
	}
	std::shared_ptr<DB_user> data  = std::shared_ptr<DB_user>(new DB_user(atoi(argv[0]), std::string(argv[1]), std::string(argv[2]), std::string(argv[3]), std::string(argv[4])));
	reinterpret_cast<DBHelper*>(self)->push_back_user(data);	
	return 0;
}

int DBHelper::calculate_rows_user_callback(void* self, int argc, char** argv, char** azColName)
{
	reinterpret_cast<DBHelper*>(self)->set_rows_count_user(atoi(argv[0]));
	return 0;
}