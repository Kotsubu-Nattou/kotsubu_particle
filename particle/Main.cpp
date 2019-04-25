#include "Particle.h"



void Main()
{
    Particle particle;

    while (System::Update()) {
        if (MouseL.pressed()) {
            int x = Cursor::Pos().x;
            int y = Cursor::Pos().y;

            particle.pos(Vec2(x, y)).size(7).speed(5);
            particle.angle(0).angleRange(360);
            particle.drawType(Particle::DrawType::CircleShadow);  // これによってクラスの内容を変えたい！
            particle.color(Color(255, 220, 100, 150)).create(10);
        }

        particle.update();
        particle.draw();
    }
}
