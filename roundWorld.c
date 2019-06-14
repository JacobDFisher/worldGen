#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint uniform_mode = 0;
static GLuint uniform_enhance = 0;
static double enhance = 1;
static double rotate = 0;

static GLdouble *drawnPositions;

struct len_doublebuf{
  int len;
  double * buf;
};

static int offset_y=0;
static int xDim = 600;
static int yDim = 600;
static struct len_doublebuf point_buf;
double * distances;

double distance(double * pt1, double * pt2);
double *generatePoints(int seed, int nPoints);
struct len_doublebuf genSpherePoints(int xDim, int yDim);
double *generateDistances(double *points, int nPoints, int nFeatures, int nClosest);
void key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods);
void display(GLuint *vao, int nPoints, GLFWwindow *window);
GLFWwindow *initialize(GLuint *vao, double *pos, double * dist, int nPoints, int nDist);
long getPos(double * points, int nPoints, double x, double y);

int main(int argc, char **argv){
  int i;
  int nPoints = 100000;
  int nFeatures = 1000;
  point_buf = genSpherePoints(xDim, yDim);
  nPoints = point_buf.len;
  double * points = point_buf.buf;
  printf("Number of points: %d\n", nPoints);
  //double *points = generatePoints(time(NULL), nPoints);
  srand(time(NULL));
  distances = generateDistances(points, nPoints, nFeatures, 3);
  GLuint vao;
  GLFWwindow *window;
  for(i=0; i<12; i++){
    printf("%f\n", distances[i]);
  }
  double maximum = 0;
  for(i=0; i<nPoints*3; i++){
    if(distances[i]>maximum)
      maximum = distances[i];
  }
  printf("\n%f\n", maximum);
  window = initialize(&vao, points, distances, nPoints, 3);
  if(window==NULL){
    free(points);
    free(distances);
    return -1;
  }
  while(!glfwWindowShouldClose(window)){
    display(&vao, nPoints, window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  free(points);
  free(distances);
}

double distance(double *pt1, double *pt2){
  int i;
  double prodSum = 0;
  for(i=0; i<3; i++){
    prodSum += pt1[i]*pt2[i];
  }
  return acos(prodSum);
}

struct len_doublebuf genSpherePoints(int xDim, int yDim){
  int i, j;
  int nPoints = 0;
  int maxPoints = xDim*yDim;
  double x,y,z;
  double * points = malloc(sizeof(double)*3*maxPoints);
  struct len_doublebuf retVal = {0, NULL};
  for(i=0; i<xDim; i++){
    for(j=0; j<yDim; j++){
      x=-1+i*2.0/xDim;
      y=-1+j*2.0/yDim;
      z=x*x+y*y;
      if(z<1){
	z = sqrt(1-z);
	//printf("%.02f\t%.02f\t%.02f\n", x, y, z);
	if(nPoints>maxPoints){
	  maxPoints *= 2;
	  double * buf = realloc(points, sizeof(double)*3*maxPoints);
	  if(buf)
	    points = buf;
	  else
	    return retVal;
	}
	points[nPoints*3] = x;
	points[nPoints*3+1] = y;
	points[nPoints*3+2] = z;
	nPoints += 1;
	if(nPoints>maxPoints){
	  maxPoints *= 2;
	  double * buf = realloc(points, sizeof(double)*3*maxPoints);
	  if(buf)
	    points = buf;
	  else
	    return retVal;
	}
	points[nPoints*3] = x;
	points[nPoints*3+1] = y;
	points[nPoints*3+2] = -z;
	nPoints += 1;
      } else if(z==1){
	//printf("%.02f\t%.02f\t%.02f\n", x, y, z);
	if(nPoints>maxPoints){
	  maxPoints *= 2;
	  double * buf = realloc(points, sizeof(double)*3*maxPoints);
	  if(buf)
	    points = buf;
	  else
	    return retVal;
	}
	points[nPoints*3] = x;
	points[nPoints*3+1] = y;
	points[nPoints*3+2] = 0;
	nPoints += 1;
      }
    }
  }
  double * buf = realloc(points, sizeof(double)*3*nPoints);
  if(!buf)
    return retVal;
  retVal.len = nPoints;
  retVal.buf = buf;
  return retVal;
}

long getPos(double * points, int nPoints, double x, double y){
  int min, max, dir;
  long pos;
  for(pos=0, min=0, max=nPoints; pos<(nPoints*3)&&
	((fabs(points[pos]-x)>(1.0/xDim))||(fabs(points[pos+1]-y)>(1.0/yDim)));
      pos+=3){
    //printf("Pos %d:\t%.02f\t%.02f\n", pos, points[pos], points[pos+1]);
  }
  if(pos>=nPoints*3)
    return -1;
  //printf("%.02f\t%.02f\t%.02f\n", points[pos], x, fabs(points[pos]-x));
  //printf("%d\t%d\t%d\n",pos>(nPoints*3),((fabs(points[pos]-x)<(1.0/xDim)),(fabs(points[pos+1]-y)<(1.0/yDim))));
  return pos;
}

double * generatePoints(int seed, int nPoints){
  int i, j, k, x, y, z;
  double denom, dist;
  double feature[3];
  srand(seed);
  double * points = malloc(sizeof(double)*3*nPoints);
  for(i=0; i<nPoints; i++){
    x = rand()%20001-10000;
    y = rand()%20001-10000;
    z = rand()%20001-10000;
    denom = sqrt(x*x+y*y+z*z);
    if(denom == 0){
      points[i*3] = 1;
      points[i*3+1] = 0;
      points[i*3+2] = 0;
    } else {
      points[i*3] = x/denom;
      points[i*3+1] = y/denom;
      points[i*3+2] = z/denom;
    }
  }
  return points;
}

double *generateDistances(double *points, int nPoints, int nFeatures, int nClosest){
  int i, j, k, x, y, z;
  double denom, dist;
  double feature[3];
  double * distances = malloc(sizeof(double)*nClosest*nPoints);
  for(i=0; i<nPoints*3; i++){
    distances[i] = 1;
  }
  for(j=0; j<nFeatures; j++){
    printf("\r%d/%d", j, nFeatures);
    fflush(stdout);
    x = rand()%20001-10000;
    y = rand()%20001-10000;
    z = rand()%20001-10000;
    denom = sqrt(x*x + y*y + z*z);
    if(denom == 0){
      feature[0] = 1;
      feature[1] = 0;
      feature[2] = 0;
    } else {
      feature[0] = x/denom;
      feature[1] = y/denom;
      feature[2] = z/denom;
    }
    for(i=0; i<nPoints; i++){
      dist = distance(points+(i*nClosest), feature)/M_PI;
      if(dist<distances[(i+1)*nClosest-1]){
	//printf("%f\n", dist);
	//points[(i+1)*nVals-1] = dist;
	for(k=1; k>=0; k--){
	  if(dist<distances[i*nClosest+k]){
	    distances[i*3+k+1]=distances[i*nClosest+k];
	  } else break;
	}
	distances[i*nClosest+k+1] = dist;
      }
    }
  }
  printf("\r%d/%d\n", j, nFeatures);
  return distances;
}

void key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(action!=GLFW_PRESS)
    return;
  if(key=='1'){
    glUniform1f(uniform_mode, 1);
  } else if (key == '2'){
    glUniform1f(uniform_mode, 2);
  } else if (key =='3'){
    glUniform1f(uniform_mode, 3);
  } else if (key =='4'){
    glUniform1f(uniform_mode, 4);
  }else if (key =='0'){
    glUniform1f(uniform_mode, 0);
  } else if (key == 'Q'){
    glfwSetWindowShouldClose(window, 1);
  } else if (key == 'N'){
    printf("%d\n", point_buf.len);
  } else if (key == 262){ //RIGHT
    printf("RIGHT\n");
    rotate += 5;
    if(rotate>360)
      rotate-=360;
  } else if (key == 263){ //LEFT
    printf("LEFT\n");
    rotate -= 5;
    if(rotate<0)
      rotate+=360;
  } else if (key == 264){ // DOWN
    enhance -= 0.1;
    glUniform1f(uniform_enhance, enhance);
  } else if (key == 265){ // UP
    enhance += 0.1;
    glUniform1f(uniform_enhance, enhance);
  }
}

void mouse_clicked(GLFWwindow* window, int button, int action, int mods){
  double mouse_x, mouse_y;
  if(action!=GLFW_PRESS)
    return;
  if(button==GLFW_MOUSE_BUTTON_LEFT){
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    printf("%.02f\t%.02f\n", mouse_x, mouse_y-offset_y);
    long pos = getPos(point_buf.buf, point_buf.len, -1+mouse_x*2.0/xDim, -1+(mouse_y-offset_y)*2.0/yDim);
    if(pos < 0){
      printf("Out of bounds.\n");
      return;
    }
    printf("Expected:\t%.02f\t%.02f\n", -1+mouse_x*2.0/xDim, -1+(mouse_y-offset_y)*2.0/yDim);
    printf("Coords at point %d:\t%.02f\t%.02f\t%.02f\n", pos, point_buf.buf[pos], point_buf.buf[pos+1], point_buf.buf[pos+2]); 
    printf("Heights at point %d:\t%.02f\t%.02f\t%.02f\n", pos, distances[pos], distances[pos+1], distances[pos+2]);
  }
}

void window_resized(GLFWwindow* window, int width, int height){
  offset_y = height-yDim;
}

char * read_shader_src(const char *fname){
  FILE * fp;
  int i, sz;
  fp = fopen(fname, "r");
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  rewind(fp);
  char * retVal = malloc((sz+1)*sizeof(char));
  if(retVal==NULL){
    fprintf(stderr, "Failed to malloc space for shader\n");
    exit(-1);
  }
  fread(retVal, sizeof(char), sz, fp);
  retVal[sz]='\0';
  return retVal;
}

GLuint load_and_compile_shader(const char *fname, GLenum shaderType){
  char * src = read_shader_src(fname);

  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, (const GLchar**) &src, NULL);
  glCompileShader(shader);

  GLint test;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
  if(!test){
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    char* anError = malloc(sizeof(char)*(logSize+1));
    glGetShaderInfoLog(shader, logSize, NULL, anError);
    anError[logSize]='\0';
    fprintf(stderr, "Shader compilation failed with message: %s\n", anError);
    free(anError);
    glfwTerminate();
    exit(-1);
  }
  free(src);
  return shader;
}

