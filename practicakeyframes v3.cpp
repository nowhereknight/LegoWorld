/*
Semestre 2020-2
Animaci�n por keyframes
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <conio.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
//para iluminaci�n
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"

#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"
#include"SpotLight.h"

#include <irrKlang.h>
#include "Sonido.h"
//#if defined(WIN32)
//#include <conio.h>
//#else
//#include "../common/conio.h"
//#endif

using namespace irrklang;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll


const float toRadians = 3.14159265f / 180.0f;
float movCoche;
float movOffset;
float reproduciranimacion, habilitaranimacion, guardoFrame, reinicioFrame, ciclo, ciclo2, contador = 0;
float rot; // para animacion de arboles
float movPerro;
glm::vec3 positionP;
int segundos = 0;
bool isDay;
//float rotapato;
bool avanza;// , patoadelante;
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

GLuint uniformColor = 0;


//Variables de texturizado
Texture brickTexture;
Texture dirtTexture;
Texture plainTexture; //textura en blanco
Texture pisoTexture;
Texture Tagave;
Texture Tladrillo;
Texture Tbasura;
Texture Tpasillo;
Texture Tarbol1;
Texture Tarbol2;
Texture Tarbol3;
Texture Tnaranja;
Texture Trosa;
Texture Tcafe;
Texture Tazul;
Texture Tnegro;
Texture Tverde;
Texture TlegoGreen;
Texture Ttransparente;
Texture Tamarillo;
Texture Tagua;
//Texture Trojo;
Texture Tarbol4;
Texture Tblanco;
Texture Tcolores;
Texture Tcolores3;
Texture Tvidrio;


//materiales
Material Material_brillante;
Material Material_opaco;

//luz direccional
DirectionalLight mainLight;

//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Model Kitt_M;//Carrito
Model Llanta_M;
Model Camino_M;
Model Blackhawk_M;//helicoptero
Model Ping_M; //Pinguino
Model wc_M;

Skybox skybox;

Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;//
int lightsOn = 1;

//void my_input(GLFWwindow *window);
//void inputKeyframes(bool* keys);
void inputkey(bool* keys);

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";
//c�lculo del promedio de las normales para sombreado de Phong 

double getLocVariation(int frames) {
	//120 va a ser 2Pi <=>360�
	double t = (frames *360) / 120;

	return sin(t*toRadians);
}
double getSunIntensity(int hours) {

	//sacamos la hora del dia asumiendo que cada 24 segundos es un d�a
	double actualHour = hours % 24;
	//printf("Actual hour: %f\n", actualHour);
	//Lo limitamos al intervalo -0.5:0.5 para aplicar la distribuci�n normal y el valor m�ximo sea 1
	double x = actualHour / 24 - 0.5;
	//printf("x: %.2f\n", x);
	int m = 0;
	double s = 0.4;
	static const float inv_sqrt_2pi = 0.3989422804014327;
	float a = (x - m) / s;

	return inv_sqrt_2pi / s * std::exp(-2 * a * a);
}

int getPublicLightsStatus() {
	double horaActual = segundos % 24;

	if (horaActual < 8 or horaActual>18)
		return 1;
	else
		return 0;
}

void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}




void CreateObjects() 
{
	unsigned int indices[] = {		
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
	//	x      y      z			u	  v			nx	  ny    nz
		-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
		0, 1, 2,
		0, 2, 3,
		4,5,6,
		4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};
	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

	Mesh *obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

}

void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		4, 5, 6,
		6, 7, 4,
		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,
	};
	//Ejercicio 1: reemplazar con sus dados de 6 caras texturizados, agregar normales
// average normals
	GLfloat cubo_vertices[] = {
		// front
		//x		y		z		S		T			NX		NY		NZ
		-0.5f, -0.5f,  0.5f,	0.27f,  0.35f,		0.0f,	0.0f,	-1.0f,	//0
		0.5f, -0.5f,  0.5f,		0.48f,	0.35f,		0.0f,	0.0f,	-1.0f,	//1
		0.5f,  0.5f,  0.5f,		0.48f,	0.64f,		0.0f,	0.0f,	-1.0f,	//2
		-0.5f,  0.5f,  0.5f,	0.27f,	0.64f,		0.0f,	0.0f,	-1.0f,	//3
		// right
		//x		y		z		S		T
		0.5f, -0.5f,  0.5f,	    0.52f,  0.35f,		-1.0f,	0.0f,	0.0f,
		0.5f, -0.5f,  -0.5f,	0.73f,	0.35f,		-1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  -0.5f,	0.73f,	0.64f,		-1.0f,	0.0f,	0.0f,
		0.5f,  0.5f,  0.5f,	    0.52f,	0.64f,		-1.0f,	0.0f,	0.0f,
		// back
		-0.5f, -0.5f, -0.5f,	0.77f,	0.35f,		0.0f,	0.0f,	1.0f,
		0.5f, -0.5f, -0.5f,		0.98f,	0.35f,		0.0f,	0.0f,	1.0f,
		0.5f,  0.5f, -0.5f,		0.98f,	0.64f,		0.0f,	0.0f,	1.0f,
		-0.5f,  0.5f, -0.5f,	0.77f,	0.64f,		0.0f,	0.0f,	1.0f,

		// left
		//x		y		z		S		T
		-0.5f, -0.5f,  -0.5f,	0.0f,	0.35f,		1.0f,	0.0f,	0.0f,
		-0.5f, -0.5f,  0.5f,	0.23f,  0.35f,		1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  0.5f,	0.23f,	0.64f,		1.0f,	0.0f,	0.0f,
		-0.5f,  0.5f,  -0.5f,	0.0f,	0.64f,		1.0f,	0.0f,	0.0f,

		// bottom
		//x		y		z		S		T
		-0.5f, -0.5f,  0.5f,	0.27f,	0.02f,		0.0f,	1.0f,	0.0f,
		0.5f,  -0.5f,  0.5f,	0.48f,  0.02f,		0.0f,	1.0f,	0.0f,
		 0.5f,  -0.5f,  -0.5f,	0.48f,	0.31f,		0.0f,	1.0f,	0.0f,
		-0.5f, -0.5f,  -0.5f,	0.27f,	0.31f,		0.0f,	1.0f,	0.0f,

		//UP
		 //x		y		z		S		T
		 -0.5f, 0.5f,  0.5f,	0.27f,	0.68f,		0.0f,	-1.0f,	0.0f,
		 0.5f,  0.5f,  0.5f,	0.48f,  0.68f,		0.0f,	-1.0f,	0.0f,
		  0.5f, 0.5f,  -0.5f,	0.48f,	0.98f,		0.0f,	-1.0f,	0.0f,
		 -0.5f, 0.5f,  -0.5f,	0.27f,	0.98f,		0.0f,	-1.0f,	0.0f,

	};
	
	Mesh *cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
	meshList.push_back(cubo);

}


void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


///////////////////////////////KEYFRAMES/////////////////////
bool animacion = false;


//NEW// Keyframes
float posXavion = 2.0, posYavion = 2.0, posZavion = 0;
float	movAvion_x = 0.0f, movAvion_y = 0.0f;
float giroAvion = 0;

/*Cambio*/
#define MAX_FRAMES 30
int i_max_steps = 90;
int i_curr_steps = 19;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float movAvion_x;		//Variable para PosicionX
	float movAvion_y;		//Variable para PosicionY
	float movAvion_xInc;		//Variable para IncrementoX
	float movAvion_yInc;		//Variable para IncrementoY
	float giroAvion;
	float giroAvionInc;
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 19;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	printf("frameindex %d\n", FrameIndex);

	KeyFrame[FrameIndex].movAvion_x = movAvion_x;
	KeyFrame[FrameIndex].movAvion_y = movAvion_y;
	KeyFrame[FrameIndex].giroAvion;

	FrameIndex++;
}

