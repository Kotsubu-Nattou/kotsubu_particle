#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <Siv3D.hpp>
#include "kotsubu_math.h"



namespace KotsubuParticle
{
    /////////////////////////////////////////////////////////////////////////////////////
    // 【基底クラス】すべてのパーティクルの元となるクラス。単独利用不可
    //
    class Works
    {
    protected:
        KotsubuMath& math = KotsubuMath::getInstance();
        // 【テスト】
        Font font = Font(24);
        Stopwatch timer;

        // 【内部定数】
        static inline const double Pi = 3.141592653589793;
        static inline const double TwoPi = Pi * 2.0;            // Radianの最大値
        static inline const double Deg2Rad = Pi / 180.0;        // Degに掛けるとRad
        static inline const double Rad2Deg = 180.0 / Pi;        // Radに掛けるとDeg
        static inline const double RootTwo = 1.414213562373095; // 斜辺が45°の直角三角形における、斜辺の比（他の辺は共に1）
        static inline const double One  = 1.0;                  // 1.0
        static inline const double Half = 0.5;                  // 0.5
        static inline const double FrameSecOf60Fps  = 1.0 / 60; // 60FPSのときの1フレームの秒数
        static inline const double ReflectionPowerRate = 0.8;
        static inline const double FadeoutLimit        = 0.01;
        static inline const double WorldMargin         = 10.0;



        // 【内部構造体】粒子パラメータ、全体パラメータ
        struct Element
        {
            Vec2   pos;
            Vec2   oldPos;
            double radian;
            double speed;
            ColorF color;
            double gravity;
            double liveTime;
            bool   fadeout;
            bool   enable;
            Element() :
                pos(Vec2(0, 0)), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0),
                liveTime(0.0), fadeout(false), enable(true)
            {}
            Element(Vec2 _pos, double _radian, double _speed, ColorF _color) :
                pos(_pos), radian(_radian), speed(_speed), color(_color), gravity(0.0),
                liveTime(0.0), fadeout(false), enable(true)
            {}
        };


        struct Property
        {
            double     randPow;
            double     radianRange;
            double     accelSpeed;
            ColorF     accelColor;
            double     gravityPow;
            double     gravityRad;
            double     fadeoutTime;
            double     fadeoutRate;
            BlendState blendState;
            Property() :
                randPow(3.0), radianRange(TwoPi),
                accelSpeed(-0.1), accelColor(-0.01, -0.02, -0.03, -0.001),
                gravityPow(0.2), gravityRad(Pi / 2.0), 
                fadeoutTime(1.0), fadeoutRate(0.975),
                blendState(s3d::BlendState::Additive)
            {}
        };



        // 【内部フィールド】衝突判定用
        std::vector<KotsubuMath::Line>   obstacleLines;
        std::vector<KotsubuMath::Rect>   obstacleRects;
        std::vector<KotsubuMath::Circle> obstacleCircles;
        std::vector<std::vector<Vec2>>   obstaclePolygons;
        std::vector<std::vector<Vec2>>   obstaclePolylines;



        // 【隠しコンストラクタ】
        Works()
        {}


        // 【内部メソッド】
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


        // 【内部メソッド】解像度を、座標スケールに変換
        double toScale(double resolution)
        {
            return 1.0 / resolution;
        }


        //// 【内部メソッド】無効な粒子を削除（Erase-Removeイディオム）
        //template<typename T>
        //void cleanElements(T& elements)
        //{
        //    // いらない物を後ろにどける
        //    auto dustIt = std::remove_if(elements.begin(), elements.end(),
        //        [](Element& elm) { return !elm.enable; });

        //    // どけた部分を抹消
        //    elements.erase(dustIt, elements.end());
        //}


        // 【内部メソッド】無効な粒子を削除（軽量版。並びの安定性なし）
        template<typename T>
        void cleanElements(T& elements)
        {
            // 【テスト】
            timer.restart();

            int i = 0;

            while (i < elements.size()) {
                if (!elements[i].enable) {
                    std::swap(elements[i], elements.back());  // 末尾と交換
                    elements.pop_back();                      // 末尾を削除
                }
                else ++i;
            }

            // 【テスト】
            timer.pause();
            //font(U"elements.size    : ", elements.size()).draw(0, 30);
            //font(U"elements.capacity: ", elements.capacity()).draw(0, 60);
            //font(U"cleanElements time(ms): ", timer.ms()).draw(0, 90);
        }


