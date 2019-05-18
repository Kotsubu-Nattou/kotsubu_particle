#include "Particle.h"



void Main()
{
    // パーティクルのインスタンスを生成
    Particle2D::DotBlended dot;
    Particle2D::Texture    neko;
    //neko.setTexture(s3d::Texture(Emoji(U"🐈"), TextureDesc::Mipped));


    while (System::Update()) {
        //if (MouseL.down()) {
        //    // 猫のパーティクルを発生
        //    neko.pos(Cursor::Pos()).speed(3).accelSpeed(0).size(20).accelSize(4).random(5);
        //    neko.create(5);
        //}

        //if (MouseL.pressed()) {
        //    // 点のパーティクルを発生
        //    dot.pos(Cursor::Pos()).speed(2.5).accelSpeed(-0.1).random(4);
        //    dot.color(ColorF(1.0, 0.6, 0.8, 0.8)).accelColor(ColorF(-0.03, 0.0, -0.01, -0.001));
        //    dot.walls(true, true, true, true);
        //    dot.create(200);

        //}

        if (MouseL.pressed()) {
            // 点のパーティクルを発生
            dot.pos(Cursor::Pos()).speed(5).accelSpeed(-0.05).random(5);
            dot.gravity(0);
            dot.angle(270).angleRange(0);
            dot.create(100);

        }

        // パーティクルをアップデート
        dot.update();
        neko.update();

        // 背景とパーティクルをドロー
        Circle(Window::Center(), 100).draw(Palette::Cyan);
        neko.draw();
        dot.draw();
    }
}
