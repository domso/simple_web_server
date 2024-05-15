#pragma once

#include <vector>
#include <string>

namespace web_server {
    class table_converter {
    public:
        void add_cell(const std::string& content);
        void close_row();
        std::string table() const;
    private:
        void balance_rows();
        void balance_cell_height();
        void balance_cell_width();

        std::vector<std::vector<std::vector<std::string>>> m_rows;
    };
}
