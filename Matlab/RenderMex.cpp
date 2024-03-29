/*

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

*/

#include "mex.h" 
#include <GL/osmesa.h>
#include <GL/glu.h>
#include <cstring>

#include "vector3.h"

void uint2uchar(unsigned int in, unsigned char* out){
  out[0] = (in & 0x00ff0000) >> 16;
  out[1] = (in & 0x0000ff00) >> 8;
  out[2] =  in & 0x000000ff;
}

unsigned int uchar2uint(unsigned char* in){
  unsigned int out = (((unsigned int)(in[2])) << 16) + (((unsigned int)(in[1])) << 8) + ((unsigned int)(in[0]));
  return out;
}

// Input: 
//     arg0: 3x4 Projection matrix, 
//     arg1: image width, 
//     arg2: image height, 
//     arg3: 3xn double vertices matrix, 
//     arg4: 2xn uint32 edge matrix, index from zero 
//     arg5: 4xn uint32 face matrix, index from zero
// Output: you will need to transpose the result in Matlab manually
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  mexPrintf("RenderMex_modified\n"); 

  int m_pointsize = 1;

  double* view_mat = mxGetPr(prhs[0]); // 4x4 matrix
  double* proj_mat = mxGetPr(prhs[1]); // 4x4 matrix
  int m_width = (int)mxGetScalar(prhs[2]);
  int m_height = (int)mxGetScalar(prhs[3]);
  double*       vertex = mxGetPr(prhs[4]); // 3xn double vertices matrix
  unsigned int  num_vertex = mxGetN(prhs[4]);
  unsigned int* face = (unsigned int*) mxGetData(prhs[5]); // 3xn uint32 face matrix
  unsigned int  num_face = mxGetN(prhs[5]);

  double* lp0 = mxGetPr(prhs[6]);	// position of light one
  double* lp1 = mxGetPr(prhs[7]);	// position of light two
  double* lp2 = mxGetPr(prhs[8]);	// position of light three

  // Step 1: setup off-screen mesa's binding 
  OSMesaContext ctx;
  ctx = OSMesaCreateContextExt(OSMESA_RGB, 32, 0, 0, NULL );
  unsigned char * pbuffer = new unsigned char [3 * m_width * m_height];
  // Bind the buffer to the context and make it current
  if (!OSMesaMakeCurrent(ctx, (void*)pbuffer, GL_UNSIGNED_BYTE, m_width, m_height)) {
    mexErrMsgTxt("OSMesaMakeCurrent failed!: ");
  }
  OSMesaPixelStore(OSMESA_Y_UP, 0);

  // Step 2: Setup basic OpenGL setting
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glPolygonMode(GL_FRONT, GL_FILL);
  glClearColor(1, 1, 1, 1);//m_clearColor[0], m_clearColor[1], m_clearColor[2], 1.0f); // this line seems useless
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_width, m_height);

  // Setup the lighting
  glEnable(GL_NORMALIZE);

  GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
  
  GLfloat lightColor[] = {0.5f, 0.5f, 0.5f, 1.0f};
  GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
  
  GLfloat lightPos0[] = {(float) *lp0, (float) *(lp0+1), (float) *(lp0+2), 1.0f};
  GLfloat lightPos1[] = {(float) *lp1, (float) *(lp1+1), (float) *(lp1+2), 1.0f};
  GLfloat lightPos2[] = {(float) *lp2, (float) *(lp2+1), (float) *(lp2+2), 1.0f};
  
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
  
  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lightColor);
  glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
  
  glEnable(GL_LIGHT2);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor);
  glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
  
  GLfloat faceColor[] = {0.722f, 0.494f, 0.216f, 1.0f};
  glMaterialf(GL_FRONT, GL_SHININESS, 20);
  glMaterialfv(GL_FRONT, GL_SPECULAR, faceColor);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, faceColor);
  
  glShadeModel(GL_FLAT);

  // matrix is ready. use it
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(proj_mat);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(view_mat);
  
  // Step 3: render the mesh with encoded color from their ID
  float faceNormal[3];
  unsigned int base_offset;
  double *v1, *v2, *v3;

  // render face
  base_offset = 1;
  for (unsigned int i = 0; i < num_face; ++i) {
    v1 = vertex + 3*(*face++);
    v2 = vertex + 3*(*face++);
    v3 = vertex + 3*(*face++);
    
    vector3 vec1 = vector3(v1);
    vector3 vec2 = vector3(v2);
    vector3 vec3 = vector3(v3);

    vec2.sub(vec1);
    vec3.sub(vec1);

    vec2.cross(vec3);
    vec2.normalize();

    glBegin(GL_TRIANGLES);

    glNormal3f(vec2.x, vec2.y, vec2.z);
    
    glVertex3dv(v1);
    glVertex3dv(v2);
    glVertex3dv(v3);
   
    glEnd();
  }

  glFinish(); // done rendering

  // Step 5: convert the result from color to interger array
  plhs[0] = mxCreateNumericMatrix(1, 3*m_width*m_height, mxUINT8_CLASS, mxREAL);
  unsigned char* result = (unsigned char*) mxGetData(plhs[0]);
  
  unsigned char* resultCur = result;
  unsigned char* resultEnd = result + 3 * m_width * m_height;
  unsigned char * pbufferCur = pbuffer;
  while(resultCur != resultEnd){
    *resultCur = *pbufferCur;
    ++pbufferCur;
    ++resultCur;
  }

  OSMesaDestroyContext(ctx);
  delete [] pbuffer;

} 
