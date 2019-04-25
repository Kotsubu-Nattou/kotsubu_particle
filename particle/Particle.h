#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <Siv3D.hpp>

namespace
{
    const double Pi = 3.141592653589793;
    const double TwoPi = Pi * 2.0;            // Radianの最大値
    const double PiDivStraight = Pi / 180.0;  // Degに掛けるとRad
    const double StraightDivPi = 180.0 / Pi;  // Radに掛けるとDeg
}





class Particle
{
public:
    // 【enum】ドロータイプ
    enum DrawType
    {
        Circle,
        CircleShadow,
    };


private:
    // 【フィールド】
    struct Obj
    {
        Obj() :
            pos(Vec2(0, 0)), size(1.0), radian(0.0), speed(5.0),
            color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
        {}
        Obj(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
            pos(_pos), size(_size), radian(_radian), speed(_speed), color(_color),
            gravity(0.0), enable(true)
        {}
        Vec2   pos;
        double size;
        double radian;
        double speed;
        ColorF color;
        double gravity;
        bool   enable;
    };
    
    struct Status : public Obj
    {
        Status() :
            randPow(5.0), accelSpeed(-0.1), accelSize(-0.1),
            accelColor(0.0, -0.02, -0.03, -0.005),
            gravityPow(0.2), gravityRad(Pi / 2.0),
            radianRange(TwoPi), blendState(s3d::BlendState::Additive),
            drawType(DrawType::Circle)
        {}
        double     randPow;
        double     accelSpeed;
        double     accelSize;
        ColorF     accelColor;
        double     gravityPow;
        double     gravityRad;
        double     radianRange;
        BlendState blendState;
        DrawType   drawType;
    };

    Status status;
    std::vector<Obj> obj;
    


public:
    // 【コンストラクタ】
    Particle(size_t reserve = 3000)
    {
        obj.reserve(reserve);
    }
    

    // 【セッタ】各初期パラメータ。メソッドチェーン方式
    Particle& pos(Vec2 pos)
    {
        status.pos = pos;
        return *this;
    }

    Particle& size(double size)
    {
        status.size = size;
        return *this;
    }

    Particle& angle(double angle)
    {
        if (angle < 0.0) {
            angle = fmod(angle, 360.0) + 360.0;
            if (angle == 360.0) angle = 0.0;
        }
        else if (angle >= 360.0)
            angle = fmod(angle, 360.0);
        
        status.radian = angle * PiDivStraight;
        return *this;
    }

    Particle& angleRange(double angleRange)
    {
        if (angleRange < 0.0) angleRange = 0.0;
        if (angleRange > 360.0) angleRange = 360.0;

        status.radianRange = angleRange * PiDivStraight;
        return *this;
    }

    Particle& speed(double speed)
    {
        status.speed = speed;
        return *this;
    }

    Particle& color(Color color)
    {
        status.color = color;
        return *this;
    }

    Particle& drawType(DrawType type)
    {
        status.drawType = type;
        return *this;
    }


    // 【メソッド】生成
    void create(int quantity)
    {
        static double fix, size, rad, speed;

        for (int i = 0; i < quantity; ++i) {
            // サイズ
            fix  = status.size * status.randPow / 30.0;
            size = status.size + Random(-fix, fix);
            if (size < 0.5) size = 0.5;

            // 角度
            fix = Random(status.radianRange) - status.radianRange / 2.0;
            rad = fmod(status.radian + fix + TwoPi, TwoPi);

            // スピード
            speed = status.speed + Random(-status.randPow, status.randPow);

            // 要素を追加
            obj.emplace_back(Obj(status.pos, size, rad, speed, status.color));
        }
    }



    // 【メソッド】アップデート
    void update()
    {
        double windowWidth  = Window::Width();
        double windowHeight = Window::Height();
        double gravitySin = sin(status.gravityRad);
        double gravityCos = cos(status.gravityRad);

        for (auto &r : obj) {
            // 色の変化
            r.color += status.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
            r.color.a += status.accelColor.a;
            if (r.color.a <= 0.0) {
                r.enable = false;
                continue;
            }

            // サイズの変化
            r.size += status.accelSize;
            if (r.size <= 0.0) {
                r.enable = false;
                continue;
            }

            // 移動
            r.pos.x += cos(r.radian) * r.speed;
            r.pos.y += sin(r.radian) * r.speed;

            // 引力
            r.gravity += status.gravityPow;
            r.pos.x += gravityCos * r.gravity;
            r.pos.y += gravitySin * r.gravity;

            // 画面外かどうか
            if ((r.pos.x < 0.0) || (r.pos.x > windowWidth) ||
                (r.pos.y < 0.0) || (r.pos.y > windowHeight)) {
                r.enable = false;
                continue;
            }

            // スピードの変化
            r.speed += status.accelSpeed;
            if (r.speed < 0.0) r.speed = 0.0;
        }

        // 無効な粒子を削除
        auto dustIt = std::remove_if(obj.begin(), obj.end(),
            [](Obj &obj) { return !obj.enable; });
        obj.erase(dustIt, obj.end());

        Print << U"obj.size: " << obj.size();
    }


    
    // 【メソッド】ドロー
    void draw()
    {
        s3d::RenderStateBlock2D tmp(status.blendState);  // tmpが生きている間だけ設定が有効。破棄時に設定は元に戻る
        
        switch (status.drawType) {
        case DrawType::CircleShadow:
            for (auto &r : obj)
                s3d::Circle(r.pos, r.size).drawShadow(Vec2(0, 0), 10.0, 2.0, r.color);
            break;

        default:
            for (auto &r : obj)
                s3d::Circle(r.pos, r.size).draw(r.color);
            break;
        }
    }
};

