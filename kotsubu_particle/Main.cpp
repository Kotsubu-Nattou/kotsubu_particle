/////////////////////////////////////////////////////////////////////////////////////
//
// パーティクルクラスの使用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include "kotsubu_particle.h"  // ヘッダをインクルードするだけで使用可能



void Main()
{
    // パーティクルのインスタンスを生成
    //KotsubuParticle::DotBlended  dot;
    KotsubuParticle::DotTailed  dot;
    KotsubuParticle::CircleSmoke smoke;
    KotsubuParticle::Texture     neko;
    neko.setTexture(s3d::Texture(Emoji(U"🐈"), TextureDesc::Mipped));  // テクスチャは先に設定する

    // 衝突判定用図形のデータを先に作っておく
    std::vector<Vec2> obstacleVtx = { {200, 420}, {550, 350}, {700, 550}, {120, 500} };
    Polygon obstaclePolygon(obstacleVtx.data(), obstacleVtx.size());


    while (System::Update()) {
        if (!MouseR.pressed()) {
            //if (MouseL.down()) {
            //    // 猫のパーティクルを発生させる
            //    neko.pos(Cursor::Pos()).speed(3).accelSpeed(0).size(20).accelSize(3).random(5);
            //    neko.create(3);
            //}

            if (MouseL.pressed()) {
                // 点のパーティクルを発生させる
                //dot.pos(Cursor::Pos()).speed(2.5).accelSpeed(-0.1).random(4);
                dot.pos(Cursor::Pos()).speed(3).accelSpeed(-0.1).random(5);
                dot.color(ColorF(1.0, 0.6, 0.8, 0.8)).accelColor(ColorF(-0.005, -0.002, -0.02, -0.001));
                //dot.create(100);
                dot.create(100);

                //// 煙のパーティクルを発生させる
                //smoke.pos(Cursor::Pos()).speed(1).size(5).accelSize(1).random(5);
                //smoke.gravity(0.05).gravityAngle(270);
                //smoke.color(ColorF(0.6, 0.6, 0.6, 0.2)).accelColor(ColorF(-0.008, -0.008, -0.008, -0.002));
                //smoke.blendState(BlendState::Default);
                //smoke.create(5);
            }

            //// 同じ種類であれば、1つのインスタンスでいつでも追加できる
            //// 【メモ】パラメータの種類と、設定が反映するタイミング
            ////   位置、色、速度など  --- 粒子単体のパラメータ。最後に指定したものがcreate時に反映
            ////   各加減値、引力など  --- 粒子全体のパラメータ。最後に指定したものがupdate時に反映（別の設定にしたい場合はインスタンスを分ける）
            //dot.pos(Window::Center() + Point(200, -150)).speed(1).color(ColorF(0.0, 0.4, 1.0, 1.0));
            //dot.create(3);

            // 障害物を登録
            dot.registObstaclePolygon(obstacleVtx);

            // パーティクルをアップデート（移動や色の経過処理を行う）
            dot.update();
            //smoke.update();
            //neko.update();
        }

        // 背景、障害物、パーティクルの順にドロー
        Rect(Window::Center() + Point(-200, -200), 250).draw(Palette::Darkblue);
        Circle(Window::Center() + Point(70, 50), 150).draw(Palette::Darkgreen);
        Circle(Window::Center() + Point(200, -150), 30).drawFrame(5.0, Palette::Blueviolet);
        obstaclePolygon.drawFrame(3.0, Palette::White);
        //smoke.draw();  // ドローする順番で印象が変わる
        //neko.draw();
        dot.draw();
    }
}




/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// 衝突判定の処理速度テスト用
//
void Main()
{
    Font font(24);
    Stopwatch timer;
    KotsubuParticle::Circle test;
    //std::vector<Vec2> vertices = { {200, 150}, {550, 250}, {500, 400}, {250, 500}, {100, 300} };
    std::vector<Vec2> vertices1 = { {250, 180}, {300, 200}, {350, 400}, {150, 500} };
    //std::vector<Vec2> vertices2 = { {350, 400}, {650, 350}, {150, 500} };
    std::vector<Vec2> vertices2 = { {650, 350}, {700, 550}, {150, 500}, {350, 400} };
    std::vector<Vec2> vertices3 = { {400, 180}, {580, 320}, {380, 280} };
    std::vector<Vec2> vertices4 = { {725, 50}, {750, 100}, {700, 100} };
    Polygon polygon1(vertices1.data(), vertices1.size());
    Polygon polygon2(vertices2.data(), vertices2.size());
    Polygon polygon3(vertices3.data(), vertices3.size());
    Polygon polygon4(vertices4.data(), vertices4.size());


    while (System::Update()) {
        test.pos(Vec2(430+Random(20), 310+Random(20))).size(3).speed(3).accelSpeed(-0.1).random(4).accelColor(ColorF(0, 0, 0, 0));
        test.create(50000);
        //if (MouseL.pressed()) {
        //    test.pos(Cursor::Pos()).size(3).speed(3).accelSpeed(-0.1).random(5).accelColor(ColorF(0, 0, 0, 0));
        //    test.create(1000);
        //}

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
        // 【テスト】
        test.registObstaclePolygon(vertices4);
        test.registObstaclePolygon(vertices4);
        test.registObstaclePolygon(vertices4);
        test.registObstaclePolygon(vertices4);
        test.registObstaclePolygon(vertices4);
        test.registObstaclePolygon(vertices1);
        test.registObstaclePolygon(vertices2);
        test.registObstaclePolygon(vertices3);
        test.registObstaclePolygon(vertices4);

        // パーティクルをアップデート（移動や色の経過処理を行う）
        timer.restart();
        test.update();
        timer.pause();

        // 背景とパーティクルをドロー
        polygon1.drawFrame();
        polygon2.drawFrame();
        polygon3.drawFrame();
        polygon4.drawFrame();
        font(U"update time(ms): ", timer.ms()).draw();
        if (MouseL.pressed()) test.draw();
        //test.draw();
    }
}
*/