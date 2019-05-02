#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <Siv3D.hpp>


namespace Particle2D
{
    /////////////////////////////////////////////////////////////////////////////////////
    // 【基底クラス】すべてのパーティクルの元となる、内部で使用するメンバなど。単独利用不可
    //
    class InternalWorks
    {
    protected:
        // 各クラスで使用する定数
        static inline const double Pi = 3.141592653589793;
        static inline const double TwoPi = Pi * 2.0;            // Radianの最大値
        static inline const double PiDivStraight = Pi / 180.0;  // Degに掛けるとRad
        static inline const double StraightDivPi = 180.0 / Pi;  // Radに掛けるとDeg

        struct ReflectionAxis  // 反射軸の定数。向きをラジアンで表す
        {
            static inline const double Horizontal = Pi * 0.0;  // 水平の軸
            static inline const double LowerRight = Pi * 0.5;  // 右下に伸びる軸
            static inline const double Vertical   = Pi * 1.0;  // 垂直の軸
            static inline const double LowerLeft  = Pi * 1.5;  // 左下に伸びる軸
        };


        // 各クラスで使用する構造体
        struct Element
        {
            Vec2   pos;
            double radian;
            double speed;
            ColorF color;
            double gravity;
            bool   stucking;
            bool   enable;
            Element() :
                pos(Vec2(0, 0)), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
            {}
            Element(Vec2 _pos, double _radian, double _speed, ColorF _color) :
                pos(_pos), radian(_radian), speed(_speed), color(_color),
                gravity(0.0), stucking(false), enable(true)
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
            bool         wallRight;
            bool         wallBottom;
            bool         wallLeft;
            bool         wallTop;
            Property() :
                randPow(5.0), radianRange(TwoPi),
                accelSpeed(-0.1), accelColor(0.0, -0.02, -0.03, -0.001),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                blendState(s3d::BlendState::Additive),
                wallRight(true), wallBottom(true), wallLeft(true), wallTop(true)
            {}
        };


        // 【隠しコンストラクタ】
        InternalWorks()
        {}


        // 【メソッド】
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


        // 【メソッド】粒子の反射
        // ＜引数＞reflectionAxisRad --- 反射軸の「向き」をラジアンで指定
        // 反射軸が「0 rad（  0°）」のとき、進入角が90°なら結果は270°、270°なら90°
        // 反射軸が「π rad（180°）」のとき、進入角が 0°なら結果は180°、180°なら 0°
        void reflection(double reflectionAxisRad, Element& elem, Vec2 oldPos)
        {
            Vec2 dist;
            double rad;
            static const double AlphaFadeRatio  = 0.6;
            static const double AlphaFadeLimit  = 0.03;
            static const double ReflectionRatio = 0.7;
            static const double TurnbackRatio   = 0.3;

            // アルファを減衰
            elem.color.a *= AlphaFadeRatio;
            if (elem.color.a < AlphaFadeLimit) {
                elem.enable = false;
                return;
            }

            // 1フレーム前からのXとYの移動量（ベクトルOldPos）
            dist = elem.pos - oldPos;

            // ベクトルOldPosの角度を求める
            // このプログラムの移動処理は、elem.radianとgravityRadの
            // 「2系統」を合算して、実際の「見た目の方向」となる。
            // よって、「見た目の方向」を反射させるためには、1系統だけを反射
            // しても意味はなく、「実際に移動した量、および角度」を元に算出する。
            rad = atan2(dist.y, dist.x);

            // 角度を反射
            elem.radian = fmod(reflectionAxisRad - rad, TwoPi);

            // 速度を「移動距離」とし、力を減衰させる。（直前までの引力成分も含まれる）
            elem.speed = sqrt(dist.x*dist.x + dist.y*dist.y) * ReflectionRatio;

            // 引力をリセット（上でelem.speedに引力成分は引き継がれている）
            elem.gravity = 0.0;

            // 壁に潜り込んだ位置を、「oldからの割合」分だけ戻す
            elem.pos = oldPos + dist * TurnbackRatio;

            // 「反射した」フラグ
            elem.stucking = true;
        }


        // 【メソッド】無効な粒子を削除
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
        Circle& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }

