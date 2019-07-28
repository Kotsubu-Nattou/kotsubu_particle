/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_math

・概要
  数学一般で使用する、定数、構造体、メソッドを集めたシングルトン。
  構造体および、定数と一部のメソッドはstaticなので、インクルードするだけで利用可能。
  解放は不要（アプリケーション終了時に自動）
  座標は、OpenSiv3DのVec2の使用を前提（もし、クラスstruct_vecのVEC2を使う場合は、この
  ファイル冒頭、またはこのファイルをインクルードする前に、"USE_STRUCT_VEC"をdefineしておく）
  その他、一般的な図形の構造体、テーブル引き三角関数、衝突判定、直角三角形の要素を求める、など

・使い方
  #include "kotsubu_math.h"
  n = MyMath::Pi;                         // 定数はインクルードするだけで利用可能
  MyMath &math = MyMath::getInstance();   // インスタンスを取得。これで全てのメンバにアクセス可能
  n = math.direction(v);                  // 数学一般
  if (math.hit.lineOnline(lineA, lineB))  // 衝突判定（.hitはクラス内クラスで実装）
**************************************************************************************************/

#pragma once

//#define USE_STRUCT_VEC  // クラスstruct_vecのVEC2を利用するなら定義

#include <cmath>
#ifdef USE_STRUCT_VEC
    #include "struct_vec.h"
#else
    #include <Siv3D.hpp>
#endif





///////////////////////////////////////////////////////////////////////////////////////////////
// 【クラス】MyMath
//
class MyMath
{
#ifdef USE_STRUCT_VEC
    using Vec2 = VEC2<double>;  // クラスstruct_vecのVEC2
#endif
    


public:
    // 【構造体】図形定義用
    struct Line
    {
        Vec2 startPos, endPos;
        Line(Vec2 startPos, Vec2 endPos) : startPos(startPos), endPos(endPos)
        {}
    };

    struct Rect
    {
        double left, top, right, bottom;
        Rect(double left, double top, double right, double bottom) : left(left), top(top), right(right), bottom(bottom)
        {}
    };

    struct Circle
    {
        Vec2 pos;
        double radius;
        Circle(Vec2 pos, double radius) : pos(pos), radius(radius)
        {}
    };



    // 【定数】数学一般
    static constexpr double Epsilon    = 0.00001;           // これ未満を0とする
    static constexpr double Pi         = 3.141592653589793; // π
    static constexpr double TwoPi      = Pi * 2.0;          // Radianの最大値
    static constexpr double RightAngle = Pi / 2.0;          // 直角（90°）のRadian
    static constexpr double Deg2Rad    = Pi / 180.0;        // Degに掛けるとRad
    static constexpr double Rad2Deg    = 180.0 / Pi;        // Radに掛けるとDeg
    static constexpr double RootTwo    = 1.414213562373095; // 斜辺が45°の直角三角形における、斜辺の比（他の辺は共に1）
    static constexpr double RoundFix   = 0.5;               // これを正の小数に足して整数にすると四捨五入
    static constexpr double One        = 1.0;               // double 1.0
    static constexpr double Two        = 2.0;               // double 2.0
    static constexpr double Half       = 0.5;               // double 0.5



