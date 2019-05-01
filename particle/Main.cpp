﻿#include "Particle.h"



void Main()
{
    Particle2D::DotBlended particle;

    while (System::Update()) {
        if (MouseL.pressed()) {
            int x = Cursor::Pos().x;
            int y = Cursor::Pos().y;

            particle.pos(Vec2(x, y)).speed(4);
            particle.angle(0).angleRange(360);
            particle.color(ColorF(1.0, 0.85, 0.5, 0.8));
            //particle.accelColor(ColorF(0.0, -0.02, -0.03, 0.05));
            //particle.accelSize(0.1);
            //particle.layer(2).size(10);
            particle.create(1000);
        }
        
        Circle(Window::Center(), 100).draw(Palette::Cyan);
        
        particle.update();
        particle.draw();
    }
}
