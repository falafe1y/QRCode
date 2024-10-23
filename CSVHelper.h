#pragma once

#include <string>
#include <vector>
#include "rapidcsv.h"

/*
Data provided have next format:
user_name|title
*/

class CSVHelper {
public:
	CSVHelper();
	~CSVHelper();

	template <typename T> std::vector<T> get_cell_data(std::string column_name, int row_index);
	template <typename T> std::vector<std::vector<T>> get_all_data();
	void read_csv_file(std::string csv_file_path);

public:
	int rows_count;

private:
	rapidcsv::Document file_data;	
};

template<typename T>
std::vector<T> CSVHelper::get_cell_data(std::string column_name, int row_index)
{
	if (row_index > this->rows_count || row_index < 0) {
		throw std::invalid_argument("Incorrect row index");
	}
	return this->file_data.GetCell<T>(column_name, row_index);
}

template<typename T>
std::vector<std::vector<T>> CSVHelper::get_all_data()
{
	std::vector<std::vector<T>> result;
	for (size_t i = 0; i < rows_count; i++)
	{
		result.push_back(this->file_data.GetRow<T>(i));
	}
	return result;
}
