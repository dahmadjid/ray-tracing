#pragma once
#include "linear_algebra/Vec3.hpp"
#include <array>
#include <optional>

template <typename T>
struct Mat3 {
    std::array<T, 9> data;


    T get(int y, int x) const {
        return this->data[y * 3 + x];
    }

    Mat3 mat_mul(const Mat3<T>& rhs) const {
        return Mat3({
            this->get(0, 0) * rhs.get(0, 0) + this->get(1, 0) * rhs.get(0, 1) + this->get(2, 0) * rhs.get(0, 2),
            this->get(0, 1) * rhs.get(0, 0) + this->get(1, 1) * rhs.get(0, 1) + this->get(2, 1) * rhs.get(0, 2),
            this->get(0, 2) * rhs.get(0, 0) + this->get(1, 2) * rhs.get(0, 1) + this->get(2, 2) * rhs.get(0, 2),
            this->get(0, 0) * rhs.get(1, 0) + this->get(1, 0) * rhs.get(1, 1) + this->get(2, 0) * rhs.get(1, 2),
            this->get(0, 1) * rhs.get(1, 0) + this->get(1, 1) * rhs.get(1, 1) + this->get(2, 1) * rhs.get(1, 2),
            this->get(0, 2) * rhs.get(1, 0) + this->get(1, 2) * rhs.get(1, 1) + this->get(2, 2) * rhs.get(1, 2),
            this->get(0, 0) * rhs.get(2, 0) + this->get(1, 0) * rhs.get(2, 1) + this->get(2, 0) * rhs.get(2, 2),
            this->get(0, 1) * rhs.get(2, 0) + this->get(1, 1) * rhs.get(2, 1) + this->get(2, 1) * rhs.get(2, 2),
            this->get(0, 2) * rhs.get(2, 0) + this->get(1, 2) * rhs.get(2, 1) + this->get(2, 2) * rhs.get(2, 2)
        });
    }

    Vec3<T> vec_mul(const Vec3<T>& rhs) const {
        return Vec3(
            this->get(0, 0) * rhs.x + this->get(1, 0) * rhs.y + this->get(2, 0) * rhs.z,
            this->get(0, 1) * rhs.x + this->get(1, 1) * rhs.y + this->get(2, 1) * rhs.z,
            this->get(0, 2) * rhs.x + this->get(1, 2) * rhs.y + this->get(2, 2) * rhs.z
        );
    } 

    std::optional<Mat3<T>> inverse() const {
        auto det = this->get(0, 0) * (this->get(1, 1) * this->get(2, 2) - this->get(2, 1) * this->get(1, 2)) -
        this->get(0, 1) * (this->get(1, 0) * this->get(2, 2) - this->get(1, 2) * this->get(2, 0)) +
        this->get(0, 2) * (this->get(1, 0) * this->get(2, 1) - this->get(1, 1) * this->get(2, 0));

        if (det == static_cast<T>(0)) {
            return {};
        } else {
            auto invdet = static_cast<T>(1) / det;
    
            return Mat3({
                (this->get(1, 1) * this->get(2, 2) - this->get(2, 1) * this->get(1, 2)) * invdet,
                (this->get(0, 2) * this->get(2, 1) - this->get(0, 1) * this->get(2, 2)) * invdet,
                (this->get(0, 1) * this->get(1, 2) - this->get(0, 2) * this->get(1, 1)) * invdet,
                (this->get(1, 2) * this->get(2, 0) - this->get(1, 0) * this->get(2, 2)) * invdet,
                (this->get(0, 0) * this->get(2, 2) - this->get(0, 2) * this->get(2, 0)) * invdet,
                (this->get(1, 0) * this->get(0, 2) - this->get(0, 0) * this->get(1, 2)) * invdet,
                (this->get(1, 0) * this->get(2, 1) - this->get(2, 0) * this->get(1, 1)) * invdet,
                (this->get(2, 0) * this->get(0, 1) - this->get(0, 0) * this->get(2, 1)) * invdet,
                (this->get(0, 0) * this->get(1, 1) - this->get(1, 0) * this->get(0, 1)) * invdet,
            });
        }
    }

    T operator[](u32 index) {
        return this->data[index];
    }
};

