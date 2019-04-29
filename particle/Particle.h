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



namespace Particle2D
{
   /////////////////////////////////////////////////////////////////////////////////////
    // �y���N���X�z���ׂẴp�[�e�B�N���̌��ƂȂ�A�����Ŏg�p���郁���o�ȂǁB�P�Ɨ��p�s��
    //
    class InternalWorks
    {
    protected:
        InternalWorks()
        {}

        struct Element
        {
            Vec2   pos;
            double radian;
            double speed;
            ColorF color;
            double gravity;
            bool   enable;
            Element() :
                pos(Vec2(0, 0)), radian(0.0), speed(5.0),
                color(ColorF(1.0, 0.9, 0.6, 0.8)), gravity(0.0), enable(true)
            {}
            Element(Vec2 _pos, double _radian, double _speed, ColorF _color) :
                pos(_pos), radian(_radian), speed(_speed), color(_color),
                gravity(0.0), enable(true)
            {}
        };


        struct Property
        {
            double       randPow;
            double       radianRange;
            double       accelSpeed;
            ColorF       accelColor;
            double       gravityPow;
            double       gravityRad;
            BlendState   blendState;
            Property() :
                randPow(5.0), radianRange(TwoPi),
                accelSpeed(-0.1), accelColor(0.0, -0.02, -0.03, -0.001),
                gravityPow(0.2), gravityRad(Pi / 2.0),
                blendState(s3d::BlendState::Additive)
            {}
        };


        double fixSize(double size)
        {
            if (size < 1.0)size = 1.0;
            return size;
        }


        double fixSpeed(double speed)
        {
            if (speed < 0.0)speed = 0.0;
            return speed;
        }


        double fixGravityPower(double power)
        {
            if (power < 0.0) power = 0.0;
            return power;
        }


        double fixRandomPower(double power)
        {
            if (power < 0.0)power = 0.0;
            return power;
        }

        double convRadian(double degree)
        {
            if (degree < 0.0) {
                degree = fmod(degree, 360.0) + 360.0;
                if (degree == 360.0) degree = 0.0;
            }
            else if (degree >= 360.0)
                degree = fmod(degree, 360.0);

            return degree * PiDivStraight;
        }


        double convRadianRange(double degree)
        {
            if (degree < 0.0) degree = 0.0;
            if (degree > 360.0) degree = 360.0;

            return degree * PiDivStraight;
        }


