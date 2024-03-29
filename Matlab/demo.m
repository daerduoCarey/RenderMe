%{

This code is to render a Mesh given a 3x4 camera matrix with an image resolution widthxheight. The rendering result is an ID map for facets, edges and vertices. This can usually used for occlusion testing in texture mapping a model from an image, such as the texture mapping in the following two papers.

--Jianxiong Xiao http://mit.edu/jxiao/

Citation:

[1] J. Xiao, T. Fang, P. Zhao, M. Lhuillier, and L. Quan
Image-based Street-side City Modeling
ACM Transaction on Graphics (TOG), Volume 28, Number 5
Proceedings of ACM SIGGRAPH Asia 2009

[2] J. Xiao, T. Fang, P. Tan, P. Zhao, E. Ofek, and L. Quan
Image-based Facade Modeling
ACM Transaction on Graphics (TOG), Volume 27, Number 5
Proceedings of ACM SIGGRAPH Asia 2008

%}


clear
clc
close all

compile

%load([num2str(i), '.mat']);
load('tmp.mat');

result = RenderMex(obj.view, obj.proj, obj.img_width, obj.img_height, obj.vertex, obj.face, obj.lp0, obj.lp1, obj.lp2)';
result = reshape(result, [3, obj.img_width, obj.img_height]);
result = permute(result, [3, 2, 1]);

max(max(max(result)))

imwrite(result, 'res.png');