    // 【メソッド】唯一のインスタンスの参照を返す
    // なるべく計算のロジックに近い所で受け取るようにすると、キャッシュに乗るためか高速化する。
    // 初回時のみ、インスタンスの生成と、数学用テーブルの作成が行われる
    static MyMath& getInstance()
    {
        static MyMath inst;
        return inst;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 数学メソッド

    // 【メソッド】sin（テーブル引き）
    double sin(double radian)
    {
        int id = abs(static_cast<int>(radian * Sin.Resolution)) % Sin.ScaledTwoPi;

        if (id < Sin.TableMax)
            return (radian < 0.0) ? -Sin.table[id] :  Sin.table[id];
        else {
            id -= Sin.TableMax;
            return (radian < 0.0) ?  Sin.table[id] : -Sin.table[id];
        }
    }



    // 【メソッド】cos（テーブル引き）
    double cos(double radian)
    {
        return sin(radian + RightAngle);
    }



    // 【メソッド】asin（テーブル引き）
    double asin(double ratio)
    {
        int id = abs(static_cast<int>(ratio * ratio * Asin.TableMax + RoundFix));
        if (id >= Asin.TableMax) id = Asin.TableMax - 1;

        return (ratio < 0.0) ? -Asin.table[id] : Asin.table[id];
    }



    // 【メソッド】acos（テーブル引き）
    double acos(double ratio)
    {
        return RightAngle - asin(ratio);
    }



    // 【メソッド】ベクトルの長さを返す
    static double length(Vec2 v)
    {
        return sqrt(lengthPow(v));
    }



    // 【メソッド】ベクトルの長さを返す（ルートを取らない）
    static double lengthPow(Vec2 v)
    {
        return v.x * v.x + v.y * v.y;
    }



    // 【メソッド】2点間の距離を返す
    static double distance(Vec2 a, Vec2 b)
    {
        return sqrt(distancePow(a, b));
    }



    // 【メソッド】2点間の距離を返す（ルートを取らない）
    static double distancePow(Vec2 a, Vec2 b)
    {
        Vec2 v(a - b);
        return v.x * v.x + v.y * v.y;
    }



    // 【メソッド】ベクトルを正規化して返す
    static Vec2 normalize(Vec2 v)
    {
        double len = length(v);
        if (len < Epsilon) return v;

        len = One / len;
        return v *= len;
    }



    // 【メソッド】内積を返す
    static double innerProduct(Vec2 a, Vec2 b)
    {
        return a.x * b.x + a.y * b.y;
    }



    // 【メソッド】内積を返す（aとスクリーンx）
    static double innerProduct(Vec2 a)
    {
        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        // return a.x * 1.0 + a.y * 0.0;
        return a.x;
    }



    // 【メソッド】外積を返す
    static double outerProduct(Vec2 a, Vec2 b)
    {
        return a.x * b.y - b.x * a.y;
    }



    // 【メソッド】外積を返す（aとスクリーンx）
    static double outerProduct(Vec2 a)
    {
        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        // return a.x * 0.0 - 1.0 * a.y;
        // return 0.0 - a.y;
        return -a.y;
    }



    // 【メソッド】ベクトルの向きを返す（スクリーン座標系。atan2の代わりに使えて高速）
    // ＜戻り値＞ -180°から180°のradian
    double direction(double vx, double vy)
    {
        double len = sqrt(vx * vx + vy * vy);
        if (len < Epsilon) return 0.0;

        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        double cosVal = vx / len;  // 「内積/長さ」で余弦値を求める
        return (vy < 0.0) ? -acos(cosVal) : acos(cosVal);  // 外積を見て、360度角を得る
    }

    double direction(Vec2 v)
    {
        return direction(v.x, v.y);
    }



    // 【メソッド】2ベクトル間の角度を返す
    // ＜戻り値＞ -180°から180°のradian
    double angle(Vec2 a, Vec2 b)
    {
        return fmod(direction(b) - direction(a), TwoPi);
    }



    // 【メソッド】ベクトルを回転して返す
    static Vec2 rotation(Vec2 v, double sinVal, double cosVal)
    {
        return { v.x * cosVal - v.y * sinVal,
                 v.x * sinVal + v.y * cosVal };
    }

    Vec2 rotation(Vec2 v, double radian)
    {
        return rotation(v, sin(radian), cos(radian));
    }



    // 【メソッド】反射角を返す
    // ＜引数＞incidenceRadは入射角、reflectionAxisRadは壁となる軸の角度
    static double reflection(double incidenceRad, double reflectionAxisRad)
    {
        // 式。「壁となる軸の角度 * 2」から入射角を引く
        return fmod(reflectionAxisRad * Two - incidenceRad, TwoPi);
    }



    // 【メソッド】「割る数」を「掛ける数」に変換
    static double convDiv2Mul(double divVal)
    {
        return 1.0 / divVal;
    }



    // 【メソッド】度数をラジアンに変換
    // 0～2πの範囲に整形する。通常は degree * MyMath::Deg2Rad でよい
    static double convRadian(double degree)
    {
        if (degree < 0.0) {
            degree = fmod(degree, 360.0) + 360.0;
            if (degree == 360.0) degree = 0.0;
        }
        else if (degree >= 360.0)
            degree = fmod(degree, 360.0);

        return degree * Deg2Rad;
    }



    // 【メソッド】度数の角度範囲をラジアンに変換
    // 0～2πの範囲に整形する
    static double convRadianRange(double degreeRange)
    {
        if (degreeRange <   0.0) degreeRange = 0.0;
        if (degreeRange > 360.0) degreeRange = 360.0;

        return degreeRange * Deg2Rad;
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【クラス内クラス】直角三角形の要素
    // このクラスは、実用性よりもコードのサンプルとして使うことを想定
    //
    class RightTriangle
    {
    public:
        // 【メソッド】底辺の長さを返す（直角三角形と内積）
        // 斜辺ab、底辺bc（どんな長さでもよい）を指定する。
        // 斜辺はそのままで直角三角形を定義。その底辺の長さを返す
        static double baseLen(Vec2 a, Vec2 b, Vec2 c)
        {
            // ふつうの三角形を定義
            Vec2 abV(a - b);             // 斜辺ベクトル
            Vec2 bcV(c - b);             // 底辺ベクトル
            double bcLen = length(bcV);  // 底辺の長さ（まだ直角三角形にしたときの底辺は不明）
            if (bcLen < Epsilon) return 0.0;

            // 直角三角形の底辺長 = abとbcの内積を、bc長で割る。
            // これは、斜辺を線分bcに正投影したときの「影の長さ」に相当。
            // もし、影が逆方向（線分始点より手前。鈍角）なら負の数になる
            return innerProduct(abV, bcV) / bcLen;
        }



        // 【メソッド】高さを返す（直角三角形と外積）
        static double height()
        {

        }

    } rightTriangle;





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【クラス内クラス】点と線分の関係
    // このクラスは、実用性よりもコードのサンプルとして使うことを想定
    //
    class PointToLine
    {
    public:
        // 【メソッド】最短距離を返す（点と線分）
        // まず、点⇔直線を結ぶ垂線を考える。
        // 交点が線分上にあるなら、垂線の長さが「最短距離」となる。
        // 線分上に無いなら、近いほうの線分端までが「最短距離」となる。
        // また、戻り値が、ある半径以下かどうかを見て「円と線分の衝突判定」に利用できる
        static double distance(Vec2 point, Line line)
        {
            Vec2   lineV(line.endPos - line.startPos);
            double lineLen = length(lineV);
            if (lineLen < Epsilon) return 0.0;

            // 点⇔線分始点を結ぶ辺が「鈍角」なら、始点が最も近い
            if (innerProduct(point - line.startPos, lineV) < 0.0)
                return MyMath::distance(point, line.startPos);

            // 点⇔線分終点を結ぶ辺が「鋭角」なら、終点が最も近い
            if (innerProduct(point - line.endPos, lineV) >= 0.0)
                return MyMath::distance(point, line.endPos);

            // 上記以外（交点が線分上にある）なら、垂線の長さが最短距離
            return std::abs(outerProduct(point - line.startPos, lineV)) / lineLen;
        }



        // 【メソッド】垂線の交点を返す（点と線分）
        // まず、点⇔直線を結ぶ垂線を考える。
        // 交点が線分始点より手前（鈍角）なら負の数。それ以外なら正の数を返す
        static Vec2 crosspoint(Vec2 point, Line line)
        {
            Vec2   lineV(line.endPos - line.startPos);
            double lineLen = length(lineV);
            if (lineLen < Epsilon) return Vec2(0.0, 0.0);

            // 線分始点 + (lineベクトル * lineベクトルの割合) <-- 影の長さに相当
            return line.startPos + lineV * innerProduct(point - line.startPos, lineV) / lineLen;
        }

    } pointToLine;





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【クラス内クラス】衝突判定
    //
    class Hit
    {
    public:
        // 【メソッド】交差判定。線分と線分
        // ＜引数＞
        // posA --- 線分1の始点
        // posB --- 線分1の終点
        // posC --- 線分2の始点
        // posD --- 線分2の終点
        static bool lineOnLine(Vec2 posA, Vec2 posB, Vec2 posC, Vec2 posD)
        {
            Vec2 vecAB(posB - posA);
            Vec2 vecCD(posD - posC);
            Vec2 vecAC(posC - posA);
            Vec2 vecAD(posD - posA);
            Vec2 vecCA(posA - posC);
            Vec2 vecCB(posB - posC);
            
            return (outerProduct(vecAB, vecAC) * outerProduct(vecAB, vecAD) < 0.0) &&
                   (outerProduct(vecCD, vecCA) * outerProduct(vecCD, vecCB) < 0.0);
        }

        static bool lineOnLine(Line lineA, Line lineB)
        {
            return lineOnLine(lineA.startPos, lineA.endPos, lineB.startPos, lineB.endPos);
        }



        // 【メソッド】交差判定。線分とスクリーン横軸
        // ＜引数＞
        // lineStartY  --- 線分の始点y
        // lineEndY    --- 線分の終点y
        // horizontalY --- 横軸のy座標
        static bool lineOnHorizontal(double lineStartY, double lineEndY, double horizontalY)
        {
            double a = horizontalY - lineStartY;
            double b = horizontalY - lineEndY;
            return a * b < 0.0;
        }



        // 【メソッド】交差判定。線分とスクリーン縦軸
        // ＜引数＞
        // lineStartX --- 線分の始点x
        // lineEndX   --- 線分の終点x
        // verticalX  --- 縦軸のx座標
        static bool lineOnVertical(double lineStartX, double lineEndX, double verticalX)
        {
            double a = verticalX - lineStartX;
            double b = verticalX - lineEndX;
            return a * b < 0.0;
        }



        // 【メソッド】内包判定。点と矩形
        // ＜引数＞
        // point --- 点の座標
        // boxLeft, boxTop, boxRight, boxBottom --- 矩形の座標
        static bool pointInBox(Vec2 point, double boxLeft, double boxTop, double boxRight, double boxBottom)
        {
            return (point.x >= boxLeft) && (point.y >= boxTop) &&
                   (point.x < boxRight) && (point.y < boxBottom);
        }

        static bool pointInBox(Vec2 point, Rect box)
        {
            return pointInBox(point, box.left, box.top, box.right, box.bottom);
        }



        // 【メソッド】内包判定。点と多角形（すべての辺の内側かどうか）
        // ＜引数＞
        // point    --- 点の座標
        // vertices --- 多角形を構成する頂点。vector<Vec2>
        // ＜補足＞
        // 正しい結果を得るには、頂点が右回り（左回りなら結果は逆）、閉じた図形、全ての内角は180°以下であること。
        // 上記の条件を満たさない場合は、エラーにならず不定な動作となる
        static bool pointInPolygon(Vec2 point, const std::vector<Vec2>& vertices)
        {
            // 頂点nと頂点n+1を結ぶ辺から見て、点が「左側」にあった時点で判定をやめる
            for (int i = 0, edgeQty = vertices.size() - 1; i < edgeQty; ++i) {
                Line edge(vertices[i], vertices[i + 1]);
                if (outerProduct(edge.endPos - edge.startPos, point - edge.startPos) < 0.0)
                    return false;
            }
            return true;
        }

    } hit;





private:
    // 【構造体】テーブル用
    struct SinTable
    {
        static constexpr int Resolution  = 2000;
        static constexpr int TableMax    = static_cast<int>(Pi * Resolution);
        static constexpr int ScaledTwoPi = static_cast<int>(TwoPi * Resolution);
        double table[TableMax];
    } Sin;

    struct AsinTable
    {
        static constexpr int Resolution  = 3000;
        static constexpr int TableMax    = Resolution;
        double table[TableMax];
    } Asin;



    // 【隠しメソッド】
    // 隠しコンストラクタ
    MyMath()
    {
        // sinテーブルを作成（cos兼用）
        double n;
        for (int i = 0; i < Sin.TableMax; ++i) {
            n = std::sin(static_cast<double>(i) / Sin.Resolution);
            Sin.table[i] = (n < Epsilon) ? 0.0 : n;
        }
        
        // asinテーブルを作成（acos兼用）
        double max = Asin.TableMax - 1;
        for (int i = 0; i < Asin.TableMax; ++i) {
            n = std::asin(sqrt(i / max));
            Asin.table[i] = (n < Epsilon) ? 0.0 : n;
        }
    }

    ~MyMath(){}                        // 隠しデストラクタ
    MyMath(const MyMath&);             // 隠しコピーコンストラクタ
    MyMath& operator=(const MyMath&);  // 隠しコピー代入演算子
};
