/**************************************************************************************************
【ヘッダオンリークラス】my_math

・概要
  数学で使う定数とメソッドを集めたシングルトン。
  一部のメソッド、および全ての定数はstaticなので、インクルードだけで利用可能。
  解放は不要（アプリケーション終了時に自動）
  基本的にベクトルは、OpenSiv3DのVec2で運用することを前提とした。
  もし、クラスstruct_vecのVEC2を使う場合は、このファイル冒頭、または
  このファイルをインクルードする前に、"USE_STRUCT_VEC"をdefineしておく。

・使い方
  #include "my_math.h"
  n = MyMath::Pi;                        // 定数はインクルードするだけで利用可能
  MyMath &math = MyMath::getInstance();  // 唯一のインスタンスを取得。これで全てのメンバにアクセス可能
  n = math.direction(v);
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
class MyMath {

#ifdef USE_STRUCT_VEC
    using Vec2 = VEC2<double>;  // クラスstruct_vecのVEC2
#endif
    


public:
    // 【定数】数学用の定数
    static constexpr double Epsilon    = 0.00001;           // これ未満を0とする値
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
    double direction(Vec2 v)
    {
        return direction(v.x, v.y);
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



    // 【メソッド】2ベクトル間の角度を返す
    // ＜戻り値＞ -180°から180°のradian
    double angle(Vec2 a, Vec2 b)
    {
        return fmod(direction(b) - direction(a), TwoPi);
    }



    // 【メソッド】ベクトルを回転して返す
    Vec2 rotation(Vec2 v, double radian)
    {
        return rotation(v, sin(radian), cos(radian));
    }



    // 【メソッド】ベクトルを回転して返す
    static Vec2 rotation(Vec2 v, double sinVal, double cosVal)
    {
        return { v.x * cosVal - v.y * sinVal,
                 v.x * sinVal + v.y * cosVal };
    }



    // 【メソッド】反射角を返す
    // ＜引数＞incidenceRadは入射角、reflectionAxisRadは壁となる軸の角度
    static double reflection(double incidenceRad, double reflectionAxisRad)
    {
        // 式。「壁となる軸の角度 * 2」から入射角を引く
        return fmod(reflectionAxisRad * Two - incidenceRad, TwoPi);
    }



    // 【メソッド】当たり判定。線分と線分
    // ＜引数＞
    // posA --- 線分1の始点
    // posB --- 線分1の終点
    // posC --- 線分2の始点
    // posD --- 線分2の終点
    bool isHit_lineLine(Vec2 posA, Vec2 posB, Vec2 posC, Vec2 posD)
    {
        Vec2 vecAB = posB - posA;
        Vec2 vecCD = posD - posC;
        Vec2 vecAC = posC - posA;
        Vec2 vecAD = posD - posA;
        Vec2 vecCA = posA - posC;
        Vec2 vecCB = posB - posC;

        return (outerProduct(vecAB, vecAC) * outerProduct(vecAB, vecAD) < 0.0) &&
               (outerProduct(vecCD, vecCA) * outerProduct(vecCD, vecCB) < 0.0);
    }





private:
    // @@@ テーブル構造体
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



    // @@@ 隠しメソッド
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
