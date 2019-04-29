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



namespace Particle2D
{
   /////////////////////////////////////////////////////////////////////////////////////
    // 【基底クラス】すべてのパーティクルの元となる、内部で使用するメンバなど。単独利用不可
    //
    class InternalWorks
    {
    protected:
        InternalWorks()
        {}

        struct Element
        {
            Vec2   pos;
            double radian;
            double speed;
            ColorF color;
            double gravity;
            bool   enable;
            Element() :
                pos(Vec2(0, 0)), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
            {}
            Element(Vec2 _pos, double _radian, double _speed, ColorF _color) :
                pos(_pos), radian(_radian), speed(_speed), color(_color),
                gravity(0.0), enable(true)
            {}
        };


        struct Property
        {
            double       randPow;
            double       radianRange;
            double       accelSpeed;
            ColorF       accelColor;
            double       gravityPow;
            double       gravityRad;
            BlendState   blendState;
            Property() :
                randPow(5.0), radianRange(TwoPi),
                accelSpeed(-0.1), accelColor(0.0, -0.02, -0.03, -0.001),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                blendState(s3d::BlendState::Additive)
            {}
        };


        double fixSize(double size)
        {
            if (size < 1.0)size = 1.0;
            return size;
        }


        double fixSpeed(double speed)
        {
            if (speed < 0.0)speed = 0.0;
            return speed;
        }


        double fixGravityPower(double power)
        {
            if (power < 0.0) power = 0.0;
            return power;
        }


        double fixRandomPower(double power)
        {
            if (power < 0.0)power = 0.0;
            return power;
        }

        double convRadian(double degree)
        {
            if (degree < 0.0) {
                degree = fmod(degree, 360.0) + 360.0;
                if (degree == 360.0) degree = 0.0;
            }
            else if (degree >= 360.0)
                degree = fmod(degree, 360.0);

            return degree * PiDivStraight;
        }


        double convRadianRange(double degree)
        {
            if (degree < 0.0) degree = 0.0;
            if (degree > 360.0) degree = 360.0;

            return degree * PiDivStraight;
        }


