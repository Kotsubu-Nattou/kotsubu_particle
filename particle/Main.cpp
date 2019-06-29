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
    
    Particle2D::Circle test;

    
    while (System::Update()) {
        if (MouseL.down()) {
            //// 猫のパーティクルを発生させる
            //neko.pos(Cursor::Pos()).speed(3).accelSpeed(0).size(20).accelSize(3).random(5);
            //neko.create(3);
        }

        if (MouseL.pressed()) {
            //// 点のパーティクルを発生させる
            //dot.pos(Cursor::Pos()).speed(2.5).accelSpeed(-0.1).random(4);
            //dot.color(ColorF(1.0, 0.6, 0.8, 0.8)).accelColor(ColorF(-0.005, -0.002, -0.02, -0.001));
            //dot.create(100);

            //// 煙のパーティクルを発生させる
            //smoke.pos(Cursor::Pos()).speed(1).size(5).accelSize(1).random(5);
            //smoke.gravity(0.05).gravityAngle(270);
            //smoke.color(ColorF(0.6, 0.6, 0.6, 0.2)).accelColor(ColorF(-0.008, -0.008, -0.008, -0.002));
            //smoke.blendState(BlendState::Default);
            //smoke.create(5);

            // テスト
            test.pos(Cursor::Pos()).size(5).speed(4).accelSpeed(-0.1).random(10);
            test.create(50);
        }

        // 同じ種類であれば、1つのインスタンスでいつでも追加できる
        // 【メモ】パラメータの種類と、設定が反映するタイミング
        //   位置、色、速度など  --- 粒子単体のパラメータ。最後に指定したものがcreate時に反映
        //   各加減値、引力など  --- 粒子全体のパラメータ。最後に指定したものがupdate時に反映（別の設定にしたい場合はインスタンスを分ける）
        //dot.pos(Window::Center() + Point(200, -150)).speed(1).color(ColorF(0.0, 0.4, 1.0, 1.0));
        //dot.create(3);

        // 障害物を登録
        //Vec2 line1start(270, 360), line1end(540, 240);  // 障害物（線分）その1
        //Vec2 line2start(120, 480), line2end(720, 540);  // 障害物（線分）その2
        //test.registObstacleLine(line1start, line1end);
        //test.registObstacleLine(line2start, line2end);
        //double left = 180, top = 180, right = 300, bottom = 300;
        //test.registObstacleRect(left, top, right, bottom);
        //Vec2 pos(600, 400);
        //double radius = 150;
        //test.registObstacleCircle(pos, radius);
        std::vector<Vec2> vertices = { {200, 150}, {100, 300}, {250, 500}, {500, 400}, {550, 250} };
        test.registObstaclePolygon(vertices);

        // パーティクルをアップデート（移動や色の経過処理を行う）
        //dot.update(System::DeltaTime());
        //smoke.update(System::DeltaTime());
        //neko.update(System::DeltaTime());
        test.update(System::DeltaTime());

        // 背景とパーティクルをドロー
        //Rect(Window::Center() + Point(-200, -200), 250).draw(Palette::Brown);
        //Circle(Window::Center() + Point(70, 50), 150).draw(Palette::Greenyellow);
        //Circle(Window::Center() + Point(200, -150), 30).drawFrame(5.0, Palette::Blueviolet);
        //smoke.draw();  // ドローする順番で印象が変わる
        //neko.draw();
        //dot.draw();
        test.draw();
    }
}