        // �����ȗ��q���폜
        template<typename T>
        void cleanElements(T& elements)
        {

            auto dustIt = std::remove_if(elements.begin(), elements.end(),
                [](Element &element) { return !element.enable; });

            elements.erase(dustIt, elements.end());

            //Print << U"elements.size: " << elements.size();
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // �y���C���N���X�z�~�`�̃p�[�e�B�N��
    // �~�n�p�[�e�B�N���̌��ƂȂ�N���X�B���̉~�n�p�[�e�B�N���͂�����g���i�p���j��������
    //
    class Circle : public InternalWorks
    {
    protected:
        // �N���X�����Ŏg�p����\����
        struct CircleElement : public Element
        {
            double size;
            CircleElement() : size(5.0)
            {}
            CircleElement(Vec2 _pos, double _size, double _radian, double _speed, ColorF _color) :
                Element(_pos, _radian, _speed, _color), size(_size)
            {}
        };


        struct CircleProperty : public Property, public CircleElement
        {
            double  accelSize;
            CircleProperty() : accelSize(-0.01)
            {}
        };


        // �y�t�B�[���h�z
        CircleProperty property;
        std::vector<CircleElement> elements;



    public:
        // �y�R���X�g���N�^�z
        Circle(size_t reserve = 3000)
        {
            elements.reserve(reserve);
        }


        // �y�Z�b�^�z�e�����p�����[�^�B���\�b�h�`�F�[������
        Circle& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Circle& size(        double size)   { property.size        = fixSize(size);           return *this; }
        Circle& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Circle& color(       Color  color)  { property.color       = color;                   return *this; }
        Circle& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Circle& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Circle& accelSize(   double size)   { property.accelSize   = size;                    return *this; }
        Circle& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Circle& accelColor(  Color  color)  { property.accelColor  = color;                   return *this; }
        Circle& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Circle& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Circle& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Circle& blendState(s3d::BlendState state) { property.blendState = state; return *this; }


        // �y���\�b�h�z����
        void create(int quantity)
        {
            static double fix, size, rad, speed;
            
            for (int i = 0; i < quantity; ++i) {
                // �T�C�Y
                fix = property.size * property.randPow / 30.0;
                size = property.size + Random(-fix, fix);
                if (size < 0.5) size = 0.5;

                // �p�x
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);
                
                // �X�s�[�h
                speed = property.speed + Random(-property.randPow, property.randPow);

                // �v�f��ǉ�
                elements.emplace_back(CircleElement(property.pos, size, rad, speed, property.color));
            }
        }



        // �y���\�b�h�z�A�b�v�f�[�g
        void update()
        {
            int margin       = property.size + property.size * property.randPow / 30.0;
            int windowLeft   = -margin;
            int windowTop    = -margin;
            int windowRight  = Window::Width() + margin;
            int windowBottom = Window::Height() + margin;
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
                if ((r.pos.x < windowLeft) || (r.pos.x > windowRight) ||
                    (r.pos.y < windowTop)  || (r.pos.y > windowBottom)) {
                    r.enable = false;
                    continue;
                }

                // �X�s�[�h�̕ω�
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // �����ȗ��q���폜
            cleanElements(elements);
        }



        // �y���\�b�h�z�h���[
        void draw()
        {
            s3d::RenderStateBlock2D tmp(property.blendState);  // tmp�������Ă���Ԃ����L���B�j�����Ɍ��ɖ߂�

            for (auto &r : elements)
                s3d::Circle(r.pos, r.size).draw(r.color);
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // �yCircle���p���z�W�����̃p�[�e�B�N���i�Ȃ߂炩�����d���j
    //
    class CircleLight : public Circle
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





    /////////////////////////////////////////////////////////////////////////////////////
    // �yCircle���p���z���̃p�[�e�B�N��
    //
    class CircleSmoke : public Circle
    {
    private:
        // �y�ǉ��t�B�[���h�z
        int layerQuantity;

    public:
        // �y�R���X�g���N�^�z
        CircleSmoke() : layerQuantity(5)
        {}

        // �y�Z�b�^�z�����p�����[�^�B���\�b�h�`�F�[������
        CircleSmoke& layer(int quantity)
        { 
            if (quantity < 1) quantity = 1;
            if (quantity > 10) quantity = 10;
            layerQuantity = quantity;
            return *this;
        }

        // �y���\�b�h�z�h���[�i�I�[�o�[���C�h�j
        void draw()
        {
            double ratio;
            s3d::RenderStateBlock2D tmp(property.blendState);

            for (int i = 0; i < layerQuantity; ++i) {
                ratio = i / static_cast<double>(layerQuantity);
                for (auto &r : elements)
                    s3d::Circle(r.pos, r.size - r.size * ratio).draw(r.color);
            }
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // �y���C���N���X�z�_�̃p�[�e�B�N��
    // �_�n�p�[�e�B�N���̌��ƂȂ�N���X�B���̓_�n�p�[�e�B�N���͂�����g���i�p���j�������́B
    // �ł������̃p�[�e�B�N����`��ł���B�������A���̃N���X�̓p�[�e�B�N������
    // �u0�v�ł���ʑS�̂̃C���[�W�𕡐����`�悷�邽�߁A�Œᕉ�ׂ͍��߁B
    // scale���\�b�h�� 1.0�`8.0 ���w��\�B�����قǊg�傳��邪���ׂ��y���ł���B
    // �����̎d�g�݂́A�}�`�̕`�悪�d���A�u�����f�B���O�������Ȃ����߁u�_�n�v�ł̂ݍ̗p
    //
    class Dot : public InternalWorks
    {
    protected:
        // �N���X�����Ŏg�p����\����
        struct DotProperty : public Property, public Element
        {
            double         scale;
            SamplerState   samplerState;
            DynamicTexture tex;
            Image          img;
            Image          blankImg;
            DotProperty() : samplerState(s3d::SamplerState::ClampNearest)
            {}
        };


        // �y�t�B�[���h�z
        DotProperty property;
        std::vector<Element> elements;



    public:
        // �y�R���X�g���N�^�z
        Dot(size_t reserve = 10000)
        {
            elements.reserve(reserve);
            scale(3.0);
        }


        // �y�Z�b�^�z�e�����p�����[�^�B���\�b�h�`�F�[������
        Dot& pos(         Vec2   pos)    { property.pos         = pos;                     return *this; }
        Dot& speed(       double speed)  { property.speed       = fixSpeed(speed);         return *this; }
        Dot& color(       Color  color)  { property.color       = color;                   return *this; }
        Dot& angle(       double degree) { property.radian      = convRadian(degree);      return *this; }
        Dot& angleRange(  double degree) { property.radianRange = convRadianRange(degree); return *this; }
        Dot& accelSpeed(  double speed)  { property.accelSpeed  = speed;                   return *this; }
        Dot& accelColor(  Color  color)  { property.accelColor  = color;                   return *this; }
        Dot& gravity(     double power)  { property.gravityPow  = fixGravityPower(power);  return *this; }
        Dot& gravityAngle(double degree) { property.gravityRad  = convRadian(degree);      return *this; }
        Dot& random(      double power)  { property.randPow     = fixRandomPower(power);   return *this; }
        Dot& blendState(s3d::BlendState state) { property.blendState = state; return *this; }
        
        Dot& smooth(bool val)
        {
            property.samplerState = val ? s3d::SamplerState::Default2D :
                                          s3d::SamplerState::ClampNearest;
            return *this;
        }

        Dot& scale(double _scale)
        {
            static double oldScale = -1;

            if (_scale < 1.0) _scale = 1.0;
            if (_scale > 8.0) _scale = 8.0;

            if (_scale != oldScale) {
                property.scale = _scale;

                // �V�����T�C�Y�̃u�����N�C���[�W�����
                property.blankImg = s3d::Image(static_cast<int>(Window::Width()  / property.scale),
                                               static_cast<int>(Window::Height() / property.scale));

                // ���I�e�N�X�`���́u�����T�C�Y�v�̃C���[�W���������Ȃ��ƕ`�悳��Ȃ����߃��Z�b�g�B
                // �܂��A�e�N�X�`����C���[�W��release��clear�́A�A���ŌĂяo���ƃG���[����
                property.tex.release();

                oldScale = property.scale;
            }

            return *this;
        }


        // �y���\�b�h�z����
        void create(int quantity)
        {
            static double fix, rad, speed;
            Vec2 pos = property.pos / property.scale;

            for (int i = 0; i < quantity; ++i) {
                // �p�x
                fix = Random(property.radianRange) - property.radianRange / 2.0;
                rad = fmod(property.radian + fix + TwoPi, TwoPi);

                // �X�s�[�h
                speed = property.speed + Random(-property.randPow, property.randPow);

                // �v�f��ǉ�
                elements.emplace_back(Element(pos, rad, speed, property.color));
            }
        }



        // �y���\�b�h�z�A�b�v�f�[�g
        void update()
        {
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

                // �ړ�
                r.pos.x += cos(r.radian) * r.speed;
                r.pos.y += sin(r.radian) * r.speed;

                // ����
                r.gravity += property.gravityPow;
                r.pos.x += gravityCos * r.gravity;
                r.pos.y += gravitySin * r.gravity;

                // ��ʊO���ǂ����ipos�̓C���[�W�z��̓Y�����ɂȂ�̂ŁA���̃`�F�b�N�����˂�j
                if ((r.pos.x < 0.0) || (r.pos.x >= property.blankImg.width()) ||
                    (r.pos.y < 0.0) || (r.pos.y >= property.blankImg.height())) {
                    r.enable = false;
                    continue;
                }

                // �X�s�[�h�̕ω�
                r.speed += property.accelSpeed;
                if (r.speed < 0.0) r.speed = 0.0;
            }

            // �����ȗ��q���폜
            cleanElements(elements);
        }



        // �y���\�b�h�z�h���[
        void draw()
        {
            static ColorF src, dst;
            static Point pos;

            // �C���[�W���N���A�iclear�֐������邪�A���ŌĂяo���ƃG���[����j
            property.img = property.blankImg;

            // �C���[�W���쐬�i���q�̐����������Bpos���m����img[n]�͈͓̔��ł��邱�Ɓj
            for (auto &r : elements)
                property.img[r.pos.asPoint()].set(r.color);

            // ���I�e�N�X�`�����X�V
            property.tex.fill(property.img);

            // ���I�e�N�X�`�����h���[
            s3d::RenderStateBlock2D tmp(property.blendState, property.samplerState);
            property.tex.scaled(property.scale).draw();
        }
    };





    /////////////////////////////////////////////////////////////////////////////////////
    // �yDot���p���z�_�̃p�[�e�B�N���i���Z�����j
    //
    class DotBlended : public Dot
    {
    public:
        // �y���\�b�h�z�h���[�i�I�[�o�[���C�h�j
        void draw()
        {
            static ColorF src, dst;
            static Point pos;

            // �C���[�W���N���A�iclear�֐������邪�A���ŌĂяo���ƃG���[����j
            property.img = property.blankImg;

            // �C���[�W���쐬�i���q�̐����������Bpos���m����img[n]�͈͓̔��ł��邱�Ɓj
            for (auto &r : elements) {
                pos = r.pos.asPoint();  // Vec2�^��pos���APoint�^�ɕϊ�

                // ���݈ʒu�i���W�j�̐F�����߂�i���O�̉��Z�u�����f�B���O�j
                src = property.img[pos];
                dst.r = src.r + r.color.r * r.color.a;
                dst.g = src.g + r.color.g * r.color.a;
                dst.b = src.b + r.color.b * r.color.a;
                dst.a = src.a + r.color.a;  // �{���͈Ⴄ��������Ȃ������h�����悢�i�L���L������j

                // ���߂��F���Z�b�g
                property.img[pos].set(dst);
            }
            
            // ���I�e�N�X�`�����X�V
            property.tex.fill(property.img);

            // ���I�e�N�X�`�����h���[
            s3d::RenderStateBlock2D tmp(property.blendState, property.samplerState);
            property.tex.scaled(property.scale).draw();
        }
    };
}