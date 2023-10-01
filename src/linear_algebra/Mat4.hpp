#pragma once
#include "linear_algebra/Quaternion.hpp"
#include "linear_algebra/Vec4.hpp"
#include <array>
#include <optional>

template <typename T>
struct Mat4 {
    std::array<T, 16> data;
    T get(int y, int x) const {
        return this->data[y * 4 + x];
    }

    Mat4 mat_mul(const Mat4& rhs) {
        Mat4<T>({
            this->get(0, 0) * rhs.get(0, 0) + this->get(0, 1) * rhs.get(1, 0) + this->get(0, 2) * rhs.get(2, 0) + this->get(0, 3) * rhs.get(3, 0),
            this->get(0, 0) * rhs.get(0, 1) + this->get(0, 1) * rhs.get(1, 1) + this->get(0, 2) * rhs.get(2, 1) + this->get(0, 3) * rhs.get(3, 1),
            this->get(0, 0) * rhs.get(0, 2) + this->get(0, 1) * rhs.get(1, 2) + this->get(0, 2) * rhs.get(2, 2) + this->get(0, 3) * rhs.get(3, 2),
            this->get(0, 0) * rhs.get(0, 3) + this->get(0, 1) * rhs.get(1, 3) + this->get(0, 2) * rhs.get(2, 3) + this->get(0, 3) * rhs.get(3, 3),

            this->get(1, 0) * rhs.get(0, 0) + this->get(1, 1) * rhs.get(1, 0) + this->get(1, 2) * rhs.get(2, 0) + this->get(1, 3) * rhs.get(3, 0),
            this->get(1, 0) * rhs.get(0, 1) + this->get(1, 1) * rhs.get(1, 1) + this->get(1, 2) * rhs.get(2, 1) + this->get(1, 3) * rhs.get(3, 1),
            this->get(1, 0) * rhs.get(0, 2) + this->get(1, 1) * rhs.get(1, 2) + this->get(1, 2) * rhs.get(2, 2) + this->get(1, 3) * rhs.get(3, 2),
            this->get(1, 0) * rhs.get(0, 3) + this->get(1, 1) * rhs.get(1, 3) + this->get(1, 2) * rhs.get(2, 3) + this->get(1, 3) * rhs.get(3, 3),
            
            this->get(2, 0) * rhs.get(0, 0) + this->get(2, 1) * rhs.get(1, 0) + this->get(2, 2) * rhs.get(2, 0) + this->get(2, 3) * rhs.get(3, 0),
            this->get(2, 0) * rhs.get(0, 1) + this->get(2, 1) * rhs.get(1, 1) + this->get(2, 2) * rhs.get(2, 1) + this->get(2, 3) * rhs.get(3, 1),
            this->get(2, 0) * rhs.get(0, 2) + this->get(2, 1) * rhs.get(1, 2) + this->get(2, 2) * rhs.get(2, 2) + this->get(2, 3) * rhs.get(3, 2),
            this->get(2, 0) * rhs.get(0, 3) + this->get(2, 1) * rhs.get(1, 3) + this->get(2, 2) * rhs.get(2, 3) + this->get(2, 3) * rhs.get(3, 3),

            this->get(3, 0) * rhs.get(0, 0) + this->get(3, 1) * rhs.get(1, 0) + this->get(3, 2) * rhs.get(2, 0) + this->get(3, 3) * rhs.get(3, 0),
            this->get(3, 0) * rhs.get(0, 1) + this->get(3, 1) * rhs.get(1, 1) + this->get(3, 2) * rhs.get(2, 1) + this->get(3, 3) * rhs.get(3, 1),
            this->get(3, 0) * rhs.get(0, 2) + this->get(3, 1) * rhs.get(1, 2) + this->get(3, 2) * rhs.get(2, 2) + this->get(3, 3) * rhs.get(3, 2),
            this->get(3, 0) * rhs.get(0, 3) + this->get(3, 1) * rhs.get(1, 3) + this->get(3, 2) * rhs.get(2, 3) + this->get(3, 3) * rhs.get(3, 3)
        });
    }