        // 【内部メソッド】すべての障害物をスケーリング
        void scalingObstacles(double scale)
        {
            if (scale == 1.0) return;  // 等倍なら帰る

            for (auto& r : obstacleLines) {
                r.startPos *= scale;
                r.endPos   *= scale;
            }
            for (auto& r : obstacleRects) {
                r.left   *= scale;
                r.top    *= scale;
                r.right  *= scale;
                r.bottom *= scale;
            }
            for (auto& r : obstacleCircles) {
                r.pos    *= scale;
                r.radius *= scale;
            }
            for (auto& polygon : obstaclePolygons) {
                for (auto& vertex : polygon) {
                    vertex.x *= scale;
                    vertex.y *= scale;
                }
            }
            for (auto& polyline : obstaclePolylines) {
                for (auto& vertex : polyline) {
                    vertex.x *= scale;
                    vertex.y *= scale;
                }
            }
        }


        // 【内部メソッド】すべての衝突判定を行う（障害物は破棄）
        template<typename T>
        void collisionAll(T& elements, double deltaTimeSec)
        {
            double timeScale = FrameSecOf60Fps / deltaTimeSec;
            // 【テスト】
            timer.restart();

            // すべての障害物に対する衝突判定
            collisionLines(elements, timeScale);
            collisionRects(elements, timeScale);
            collisionCircles(elements, timeScale);
            collisionPolygons(elements, timeScale);
            collisionPolylines(elements, timeScale);
                
            // すべての障害物をクリア
            obstacleLines.clear();
            obstacleRects.clear();
            obstacleCircles.clear();
            obstaclePolygons.clear();
            obstaclePolylines.clear();
            
            // 【テスト】
            timer.pause();
            //font(U"collisionAll time(ms): ", timer.ms()).draw(0, 120);
        }


        // 【内部メソッド】線分との衝突判定
        template<typename T>
        void collisionLines(T& elements, double timeScale)
        {
            if (obstacleLines.empty()) return;
            std::rotate(obstacleLines.begin(), obstacleLines.begin() + Random(obstacleLines.size() - 1), obstacleLines.end());

            for (auto& elm : elements) {
                for (auto& line : obstacleLines) {
                    if (math.hit.lineOnLine(line.startPos, line.endPos, elm.oldPos, elm.pos)) {
                        double rad = math.direction(line.endPos - line.startPos);
                        reverseDirection(elm, rad, timeScale);
                        elm.pos = elm.oldPos;
                        elm.fadeout = true;
                        break;
                    }
                }
            }
        }


        // 【内部メソッド】矩形との衝突判定
        template<typename T>
        void collisionRects(T& elements, double timeScale)
        {
            if (obstacleRects.empty()) return;
            std::rotate(obstacleRects.begin(), obstacleRects.begin() + Random(obstacleRects.size() - 1), obstacleRects.end());

            for (auto& elm : elements) {
                for (auto& rect : obstacleRects) {
                    if (math.hit.pointOnBox(elm.pos, rect)) {
                        if (math.hit.lineOnHorizontal(elm.oldPos.y, elm.pos.y, rect.top) ||
                            math.hit.lineOnHorizontal(elm.oldPos.y, elm.pos.y, rect.bottom)) {
                            reverseDirection(elm, 0.0, timeScale);
                        }
                        else {
                            reverseDirection(elm, math.RightAngle, timeScale);
                        }
                        elm.pos = elm.oldPos;
                        elm.fadeout = true;
                        break;
                    }
                }
            }
        }


        // 【内部メソッド】円との衝突判定
        template<typename T>
        void collisionCircles(T& elements, double timeScale)
        {
            if (obstacleCircles.empty()) return;
            std::rotate(obstacleCircles.begin(), obstacleCircles.begin() + Random(obstacleCircles.size() - 1), obstacleCircles.end());

            for (auto& elm : elements) {
                for (auto& circle : obstacleCircles) {
                    double radiusPow = circle.radius * circle.radius;
                    if (math.distancePow(elm.pos, circle.pos) < radiusPow) {
                        reverseDirection(elm, math.direction(circle.pos - elm.pos) + math.RightAngle, timeScale);
                        elm.pos = elm.oldPos;
                        elm.fadeout = true;
                        break;
                    }
                }
            }
        }