        // 無効な粒子を削除
        template<typename T>
        void cleanElements(T& elements)
        {

            auto dustIt = std::remove_if(elements.begin(), elements.end(),
                [](Element &element) { return !element.enable; });

            elements.erase(dustIt, elements.end());

            //Print << U"elements.size: " << elements.size();
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】円形のパーティクル
    // 円系パーティクルの元となるクラス。他の円系パーティクルはこれを拡張（継承）したもの
    //
    class Circle : public InternalWorks
    {
    protected:
        // クラス内部で使用する構造体
        struct CircleElement : public Element
        {
            double size;
            CircleElement() : size(5.0)
            {}
            CircleElement(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
                Element(_pos, _radian, _speed, _color), size(_size)
            {}
        };


        struct CircleProperty : public Property, public CircleElement
        {
            double  accelSize;
            CircleProperty() : accelSize(-0.01)
            {}
        };


        // 【フィールド】
        CircleProperty property;
        std::vector<CircleElement> elements;



    public:
        // 【コンストラクタ】
        Circle(size_t reserve = 3000)
        {
            elements.reserve(reserve);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Circle& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Circle& size(        double size)   { property.size        = fixSize(size);           return *this; }
        Circle& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Circle& color(       Color  color)  { property.color       = color;                   return *this; }
        Circle& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Circle& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Circle& accelSize(   double size)   { property.accelSize   = size;                    return *this; }
        Circle& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Circle& accelColor(  Color  color)  { property.accelColor  = color;                   return *this; }
        Circle& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Circle& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Circle& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Circle& blendState(s3d::BlendState state) { property.blendState = state; return *this; }


        // 【メソッド】生成
        void create(int quantity)
        {
            static double fix, size, rad, speed;
            
            for (int i = 0; i < quantity; ++i) {
                // サイズ
                fix = property.size * property.randPow / 30.0;
                size = property.size + Random(-fix, fix);
                if (size < 0.5) size = 0.5;

                // 角度
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);
                
                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(CircleElement(property.pos, size, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
            int margin       = property.size + property.size * property.randPow / 30.0;
            int windowLeft   = -margin;
            int windowTop    = -margin;
            int windowRight  = Window::Width() + margin;
            int windowBottom = Window::Height() + margin;
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
                if ((r.pos.x < windowLeft) || (r.pos.x > windowRight) ||
                    (r.pos.y < windowTop)  || (r.pos.y > windowBottom)) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // 無効な粒子を削除
            cleanElements(elements);
        }



        // 【メソッド】ドロー
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る

            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).draw(r.color);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Circleを継承】淡い光のパーティクル（なめらかだが重い）
    //
    class CircleLight : public Circle
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





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Circleを継承】煙のパーティクル
    //
    class CircleSmoke : public Circle
    {
    private:
        // 【追加フィールド】
        int layerQuantity;

    public:
        // 【コンストラクタ】
        CircleSmoke() : layerQuantity(5)
        {}

        // 【セッタ】初期パラメータ。メソッドチェーン方式
        CircleSmoke& layer(int quantity)
        { 
            if (quantity < 1) quantity = 1;
            if (quantity > 10) quantity = 10;
            layerQuantity = quantity;
            return *this;
        }

        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQuantity; ++i) {
                ratio = i / static_cast<double>(layerQuantity);
                for (auto &r : elements)
                    s3d::Circle(r.pos, r.size - r.size * ratio).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】点のパーティクル
    // 点系パーティクルの元となるクラス。他の点系パーティクルはこれを拡張（継承）したもの。
    // 最も多くのパーティクルを描画できる。ただし、このクラスはパーティクル数が
    // 「0」でも画面全体のイメージを複製＆描画するため、最低負荷は高め。
    // scaleメソッドで 1.0～8.0 が指定可能。高いほど拡大されるが負荷を軽減できる。
    // ※この仕組みは、図形の描画が重く、ブレンディングも効かないため「点系」でのみ採用
    //
    class Dot : public InternalWorks
    {
    protected:
        // クラス内部で使用する構造体
        struct DotProperty : public Property, public Element
        {
            double         scale;
            SamplerState   samplerState;
            DynamicTexture tex;
            Image          img;
            Image          blankImg;
            DotProperty() : samplerState(s3d::SamplerState::ClampNearest)
            {}
        };


        // 【フィールド】
        DotProperty property;
        std::vector<Element> elements;



    public:
        // 【コンストラクタ】
        Dot(size_t reserve = 10000)
        {
            elements.reserve(reserve);
            scale(3.0);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Dot& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Dot& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Dot& color(       Color  color)  { property.color       = color;                   return *this; }
        Dot& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Dot& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Dot& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Dot& accelColor(  Color  color)  { property.accelColor  = color;                   return *this; }
        Dot& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Dot& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Dot& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Dot& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        
        Dot& smooth(bool val)
        {
            property.samplerState = val ? s3d::SamplerState::Default2D :
                                          s3d::SamplerState::ClampNearest;
            return *this;
        }

        Dot& scale(double _scale)
        {
            static double oldScale = -1;

            if (_scale < 1.0) _scale = 1.0;
            if (_scale > 8.0) _scale = 8.0;

            if (_scale != oldScale) {
                property.scale = _scale;

                // 新しいサイズのブランクイメージを作る
                property.blankImg = s3d::Image(static_cast<int>(Window::Width()  / property.scale),
                                               static_cast<int>(Window::Height() / property.scale));

                // 動的テクスチャは「同じサイズ」のイメージを供給しないと描画されないためリセット。
                // また、テクスチャやイメージのreleaseやclearは、連続で呼び出すとエラーする
                property.tex.release();

                oldScale = property.scale;
            }

            return *this;
        }


        // 【メソッド】生成
        void create(int quantity)
        {
            static double fix, rad, speed;
            Vec2 pos = property.pos / property.scale;

            for (int i = 0; i < quantity; ++i) {
                // 角度
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(Element(pos, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
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

                // 移動
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 画面外かどうか（posはイメージ配列の添え字になるので、そのチェックも兼ねる）
                if ((r.pos.x < 0.0) || (r.pos.x >= property.blankImg.width()) ||
                    (r.pos.y < 0.0) || (r.pos.y >= property.blankImg.height())) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // 無効な粒子を削除
            cleanElements(elements);
        }



        // 【メソッド】ドロー
        void draw()
        {
            static ColorF src, dst;
            static Point pos;

            // イメージをクリア（clear関数もあるが連続で呼び出すとエラーする）
            property.img = property.blankImg;

            // イメージを作成（粒子の数だけ処理。posが確実にimg[n]の範囲内であること）
            for (auto &r : elements)
                property.img[r.pos.asPoint()].set(r.color);

            // 動的テクスチャを更新
            property.tex.fill(property.img);

            // 動的テクスチャをドロー
            s3d::RenderStateBlock2D tmp(property.blendState, property.samplerState);
            property.tex.scaled(property.scale).draw();
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Dotを継承】点のパーティクル（加算合成）
    //
    class DotBlended : public Dot
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            static ColorF src, dst;
            static Point pos;

            // イメージをクリア（clear関数もあるが連続で呼び出すとエラーする）
            property.img = property.blankImg;

            // イメージを作成（粒子の数だけ処理。posが確実にimg[n]の範囲内であること）
            for (auto &r : elements) {
                pos = r.pos.asPoint();  // Vec2型のposを、Point型に変換

                // 現在位置（座標）の色を求める（自前の加算ブレンディング）
                src = property.img[pos];
                dst.r = src.r + r.color.r * r.color.a;
                dst.g = src.g + r.color.g * r.color.a;
                dst.b = src.b + r.color.b * r.color.a;
                dst.a = src.a + r.color.a;  // 本来は違うかもしれないが見栄えがよい（キラキラする）

                // 求めた色をセット
                property.img[pos].set(dst);
            }
            
            // 動的テクスチャを更新
            property.tex.fill(property.img);

            // 動的テクスチャをドロー
            s3d::RenderStateBlock2D tmp(property.blendState, property.samplerState);
            property.tex.scaled(property.scale).draw();
        }
    };
}