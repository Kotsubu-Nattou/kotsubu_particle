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



namespace Particle
{
    // 【基底クラス】ふつうのパーティクル
    // すべてのパーティクルの基礎となるクラス。他のパーティクルはこれを拡張（継承）したもの
    class Plain
    {
    protected:
        // クラス内部で使用する構造体
        struct Element
        {
            Element() :
                pos(Vec2(0, 0)), size(1.0), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
            {}
            Element(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
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


        struct ElementProperty : public Element
        {
            ElementProperty() :
                randPow(5.0), accelSpeed(-0.1), accelSize(-0.1),
                accelColor(0.0, -0.02, -0.03, -0.005),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                radianRange(TwoPi), blendState(s3d::BlendState::Additive)
            {}
            double     randPow;
            double     accelSpeed;
            double     accelSize;
            ColorF     accelColor;
            double     gravityPow;
            double     gravityRad;
            double     radianRange;
            BlendState blendState;
        };

        // 【フィールド】
        ElementProperty property;
        std::vector<Element> elements;
    


    public:
        // 【コンストラクタ】
        Plain(size_t reserve = 3000)
        {
            elements.reserve(reserve);
        }
    

        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Plain& pos(  Vec2   pos)   { property.pos   = pos;   return *this; }
        Plain& size( double size)  { property.size  = size;  return *this; }
        Plain& speed(double speed) { property.speed = speed; return *this; }
        Plain& color(Color  color) { property.color = color; return *this; }

        Plain& angle(double angle)
        {
            if (angle < 0.0) {
                angle = fmod(angle, 360.0) + 360.0;
                if (angle == 360.0) angle = 0.0;
            }
            else if (angle >= 360.0)
                angle = fmod(angle, 360.0);
        
            property.radian = angle * PiDivStraight;
            return *this;
        }

        Plain& angleRange(double angleRange)
        {
            if (angleRange < 0.0) angleRange = 0.0;
            if (angleRange > 360.0) angleRange = 360.0;

            property.radianRange = angleRange * PiDivStraight;
            return *this;
        }


        // 【メソッド】生成
        void create(int quantity)
        {
            static double fix, size, rad, speed;

            for (int i = 0; i < quantity; ++i) {
                // サイズ
                fix  = property.size * property.randPow / 30.0;
                size = property.size + Random(-fix, fix);
                if (size < 0.5) size = 0.5;

                // 角度
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(Element(property.pos, size, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
            double windowWidth  = Window::Width();
            double windowHeight = Window::Height();
            double gravitySin = sin(property.gravityRad);
            double gravityCos = cos(property.gravityRad);

            for (auto &r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                r.color.a += property.accelColor.a;
                if (r.color.a < 0.01) {
                    r.enable = false;
                    continue;
                }

                // サイズの変化
                r.size += property.accelSize;
                if (r.size < 0.1) {
                    r.enable = false;
                    continue;
                }

                // 移動
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 画面外かどうか
                if ((r.pos.x < 0.0) || (r.pos.x > windowWidth) ||
                    (r.pos.y < 0.0) || (r.pos.y > windowHeight)) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // 無効な粒子を削除
            auto dustIt = std::remove_if(elements.begin(), elements.end(),
                [](Element &element) { return !element.enable; });
            elements.erase(dustIt, elements.end());

            Print << U"elements.size: " << elements.size();
        }


    
        // 【メソッド】ドロー
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る
        
            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).draw(r.color);
        }
    };





    // 【派生クラス】微光のパーティクル（なめらかだが重い）
    class Light : public Plain
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).drawShadow(Vec2(0, 0), 10.0, 2.0, r.color);
        }
    };





    // 【派生クラス】煙のパーティクル
    class Smoke : public Plain
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double fix;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < 5; ++i) {
                fix = i * 0.2;
                for (auto &r : elements)
                    s3d::Circle(r.pos, r.size - r.size * fix).draw(r.color);
            }
        }
    };
}