        // 【内部メソッド】凸多角形（全ての内角は180°以下）との衝突判定
        // 処理速度優先のため、細長い部分は「壁抜け」が発生する
        // ＜引数＞ vertices
        // ・多角形の各頂点の座標を、vector<Vec2>に「時計回り」の順に格納したもの
        template<typename T>
        void collisionPolygons(T& elements, double timeScale)
        {
            if (obstaclePolygons.empty()) return;
            std::rotate(obstaclePolygons.begin(), obstaclePolygons.begin() + Random(obstaclePolygons.size() - 1), obstaclePolygons.end());

            for (auto& elm : elements) {
                for (auto& vertices : obstaclePolygons) {
                    if (math.hit.pointOnPolygon(elm.pos, vertices)) {
                        // どの辺と交差したかを調べて跳ね返す
                        bool isIntersect = false;
                        for (int i = 0, edgeQty = vertices.size() - 1; i < edgeQty; ++i) {
                            KotsubuMath::Line edge(vertices[i], vertices[i + 1]);
                            if (math.hit.lineOnLine(edge.startPos, edge.endPos, elm.oldPos, elm.pos)) {
                                double rad = math.direction(edge.endPos - edge.startPos);
                                reverseDirection(elm, rad, timeScale);
                                elm.pos = elm.oldPos;
                                elm.fadeout = true;
                                isIntersect = true;
                                break;
                            }
                        }
                        // 交差している辺が無い（図形の内部）なら、図形の外に出ない限り
                        // 上の処理が行われ続けて重くなるので、粒子を消す
                        elm.enable = isIntersect;
                        break;
                    }
                }
            }
        }


        // 【内部メソッド】ポリライン（数珠繋ぎの線分）との衝突判定
        template<typename T>
        void collisionPolylines(T& elements, double timeScale)
        {
            if (obstaclePolylines.empty()) return;
            std::rotate(obstaclePolylines.begin(), obstaclePolylines.begin() + Random(obstaclePolylines.size() - 1), obstaclePolylines.end());

            for (auto& elm : elements) {
                for (auto& vertices : obstaclePolylines) {
                    bool isIntersect = false;
                    for (int i = 0, edgeQty = vertices.size() - 1; i < edgeQty; ++i) {
                        KotsubuMath::Line edge(vertices[i], vertices[i + 1]);
                        if (math.hit.lineOnLine(edge.startPos, edge.endPos, elm.oldPos, elm.pos)) {
                            double rad = math.direction(edge.endPos - edge.startPos);
                            reverseDirection(elm, rad, timeScale);
                            elm.pos = elm.oldPos;
                            elm.fadeout = true;
                            isIntersect = true;
                            break;
                        }
                    }
                    if (isIntersect) break;
                }
            }
        }


        // 【内部メソッド】粒子の進行方向を反転（位置修正なし）
        // ＜引数＞
        // reflectionAxisRad --- 反射軸の角度
        void reverseDirection(Element& element, double reflectionAxisRad, double timeScale)
        {
            Vec2 move = element.pos - element.oldPos;
            
            // 進行方向を反転
            // このプログラムの移動処理は、element.radianとgravityRadの
            // 「2系統」を合算して、実際の「見た目の方向」となる。
            // よって、「見た目の方向」を反射させるためには、1系統だけを反射
            // しても意味はなく、「実際に移動した量、および角度」を元に算出する。
            element.radian = math.reflection(math.direction(move), reflectionAxisRad);

            // 速度を「移動距離」とし、力を減衰させる。（直前までの引力成分も含まれる）
            element.speed = math.length(move) * timeScale * ReflectionPowerRate;

            // 引力をリセット（引力成分はelement.speedに引き継がれている）
            element.gravity = 0.0;
        }


        //// 【内部メソッド】衝突面を通り過ぎた粒子位置を修正（厳密。負荷高め）
        //// ＜引数＞
        //// lineStartPos --- 衝突面（線分）の始点
        //// lineNormal   --- 衝突面（線分）の正規化ベクトル
        //void fixCollisionOverrun(Element& element, Vec2 lineStartPos, Vec2 lineNormal)
        //{
        //    Vec2 normal = math.normalize(element.pos - element.oldPos);
        //    Vec2 hypot  = element.pos - lineStartPos;
        //    double len  = abs(math.outerProduct(hypot, lineNormal));

