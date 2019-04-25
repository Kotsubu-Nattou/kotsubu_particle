#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <Siv3D.hpp>

namespace
{
    const double Pi = 3.141592653589793;
    const double TwoPi = Pi * 2.0;            // Radian�̍ő�l
    const double PiDivStraight = Pi / 180.0;  // Deg�Ɋ|�����Rad
    const double StraightDivPi = 180.0 / Pi;  // Rad�Ɋ|�����Deg
}





class Particle
{
public:
    // �yenum�z�h���[�^�C�v
    enum DrawType
    {
        Circle,
        CircleShadow,
    };


private:
    // �y�t�B�[���h�z
    struct Obj
    {
        Obj() :
            pos(Vec2(0, 0)), size(1.0), radian(0.0), speed(5.0),
            color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
        {}
        Obj(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
            pos(_pos), size(_size), radian(_radian), speed(_speed), color(_color),
            gravity(0.0), enable(true)
        {}
        Vec2   pos;
        double size;
        double radian;
        double speed;
        ColorF color;
        double gravity;
        bool   enable;
    };
    
    struct Status : public Obj
    {
        Status() :
            randPow(5.0), accelSpeed(-0.1), accelSize(-0.1),
            accelColor(0.0, -0.02, -0.03, -0.005),
            gravityPow(0.2), gravityRad(Pi / 2.0),
            radianRange(TwoPi), blendState(s3d::BlendState::Additive),
            drawType(DrawType::Circle)
        {}
        double     randPow;
        double     accelSpeed;
        double     accelSize;
        ColorF     accelColor;
        double     gravityPow;
        double     gravityRad;
        double     radianRange;
        BlendState blendState;
        DrawType   drawType;
    };

    Status status;
    std::vector<Obj> obj;
    


public:
    // �y�R���X�g���N�^�z
    Particle(size_t reserve = 3000)
    {
        obj.reserve(reserve);
    }
    

    // �y�Z�b�^�z�e�����p�����[�^�B���\�b�h�`�F�[������
    Particle& pos(Vec2 pos)
    {
        status.pos = pos;
        return *this;
    }

    Particle& size(double size)
    {
        status.size = size;
        return *this;
    }

    Particle& angle(double angle)
    {
        if (angle < 0.0) {
            angle = fmod(angle, 360.0) + 360.0;
            if (angle == 360.0) angle = 0.0;
        }
        else if (angle >= 360.0)
            angle = fmod(angle, 360.0);
        
        status.radian = angle * PiDivStraight;
        return *this;
    }

    Particle& angleRange(double angleRange)
    {
        if (angleRange < 0.0) angleRange = 0.0;
        if (angleRange > 360.0) angleRange = 360.0;

        status.radianRange = angleRange * PiDivStraight;
        return *this;
    }

    Particle& speed(double speed)
    {
        status.speed = speed;
        return *this;
    }

    Particle& color(Color color)
    {
        status.color = color;
        return *this;
    }

    Particle& drawType(DrawType type)
    {
        status.drawType = type;
        return *this;
    }


    // �y���\�b�h�z����
    void create(int quantity)
    {
        static double fix, size, rad, speed;

        for (int i = 0; i < quantity; ++i) {
            // �T�C�Y
            fix  = status.size * status.randPow / 30.0;
            size = status.size + Random(-fix, fix);
            if (size < 0.5) size = 0.5;

            // �p�x
            fix = Random(status.radianRange) - status.radianRange / 2.0;
            rad = fmod(status.radian + fix + TwoPi, TwoPi);

            // �X�s�[�h
            speed = status.speed + Random(-status.randPow, status.randPow);

            // �v�f��ǉ�
            obj.emplace_back(Obj(status.pos, size, rad, speed, status.color));
        }
    }



    // �y���\�b�h�z�A�b�v�f�[�g
    void update()
    {
        double windowWidth  = Window::Width();
        double windowHeight = Window::Height();
        double gravitySin = sin(status.gravityRad);
        double gravityCos = cos(status.gravityRad);

        for (auto &r : obj) {
            // �F�̕ω�
            r.color += status.accelColor;  // ColorF���u+=�v�����ꍇ�A�Ώۂ�RGB�̂�
            r.color.a += status.accelColor.a;
            if (r.color.a <= 0.0) {
                r.enable = false;
                continue;
            }

            // �T�C�Y�̕ω�
            r.size += status.accelSize;
            if (r.size <= 0.0) {
                r.enable = false;
                continue;
            }

            // �ړ�
            r.pos.x += cos(r.radian) * r.speed;
            r.pos.y += sin(r.radian) * r.speed;

            // ����
            r.gravity += status.gravityPow;
            r.pos.x += gravityCos * r.gravity;
            r.pos.y += gravitySin * r.gravity;

            // ��ʊO���ǂ���
            if ((r.pos.x < 0.0) || (r.pos.x > windowWidth) ||
                (r.pos.y < 0.0) || (r.pos.y > windowHeight)) {
                r.enable = false;
                continue;
            }

            // �X�s�[�h�̕ω�
            r.speed += status.accelSpeed;
            if (r.speed < 0.0) r.speed = 0.0;
        }

        // �����ȗ��q���폜
        auto dustIt = std::remove_if(obj.begin(), obj.end(),
            [](Obj &obj) { return !obj.enable; });
        obj.erase(dustIt, obj.end());

        Print << U"obj.size: " << obj.size();
    }


    
    // �y���\�b�h�z�h���[
    void draw()
    {
        s3d::RenderStateBlock2D tmp(status.blendState);  // tmp�������Ă���Ԃ����ݒ肪�L���B�j�����ɐݒ�͌��ɖ߂�
        
        switch (status.drawType) {
        case DrawType::CircleShadow:
            for (auto &r : obj)
                s3d::Circle(r.pos, r.size).drawShadow(Vec2(0, 0), 10.0, 2.0, r.color);
            break;

        default:
            for (auto &r : obj)
                s3d::Circle(r.pos, r.size).draw(r.color);
            break;
        }
    }
};

