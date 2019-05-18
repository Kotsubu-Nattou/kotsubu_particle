/////////////////////////////////////////////////////////////////////////////////////
//
// パーティクルクラスの使用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include "Particle.h"  // ヘッダをインクルードするだけで使用可能



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
            dot.color(ColorF(1.0, 0.6, 0.8, 0.8)).accelColor(ColorF(-0.03, 0.0, -0.01, -0.001));
            dot.walls(true, true, true, false);
            dot.create(100);

            // 煙のパーティクルを発生させる
            smoke.pos(Cursor::Pos()).speed(1).size(5).accelSize(1).random(5);
            smoke.gravity(0.05).gravityAngle(270);
            smoke.color(ColorF(0.6, 0.6, 0.6, 0.2)).accelColor(ColorF(-0.008, -0.008, -0.008, -0.002));
            smoke.blendState(BlendState::Default);
            smoke.create(5);
        }

        // パーティクルをアップデート（移動や色の推移などを行う）
        dot.update();
        smoke.update();
        neko.update();

        // 背景とパーティクルをドロー
        Rect(Window::Center().x - 200, Window::Center().y - 200, 250).draw(Palette::Brown);
        Circle(Window::Center() + Point(70, 50), 150).draw(Palette::Greenyellow);
        smoke.draw();  // ドローする順番で印象が変わる
        neko.draw();
        dot.draw();
    }
}