        //    element.pos -= normal * len * 2.0;
        //}


        //// 【内部メソッド】衝突面を通り過ぎた粒子位置を修正（軽量版）
        //// 次の移動時に衝突ループさせない最低限の修正のため、潜り込みは発生する。
        //// もっと軽量、かつ潜り込まないようにするには、単純に pos = oldPos とすればよい
        //void fixCollisionOverrunLite(Element& element)
        //{
        //    Vec2 v(element.pos - element.oldPos);
        //    element.pos = element.oldPos + v * ReflectionPowerRate;
        //}



    public:
        // 【メソッド】衝突判定の図形を登録（線分）
        // 順次登録可能。次回update時に反映＆すべて破棄
        void registObstacleLine(Vec2 lineStartPos, Vec2 lineEndPos)
        {
            obstacleLines.emplace_back(KotsubuMath::Line(lineStartPos, lineEndPos));
        }


        // 【メソッド】衝突判定の図形を登録（矩形）
        // 順次登録可能。次回update時に反映＆すべて破棄
        void registObstacleRect(double left, double top, double right, double bottom)
        {
            obstacleRects.emplace_back(KotsubuMath::Rect(left, top, right, bottom));
        }


        // 【メソッド】衝突判定の図形を登録（円）
        // 順次登録可能。次回update時に反映＆すべて破棄
        void registObstacleCircle(Vec2 pos, double radius)
        {
            obstacleCircles.emplace_back(KotsubuMath::Circle(pos, radius));
        }


        // 【メソッド】衝突判定の図形を登録（凸多角形。全ての内角は180°以下）
        // 順次登録可能。次回update時に反映＆すべて破棄。
        // 処理速度優先のため、細長い部分は「壁抜け」が発生する。問題がある場合は、
        // 図形をそこだけ「線分」で構成するとよい（registObstacleLineは正確）
        // ＜引数＞ vertices
        // ・各頂点の座標を、vector<Vec2>に「時計回り」の順に格納したもの
        // ・凹型にならないよう注意する（動作不定。どうしても凹型にしたい場合は、凸型に分けて複数登録する）
        // ・最後の頂点と最初の頂点は自動的に閉じられる
        void registObstaclePolygon(const std::vector<Vec2>& vertices)
        {
            if (vertices.size() < 3) return;  // 頂点が3個未満なら登録しない
            obstaclePolygons.emplace_back(vertices);
            obstaclePolygons.back().emplace_back(vertices[0]);  // 図形を閉じるために「最初の頂点」を追加
        }


