#include <iostream>
#include <fstream>
#include <string>
#define GLEW_STATIC
#include <gl\glew.h>
#include <GLFW\glfw3.h>
#include "ck.cuh"
#include <cuda_gl_interop.h>

GLuint compileAndCheckShader(GLenum shader_type, const char *src)
{
	GLuint vshd;
	vshd = glCreateShader(shader_type);

	std::string str = "";
	std::string str_total = "";
	std::ifstream vshdfile;

	vshdfile.open(src);
	if (!vshdfile) {
		std::cout << "ERROR : Cannot open the file (" + std::string(src) + ")." << std::endl;
	}
	while (std::getline(vshdfile, str)) {
		str_total += str + '\n';
	}
	std::cout << str_total << std::endl;
	const char * vshdsrc = str_total.c_str();
	vshdfile.close();


	glShaderSource(vshd, 1, &vshdsrc, NULL);
	glCompileShader(vshd);

	GLint compileFlg;
	glGetShaderiv(vshd, GL_COMPILE_STATUS, &compileFlg);
	if (compileFlg == GL_FALSE)
	{
		/*1 : Get info log size*/
		GLint infoLogSize;
		glGetShaderiv(vshd, GL_INFO_LOG_LENGTH, &infoLogSize);

		/*2 : Get the log information details and print*/
		if (infoLogSize > 0)
		{
			GLchar *log = (GLchar *)malloc(sizeof(GLchar)*infoLogSize);
			glGetShaderInfoLog(vshd, infoLogSize, NULL, log);
			std::cout << log << std::endl;
			if (log != NULL)free(log);
		}

	}

	return vshd;
}

void checkProgram(GLuint program)
{
	GLint linkFlg;
	glGetProgramiv(program, GL_LINK_STATUS, &linkFlg);
	if (linkFlg == GL_FALSE)
	{
		/*1 : Get info log size*/
		GLint infoLogSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);

		/*2 : Get the log information details and print*/
		if (infoLogSize > 0)
		{
			GLchar *log = (GLchar *)malloc(sizeof(GLchar)*infoLogSize);
			glGetProgramInfoLog(program, infoLogSize, NULL, log);
			std::cout << log << std::endl;
			if (log != NULL)free(log);
		}
	}
}

int main()
{
	if (!glfwInit()) {
		std::cerr << "Can't initialize GLFW." << std::endl;
		return 1;
	}

	int screenW = 1280;
	int screenH = 720;

	int tex0W = screenW;
	int tex0H = screenH;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *const window = glfwCreateWindow(screenW, screenH, "SAMPLE", NULL, NULL);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	GLfloat screenVertexPosition[3 * 4] =
	{
		-1.0f,-1.0f,0.0f,
		-1.0f, 1.0f,0.0f,
		1.0f,-1.0f,0.0f,
		1.0f, 1.0f,0.0f
	};
	GLfloat screenUVPosition[2 * 4] =
	{
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,1.0f
	};


	GLuint tex0;
	glGenTextures(1, &tex0);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex0W, tex0H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*サーフェスメモリ用cudaArrayの作成*/
	cudaArray_t cudatex;

	/*グラフィックリソースにOpenGLテクスチャをサーフェスとして登録*/
	cudaGraphicsResource_t resourceRef;
	cudaGraphicsGLRegisterImage(&resourceRef, tex0, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsSurfaceLoadStore);



	GLuint vao;
	GLuint vboPos, vboUV;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vboPos);
	glBindBuffer(GL_ARRAY_BUFFER, vboPos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, screenVertexPosition, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vboUV);
	glBindBuffer(GL_ARRAY_BUFFER, vboUV);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4, screenUVPosition, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);



	GLuint shdprogram;
	shdprogram = glCreateProgram();

	GLuint vshd = compileAndCheckShader(GL_VERTEX_SHADER, "C:\\Users\\mayasashi\\Documents\\Visual Studio 2015\\Projects\\Win32Project3x\\Win32Project3\\vshd.glsl");
	GLuint fshd = compileAndCheckShader(GL_FRAGMENT_SHADER, "C:\\Users\\mayasashi\\Documents\\Visual Studio 2015\\Projects\\Win32Project3x\\Win32Project3\\fshd.glsl");

	glAttachShader(shdprogram, vshd);
	glAttachShader(shdprogram, fshd);
	glBindAttribLocation(shdprogram, 0, "pos");
	glBindAttribLocation(shdprogram, 1, "uv");

	glLinkProgram(shdprogram);
	checkProgram(shdprogram);



	glClearColor(1.0f, 0.4f, 1.0f, 1.0f);

	int count = 0;

	do
	{
		count++;
		printf("%d\n", count);

		/*マッピング処理*/
		cudaGraphicsMapResources(1, &resourceRef, 0);

		/*cudaArrayの取得*/
		cudaGraphicsSubResourceGetMappedArray(&cudatex, resourceRef, 0, 0);

		/*取得したcudaArrayオブジェクトを用いてkernel関数を呼び出す (OpenGLテクスチャの更新)*/
		dim3 tpb;
		tpb.x = 16;
		tpb.y = 9;
		dim3 tb;
		tb.x = tex0W / tpb.x;
		tb.y = tex0H / tpb.y;

		cudaKernelReadWriteSurface(tpb, tb, cudatex, tex0W, tex0H);  //Kernel

																	 /*マッピングを解除する*/
		cudaGraphicsUnmapResources(1, &resourceRef, 0);


		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(vao);
		glUseProgram(shdprogram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		glProgramUniform1i(shdprogram, glGetUniformLocation(shdprogram, "tex"), 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwWindowShouldClose(window) == GL_FALSE);

	/*グラフィックリソースの登録解除*/
	cudaGraphicsUnregisterResource(resourceRef);

	glDeleteTextures(1, &tex0);
	glDeleteBuffers(1, &vboPos);
	glDeleteBuffers(1, &vboUV);
	glDeleteVertexArrays(1, &vao);
	glDeleteShader(vshd);
	glDeleteShader(fshd);
	glDeleteProgram(shdprogram);

	glfwTerminate();

	cudaDeviceReset();
	return 0;
}