void resetElements(void)
{
	movAvion_x = KeyFrame[0].movAvion_x;
	movAvion_y = KeyFrame[0].movAvion_y;
	giroAvion= KeyFrame[0].giroAvion;
}

void interpolation(void)
{
	KeyFrame[playIndex].movAvion_xInc = (KeyFrame[playIndex + 1].movAvion_x - KeyFrame[playIndex].movAvion_x) / i_max_steps;
	KeyFrame[playIndex].movAvion_yInc = (KeyFrame[playIndex + 1].movAvion_y - KeyFrame[playIndex].movAvion_y) / i_max_steps;
	KeyFrame[playIndex].giroAvionInc = (KeyFrame[playIndex + 1].giroAvion - KeyFrame[playIndex].giroAvion) / i_max_steps;
}


void animate(void)
{
	//Movimiento del objeto
	if (play)
	{	
		rot = rot + 0.2;
		/*if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			printf("playindex : %d\n", playIndex);
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("Frame index= %d\n", FrameIndex);
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				//printf("entro aqu�\n");
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//printf("se qued� aqui\n");
			//printf("max steps: %f", i_max_steps);
			//Draw animation
			movAvion_x += KeyFrame[playIndex].movAvion_xInc;
			movAvion_y += KeyFrame[playIndex].movAvion_yInc;
			giroAvion += KeyFrame[playIndex].giroAvionInc;
			i_curr_steps++;
		}*/

	}
}

/* FIN KEYFRAMES*/

