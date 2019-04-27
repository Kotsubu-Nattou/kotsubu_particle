#include "Particle.h"



void Main()
{
    Particle::Smoke particle;
    
    while (System::Update()) {
        if (MouseL.pressed()) {
            int x = Cursor::Pos().x;
            int y = Cursor::Pos().y;

            particle.pos(Vec2(x, y)).size(20).speed(3);
            particle.angle(0).angleRange(360);
            particle.color(ColorF(1.0, 0.85, 0.4, 0.1)).create(30);
        }
        
        particle.update();
        particle.draw();
    }
}
