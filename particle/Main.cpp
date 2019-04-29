#include "Particle.h"



void Main()
{
    Particle2D::Circle particle;
    
    while (System::Update()) {
        if (MouseL.pressed()) {
            int x = Cursor::Pos().x;
            int y = Cursor::Pos().y;

            particle.pos(Vec2(x, y)).speed(5);
            particle.angle(0).angleRange(360);
            particle.color(ColorF(1.0, 0.85, 0.5, 0.7));
            particle.create(100);
        }
        
        Circle(Window::Center(), 100).draw(Palette::Cyan);

        particle.update();
        particle.draw();
    }
}
