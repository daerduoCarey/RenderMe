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

  float m_near = 1;
  float m_far = 100;
  int m_level = 0;
  
  int m_linewidth = 1;
  int m_pointsize = 1;

  double* view_mat = mxGetPr(prhs[0]); // 4x4 matrix
  double* proj_mat = mxGetPr(prhs[1]); // 4x4 matrix
  int m_width = (int)mxGetScalar(prhs[2]);
  int m_height = (int)mxGetScalar(prhs[3]);
  double*       vertex = mxGetPr(prhs[4]); // 3xn double vertices matrix
  unsigned int  num_vertex = mxGetN(prhs[4]);
  unsigned int* edge = (unsigned int*) mxGetData(prhs[5]); // 2xn uint32 edge matrix
  unsigned int  num_edge = mxGetN(prhs[5]);
  unsigned int* face = (unsigned int*) mxGetData(prhs[6]); // 3xn uint32 face matrix
  unsigned int  num_face = mxGetN(prhs[6]);

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
  glDisable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glPolygonMode(GL_FRONT, GL_FILL);
  glClearColor(0, 0, 0, 1);//m_clearColor[0], m_clearColor[1], m_clearColor[2], 1.0f); // this line seems useless
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_width, m_height);

  // matrix is ready. use it
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(proj_mat);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(view_mat);
  
  // Step 3: render the mesh with encoded color from their ID
  unsigned char colorBytes[3];
  unsigned int base_offset;
  
  // render face
  base_offset = 1;
  for (unsigned int i = 0; i < num_face; ++i) {
    uint2uchar(base_offset+i,colorBytes);
    glColor3ubv(colorBytes);
    glBegin(GL_POLYGON);
    glVertex3dv(vertex+3*(*face++));
    glVertex3dv(vertex+3*(*face++));
    glVertex3dv(vertex+3*(*face++));
   // glVertex3dv(vertex+3*(*face++));
    glEnd();
  }

  // render edge
  base_offset = 1+num_face;
  glLineWidth(m_linewidth);
  glBegin(GL_LINES);
  for (unsigned int i = 0; i < num_edge; ++i) {
    uint2uchar(base_offset+i,colorBytes);
    glColor3ubv(colorBytes);
    glVertex3dv(vertex+3*(*edge++));
    glVertex3dv(vertex+3*(*edge++));
  }
  glEnd();

  // render vertex
  base_offset = 1+num_face+num_edge;
  glPointSize(m_pointsize);
  glBegin(GL_POINTS);
  for (unsigned int i = 0; i < num_vertex; ++i) {
    uint2uchar(base_offset+i,colorBytes);
    glColor3ubv(colorBytes);
    glVertex3dv(vertex);
    vertex+=3;
  }
  glEnd();
  
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
