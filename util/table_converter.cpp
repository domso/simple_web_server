#include "table_converter.h"

namespace web_server {
    void table_converter::add_cell(const std::string& content) {
        std::vector<std::string> cell;
        std::string line;

        for (const auto c : content) {
            if (c == '\n') {
                cell.push_back(line);
                line = "";
            } else {
                line += c;
            }
        }
        cell.push_back(line);

        if (m_rows.empty()) {
            m_rows.resize(1);
        }
        
        m_rows.rbegin()->push_back(cell);
    }

    void table_converter::close_row() {
        balance_rows();
        m_rows.resize(m_rows.size() + 1);
    }

    std::string table_converter::table() const {
        std::string result;
        std::string line_sep = " ";

        if (m_rows.empty()) {
            return "";
        }

        for (const auto& cell : *m_rows.begin()) {
            if (!cell.empty()) {
                line_sep += "+-" + std::string(cell[0].length() + 1, '-');
            }
        }           
        line_sep += "+\n";
        
        result += line_sep;
        for (const auto& row : m_rows) {
            if (row.empty()) {
                continue;
            }
            auto lines = row.begin()->size();

            for (auto i = 0UL; i < lines; i++) {
                for (const auto& cell : row) {
                    if (i < cell.size()) {
                        result += " | " + cell[i];
                    }
                }           
                result += " |\n";
            }
            result += line_sep;
        }

        return result;
    }

    void table_converter::balance_rows() {
        size_t num_column_cells = 0UL;

        for (const auto& row : m_rows) {
            num_column_cells = std::max(row.size(), num_column_cells);
        }

        for (auto& row : m_rows) {
            row.resize(num_column_cells);
        }

        balance_cell_height();
    }

    void table_converter::balance_cell_height() {
        for (auto& row : m_rows) {
            size_t height = 0;

            for (const auto& cell : row) {
                height = std::max(cell.size(), height);
            }           

            for (auto& cell : row) {
                cell.resize(height);
            }           
        }

        balance_cell_width();
    }

    void table_converter::balance_cell_width() {
        std::vector<size_t> widths;

        for (const auto& row : m_rows) {
            size_t n = 0;
            for (const auto& cell : row) {
                widths.resize(std::max(widths.size(), n + 1));
                
                for (const auto& line : cell) {
                    widths[n] = std::max(widths[n], line.length());
                }
                n++;
            }           
        }

        for (auto& row : m_rows) {
            size_t n = 0;
            for (auto& cell : row) {
                widths.resize(std::max(widths.size(), n + 1));
                
                for (auto& line : cell) {
                    line = line + std::string(widths[n] - line.length(), ' ');
                }
                n++;
            }           
        }
    }
}
