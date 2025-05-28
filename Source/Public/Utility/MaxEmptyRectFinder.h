#pragma once

#include "Widget/Widget.h"
#include <vector>
#include <stack>
#include <cmath> // for std::floor, std::ceil
#include <glm/glm.hpp>

namespace aby::util {

    enum class EGridType {
        FLAT_VECTOR,
        VECTOR_2D,
        RAW_FLAT_ARRAY,
        RAW_2D_ARRAY
    };

    template <EGridType GridType>
    class MaxEmptyRectFinder {
    public:
        glm::vec4 find(const glm::vec2& size, const std::vector<Ref<ui::Widget>>& widgets) {
            const int width = static_cast<int>(size.x);
            const int height = static_cast<int>(size.y);

            allocate_grid(size);
            fill_grid(size, widgets);

            std::vector<int> height_hist(width, 0);
            glm::vec4 best = {0, 0, 0, 0};
            int max_area = 0;

            for (int y = 0; y < height; ++y) {
                // Update height histogram
                for (int x = 0; x < width; ++x)
                    height_hist[x] = (grid_at(x, y, height) == 0) ? height_hist[x] + 1 : 0;

                std::stack<int> s;

                int x = 0;
                while (x <= width) {
                    int h = (x == width) ? 0 : height_hist[x];
                    if (s.empty() || h >= height_hist[s.top()]) {
                        s.push(x++);
                    } else {
                        int top = s.top(); s.pop();
                        int w = s.empty() ? x : x - s.top() - 1;
                        int area = height_hist[top] * w;
                        if (area > max_area) {
                            max_area = area;
                            best = {
                                static_cast<float>(s.empty() ? 0 : s.top() + 1),
                                static_cast<float>(y - height_hist[top] + 1),
                                static_cast<float>(w),
                                static_cast<float>(height_hist[top])
                            };
                        }
                    }
                }
            }

            deallocate_grid(height);
            return best;
        }
    private:
        void allocate_grid(const glm::vec2& size) {
            if constexpr (GridType == EGridType::FLAT_VECTOR) {
                m_Grid.assign(size.x * size.y, 0);
            } else if constexpr (GridType == EGridType::VECTOR_2D) {
                m_Grid.assign(static_cast<int>(size.y), std::vector<int>(static_cast<int>(size.x), 0));
            } else if constexpr (GridType == EGridType::RAW_FLAT_ARRAY) {
                m_Grid = new int[size.x * size.y]();
            } else if constexpr (GridType == EGridType::RAW_2D_ARRAY) {
                m_Grid = new int*[size.y];
                for (int i = 0; i < size.y; ++i)
                    m_Grid[i] = new int[size.x]();
            }
        }

        void deallocate_grid(int height) {
            if constexpr (GridType == EGridType::RAW_FLAT_ARRAY) {
                delete[] m_Grid;
                m_Grid = nullptr;
            } else if constexpr (GridType == EGridType::RAW_2D_ARRAY) {
                for (int i = 0; i < height; ++i)
                    delete[] m_Grid[i];
                delete[] m_Grid;
                m_Grid = nullptr;
            }
        }

        void fill_grid(const glm::vec2& size, const std::vector<Ref<ui::Widget>>& widgets) {
            if constexpr (GridType == EGridType::FLAT_VECTOR) {
                std::fill(m_Grid.begin(), m_Grid.end(), 0);
            } else if constexpr (GridType == EGridType::VECTOR_2D) {
                for (auto& row : m_Grid) std::fill(row.begin(), row.end(), 0);
            } else if constexpr (GridType == EGridType::RAW_FLAT_ARRAY) {
                std::fill(m_Grid, m_Grid + size.x * size.y, 0);
            } else if constexpr (GridType == EGridType::RAW_2D_ARRAY) {
                for (int y = 0; y < size.y; ++y)
                    std::fill(m_Grid[y], m_Grid[y] + size.x, 0);
            }

            for (const auto& widget : widgets) {
                const auto& tf = widget->transform();
                int x0 = std::max(0, static_cast<int>(std::floor(tf.position.x)));
                int y0 = std::max(0, static_cast<int>(std::floor(tf.position.y)));
                int x1 = std::min(static_cast<int>(size.x), static_cast<int>(std::ceil(tf.position.x + tf.size.x)));
                int y1 = std::min(static_cast<int>(size.y), static_cast<int>(std::ceil(tf.position.y + tf.size.y)));

                for (int y = y0; y < y1; ++y)
                    for (int x = x0; x < x1; ++x) {
                        if constexpr (GridType == EGridType::FLAT_VECTOR)
                            m_Grid[y * size.x + x] = 1;
                        else if constexpr (GridType == EGridType::VECTOR_2D)
                            m_Grid[y][x] = 1;
                        else if constexpr (GridType == EGridType::RAW_FLAT_ARRAY)
                            m_Grid[y * size.x + x] = 1;
                        else if constexpr (GridType == EGridType::RAW_2D_ARRAY)
                            m_Grid[y][x] = 1;
                    }
            }
        }

        int grid_at(int x, int y, int width) const {
            if constexpr (GridType == EGridType::FLAT_VECTOR) {
                return m_Grid[y * width + x];
            } else if constexpr (GridType == EGridType::VECTOR_2D) {
                return m_Grid[y][x];
            } else if constexpr (GridType == EGridType::RAW_FLAT_ARRAY) {
                return m_Grid[y * width + x];
            } else if constexpr (GridType == EGridType::RAW_2D_ARRAY) {
                return m_Grid[y][x];
            } else {
                static_assert("WHAT THE FUCK!?!?!?!");
            }
        }
    private:
        using GridType_t = std::conditional_t<GridType == EGridType::FLAT_VECTOR, std::vector<int>,
                 std::conditional_t<GridType == EGridType::VECTOR_2D, std::vector<std::vector<int>>,
                 std::conditional_t<GridType == EGridType::RAW_FLAT_ARRAY, int*,
                 int**>>>;
        GridType_t m_Grid;
    };


}