int main()
{
	mainWindow = Window(1280, 1024); // 1280, 1024 or 1024, 768 or 1366, 768
	mainWindow.Initialise();

	CreateObjects();
	CrearCubo();

	CreateShaders();
	Sonido musica = Sonido();
	//musica.Reproduce();
	/*
	ISoundEngine* engine = createIrrKlangDevice();

	if (!engine)
	{
		printf("Could not startup engine\n");
		return 0; // error starting up the engine
	}
	engine->play2D("media/getout.ogg", true);
	*/


	// ------------- Defines tus texturas ---------------------
	//brickTexture = Texture("Textures/brick.png");
	//brickTexture.LoadTextureA();
	//dirtTexture = Texture("Textures/dirt.png");
	//dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/proyecto/lego_wall.tga"); //jpg
	pisoTexture.LoadTextureA();
	Tagave = Texture("Textures/Agave.tga");
	Tagave.LoadTextureA();
	Tladrillo = Texture("Textures/proyecto/ladrillo.tga");
	Tladrillo.LoadTextureA();
	Tbasura = Texture("Textures/proyecto/basura.tga");
	Tbasura.LoadTextureA();
	Tpasillo = Texture("Textures/proyecto/pasillo.tga");
	Tpasillo.LoadTextureA();
	Tarbol1 = Texture("Textures/proyecto/legotree.tga");
	Tarbol1.LoadTextureA();
	Tarbol2 = Texture("Textures/proyecto/lego_three4.tga");
	Tarbol2.LoadTextureA();
	TlegoGreen = Texture("Textures/proyecto/lego green.tga");
	TlegoGreen.LoadTextureA();
	Tarbol3 = Texture("Textures/proyecto/lego_arbolito.tga");
	Tarbol3.LoadTextureA();
	Tnaranja = Texture("Textures/proyecto/naranja.png");
	Tnaranja.LoadTextureA();
	Trosa = Texture("Textures/proyecto/rosa.png");
	Trosa.LoadTextureA();
	Tverde = Texture("Textures/proyecto/verde.jpg");
	Tverde.LoadTextureA();
	Tnegro = Texture("Textures/proyecto/negro.png");
	Tnegro.LoadTextureA();
	Tazul = Texture("Textures/proyecto/azul.jpg");
	Tazul.LoadTextureA();
	Trosa = Texture("Textures/proyecto/rosa.png");
	Trosa.LoadTextureA();
	Tcafe = Texture("Textures/proyecto/cafe.jpg");
	Tcafe.LoadTextureA();
	Ttransparente = Texture("Textures/proyecto/transparente.tga");
	Ttransparente.LoadTextureA();
	Tamarillo = Texture("Textures/proyecto/amarillo.jpg");
	Tamarillo.LoadTextureA();
	Tagua = Texture("Textures/proyecto/agua1.tga");
	Tagua.LoadTextureA();
	Tblanco = Texture("Textures/proyecto/blanco.jpg");
	Tblanco.LoadTextureA();
	Tarbol4 = Texture("Textures/proyecto/lego_arbolito3.tga");
	Tarbol4.LoadTexture();
	Tcolores = Texture("Textures/proyecto/colores2.jpeg");
	Tcolores.LoadTexture();
	Tcolores3 = Texture("Textures/proyecto/colores3.jpeg");
	Tcolores3.LoadTexture();
	Tvidrio = Texture("Textures/proyecto/vidrio.tga");
	Tvidrio.LoadTextureA();






	//Valor a tus materiales
	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);

	//Para cargar modelos
	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/k_rueda.3ds");
	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");
	Ping_M = Model();
	Ping_M.LoadModel("Models/WOLF.obj");
	Camino_M = Model();
	Camino_M.LoadModel("Models/railroad track.obj");
	wc_M = Model();
	wc_M.LoadModel("Models/unitaz.obj");


	bool isDay = false;
	//luz direccional, s�lo 1 y siempre debe de existir
	printf("\nIntensidad del sol: %.2f\n", getSunIntensity(segundos));

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 
								//1.0f, 1.0f,
								getSunIntensity(segundos),getSunIntensity(segundos),
								0.0f, 100.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
								0.0f*getPublicLightsStatus(), 1.0f*getPublicLightsStatus(),
								2.0f, 1.5f,3.0f,
								0.3f, 0.2f, 0.1f);
	pointLightCount++;
	
	pointLights[1] = PointLight(1.0f, 1.0f, 1.0f,
		0.0f*getPublicLightsStatus(), 1.0f*getPublicLightsStatus(),
		2.0f, 1.5f, -3.0f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;
	unsigned int spotLightCount = 0;



	//luz de kiosko
	spotLights[0] = SpotLight(1.0f, 0.0f, 0.0f,
		1.0f*getPublicLightsStatus(), 1.0f*getPublicLightsStatus(),
		10.0f, 0.0f, -1.0f,
		0.0f, -20.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		100.0f);
	spotLightCount++;
	//luz de kiosko2
	/*spotLights[3] = SpotLight(0.0f, 0.0f, 1.0f,
		0.0f*getPublicLightsStatus(), 1.0f*getPublicLightsStatus(),
		2.0 - movCoche, 2.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f);
	spotLightCount++;*/
	//luz de kisko 3
	spotLights[1] = SpotLight(0.0f, 1.0f, 1.0f,
		1.0f*getPublicLightsStatus(), 1.0f* getPublicLightsStatus(),
		10.0f, 0.0f, 1.0f,
		0.0f, -20.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		100.0f);
	spotLightCount++;


	glm::vec3 posblackhawk = glm::vec3(2.0f, 0.0f, 0.0f);

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/posx.tga");
	skyboxFaces.push_back("Textures/Skybox/negx.tga");
	skyboxFaces.push_back("Textures/Skybox/negy.tga");//Abajo
	skyboxFaces.push_back("Textures/Skybox/posy.tga");//Arriba
	skyboxFaces.push_back("Textures/Skybox/posz.tga");//Izquierda
	skyboxFaces.push_back("Textures/Skybox/negz.tga");

	skybox = Skybox(skyboxFaces);
	
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 300.0f);

	
	movCoche = 0.0f;
	movOffset = 2.0f;
	avanza = 1;
	
	//KEYFRAMES DECLARADOS INICIALES
	
	KeyFrame[0].movAvion_x = 0.0f;
	KeyFrame[0].movAvion_y = 0.0f;
	KeyFrame[0].giroAvion = 0;


	KeyFrame[1].movAvion_x = 1.0f;
	KeyFrame[1].movAvion_y = 2.0f;
	KeyFrame[1].giroAvion = 0;


	KeyFrame[2].movAvion_x = 2.0f;
	KeyFrame[2].movAvion_y = 0.0f;
	KeyFrame[2].giroAvion = 0;


	KeyFrame[3].movAvion_x = 3.0f;
	KeyFrame[3].movAvion_y = -2.0f;
	KeyFrame[3].giroAvion = 0;

	KeyFrame[4].movAvion_x = 4.0f;
	KeyFrame[4].movAvion_y = 0.0f;
	KeyFrame[4].giroAvion = 0.0f;

	//Agrego 4

	KeyFrame[5].movAvion_x = 5.0f;
	KeyFrame[5].movAvion_y = 2.0f;
	KeyFrame[5].giroAvion = 0;


	KeyFrame[6].movAvion_x = 6.0f;
	KeyFrame[6].movAvion_y = 0.0f;
	KeyFrame[6].giroAvion = 0;


	KeyFrame[7].movAvion_x = 7.0f;
	KeyFrame[7].movAvion_y = -2.0f;
	KeyFrame[7].giroAvion = 0;

	KeyFrame[8].movAvion_x = 8.0f;
	KeyFrame[8].movAvion_y = 0.0f;
	KeyFrame[8].giroAvion = 0.0f;

	/*Vuelta*/
	KeyFrame[9].movAvion_x = 8.0f;
	KeyFrame[9].movAvion_y = 0.0f;
	KeyFrame[9].giroAvion = 180.0f;



	KeyFrame[10].movAvion_x = 7.0f;
	KeyFrame[10].movAvion_y = -2.0f;
	KeyFrame[10].giroAvion = 180.0f;
	
	KeyFrame[11].movAvion_x = 6.0f;
	KeyFrame[11].movAvion_y = 0.0f;
	KeyFrame[11].giroAvion = 180;

	KeyFrame[12].movAvion_x = 5.0f;
	KeyFrame[12].movAvion_y = 2.0f;
	KeyFrame[12].giroAvion = 180.0f;

	KeyFrame[13].movAvion_x = 4.0f;
	KeyFrame[13].movAvion_y = 0.0f;
	KeyFrame[13].giroAvion = 180.0f;

	/*Agrego otros 4*/


	KeyFrame[14].movAvion_x = 3.0f;
	KeyFrame[14].movAvion_y = -2.0f;
	KeyFrame[14].giroAvion = 180.0f;

	KeyFrame[15].movAvion_x = 2.0f;
	KeyFrame[15].movAvion_y = 0.0f;
	KeyFrame[15].giroAvion = 180;

	KeyFrame[16].movAvion_x = 1.0f;
	KeyFrame[16].movAvion_y = 2.0f;
	KeyFrame[16].giroAvion = 180.0f;


	KeyFrame[17].movAvion_x = 0.0f;
	KeyFrame[17].movAvion_y = 0.0f;
	KeyFrame[17].giroAvion = 180.0f;

	KeyFrame[18].movAvion_x = 0.0f;
	KeyFrame[18].movAvion_y = 0.0f;
	KeyFrame[18].giroAvion = 0;

	//Agregar Kefyrame[5] para que el avi�n regrese al inicio */
	positionP = glm::vec3(camera.getCameraPosition().x + 2.5, camera.getCameraPosition().y + 1.3, camera.getCameraPosition().z);
	camera = Camera(glm::vec3(positionP), glm::vec3(0.0f, 2.0f, 0.0f), 0.0f, 0.0f, 5.0f, 0.0f);


	double startTime,endTime;
	startTime = glfwGetTime();
	//Loop mientras no se cierra la ventana
	int frames;
	frames = 0;
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime; 
		lastTime = now;
		//printf("lastTime: %f\n", &lastTime);

		spotLightCount = 6;

		printf("frames: %d sin(x): %.2f\n", frames,getLocVariation(frames));
		if (avanza)
		{
			if (movPerro < 10.0f)
			{
//				movPerro += movOffset*deltaTime;
				movPerro += movOffset;
				avanza = 1;
			}
			else
			{
				avanza = 0;
			}
		}
		else
		{
			if (movPerro > -10.0f)
			{
				movPerro -= movOffset*deltaTime;
			}
			else
			{
				avanza = 1;
			}


		}

		
		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
		//para keyframes
		//inputKeyframes(mainWindow.getsKeys());
		inputkey(mainWindow.getsKeys());
		animate();
		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		//spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		glm::mat4 model(1.0);

		//AQUI SE EMPIEZA A DIBUJAR

		//Para crear el piso
		model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.5f,2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		TlegoGreen.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();
		glDisable(GL_BLEND);

		//---- pasillos
			
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, 4.0f));
			model = glm::scale(model, glm::vec3(2.0f, 0.05f, 0.097f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();
			glDisable(GL_BLEND);

			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, 2.0f));
			model = glm::scale(model, glm::vec3(2.0f, 0.05f, 0.097f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();
			glDisable(GL_BLEND);

			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, 0.0f));
			model = glm::scale(model, glm::vec3(2.0f, 0.05f, 0.097f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();
			glDisable(GL_BLEND);

			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, -2.0f));
			model = glm::scale(model, glm::vec3(2.0f, 0.05f, 0.097f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();
			glDisable(GL_BLEND);

			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 0.01f, -4.0f));
			model = glm::scale(model, glm::vec3(2.0f, 0.5f, 0.097f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();
			glDisable(GL_BLEND);

		//luminaria
			//poste1
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(3.0f, 1.5f, -5.2f));
			model = glm::scale(model, glm::vec3(0.1f, 3.0f, 0.1f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();
			//spotLights[2].SetPos(glm::vec3(3.0f,2.5f,-5.2f));
			glDisable(GL_BLEND);

			//poste2
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(3.0f, 1.5f, 5.2f));
			model = glm::scale(model, glm::vec3(0.1f, 3.0f, 0.1f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tpasillo.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();
			//spotLights[4].SetPos(glm::vec3(3.0f, 2.5f, 5.2f));
			glDisable(GL_BLEND);

	//------ Personaje
			model = glm::mat4(1.0);
			if (camera.getCameraPosition().y > 2.0f) { model = glm::translate(model, glm::vec3(0.0, 1.3, 0.0)); }
			else { model = glm::translate(model, glm::vec3(camera.getCameraPosition())); }
			//model = glm::rotate(model, camera.getCameraDirection().x+180, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.8f, 1.0f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tnegro.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//Cuerpo
			//capa
			model = glm::translate(model, glm::vec3(-0.6f, -0.2f, 0.0f));
			model = glm::scale(model, glm::vec3(0.1f, 1.4f, 1.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tnegro.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//capa 
			//cinturon
			model = glm::scale(model, glm::vec3(10.0f, 0.71f, 0.66f));
			model = glm::translate(model, glm::vec3(0.6f, 0.2f, 0.0f));

			model = glm::translate(model, glm::vec3(0.0f, -0.625f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 0.25f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tamarillo.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//Cintura

			model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.25f));
			model = glm::scale(model, glm::vec3(1.0f, 4.0f, 0.4f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[4]->RenderMesh();//pierna derecha
			model = glm::translate(model, glm::vec3(0.0f, -0.625f, 0.0f));
			model = glm::scale(model, glm::vec3(1.2f, 0.25f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tpasillo.UseTexture();
			meshList[4]->RenderMesh();//Pie derecho

			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.25f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			meshList[4]->RenderMesh();//pie izquierdo
			model = glm::translate(model, glm::vec3(0.0f, 2.5f, 0.0f));
			model = glm::scale(model, glm::vec3(0.8f, 4.0f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[4]->RenderMesh();//Pierna izquierdo

			model = glm::mat4(1.0);
			if (camera.getCameraPosition().y > 2.0f) { model = glm::translate(model, glm::vec3(0.0, 1.3, 0.0)); }
			else { model = glm::translate(model, glm::vec3(camera.getCameraPosition())); }
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			model = glm::translate(model, glm::vec3(5.0f, -0.1f, -0.65f));
			model = glm::scale(model, glm::vec3(0.5f, 1.2f, 0.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//brazo Izquierdo
			model = glm::translate(model, glm::vec3(0.0f, -0.625f, 0.0f));
			model = glm::scale(model, glm::vec3(1.2f, 0.25f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tpasillo.UseTexture();
			meshList[4]->RenderMesh();//mano Izquierda
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 4.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tpasillo.UseTexture();
			meshList[4]->RenderMesh();//mano derecha
			model = glm::translate(model, glm::vec3(0.0f, 2.5f, 0.0f));
			model = glm::scale(model, glm::vec3(0.8f, 4.0f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[4]->RenderMesh();//brazo derecho

			//Cabeza
			model = glm::translate(model, glm::vec3(0.0f, 0.9f, -2.1f));
			model = glm::scale(model, glm::vec3(1.0f, 0.7f, 3.2f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[4]->RenderMesh();//cabeza
			model = glm::translate(model, glm::vec3(0.45f, 0.25f, -0.25f));
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			meshList[4]->RenderMesh();//ojo izquierdo
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			meshList[4]->RenderMesh();//ojo derecho
			model = glm::translate(model, glm::vec3(0.0f, -2.0f, -1.2f));
			model = glm::scale(model, glm::vec3(1.0f, 1.3f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			meshList[4]->RenderMesh();//boca
			model = glm::translate(model, glm::vec3(-2.5f, 3.5f, 0.6f));
			model = glm::scale(model, glm::vec3(2.0f, 1.0f, 0.5f));
			model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[1]->RenderMesh();//oreja izquierda
			model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Tnegro.UseTexture();
			meshList[1]->RenderMesh();//oreja derecha



/*
	//----- Edificios
		//edificio-1
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(10.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio0
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(5.0f, 1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio1
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio2
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-5.0f, 1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio3
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f, 1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio4
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-15.0f, 1.0f, -10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio5
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-15.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio6
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-10.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio7
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(-5.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio8
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores3.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);

			//edificio9
			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(5.0f, 1.0f, 10.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte1
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte2
			model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glEnable(GL_BLEND);
			Tcolores2.UseTexture();
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();//parte3
			glDisable(GL_BLEND);
		*/

		////Ba�o caballeros
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(10.0f, 2.0f, -15.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//glEnable(GL_BLEND);
		//Tnaranja.UseTexture();
		//Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//meshList[4]->RenderMesh();
		//glDisable(GL_BLEND);

		////Ba�o damas
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(15.0f, 2.0f, -15.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		//model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//glEnable(GL_BLEND);
		//Trosa.UseTexture();
		//Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		//meshList[4]->RenderMesh();
		//glDisable(GL_BLEND);

//---Ba�o caballeros
model = glm::mat4(1.0);
model = glm::translate(model, glm::vec3(5.0f, 0.1f, -15.0f));
model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.1f, 4.0f, 7.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
pisoTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared derecha
model = glm::translate(model, glm::vec3(60.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared izquierda
model = glm::scale(model, glm::vec3(10.0f, 0.25f, 0.14f));//1.0 1.0 1.0
model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -3.5f));
model = glm::scale(model, glm::vec3(6.0f, 4.0f, 0.1f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared trasera
//----lavamanos
model = glm::scale(model, glm::vec3(0.16f, 0.25f, 10.0f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(2.55f, -0.5f, 2.0f));
model = glm::scale(model, glm::vec3(1.0f, 0.25f, 4.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//base
model = glm::translate(model, glm::vec3(-0.4f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.2f, 1.0f, 1.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//costado
model = glm::scale(model, glm::vec3(5.0f, 4.0f, 0.25f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.8f, 0.25f, 4.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
Tagua.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//agua
glDisable(GL_BLEND);
model = glm::scale(model, glm::vec3(1.25f, 4.0f, 0.25f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(-0.1f, -0.125f, 2.0f));
model = glm::scale(model, glm::vec3(1.0f, 0.5f, 0.1f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//frente
//--- ba�os
model = glm::scale(model, glm::vec3(1.0f, 2.0f, 10.0f));//1.0,1.0,1.0
model = glm::scale(model, glm::vec3(2.5f, 2.5f, 3.0f));// 
model = glm::translate(model, glm::vec3(-2.1f, -0.25f, -1.0f));
model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); //Rotacion
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
wc_M.RenderModel(); //wc1
model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
wc_M.RenderModel(); //wc2
//--paredes y puertas
model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.33f));// 1.0,1.0,1.0
model = glm::translate(model, glm::vec3(1.45f, 0.5f, 1.0f));
model = glm::scale(model, glm::vec3(0.2f, 3.0f, 3.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tazul.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared 1
glDisable(GL_BLEND);
model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tazul.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared 2

model = glm::scale(model, glm::vec3(5.0f, 0.33f, 0.33f));///1,1,1
model = glm::translate(model, glm::vec3(1.0f, 0.45f, 1.5f));
model = glm::scale(model, glm::vec3(1.8f, 2.0f, 0.25f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta 2
model = glm::translate(model, glm::vec3(1.1f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta 1
glDisable(GL_BLEND);

model = glm::scale(model, glm::vec3(0.55f, 0.5f, 4.0f));///1,1,1
model = glm::translate(model, glm::vec3(0.75f, 0.0f, -0.5f));
model = glm::scale(model, glm::vec3(0.25f, 1.0f, 1.0f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tnegro.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//para papel
model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tnegro.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//para papel

model = glm::scale(model, glm::vec3(4.0f, 1.0f, 1.0f));///1,1,1
model = glm::translate(model, glm::vec3(-3.75f, 0.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.1f, 4.0f, 5.0f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tazul.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared frente
model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.75f));
model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
Tvidrio.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta
glDisable(GL_BLEND);
glDisable(GL_BLEND);


//-----Ba�o damas
model = glm::mat4(1.0);
model = glm::translate(model, glm::vec3(10.0f, 0.1f, -15.0f));
model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.1f, 4.0f, 7.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
pisoTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared derecha
model = glm::translate(model, glm::vec3(60.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared izquierda
model = glm::scale(model, glm::vec3(10.0f, 0.25f, 0.14f));//1.0 1.0 1.0
model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -3.5f));
model = glm::scale(model, glm::vec3(6.0f, 4.0f, 0.1f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared trasera
//----lavamanos
model = glm::scale(model, glm::vec3(0.16f, 0.25f, 10.0f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(2.55f, -0.5f, 2.0f));
model = glm::scale(model, glm::vec3(1.0f, 0.25f, 4.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//base
model = glm::translate(model, glm::vec3(-0.4f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.2f, 1.0f, 1.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//costado
model = glm::scale(model, glm::vec3(5.0f, 4.0f, 0.25f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.8f, 0.25f, 4.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
Tagua.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//agua
glDisable(GL_BLEND);
model = glm::scale(model, glm::vec3(1.25f, 4.0f, 0.25f));//1.0,1.0,1.0
model = glm::translate(model, glm::vec3(-0.1f, -0.125f, 2.0f));
model = glm::scale(model, glm::vec3(1.0f, 0.5f, 0.1f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
plainTexture.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//frente

model = glm::scale(model, glm::vec3(1.0f, 2.0f, 10.0f));//1.0,1.0,1.0
model = glm::scale(model, glm::vec3(2.5f, 2.5f, 3.0f));// 
model = glm::translate(model, glm::vec3(-2.1f, -0.25f, -1.0f));
model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); //Rotacion
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
wc_M.RenderModel(); //wc

model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
wc_M.RenderModel(); //wc2
//--puerta
model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.33f));// 1.0,1.0,1.0
model = glm::translate(model, glm::vec3(1.45f, 0.5f, 1.0f));
model = glm::scale(model, glm::vec3(0.2f, 3.0f, 3.0f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tazul.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared 1
glDisable(GL_BLEND);
model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tazul.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared 2
model = glm::scale(model, glm::vec3(5.0f, 0.33f, 0.33f));///1,1,1
model = glm::translate(model, glm::vec3(1.0f, 0.45f, 1.5f));
model = glm::scale(model, glm::vec3(1.8f, 2.0f, 0.25f));//-
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta 2
model = glm::translate(model, glm::vec3(1.1f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tblanco.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta 1
glDisable(GL_BLEND);

model = glm::scale(model, glm::vec3(0.55f, 0.5f, 4.0f));///1,1,1
model = glm::translate(model, glm::vec3(0.75f, 0.0f, -0.5f));
model = glm::scale(model, glm::vec3(0.25f, 1.0f, 1.0f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tnegro.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//para papel
model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 0.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tnegro.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//para papel

model = glm::scale(model, glm::vec3(4.0f, 1.0f, 1.0f));///1,1,1
model = glm::translate(model, glm::vec3(-3.75f, 0.0f, 0.0f));
model = glm::scale(model, glm::vec3(0.1f, 4.0f, 5.0f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
Tamarillo.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//pared frente
model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.75f));
model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.5f));//--
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
Tvidrio.UseTexture();
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshList[4]->RenderMesh();//puerta
glDisable(GL_BLEND);
glDisable(GL_BLEND);



		//---------Kiosko
				//Escaleras
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(10.0f, 0.1f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tpasillo.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//1er escal�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//2do escol�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//3er escal�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//4to escal�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//5to escal�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//6to escal�n
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//7mo escal�n
		glDisable(GL_BLEND);

		//base
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(11.75f, 0.8f, -1.75f));
		model = glm::scale(model, glm::vec3(1.0f, 1.6f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//cubo1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo2
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo3
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo4
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo5
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo6
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo7
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo8
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo9
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo10
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo11
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo12
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo13
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo14
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo15
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo16
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo17
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo18
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo19
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo20
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo21
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -9.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo22
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo23
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo24
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo25
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo27
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo28
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo29
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo30
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo31
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo32
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo33
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -10.0f));//quita fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo34
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo35
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo36
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo37
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo38
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo39
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo40
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo41
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo42
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo43
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo44
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -10.0f));//sexta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -9.0f));//septima fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//octava fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//novena fila (ultima)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//cubo
		glDisable(GL_BLEND);

		//---Pilares
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(11.75f, 3.6f, -1.75f));
		model = glm::scale(model, glm::vec3(0.5f, 4.0f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//pilar1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//pilar2
		model = glm::translate(model, glm::vec3(6.0f, 0.0f, 6.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//pilar3
		model = glm::translate(model, glm::vec3(4.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//pilar4
		model = glm::translate(model, glm::vec3(6.0f, 0.0f, -6.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//pilar5
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//pilar6
		model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -6.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tblanco.UseTexture();
		meshList[4]->RenderMesh();//pilar7
		model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tazul.UseTexture();
		meshList[4]->RenderMesh();//pilar8

	//Techo ******nivel1
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(11.75f, 6.0f, -1.75f));
		model = glm::scale(model, glm::vec3(1.0f, 0.8f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tazul.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//cubo1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo2
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo3
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo4
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo5
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo6
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo7
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo8
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo9
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo10
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo11
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo12
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo13
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo14
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo15
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo16
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo17
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo18
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo19
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo20
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo21
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -9.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo22
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo23
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo24
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo25
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo26
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo27
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo28
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo29
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo30
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo31
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo32
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo33
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -10.0f));//quita fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo34
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo35
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo36
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo37
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo38
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo39
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo40
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo41
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo42
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo43
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo44
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -10.0f));//sexta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo45
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo46
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo47
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo48
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -9.0f));//septima fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//octava fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//novena fila (ultima)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		glDisable(GL_BLEND);

		//Techo ******nivel2
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(12.75f, 6.8f, -1.75f));
		model = glm::scale(model, glm::vec3(1.0f, 0.8f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//cubo1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo2
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo3
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo4
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo5
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo6
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo7
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo8
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo9
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo10
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo11
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo12
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo22
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo23
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo24
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo25
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo26
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo27
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo28
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo29
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo30
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo31
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -8.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo34
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo35
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo36
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo37
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo38
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo39
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo40
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo41
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo42
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -8.0f));//sexta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo45
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo46
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo47
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo48
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -7.0f));//septima fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//octava fila(ultima)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo

		//techo ---- nivel3
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(13.75f, 7.6f, -1.75f));
		model = glm::scale(model, glm::vec3(1.0f, 0.8f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tazul.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//cubo1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo2
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo3
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo4
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo5
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo6
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo7
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo8
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo9
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo10
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo11
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo12
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -6.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo22
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo23
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo24
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo25
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo26
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo27
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo28
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo29
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -6.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo45
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo46
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo47
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo48
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//septima fila(ultima)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));

		//---Barandales
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(13.0f, 3.0f, 3.9f));
		model = glm::rotate(model, 45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla1
		model = glm::translate(model, glm::vec3(90.0f, 0.0f, -0.3f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla4
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(15.7f, 3.0f, 5.3f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tazul.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla2
		model = glm::translate(model, glm::vec3(102.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tazul.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla5
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(12.90f, 3.0f, -3.5f));
		model = glm::rotate(model, -45 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 3.9f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla6
		model = glm::translate(model, glm::vec3(90.0f, 0.0f, 0.25f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tblanco.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla3
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(19.5f, 3.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 0.1f, 0.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tazul.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//barilla4


//--------- bancas
	//banca1
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.0f, 0.7f, 6.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.25f, 0.75f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.75f, 0.3f, 6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.6f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//pata1 de banca
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//pata2 de banca
		glDisable(GL_BLEND);

		//banca2
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.0f, 0.7f, -6.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.25f, 0.75f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-14.75f, 0.3f, -6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.6f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//pata1 de banca
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//pata2 de banca
		glDisable(GL_BLEND);


		//banca3
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.7f, 6.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.25f, 0.75f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-0.75f, 0.3f, 6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.6f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//pata1 de banca
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//pata2 de banca
		glDisable(GL_BLEND);

		//banca4
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.7f, -6.0f));
		model = glm::scale(model, glm::vec3(2.0f, 0.25f, 0.75f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-0.75f, 0.3f, -6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.6f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//pata1 de banca
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[4]->RenderMesh();//pata2 de banca
		glDisable(GL_BLEND);

		//--- Fuente
		//base
		model = glm::mat4(1.0);
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::translate(model, glm::vec3(-5.0f, 0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tcafe.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();//cubo1
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo2
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo3
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -3.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo4
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo5
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo6
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo7
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo8
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo9
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo10
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo11
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.0f, 4.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo12
		model = glm::scale(model, glm::vec3(1.0f, 0.25f, 1.0f));
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -6.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.0f, 4.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::scale(model, glm::vec3(1.0f, 0.25f, 1.0f));
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -5.0f));//quinta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, -3.0f));//sexta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo

		//orilla
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));//primer fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));//segunda fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 4.0f));//
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 1.0f));//tercera fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -6.0f));//
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));//cuarta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 6.0f));//
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));//quinta fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));//
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 1.0f));//primer fila
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tcafe.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Tnaranja.UseTexture();
		meshList[4]->RenderMesh();//cubo

		//Agua
		model = glm::translate(model, glm::vec3(2.5f, -0.25f, 0.5f));
		model = glm::scale(model, glm::vec3(2.0f, 0.5f, 2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tagua.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tagua.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(0.75f, 0.0f, 0.75f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 1.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tagua.UseTexture();
		meshList[4]->RenderMesh();//cubo
		model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tagua.UseTexture();
		meshList[4]->RenderMesh();//cubo




	//--- Arboles
		//Arbol1
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(5.0f, 2.5f, -6.0f));
		model = glm::scale(model, glm::vec3(3.0f, 5.0f, 3.0f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol1.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbol2
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(5.0f, 2.5f, 6.0f));
		model = glm::scale(model, glm::vec3(3.0f, 5.0f, 3.0f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol1.UseTexture(); //
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbol3
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 2.5f, -6.0f));
		model = glm::scale(model, glm::vec3(2.5f, 5.0f, 2.5f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol2.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbol4
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-5.0f, 2.5f, 6.0f));
		model = glm::scale(model, glm::vec3(2.5f, 5.0f, 2.5f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol2.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbol5
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-15.0f, 2.5f, 15.0f));
		model = glm::scale(model, glm::vec3(2.5f, 5.0f, 2.5f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol2.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbol6
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-15.0f, 3.0f, -15.0f));
		model = glm::scale(model, glm::vec3(3.5f, 6.0f, 3.5f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol2.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);
		/*
		//Arbol7
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-15.0f, 3.0f, -9.0f));
		model = glm::scale(model, glm::vec3(3.0f, 6.0f, 3.0f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol4.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);
		
		//Arbol8
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-13.0f, 3.0f, 9.0f));;
		model = glm::scale(model, glm::vec3(3.0f, 6.0f, 3.0f));
		model = glm::rotate(model, rot * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol4.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);
		*/
		//Arbolito-1
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-8.0f, 0.5f, -7.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol3.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		//Arbolito-2
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-9.0f, 0.5f, 8.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Tarbol3.UseTexture(); // 
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);
		

		//--------- botes de basura
	//bote1 azul
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-16.0f, 0.5f, -5.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tbasura.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		//bote2 blanco
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-16.0f, 0.5f, 5.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tbasura.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		//bote1 azul
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(2.0f, 0.5f, -5.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tbasura.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		//bote1 blanco
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(2.0f, 0.5f, 5.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glEnable(GL_BLEND);
		Tbasura.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();
		glDisable(GL_BLEND);

		//Para cargar al perrito
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-3.5 + movPerro, 0.3f, -3.5f)); //Tralasion 
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));//Escala 
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); //Rotacion
		//model = glm::rotate(model, 270 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f)); //Rotacion
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Ping_M.RenderModel(); //carga del perro

		mainLight = DirectionalLight(1.0f, 1.0f, 0.6f,
			//1.0f, 1.0f,
			getSunIntensity(segundos), getSunIntensity(segundos),
			0.0f, 100.0f, -1.0f);
		pointLights[0] = PointLight(1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			0.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			2.0f, 1.5f, 3.0f,
			0.3f, 0.2f, 0.1f);

		pointLights[1] = PointLight(1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			0.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			2.0f, 1.5f, -3.0f,
			0.3f, 0.2f, 0.1f);

		spotLights[0] = SpotLight(1.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn, 0.0f * getPublicLightsStatus()* lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			12.0f, 5.0f, 1.5f,
			-1.0f * getLocVariation(frames), -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			20.0f);

		spotLights[1] = SpotLight(0.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			12.0f, 5.0f, -1.5f,
			-1.0f * getLocVariation(frames), -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			20.0f);
		spotLights[2] = SpotLight(1.0f * getPublicLightsStatus()* lightsOn, 0.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			16.0f, 5.0f, 3.0f,
			-1.0f * getLocVariation(frames), -1.0f, 1.0f*getLocVariation(frames),
			1.0f, 0.0f, 0.0f,
			20.0f);

		spotLights[3] = SpotLight(0.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn, 0.0f * getPublicLightsStatus()* lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			16.0f, 5.0f, -3.0f,
			-1.0f * getLocVariation(frames), -1.0f, -1.0f*getLocVariation(frames),
			1.0f, 0.0f, 0.0f,
			20.0f);
		spotLights[4] = SpotLight(1.0f * getPublicLightsStatus()* lightsOn, 1.0f * getPublicLightsStatus()* lightsOn, 0.5f * getPublicLightsStatus()* lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			20.0f, 5.0f, 1.5f,
			1.0f*getLocVariation(frames), -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			20.0f);

		spotLights[5] = SpotLight(0.5f * getPublicLightsStatus()*lightsOn, 1.0f * getPublicLightsStatus()*lightsOn, 1.0f * getPublicLightsStatus()*lightsOn,
			1.0f * getPublicLightsStatus(), 1.0f * getPublicLightsStatus(),
			20.0f, 5.0f, -1.5f,
			1.0f * getLocVariation(frames), -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			20.0f);

		frames++;
		endTime = glfwGetTime();
		double resultado = endTime - startTime;
		//printf("end - start: %.2f-%.2f: %.2f\n", endTime,startTime,resultado);
		if ((endTime - startTime) >= 1) {
			printf("Segundos: %d\n",segundos);
			segundos++;
			startTime = endTime;
			//Se vuelve a renderizar la luz del sol cada segundo
			printf("\nIntensidad del sol: %.2f\n", getSunIntensity(segundos));
			printf("Luces prendidas?: %d\n", getPublicLightsStatus());
			printf("Frames por segundo: %d\n\n", frames);
			frames = 0;

		}
		glUseProgram(0);

		mainWindow.swapBuffers();

	}
	//engine->drop(); // delete engine
	return 0;
	musica.Stop();
}



void inputkey(bool* keys) 
{
	if (keys[GLFW_KEY_SPACE]) {
		play = !play;
	}
	if (keys[GLFW_KEY_J])
	{
		camera = Camera(glm::vec3(positionP), glm::vec3(0.0f, 2.0f, 0.0f), 0.0f, 0.0f, 5.0f, 0.0f);
	}
	if (keys[GLFW_KEY_K])
	{
		camera = Camera(glm::vec3(0.0, 9.0, 0.0), glm::vec3(0.0f, 2.0f, 0.0f), 90.0f, 0.0f, 10.0f, 1.0f);
	}
	if (keys[GLFW_KEY_O])
	{
		if (lightsOn == 1)
			lightsOn = 0;
		else
			lightsOn = 1;
	}
}
/*
void inputKeyframes(bool* keys)
{
	if (keys[GLFW_KEY_SPACE])
	{
		if(reproduciranimacion<1)
		{
			if (play == false && (FrameIndex > 1))
			{
				resetElements();
				//First Interpolation				
				interpolation();
				play = true;
				playIndex = 0;
				i_curr_steps = 0;
				reproduciranimacion++;
				printf("presiona 0 para habilitar reproducir de nuevo la animaci�n'\n");
				habilitaranimacion = 0;

			}
			else
			{
				play = false;
			}
		}
	}
	if (keys[GLFW_KEY_0])
	{
		if (habilitaranimacion < 1)
		{
			reproduciranimacion = 0;
		}
	}
	
	if (keys[GLFW_KEY_L])
	{
		if (guardoFrame < 1)
		{
			saveFrame();
			printf("movAvion_x es: %f\n", movAvion_x);
			//printf("movAvion_y es: %f\n", movAvion_y);
			printf("presiona P para habilitar guardar otro frame'\n");
			guardoFrame++;
			reinicioFrame = 0;
		}
	}
	if (keys[GLFW_KEY_P])
	{
		if (reinicioFrame < 1)
		{
			guardoFrame=0;
		}
	}
	
	
	if (keys[GLFW_KEY_1])
	{
		if (ciclo < 1)
		{
			//printf("movAvion_x es: %f\n", movAvion_x);
			movAvion_x += 1.0f;
			printf("movAvion_x es: %f\n", movAvion_x);
			ciclo++;
			ciclo2 = 0;
			printf("reinicia con 2\n");
		}
		
	}
	if (keys[GLFW_KEY_2])
	{
		if(ciclo2<1)
		{
		ciclo = 0;
		}
	}

}*/
