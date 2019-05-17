#include "Particle.h"



void Main()
{
    Particle2D::DotBlended dot;
    Particle2D::Texture    neko;
    neko.setTexture(s3d::Texture(Emoji(U"🐈"), TextureDesc::Mipped));


    while (System::Update()) {
        if (MouseL.down()) {
            // 猫のパーティクルを生成
            neko.pos(Cursor::Pos()).speed(3).accelSpeed(0).size(20).accelSize(4).random(5);
            neko.color(ColorF(1.0, 1.0, 1.0, 1.0)).accelColor(ColorF(0.0, 0.0, 0.0, -0.005));
            neko.create(5);
        }

        if (MouseL.pressed()) {
            // 点のパーティクルを生成
            dot.pos(Cursor::Pos()).speed(2.5).accelSpeed(-0.1).random(4);
            dot.walls(true, true, true, true);
            dot.create(200);

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