    Vec4<T> vec_mul(const Vec4<T> rhs) {
        return Vec4<T>(
            this->get(0, 0) * rhs.w + this->get(0, 1) * rhs.x + this->get(0, 2) * rhs.y + this->get(0, 3) * rhs.z,
            this->get(1, 0) * rhs.w + this->get(1, 1) * rhs.x + this->get(1, 2) * rhs.y + this->get(1, 3) * rhs.z,
            this->get(2, 0) * rhs.w + this->get(2, 1) * rhs.x + this->get(2, 2) * rhs.y + this->get(2, 3) * rhs.z,
            this->get(3, 0) * rhs.w + this->get(3, 1) * rhs.x + this->get(3, 2) * rhs.y + this->get(3, 3) * rhs.z
        );
    }

    std::optional<Mat4<T>> inverse() {
        // from glu implementation
        std::array<T,16> inv = {0};
    
        inv[0] =    this->data[5]  * this->data[10] * this->data[15] - 
                    this->data[5]  * this->data[11] * this->data[14] - 
                    this->data[9]  * this->data[6]  * this->data[15] + 
                    this->data[9]  * this->data[7]  * this->data[14] +
                    this->data[13] * this->data[6]  * this->data[11] - 
                    this->data[13] * this->data[7]  * this->data[10];
    
        inv[4] =   -this->data[4]  * this->data[10] * this->data[15] + 
                    this->data[4]  * this->data[11] * this->data[14] + 
                    this->data[8]  * this->data[6]  * this->data[15] - 
                    this->data[8]  * this->data[7]  * this->data[14] - 
                    this->data[12] * this->data[6]  * this->data[11] + 
                    this->data[12] * this->data[7]  * this->data[10];
    
        inv[8] =    this->data[4]  * this->data[9]  * this->data[15] - 
                    this->data[4]  * this->data[11] * this->data[13] - 
                    this->data[8]  * this->data[5]  * this->data[15] + 
                    this->data[8]  * this->data[7]  * this->data[13] + 
                    this->data[12] * this->data[5]  * this->data[11] - 
                    this->data[12] * this->data[7]  * this->data[9];
    
        inv[12] =  -this->data[4]  * this->data[9]  * this->data[14] + 
                    this->data[4]  * this->data[10] * this->data[13] +
                    this->data[8]  * this->data[5]  * this->data[14] - 
                    this->data[8]  * this->data[6]  * this->data[13] - 
                    this->data[12] * this->data[5]  * this->data[10] + 
                    this->data[12] * this->data[6]  * this->data[9];
    
        inv[1] =   -this->data[1]  * this->data[10] * this->data[15] + 
                    this->data[1]  * this->data[11] * this->data[14] + 
                    this->data[9]  * this->data[2]  * this->data[15] - 
                    this->data[9]  * this->data[3]  * this->data[14] - 
                    this->data[13] * this->data[2]  * this->data[11] + 
                    this->data[13] * this->data[3]  * this->data[10];
    
        inv[5] =    this->data[0]  * this->data[10] * this->data[15] - 
                    this->data[0]  * this->data[11] * this->data[14] - 
                    this->data[8]  * this->data[2]  * this->data[15] + 
                    this->data[8]  * this->data[3]  * this->data[14] + 
                    this->data[12] * this->data[2]  * this->data[11] - 
                    this->data[12] * this->data[3]  * this->data[10];
    
        inv[9] =   -this->data[0]  * this->data[9]  * this->data[15] + 
                    this->data[0]  * this->data[11] * this->data[13] + 
                    this->data[8]  * this->data[1]  * this->data[15] - 
                    this->data[8]  * this->data[3]  * this->data[13] - 
                    this->data[12] * this->data[1]  * this->data[11] + 
                    this->data[12] * this->data[3]  * this->data[9];
    
        inv[13] =   this->data[0]  * this->data[9]  * this->data[14] - 
                    this->data[0]  * this->data[10] * this->data[13] - 
                    this->data[8]  * this->data[1]  * this->data[14] + 
                    this->data[8]  * this->data[2]  * this->data[13] + 
                    this->data[12] * this->data[1]  * this->data[10] - 
                    this->data[12] * this->data[2]  * this->data[9];
    
        inv[2] =    this->data[1]  * this->data[6]  * this->data[15] - 
                    this->data[1]  * this->data[7]  * this->data[14] - 
                    this->data[5]  * this->data[2]  * this->data[15] + 
                    this->data[5]  * this->data[3]  * this->data[14] + 
                    this->data[13] * this->data[2]  * this->data[7] - 
                    this->data[13] * this->data[3]  * this->data[6];
    
        inv[6] =   -this->data[0]  * this->data[6]  * this->data[15] + 
                    this->data[0]  * this->data[7]  * this->data[14] + 
                    this->data[4]  * this->data[2]  * this->data[15] - 
                    this->data[4]  * this->data[3]  * this->data[14] - 
                    this->data[12] * this->data[2]  * this->data[7] + 
                    this->data[12] * this->data[3]  * this->data[6];
    
        inv[10] =   this->data[0]  * this->data[5]  * this->data[15] - 
                    this->data[0]  * this->data[7]  * this->data[13] - 
                    this->data[4]  * this->data[1]  * this->data[15] + 
                    this->data[4]  * this->data[3]  * this->data[13] + 
                    this->data[12] * this->data[1]  * this->data[7] - 
                    this->data[12] * this->data[3]  * this->data[5];
    
        inv[14] =  -this->data[0]  * this->data[5]  * this->data[14] + 
                    this->data[0]  * this->data[6]  * this->data[13] + 
                    this->data[4]  * this->data[1]  * this->data[14] - 
                    this->data[4]  * this->data[2]  * this->data[13] - 
                    this->data[12] * this->data[1]  * this->data[6] + 
                    this->data[12] * this->data[2]  * this->data[5];
    
        inv[3] =   -this->data[1]  * this->data[6]  * this->data[11] + 
                    this->data[1]  * this->data[7]  * this->data[10] + 
                    this->data[5]  * this->data[2]  * this->data[11] - 
                    this->data[5]  * this->data[3]  * this->data[10] - 
                    this->data[9]  * this->data[2]  * this->data[7] + 
                    this->data[9]  * this->data[3]  * this->data[6];
    
        inv[7] =    this->data[0]  * this->data[6]  * this->data[11] - 
                    this->data[0]  * this->data[7]  * this->data[10] - 
                    this->data[4]  * this->data[2]  * this->data[11] + 
                    this->data[4]  * this->data[3]  * this->data[10] + 
                    this->data[8]  * this->data[2]  * this->data[7] - 
                    this->data[8]  * this->data[3]  * this->data[6];
    
        inv[11] =  -this->data[0]  * this->data[5]  * this->data[11] + 
                    this->data[0]  * this->data[7]  * this->data[9] + 
                    this->data[4]  * this->data[1]  * this->data[11] - 
                    this->data[4]  * this->data[3]  * this->data[9] - 
                    this->data[8]  * this->data[1]  * this->data[7] + 
                    this->data[8]  * this->data[3]  * this->data[5];
    
        inv[15] =   this->data[0]  * this->data[5]  * this->data[10] - 
                    this->data[0]  * this->data[6]  * this->data[9] - 
                    this->data[4]  * this->data[1]  * this->data[10] + 
                    this->data[4]  * this->data[2]  * this->data[9] + 
                    this->data[8]  * this->data[1]  * this->data[6] - 
                    this->data[8]  * this->data[2]  * this->data[5];
    
        auto det = this->data[0] * inv[0] + this->data[1] * inv[4] + this->data[2] * inv[8] + this->data[3] * inv[12];
    
        if (det == static_cast<T>(0)) {
            return {};
        } else {
            auto one_over_det = static_cast<T>(1) / det;
        
            for (int i = 0; i < 16; i++) {
                inv[i] = inv[i] * one_over_det;                
            }
        
            return Mat4(inv);
        }
    
    }


    T operator[](u32 index) {
        return this->data[index];
    }
};