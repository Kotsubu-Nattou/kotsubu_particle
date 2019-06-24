/////////////////////////////////////////////////////////////////////////////////////
//
// パーティクルクラスの使用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include "my_math.h"
#include "Particle.h"  // ヘッダをインクルードするだけで使用可能



// @@@ 接地判定
void HitTest(Particle2D::DotBlended& dot)
{
    MyMath &math = MyMath::getInstance();
    Vec2 posA(50, 150), posB(230, 120);

    // 地形（線分AB）の要素
    Vec2   vecAB    = posB - posA;
    Vec2   normalAB = math.normalize(vecAB);
    double radAB    = math.direction(vecAB);

    // 当たり判定
    for (auto& r : dot.elements) {
        if (math.isHit_lineLine(posA, posB, r.oldPos, r.pos))
            dot.reflectionElement(r, posA, normalAB, radAB);
    }
}





void Main()
{
    // パーティクルのインスタンスを生成
    Particle2D::DotBlended  dot;
    Particle2D::CircleSmoke smoke;
    Particle2D::Texture     neko;
    neko.setTexture(s3d::Texture(Emoji(U"🐈"), TextureDesc::Mipped));  // テクスチャは先に設定する


    while (System::Update()) {
        if (MouseL.down()) {
            // 猫のパーティクルを発生させる
            neko.pos(Cursor::Pos()).speed(3).accelSpeed(0).size(20).accelSize(3).random(5);
            neko.create(3);
        }

        if (MouseL.pressed()) {
            // 点のパーティクルを発生させる
            dot.pos(Cursor::Pos()).speed(2.5).accelSpeed(-0.1).random(4);
            dot.color(ColorF(1.0, 0.6, 0.8, 0.8)).accelColor(ColorF(-0.005, -0.002, -0.02, -0.001));
            dot.create(100);

            // 煙のパーティクルを発生させる
            smoke.pos(Cursor::Pos()).speed(1).size(5).accelSize(1).random(5);
            smoke.gravity(0.05).gravityAngle(270);
            smoke.color(ColorF(0.6, 0.6, 0.6, 0.2)).accelColor(ColorF(-0.008, -0.008, -0.008, -0.002));
            smoke.blendState(BlendState::Default);
            smoke.create(5);
        }

        // 同じ種類であれば、1つのインスタンスでいつでも追加できる
        // 【メモ】パラメータの種類と、設定が反映するタイミング
        //   位置、色、速度など  --- 粒子単体のパラメータ。最後に指定したものがcreate時に反映
        //   各加減値、引力など  --- 粒子全体のパラメータ。最後に指定したものがupdate時に反映（別の設定にしたい場合はインスタンスを分ける）
        dot.pos(Window::Center() + Point(200, -150)).speed(1).color(ColorF(0.0, 0.4, 1.0, 1.0));
        dot.create(3);


        // パーティクルをアップデート（移動や色の経過処理を行う）
        dot.update();
        smoke.update();
        neko.update();

        // 接地判定
        HitTest(dot);

        // 背景とパーティクルをドロー
        Rect(Window::Center() + Point(-200, -200), 250).draw(Palette::Brown);
        Circle(Window::Center() + Point(70, 50), 150).draw(Palette::Greenyellow);
        Circle(Window::Center() + Point(200, -150), 30).drawFrame(5.0, Palette::Blueviolet);
        smoke.draw();  // ドローする順番で印象が変わる
        neko.draw();
        dot.draw();
    }
}