        // 【メソッド】衝突判定の図形を登録（ポリライン。数珠繋ぎの線分）
        // 順次登録可能。次回update時に反映＆すべて破棄。
        // ＜引数＞ vertices
        // ・各頂点の座標を、順番にvector<Vec2>に格納したもの
        void registObstaclePolyline(const std::vector<Vec2>& vertices)
        {
            if (vertices.size() < 2) return;  // 頂点が2個未満なら登録しない
            obstaclePolylines.emplace_back(vertices);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // 【メインクラス】円形のパーティクル
    // 円系パーティクルの元となるクラス。他の円系パーティクルはこれを拡張（継承）したもの
    //
    class Circle : public Works
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
        Circle& pos(         Vec2   pos)    { property.pos         = pos;                        return *this; }
        Circle& size(        double size)   { property.size        = fixSize(size);              return *this; }
        Circle& speed(       double speed)  { property.speed       = fixSpeed(speed);            return *this; }
        Circle& color(       ColorF color)  { property.color       = color;                      return *this; }
        Circle& angle(       double degree) { property.radian      = math.toRadian(degree);      return *this; }
        Circle& angleRange(  double degree) { property.radianRange = math.toRadianRange(degree); return *this; }
        Circle& accelSize(   double size)   { property.accelSize   = size;                       return *this; }
        Circle& accelSpeed(  double speed)  { property.accelSpeed  = speed;                      return *this; }
        Circle& accelColor(  ColorF color)  { property.accelColor  = color;                      return *this; }
        Circle& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);     return *this; }
        Circle& gravityAngle(double degree) { property.gravityRad  = math.toRadian(degree);      return *this; }
        Circle& random(      double power)  { property.randPow     = fixRandomPower(power);      return *this; }
        Circle& blendState(s3d::BlendState state) { property.blendState = state; return *this; }


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
            double delta        = s3d::System::DeltaTime();
            double timeScale    = delta / FrameSecOf60Fps;
            int    windowWidth  = s3d::Window::Width();
            int    windowHeight = s3d::Window::Height();
            double gravitySin   = sin(property.gravityRad) * timeScale;
            double gravityCos   = cos(property.gravityRad) * timeScale;
            
            for (auto& r : elements) {
                if (r.fadeout) {
                    // フェードアウト
                    r.color.a *= property.fadeoutRate;
                    if (r.color.a < FadeoutLimit) {
                        r.enable = false;
                        continue;
                    }
                }
                else {
                    // アルファの変化
                    r.color.a += property.accelColor.a * timeScale;
                    if (r.color.a < 0.0 && property.accelColor.a < 0.0) {
                        r.enable = false;
                        continue;
                    }
                    // 生存時間を累積
                    r.liveTime += delta;
                    if (r.liveTime > property.fadeoutTime)
                        r.fadeout = true;
                }

                // RGBの変化
                r.color += property.accelColor * timeScale;  // ColorF型の演算は、アルファは対象外

                // サイズの変化
                r.size += property.accelSize * timeScale;
                if (r.size < 0.0) {
                    r.enable = false;
                    continue;
                }

                // 移動
                r.oldPos = r.pos;
                r.pos.x += cos(r.radian) * r.speed * timeScale;
                r.pos.y += sin(r.radian) * r.speed * timeScale;

                // 引力
                r.gravity += property.gravityPow * timeScale;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 領域外の判定
                double margin = r.size + WorldMargin;
                if ((r.pos.x < -margin) || (r.pos.x > windowWidth  + margin) ||
                    (r.pos.y < -margin) || (r.pos.y > windowHeight + margin)) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed * timeScale;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // 衝突判定
            collisionAll(elements, delta);

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
            double rate;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                rate = One - i / static_cast<double>(layerQty);
                for (auto& r : elements)
                    s3d::Circle(r.pos, r.size * rate).draw(r.color);
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
    class Dot : public Works
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



    public:
        // 【フィールド】
        std::vector<Element> elements;


        // 【コンストラクタ】
        Dot(size_t reserve = 10000)
        {
            elements.reserve(reserve);
            resolution(3.0);
        }


        // 【セッタ】各初期パラメータ。メソッドチェーン方式
        Dot& pos(         Vec2   pos)    { property.pos         = pos;                        return *this; }
        Dot& speed(       double speed)  { property.speed       = fixSpeed(speed);            return *this; }
        Dot& color(       ColorF color)  { property.color       = color;                      return *this; }
        Dot& angle(       double degree) { property.radian      = math.toRadian(degree);      return *this; }
        Dot& angleRange(  double degree) { property.radianRange = math.toRadianRange(degree); return *this; }
        Dot& accelSpeed(  double speed)  { property.accelSpeed  = speed;                      return *this; }
        Dot& accelColor(  ColorF color)  { property.accelColor  = color;                      return *this; }
        Dot& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);     return *this; }
        Dot& gravityAngle(double degree) { property.gravityRad  = math.toRadian(degree);      return *this; }
        Dot& random(      double power)  { property.randPow     = fixRandomPower(power);      return *this; }
        Dot& blendState(s3d::BlendState state) { property.blendState = state; return *this; }

        // スムージング
        Dot& smoothing(bool isSmooth)
        {
            property.samplerState = isSmooth ? s3d::SamplerState::ClampLinear :
                                               s3d::SamplerState::ClampNearest;
            return *this;
        }

        // 解像度 1.0（等倍） ～ 8.0
        Dot& resolution(double res)
        {
            static double oldRes = -1;
            if (res < 1.0) res = 1.0;
            if (res > 8.0) res = 8.0;


            if (res != oldRes) {
                property.resolution = res;

                // 新しいサイズのブランクイメージを作る
                double scale = toScale(res);
                property.blankImg = s3d::Image(static_cast<int>(Window::Width()  * scale),
                                               static_cast<int>(Window::Height() * scale));

                // 動的テクスチャは「同じサイズ」のイメージを供給しないと描画されないためリセット。
                // また、テクスチャやイメージのreleaseやclearは、連続で呼び出すとエラーする
                property.tex.release();

                oldRes = res;
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
            Vec2   pos            = property.pos * toScale(property.resolution);

            if (property.pos.x >= 0.0 && property.pos.x < Window::Width() &&
                property.pos.y >= 0.0 && property.pos.y < Window::Height()) {
                for (int i = 0; i < quantity; ++i) {
                    // 角度
                    shake = Random(-radShake, radShake) * Random(One) * Random(One);
                    range = Random(-radRangeHalf, radRangeHalf);
                    rad = fmod(property.radian + range + shake + TwoPi, TwoPi);

                    // スピード
                    speed = property.speed + Random(speedRandLower, property.randPow);

                    // 要素を追加
                    elements.emplace_back(Element(pos, rad, speed, property.color));
                }
            }
        }


        // 【メソッド】アップデート
        void update()
        {
            double delta       = s3d::System::DeltaTime();
            double timeScale   = delta / FrameSecOf60Fps;
            int    imgWidth    = property.blankImg.width();
            int    imgHeight   = property.blankImg.height();
            double gravitySin  = sin(property.gravityRad) * timeScale;
            double gravityCos  = cos(property.gravityRad) * timeScale;

            for (auto& r : elements) {
                if (r.fadeout) {
                    // フェードアウト
                    r.color.a *= property.fadeoutRate;
                    if (r.color.a < FadeoutLimit) {
                        r.enable = false;
                        continue;
                    }
                }
                else {
                    // アルファの変化
                    r.color.a += property.accelColor.a * timeScale;
                    if (r.color.a < 0.0 && property.accelColor.a < 0.0) {
                        r.enable = false;
                        continue;
                    }
                    // 生存時間を累積
                    r.liveTime += delta;
                    r.fadeout = (r.liveTime > property.fadeoutTime);
                }

                // RGBの変化
                r.color += property.accelColor * timeScale;  // ColorF型の演算は、アルファは対象外

                // 移動
                r.oldPos = r.pos;
                r.pos.x += cos(r.radian) * r.speed * timeScale;
                r.pos.y += sin(r.radian) * r.speed * timeScale;

                // 引力
                r.gravity += property.gravityPow * timeScale;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 領域外の判定（posはイメージ配列の添え字になるので、そのチェックも兼ねる）
                if ((r.pos.x < 0.0) || (r.pos.x >= imgWidth) ||
                    (r.pos.y < 0.0) || (r.pos.y >= imgHeight)) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed * timeScale;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // 衝突判定
            scalingObstacles(toScale(property.resolution));
            collisionAll(elements, delta);

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
    // 【Dotを継承】点のパーティクル（加算合成）
    //
    class DotTailed : public Dot
    {
    public:
        // 【メソッド】ドロー（オーバーライド）
        void draw()
        {
            // イメージをクリア（clear関数もあるが連続で呼び出すとエラーする）
            property.img = property.blankImg;

            // 【テスト】
            int lenMax = -1;

            // イメージを作成（粒子の数だけ処理。posが確実にimg[n]の範囲内であること）
            for (auto& r : elements) {
                Vec2   normal = math.normalize(r.pos - r.oldPos);
                int    len    = static_cast<int>(math.distance(r.pos, r.oldPos) * 0.99);
                Vec2   pos    = r.pos;
                double alpha  = r.color.a;
                ColorF src, dst;
                Point  point;

                // 【テスト】
                if (len > lenMax) lenMax = len;

                for (int i = 0; i <= len; ++i) {
                    // 現在位置（座標）の色を求める（自前の加算ブレンディング）
                    point = pos.asPoint();
                    src = property.img[point];
                    dst.r = src.r + r.color.r * alpha;
                    dst.g = src.g + r.color.g * alpha;
                    dst.b = src.b + r.color.b * alpha;
                    dst.a = src.a + alpha;  // 本来は違うかもしれないが見栄えがよい（キラキラする）

                    // 求めた色をセット
                    property.img[point].set(dst);

                    alpha *= 0.925;
                    if (alpha < FadeoutLimit) break;
                    pos -= normal;
                }
            }

            // 【テスト】
            font(U"lenMax: ", lenMax).draw();

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
    class Star : public Works
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
        Star& pos(         Vec2   pos)    { property.pos         = pos;                        return *this; }
        Star& size(        double size)   { property.size        = fixSize(size);              return *this; }
        Star& speed(       double speed)  { property.speed       = fixSpeed(speed);            return *this; }
        Star& color(       ColorF color)  { property.color       = color;                      return *this; }
        Star& angle(       double degree) { property.radian      = math.toRadian(degree);      return *this; }
        Star& angleRange(  double degree) { property.radianRange = math.toRadianRange(degree); return *this; }
        Star& accelSize(   double size)   { property.accelSize   = size;                       return *this; }
        Star& accelSpeed(  double speed)  { property.accelSpeed  = speed;                      return *this; }
        Star& accelColor(  ColorF color)  { property.accelColor  = color;                      return *this; }
        Star& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);     return *this; }
        Star& gravityAngle(double degree) { property.gravityRad  = math.toRadian(degree);      return *this; }
        Star& random(      double power)  { property.randPow     = fixRandomPower(power);      return *this; }
        Star& rotate(      double speed)  { property.rotateSpeed = speed;                      return *this; }
        Star& blendState(s3d::BlendState state) { property.blendState = state; return *this; }

        
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
            double delta        = s3d::System::DeltaTime();
            double timeScale    = delta / FrameSecOf60Fps;
            int    windowWidth  = s3d::Window::Width();
            int    windowHeight = s3d::Window::Height();
            double gravitySin   = sin(property.gravityRad) * timeScale;
            double gravityCos   = cos(property.gravityRad) * timeScale;

            for (auto& r : elements) {
                if (r.fadeout) {
                    // フェードアウト
                    r.color.a *= property.fadeoutRate;
                    if (r.color.a < FadeoutLimit) {
                        r.enable = false;
                        continue;
                    }
                }
                else {
                    // アルファの変化
                    r.color.a += property.accelColor.a * timeScale;
                    if (r.color.a < 0.0 && property.accelColor.a < 0.0) {
                        r.enable = false;
                        continue;
                    }
                    // 生存時間を累積
                    r.liveTime += delta;
                    r.fadeout = (r.liveTime > property.fadeoutTime);
                }

                // RGBの変化
                r.color += property.accelColor * timeScale;  // ColorF型の演算は、アルファは対象外

                // サイズの変化
                r.size += property.accelSize * timeScale;
                if (r.size < 0.0) {
                    r.enable = false;
                    continue;
                }

                // 移動
                r.oldPos = r.pos;
                r.pos.x += cos(r.radian) * r.speed * timeScale;
                r.pos.y += sin(r.radian) * r.speed * timeScale;

                // 引力
                r.gravity += property.gravityPow * timeScale;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // 領域外の判定
                double margin = r.size + WorldMargin;
                if ((r.pos.x < -margin) || (r.pos.x > windowWidth  + margin) ||
                    (r.pos.y < -margin) || (r.pos.y > windowHeight + margin)) {
                    r.enable = false;
                    continue;
                }

                // スピードの変化
                r.speed += property.accelSpeed * timeScale;
                if (r.speed < 0.0) r.speed = 0.0;

                // 回転
                r.rotateRad += r.rotateSpeed * timeScale;
                if ((r.rotateRad < 0.0) || (r.rotateRad >= TwoPi))
                    r.rotateRad = fmod(r.rotateRad, TwoPi);
            }

            // 衝突判定
            collisionAll(elements, delta);

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
            double rate;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                rate = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    Shape2D::Star(r.size * rate, r.pos, r.rotateRad).draw(r.color);
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
            double rate;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                rate = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    s3d::RectF(Arg::center = r.pos, r.size * RootTwo * rate).rotated(r.rotateRad).draw(r.color);
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
            double rate;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQty; ++i) {
                rate = One - i / static_cast<double>(layerQty) * Half;
                for (auto& r : elements)
                    Shape2D::Pentagon(r.size * rate, r.pos, r.rotateRad).draw(r.color);
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
