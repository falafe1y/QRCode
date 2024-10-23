#include <stdexcept>
#include "CSVHelper.h"

CSVHelper::CSVHelper()
{
	this->rows_count = 0;
}

CSVHelper::~CSVHelper()
{	
}

void CSVHelper::read_csv_file(std::string csv_file_path)
{
	this->file_data.Clear();
	this->file_data.Load(csv_file_path);
	this->rows_count = this->file_data.GetRowCount();

}