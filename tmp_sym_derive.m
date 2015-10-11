clear
clc

syms P00 P01 P02 P03 P10 P11 P12 P13 P20 P21 P22 P23
syms m_far m_near m_width m_height scale

projection = [P00 P01 P02 P03; P10 P11 P12 P13; P20 P21 P22 P23; 0 0 0 1];

% handle the near and far clip plane in OpenGL
protr = [1.0, 0.0, 0.0, 0.0;
    0.0, 1.0, 0.0, 0.0;
    0.0, 0.0, m_far / (m_far - m_near), - m_near * m_far /(m_far - m_near);
    0.0, 0.0, 1.0, 0.0];

% handle half pixel inconsistency
offset = [1.0, 0.0, 0.5, 0.0;
    0.0, 1.0, 0.5, 0.0;
    0.0, 0.0, 1.0, 0.0;
    0.0, 0.0, 0.0, 1.0];

% undo image aspect ratio and size
m0 = [m_width / 2, 0.0,         0.0, 0 + m_width / 2.0;
    0.0,        m_height / 2, 0.0, 0 + m_height / 2.0;
    0.0,        0.0,         0.5, 0.5;
    0.0,        0.0,         0.0, 1.0];


% handle scaling
m1=[1 / scale,   0.0,         0.0,                0.0;
    0.0,        1 / scale,    0.0,                0.0;
    0.0,        0.0,         1.0,                0.0;
    0.0,        0.0,         0.0,                1.0];

% handle upside down in vertical direction in image
m2=[1,           0.0,         0.0,                0.0;
    0.0,        -1.0,        m_height,            0.0;
    0.0,        0.0,         1.0,                0.0;
    0.0,        0.0,         0.0,                1.0];

M = (m0 \ protr * m2 * m1 * offset * projection);