GLuint create_program(const char * path_vert_shader, const char * path_frag_shader){
  GLuint vertexShader = load_and_compile_shader(path_vert_shader, GL_VERTEX_SHADER);
  GLuint fragmentShader = load_and_compile_shader(path_frag_shader, GL_FRAGMENT_SHADER);
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  return shaderProgram;
}

void display(GLuint *vao, int nPoints, GLFWwindow *window){
  glClear(GL_COLOR_BUFFER_BIT);
  glBindVertexArray(*vao);
  glMatrixMode(GL_PROJECTION);
  glRotated(rotate, 0, 1, 0);
  glDrawArrays(GL_POINTS, 0, nPoints);
  glfwSwapBuffers(window);
}

GLFWwindow *initialize(GLuint *vao, double *pos, double * dist, int nPoints, int nDist){
  if(!glfwInit()){
    return NULL;
  }
  GLFWwindow * window = glfwCreateWindow(600, 600, "TEST", NULL, NULL);
  if(!window){
    fprintf(stderr, "Failed to open a window!\n");
    glfwTerminate();
    return NULL;
  }
  glfwSetKeyCallback(window, key_pressed);
  glfwSetMouseButtonCallback(window, mouse_clicked);
  glfwSetWindowSizeCallback(window, window_resized);
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err != GLEW_OK){
    fprintf(stderr, "GLEW failed to initialize with error: %s\n", glewGetErrorString(err));
    glfwTerminate();
    exit(-1);
  }
  glEnable(GL_POINT_SMOOTH);
  glGenVertexArrays(1, vao);
  glBindVertexArray(*vao);
  int i;
  GLdouble * drawnPositions = malloc(sizeof(GLdouble)*3*nPoints);
  for(i=0; i<nPoints; i++){
    drawnPositions[3*i] = pos[3*i];
    drawnPositions[3*i+1] = pos[3*i+1];
    drawnPositions[3*i+2] = 0;
  }
  GLfloat * colors = malloc(sizeof(GLfloat)*3*nPoints);
  for(i=0; i<nPoints; i++){
    colors[3*i] = (nDist>=1)?dist[3*i]:0;
    colors[3*i+1] = (nDist>=2)?dist[3*i+1]:0;
    colors[3*i+2] = (nDist>=3)?dist[3*i+2]:0;
  }
  GLuint vbo;
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble)*3*nPoints+sizeof(GLfloat)*3*nPoints, NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLdouble)*3*nPoints, drawnPositions);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLdouble)*3*nPoints, sizeof(GLfloat)*3*nPoints, colors);
  GLuint shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");
  GLint position_attribute = glGetAttribLocation(shaderProgram, "position");

  glVertexAttribPointer(position_attribute, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(position_attribute);

  GLint color_attribute = glGetAttribLocation(shaderProgram, "color");
  glVertexAttribPointer(color_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(GLdouble)*3*nPoints));
  glEnableVertexAttribArray(color_attribute);

  uniform_mode = glGetUniformLocation(shaderProgram, "mode");
  uniform_enhance = glGetUniformLocation(shaderProgram, "enhance");
  glUniform1f(uniform_mode, 0);
  glUniform1f(uniform_enhance, 1);
  free(drawnPositions);
  free(colors);
  return window;
}
