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



namespace Particle
{
    // �y���N���X�z�ӂ��̃p�[�e�B�N��
    // ���ׂẴp�[�e�B�N���̊�b�ƂȂ�N���X�B���̃p�[�e�B�N���͂�����g���i�p���j��������
    class Plain
    {
    protected:
        // �N���X�����Ŏg�p����\����
        struct Element
        {
            Element() :
                pos(Vec2(0, 0)), size(1.0), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
            {}
            Element(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
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


        struct ElementProperty : public Element
        {
            ElementProperty() :
                randPow(5.0), accelSpeed(-0.1), accelSize(-0.1),
                accelColor(0.0, -0.02, -0.03, -0.005),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                radianRange(TwoPi), blendState(s3d::BlendState::Additive)
            {}
            double     randPow;
            double     accelSpeed;
            double     accelSize;
            ColorF     accelColor;
            double     gravityPow;
            double     gravityRad;
            double     radianRange;
            BlendState blendState;
        };

        // �y�t�B�[���h�z
        ElementProperty property;
        std::vector<Element> elements;
    


    public:
        // �y�R���X�g���N�^�z
        Plain(size_t reserve = 3000)
        {
            elements.reserve(reserve);
        }
    

        // �y�Z�b�^�z�e�����p�����[�^�B���\�b�h�`�F�[������
        Plain& pos(  Vec2   pos)   { property.pos   = pos;   return *this; }
        Plain& size( double size)  { property.size  = size;  return *this; }
        Plain& speed(double speed) { property.speed = speed; return *this; }
        Plain& color(Color  color) { property.color = color; return *this; }

        Plain& angle(double angle)
        {
            if (angle < 0.0) {
                angle = fmod(angle, 360.0) + 360.0;
                if (angle == 360.0) angle = 0.0;
            }
            else if (angle >= 360.0)
                angle = fmod(angle, 360.0);
        
            property.radian = angle * PiDivStraight;
            return *this;
        }

        Plain& angleRange(double angleRange)
        {
            if (angleRange < 0.0) angleRange = 0.0;
            if (angleRange > 360.0) angleRange = 360.0;

            property.radianRange = angleRange * PiDivStraight;
            return *this;
        }


        // �y���\�b�h�z����
        void create(int quantity)
        {
            static double fix, size, rad, speed;

            for (int i = 0; i < quantity; ++i) {
                // �T�C�Y
                fix  = property.size * property.randPow / 30.0;
                size = property.size + Random(-fix, fix);
                if (size < 0.5) size = 0.5;

                // �p�x
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);

                // �X�s�[�h
                speed = property.speed + Random(-property.randPow, property.randPow);

                // �v�f��ǉ�
                elements.emplace_back(Element(property.pos, size, rad, speed, property.color));
            }
        }



        // �y���\�b�h�z�A�b�v�f�[�g
        void update()
        {
            double windowWidth  = Window::Width();
            double windowHeight = Window::Height();
            double gravitySin = sin(property.gravityRad);
            double gravityCos = cos(property.gravityRad);

            for (auto &r : elements) {
                // �F�̕ω�
                r.color += property.accelColor;  // ColorF���u+=�v�����ꍇ�A�Ώۂ�RGB�̂�
                r.color.a += property.accelColor.a;
                if (r.color.a < 0.01) {
                    r.enable = false;
                    continue;
                }

                // �T�C�Y�̕ω�
                r.size += property.accelSize;
                if (r.size < 0.1) {
                    r.enable = false;
                    continue;
                }

                // �ړ�
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // ����
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // ��ʊO���ǂ���
                if ((r.pos.x < 0.0) || (r.pos.x > windowWidth) ||
                    (r.pos.y < 0.0) || (r.pos.y > windowHeight)) {
                    r.enable = false;
                    continue;
                }

                // �X�s�[�h�̕ω�
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // �����ȗ��q���폜
            auto dustIt = std::remove_if(elements.begin(), elements.end(),
                [](Element &element) { return !element.enable; });
            elements.erase(dustIt, elements.end());

            Print << U"elements.size: " << elements.size();
        }


    
        // �y���\�b�h�z�h���[
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmp�������Ă���Ԃ����L���B�j�����Ɍ��ɖ߂�
        
            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).draw(r.color);
        }
    };





    // �y�h���N���X�z�����̃p�[�e�B�N���i�Ȃ߂炩�����d���j
    class Light : public Plain
    {
    public:
        // �y���\�b�h�z�h���[�i�I�[�o�[���C�h�j
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).drawShadow(Vec2(0, 0), 10.0, 2.0, r.color);
        }
    };





    // �y�h���N���X�z���̃p�[�e�B�N��
    class Smoke : public Plain
    {
    public:
        // �y���\�b�h�z�h���[�i�I�[�o�[���C�h�j
        void draw()
        {
            double fix;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < 5; ++i) {
                fix = i * 0.2;
                for (auto &r : elements)
                    s3d::Circle(r.pos, r.size - r.size * fix).draw(r.color);
            }
        }
    };
}
