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
        static inline const double RootTwo = 1.414213562373095; // 斜辺が45°の直角三角形における、斜辺の比（他の辺は共に1）
        static inline const double One  = 1.0;                  // 1.0
        static inline const double Half = 0.5;                  // 0.5

        struct ReflectionAxis  // 反射軸の定数。向きをラジアンで表す
        {
            static inline const double Horizontal = Pi * 0.0  * 2.0; // 右に伸びる軸（水平）
            static inline const double LowerRight = Pi * 0.25 * 2.0; // 右下に伸びる軸
            static inline const double Vertical   = Pi * 0.5  * 2.0; // 下に伸びる軸（垂直）
            static inline const double LowerLeft  = Pi * 0.75 * 2.0; // 左下に伸びる軸
        };


        // 各クラスで使用する構造体
        struct Element
        {
            Vec2   pos;
            double radian;
            double speed;
            ColorF color;
            double gravity;
            bool   alphaLock;
            bool   enable;
            Element() :
                pos(Vec2(0, 0)), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0),
                alphaLock(false), enable(true)
            {}
            Element(Vec2 _pos, double _radian, double _speed, ColorF _color) :
                pos(_pos), radian(_radian), speed(_speed), color(_color),
                gravity(0.0), alphaLock(false), enable(true)
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
                randPow(3.0), radianRange(TwoPi),
                accelSpeed(-0.1), accelColor(-0.01, -0.02, -0.03, -0.001),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                blendState(s3d::BlendState::Additive),
                wallRight(false), wallBottom(false), wallLeft(false), wallTop(false)
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
            if (degree <   0.0) degree = 0.0;
            if (degree > 360.0) degree = 360.0;

            return degree * PiDivStraight;
        }


        // 【メソッド】alphaを指定の割合に変更。フェードアウト用
        // フェードアウト中に、通常のalpha加減算処理を行うとバッティングしてしまう。
        // そのため、1フレームだけパスさせるためにフラグを立てる。Element.alphaLock = true
        // また、alphaが下限値を下回ったら、その粒子を無効にする。Element.enable = false
        void fadeoutAlpha(Element& elem, double alphaRatio)
        {
            static const double LowerLimit  = 0.02;

            elem.color.a *= alphaRatio;
            if (elem.color.a < LowerLimit)
                elem.enable = false;
            elem.alphaLock = true;
        }


        // 【メソッド】粒子の反射
        // ＜引数＞reflectionAxisRad --- 反射軸。値は「反射軸の方向rad * 2」で、範囲は0～2π
        // 反射軸が「 0° * 2 = 0 rad」のとき、進入角が90°なら結果は270°、270°なら90°
        // 反射軸が「90° * 2 = π rad」のとき、進入角が 0°なら結果は180°、180°なら 0°
        void reflection(double reflectionAxisRad, Element& elem, Vec2 oldPos)
        {
            Vec2 dist;
            double rad;
            static const double AlphaFadeRatio  = 0.6;
            static const double ReflectionRatio = 0.7;
            static const double TurnbackRatio   = 0.3;

            // アルファを減衰
            fadeoutAlpha(elem, AlphaFadeRatio);
            if(!elem.enable) return;

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
        }


        // 【メソッド】無効な粒子を削除
        template<typename T>
        void cleanElements(T& elements)
        {

            auto dustIt = std::remove_if(elements.begin(), elements.end(),
                [](Element& element) { return !element.enable; });

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
            CircleElement() : size(20.0)
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
        Circle& color(       ColorF color)  { property.color       = color;                   return *this; }
        Circle& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Circle& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Circle& accelSize(   double size)   { property.accelSize   = size;                    return *this; }
        Circle& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Circle& accelColor(  ColorF color)  { property.accelColor  = color;                   return *this; }
        Circle& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Circle& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Circle& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Circle& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        Circle& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }


        // 【メソッド】生成
        void create(int quantity)
        {
            double size, rad, shake, range, speed;
            double sizeRandRange  = property.size * property.randPow * 0.03;
            double radShake       = (property.radianRange * property.randPow + property.randPow) * 0.05;
            double radRangeHalf   = property.radianRange * Half;
            double speedRandLower = -property.randPow * Half;

            for (int i = 0; i < quantity; ++i) {
                // サイズ
                size = property.size + Random(-sizeRandRange, sizeRandRange);

                // 角度
                shake = Random(-radShake, radShake) * Random(One) * Random(One);
                range = Random(-radRangeHalf, radRangeHalf);
                rad   = fmod(property.radian + range + shake + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(speedRandLower, property.randPow);

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
            static const double AlphaFadeRatio  = 0.95;

            for (auto& r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.alphaLock)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }
                r.alphaLock = false;

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

                // 動いていないなら、アルファを強制的に下げる
                if (r.pos == old) {
                    fadeoutAlpha(r, AlphaFadeRatio);
                    if (!r.enable) continue;
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

            for (auto& r : elements)
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

            for (auto& r : elements)
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
        int layerQty;

    public:
        // 【コンストラクタ】
        CircleSmoke() : layerQty(5)
        {}

        // 【セッタ】初期パラメータ。メソッドチェーン方式
        CircleSmoke& layerQuantity(int qty)
        { 
            if (qty < 1)  qty = 1;
            if (qty > 10) qty = 10;
            layerQty = qty;
            return *this;
        }

        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                ratio = One - i / static_cast<double>(layerQty);
                for (auto& r : elements)
                    s3d::Circle(r.pos, r.size * ratio).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】点のパーティクル
    // 点系パーティクルの元となるクラス。他の点系パーティクルはこれを拡張（継承）したもの。
    // 最も多くのパーティクルを描画できる。ただし、このクラスはパーティクル数が
    // 「0」でも画面全体のイメージを複製＆描画するため、最低負荷は高め。
    // resolutionメソッドで 1.0～8.0 が指定でき、高いほど粗くなる代わりに負荷を軽減できる。
    // ※この仕組みは、図形の描画が重く、ブレンディングも効かないため「点系」のみ
    //
    class Dot : public InternalWorks
    {
    protected:
        // クラス内部で使用する構造体
        struct DotProperty : public Property, public Element
        {
            double         resolution;
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
            resolution(3.0);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Dot& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Dot& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Dot& color(       ColorF color)  { property.color       = color;                   return *this; }
        Dot& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Dot& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Dot& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Dot& accelColor(  ColorF color)  { property.accelColor  = color;                   return *this; }
        Dot& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Dot& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Dot& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Dot& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        Dot& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }

        // スムージング
        Dot& smoothing(bool isSmooth)
        {
            property.samplerState = isSmooth ? s3d::SamplerState::Default2D :
                                               s3d::SamplerState::ClampNearest;
            return *this;
        }

        // 解像度 1.0（等倍） ～ 8.0
        Dot& resolution(double scale)
        {
            static double oldScale = -1;

            if (scale < 1.0) scale = 1.0;
            if (scale > 8.0) scale = 8.0;

            if (scale != oldScale) {
                property.resolution = scale;

                // 新しいサイズのブランクイメージを作る
                property.blankImg = s3d::Image(static_cast<int>(Window::Width()  / property.resolution),
                                               static_cast<int>(Window::Height() / property.resolution));

                // 動的テクスチャは「同じサイズ」のイメージを供給しないと描画されないためリセット。
                // また、テクスチャやイメージのreleaseやclearは、連続で呼び出すとエラーする
                property.tex.release();

                oldScale = property.resolution;
            }

            return *this;
        }


        // 【メソッド】生成
        void create(int quantity)
        {
            double rad, shake, range, speed;
            double radShake       = (property.radianRange * property.randPow + property.randPow) * 0.05;
            double radRangeHalf   = property.radianRange * Half;
            double speedRandLower = -property.randPow * Half;
            Vec2   pos            = property.pos / property.resolution;

            for (int i = 0; i < quantity; ++i) {
                // 角度
                shake = Random(-radShake, radShake) * Random(One) * Random(One);
                range = Random(-radRangeHalf, radRangeHalf);
                rad   = fmod(property.radian + range + shake + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(speedRandLower, property.randPow);

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
            static const double AlphaFadeRatio  = 0.95;

            for (auto& r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.alphaLock)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }
                r.alphaLock = false;

                // 移動
                old = r.pos;
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // 引力
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 壁
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

                // 動いていないなら、アルファを強制的に下げる
                if (r.pos == old) {
                    fadeoutAlpha(r, AlphaFadeRatio);
                    if (!r.enable) continue;
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
            // イメージをクリア（clear関数もあるが連続で呼び出すとエラーする）
            property.img = property.blankImg;

            // イメージを作成（粒子の数だけ処理。posが確実にimg[n]の範囲内であること）
            for (auto& r : elements)
                property.img[r.pos.asPoint()].set(r.color);

            // 動的テクスチャを更新
            property.tex.fill(property.img);

            // 動的テクスチャをドロー
            s3d::RenderStateBlock2D tmp(property.blendState, property.samplerState);
            property.tex.scaled(property.resolution).draw();
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
            ColorF src, dst;
            Point pos;

            // イメージをクリア（clear関数もあるが連続で呼び出すとエラーする）
            property.img = property.blankImg;

            // イメージを作成（粒子の数だけ処理。posが確実にimg[n]の範囲内であること）
            for (auto& r : elements) {
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
            property.tex.scaled(property.resolution).draw();
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】星のパーティクル
    // n角形やテクスチャパーティクルの元となるクラス
    //
    class Star : public InternalWorks
    {
    protected:
        // クラス内部で使用する構造体
        struct StarElement : public Element
        {
            double size;
            double rotateRad;
            double rotateSpeed;
            StarElement() :
                size(20.0), rotateSpeed(0.0)
            {}
            StarElement(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color, double _rotateRad, double _rotateSpeed) :
                Element(_pos, _radian, _speed, _color), size(_size), rotateRad(_rotateRad), rotateSpeed(_rotateSpeed)
            {}
        };


        struct StarProperty : public Property, public StarElement
        {
            double accelSize;
            StarProperty() : accelSize(1.3)
            {
                gravityPow = 0.0;
            }
        };


        // 【フィールド】
        StarProperty property;
        std::vector<StarElement> elements;



    public:
        // 【コンストラクタ】
        Star(size_t reserve = 2000)
        {
            elements.reserve(reserve);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Star& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Star& size(        double size)   { property.size        = fixSize(size);           return *this; }
        Star& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Star& color(       ColorF color)  { property.color       = color;                   return *this; }
        Star& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Star& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Star& accelSize(   double size)   { property.accelSize   = size;                    return *this; }
        Star& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Star& accelColor(  ColorF color)  { property.accelColor  = color;                   return *this; }
        Star& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Star& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Star& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Star& rotate(      double speed)  { property.rotateSpeed = speed;                   return *this; }
        Star& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        Star& walls(bool right, bool bottom, bool left, bool top) { property.wallRight = right; property.wallBottom = bottom; property.wallLeft = left; property.wallTop = top; return *this; }

        
        // 【メソッド】生成
        void create(int quantity)
        {
            double size, rad, shake, range, speed, rotateSpeed;
            double sizeRandRange    = property.size * property.randPow * 0.03;
            double radShake         = (property.radianRange * property.randPow + property.randPow) * 0.05;
            double radRangeHalf     = property.radianRange * Half;
            double speedRandLower   = -property.randPow * Half;
            double rotateSpeedRange = property.randPow * 0.002;

            for (int i = 0; i < quantity; ++i) {
                // サイズ
                size = property.size + Random(-sizeRandRange, sizeRandRange);

                // 角度
                shake = Random(-radShake, radShake) * Random(One) * Random(One);
                range = Random(-radRangeHalf, radRangeHalf);
                rad   = fmod(property.radian + range + shake + TwoPi, TwoPi);

                // スピード
                speed = property.speed + Random(speedRandLower, property.randPow);

                // 回転
                rotateSpeed = property.rotateSpeed + Random(-rotateSpeedRange, rotateSpeedRange);

                // 要素を追加
                elements.emplace_back(StarElement(property.pos, size, rad, speed, property.color, Random(TwoPi), rotateSpeed));
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
            static const double AlphaFadeRatio  = 0.95;

            for (auto& r : elements) {
                // 色の変化
                r.color += property.accelColor;  // ColorFを「+=」した場合、対象はRGBのみ
                if (!r.alphaLock)
                    r.color.a += property.accelColor.a;
                if (r.color.a <= 0.0) {
                    r.enable = false;
                    continue;
                }
                r.alphaLock = false;

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

                // 動いていないなら、アルファを強制的に下げる
                if (r.pos == old) {
                    fadeoutAlpha(r, AlphaFadeRatio);
                    if (!r.enable) continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;

                // 回転
                r.rotateRad += r.rotateSpeed;
                if ((r.rotateRad < 0.0) || (r.rotateRad >= TwoPi))
                    r.rotateRad = fmod(r.rotateRad, TwoPi);
            }

            // 無効な粒子を削除
            cleanElements(elements);
        }


        // 【メソッド】ドロー
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る

            for (auto& r : elements)
                Shape2D::Star(r.size, r.pos, r.rotateRad).draw(r.color);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Starを継承】正方形のパーティクル
    //
    class Rect : public Star
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        // s3dにおけるCircleやStarのサイズは「半径 * 2」であるが、Rectのサイズは
        //「左上を基点とした縦横の長さ」なので、基点が違う上、見かけの大きさは半分となる。
        // これをCircleなどと処理を共通にするには、Rectが45°のときでも「想定する円」をはみ出ない
        // ギリギリの大きさにする（想定する円に内接する正方形の大きさ）
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る

            for (auto& r : elements)
                s3d::RectF(Arg::center = r.pos, r.size * RootTwo).rotated(r.rotateRad).draw(r.color);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Starを継承】五角形のパーティクル
    //
    class Pentagon : public Star
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る
            for (auto& r : elements)
                Shape2D::Pentagon(r.size, r.pos, r.rotateRad).draw(r.color);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Starを継承】星のパーティクル（フェード）
    //
    class StarFade : public Star
    {
    protected:
        // 【追加フィールド】
        int layerQty;

    public:
        // 【コンストラクタ】
        StarFade() : layerQty(5)
        {}

        // 【セッタ】初期パラメータ。メソッドチェーン方式
        StarFade& layerQuantity(int qty)
        { 
            if (qty < 1)  qty = 1;
            if (qty > 10) qty = 10;
            layerQty = qty;
            return *this;
        }

        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                ratio = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    Shape2D::Star(r.size * ratio, r.pos, r.rotateRad).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【StarFadeを継承】正方形のパーティクル（フェード）
    //
    class RectFade : public StarFade
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                ratio = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    s3d::RectF(Arg::center = r.pos, r.size * RootTwo * ratio).rotated(r.rotateRad).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【StarFadeを継承】五角形のパーティクル（フェード）
    //
    class PentagonFade : public StarFade
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                ratio = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    Shape2D::Pentagon(r.size * ratio, r.pos, r.rotateRad).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【Starを継承】テクスチャのパーティクル
    //
    class Texture : public Star
    {
    protected:
        // 【追加フィールド】
        s3d::Texture tex = s3d::Texture(Emoji(U"🐈"), TextureDesc::Mipped);


    public:
        // 【コンストラクタ】
        Texture()
        {
            property.color      = ColorF(1.0, 1.0, 1.0, 1.0);
            property.accelColor = ColorF(0.0, 0.0, 0.0, -0.005);
        }


        // 【セッタ】描画するテクスチャーを登録
        void setTexture(s3d::Texture& texture)
        {
            tex = texture;
        }


        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmpが生きている間だけ有効。破棄時に元に戻る
            // テクスチャのサイズもRectと同じ仕様。基点を中心で描画するにはdrawAtメソッドを使う。
            for (auto& r : elements)
                tex.resized(r.size * RootTwo).rotated(r.rotateRad).drawAt(r.pos, r.color);
        }
    };
}