        // 【メソッド】生成
        void create(int quantity)
        {
            double size, rad, randRad, speed;
            double sizeRandRange = property.size * property.randPow / 30.0;
            double radRangeFix   = property.radianRange / 2.0;

            for (int i = 0; i < quantity; ++i) {
                // サイズ
                size = property.size + Random(-sizeRandRange, sizeRandRange);

                // 角度
                randRad = Random(property.radianRange) - radRangeFix;
                rad     = fmod(property.radian + randRad + TwoPi, TwoPi);
                
                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(CircleElement(property.pos, size, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
            int    windowWidth  = Window::Width();
            int    windowHeight = Window::Height();
            double gravitySin   = sin(property.gravityRad);
            double gravityCos   = cos(property.gravityRad);
            bool   wallsEnable  = property.wallRight | property.wallBottom | property.wallLeft | property.wallTop;
            Vec2   old;

            for (auto &r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.stucking)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }

                // サイズの変化
                r.size += property.accelSize;
                if (r.size <= 0.0) {
                    r.enable = false;
                    continue;
                }

                // 移動
                old = r.pos;
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 壁
                r.stucking = false;
                if (wallsEnable) {
                    if ((property.wallRight)  && (r.pos.x > windowWidth  - r.size)) reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallBottom) && (r.pos.y > windowHeight - r.size)) reflection(ReflectionAxis::Horizontal, r, old);
                    if ((property.wallLeft)   && (r.pos.x < r.size))                reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallTop)    && (r.pos.y < r.size))                reflection(ReflectionAxis::Horizontal, r, old);
                }

                // 画面外かどうか
                if ((r.pos.x <= -r.size) || (r.pos.x >= windowWidth  + r.size) ||
                    (r.pos.y <= -r.size) || (r.pos.y >= windowHeight + r.size)) {
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
        Dot& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }

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
            double rad, randRad, speed;
            double radRangeFix = property.radianRange / 2.0;
            Vec2 pos = property.pos / property.scale;

            for (int i = 0; i < quantity; ++i) {
                // 角度
                randRad = Random(property.radianRange) - radRangeFix;
                rad     = fmod(property.radian + randRad + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(Element(pos, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
            int    imgWidth    = property.blankImg.width();
            int    imgHeight   = property.blankImg.height();
            double gravitySin  = sin(property.gravityRad);
            double gravityCos  = cos(property.gravityRad);
            bool   wallsEnable = property.wallRight | property.wallBottom | property.wallLeft | property.wallTop;
            Vec2   old;

            for (auto &r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.stucking)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }

                // 移動
                old = r.pos;
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 壁
                r.stucking = false;
                if (wallsEnable) {
                    if ((property.wallRight)  && (r.pos.x > imgWidth))  reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallBottom) && (r.pos.y > imgHeight)) reflection(ReflectionAxis::Horizontal, r, old);
                    if ((property.wallLeft)   && (r.pos.x < 0.0))       reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallTop)    && (r.pos.y < 0.0))       reflection(ReflectionAxis::Horizontal, r, old);
                }

                // 画面外かどうか（posはイメージ配列の添え字になるので、そのチェックも兼ねる）
                if ((r.pos.x < 0.0) || (r.pos.x >= imgWidth) ||
                    (r.pos.y < 0.0) || (r.pos.y >= imgHeight)) {
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





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】星のパーティクル
    //
    class Star : public InternalWorks
    {
    protected:
        // クラス内部で使用する構造体
        struct StarElement : public Element
        {
            double size;
            StarElement() : size(5.0)
            {}
            StarElement(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
                Element(_pos, _radian, _speed, _color), size(_size)
            {}
        };


        struct StarProperty : public Property, public StarElement
        {
            double  accelSize;
            StarProperty() : accelSize(-0.01)
            {}
        };


        // 【フィールド】
        StarProperty property;
        std::vector<StarElement> elements;



    public:
        // 【コンストラクタ】
        Star(size_t reserve = 3000)
        {
            elements.reserve(reserve);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Star& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Star& size(        double size)   { property.size        = fixSize(size);           return *this; }
        Star& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Star& color(       Color  color)  { property.color       = color;                   return *this; }
        Star& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Star& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Star& accelSize(   double size)   { property.accelSize   = size;                    return *this; }
        Star& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Star& accelColor(  Color  color)  { property.accelColor  = color;                   return *this; }
        Star& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Star& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Star& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Star& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        Star& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }

        // 【メソッド】生成
        void create(int quantity)
        {
            double size, rad, randRad, speed;
            double sizeRandRange = property.size * property.randPow / 30.0;
            double radRangeFix   = property.radianRange / 2.0;

            for (int i = 0; i < quantity; ++i) {
                // サイズ
                size = property.size + Random(-sizeRandRange, sizeRandRange);

                // 角度
                randRad = Random(property.radianRange) - radRangeFix;
                rad     = fmod(property.radian + randRad + TwoPi, TwoPi);
                
                // スピード
                speed = property.speed + Random(-property.randPow, property.randPow);

                // 要素を追加
                elements.emplace_back(StarElement(property.pos, size, rad, speed, property.color));
            }
        }



        // 【メソッド】アップデート
        void update()
        {
            int    windowWidth  = Window::Width();
            int    windowHeight = Window::Height();
            double gravitySin   = sin(property.gravityRad);
            double gravityCos   = cos(property.gravityRad);
            bool   wallsEnable  = property.wallRight | property.wallBottom | property.wallLeft | property.wallTop;
            Vec2   old;

            for (auto &r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.stucking)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }

                // サイズの変化
                r.size += property.accelSize;
                if (r.size <= 0.0) {
                    r.enable = false;
                    continue;
                }

                // 移動
                old = r.pos;
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 壁
                r.stucking = false;
                if (wallsEnable) {
                    if ((property.wallRight)  && (r.pos.x > windowWidth  - r.size)) reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallBottom) && (r.pos.y > windowHeight - r.size)) reflection(ReflectionAxis::Horizontal, r, old);
                    if ((property.wallLeft)   && (r.pos.x < r.size))                reflection(ReflectionAxis::Vertical,   r, old);
                    if ((property.wallTop)    && (r.pos.y < r.size))                reflection(ReflectionAxis::Horizontal, r, old);
                }

                // 画面外かどうか
                if ((r.pos.x <= -r.size) || (r.pos.x >= windowWidth  + r.size) ||
                    (r.pos.y <= -r.size) || (r.pos.y >= windowHeight + r.size)) {
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
                Shape2D::Star(r.size, r.pos, Pi*0.5).draw(r.color);
        }
    };
}
