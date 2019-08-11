
#include <stdio.h>   
#include <stdlib.h>   /
#include <math.h>    
#if __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <Cg/cg.h>    
#include <Cg/cgGL.h>

#include "rapidjson/document.h"    // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"
#include <conio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using namespace std;
using namespace rapidjson;

static CGcontext   myCgContext;
static CGprofile   myCgVertexProfile,
myCgFragmentProfile;
static CGprogram   myCgVertexProgram,
myCgFragmentProgram;
static CGparameter myCgVertexParam_constantColor;

static const char *myProgramName = "03_uniform_parameter",
*myVertexProgramFileName = "C3E1v_anycolor.cg",
*myVertexProgramName = "C3E1v_anycolor",
*myFragmentProgramFileName = "C2E2f_passthru.cg", 
*myFragmentProgramName = "C2E2f_passthru";

struct key_points {
	int key;
	float x;
	float y;
}points[25];
int count_points = 0;

struct poses_offsets{
	int pose;
	float offx;
	float offy;
}offsets[11];

void initialization() {
	offsets[1].pose = 1;
	offsets[1].offx = -0.78;
	offsets[1].offy = 0.334;

	offsets[2].pose = 2;
	offsets[2].offx = -0.84;
	offsets[2].offy = 0.385;

	offsets[3].pose = 3;
	offsets[3].offx = -0.844;
	offsets[3].offy = 0.375;

	offsets[4].pose = 4;
	offsets[4].offx = -0.864;
	offsets[4].offy = 0.35;

	offsets[5].pose = 5;
	offsets[5].offx = -0.854;
	offsets[5].offy = 0.33;

	offsets[6].pose = 6;
	offsets[6].offx = -0.816;
	offsets[6].offy = 0.338;

	offsets[7].pose = 7;
	offsets[7].offx = -0.85;
	offsets[7].offy = 0.367;

	offsets[8].pose = 8;
	offsets[8].offx = -0.85;
	offsets[8].offy = 0.378;

	offsets[9].pose = 9;
	offsets[9].offx = -0.847;
	offsets[9].offy = 0.34;

	offsets[10].pose = 10;
	offsets[10].offx = -0.85;
	offsets[10].offy = 0.348;
}

static void checkForCgError(const char *situation)
{
	CGerror error;
	const char *string = cgGetLastErrorString(&error);

	if (error != CG_NO_ERROR) {
		printf("%s: %s: %s\n",myProgramName, situation, string);
		if (error == CG_COMPILER_ERROR) {
			printf("%s\n", cgGetLastListing(myCgContext));
		}
		exit(1);
	}
}

/* Forward declared GLUT callbacks registered by main. */
static void display(void);
static void keyboard(unsigned char c, int x, int y);


bool loadOBJ(
	const char * path,
	std::vector < glm::vec3 > & out_vertices
) {
	std::vector< unsigned int > vertexIndices;
	std::vector< glm::vec3 > temp_vertices;

	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}
	while (1) {

		char lineHeader[128];
		glm::vec3 uv;
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			//printf("%f %f %f\n", vertex.x, vertex.y, vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[3];
			int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			//printf("%d %d %d\n", vertexIndex[0], vertexIndex[1], vertexIndex[2]);
			if (matches != 3) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
		}
	}
	//printf("%d\n", vertexIndices.size());
	
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		//printf("%f %f %f\n", vertex.x, vertex.y, vertex.z);
		out_vertices.push_back(vertex);
	}

	//printf("%f %f %f\n", out_vertices);
	fclose(file);
}



static int json_parse()
{
	// 1. Parse a JSON text string to a document.
	printf("Parsing\n");
	
	//pose1
	//char json[] = "{\"0\":[502.712,407.187,0.937373],\"1\":[497.364,423.641,0.973065],\"2\":[478.151,423.608,0.950732],\"3\":[464.407,448.128,0.961858],\"4\":[456.274,472.806,0.902952],\"5\":[521.785,423.614,0.964897],\"6\":[540.976,450.826,0.91976],\"7\":[551.93,480.85,0.888693],\"8\":[497.265,483.536,0.912953],\"9\":[486.246,480.965,0.917543],\"10\":[478.164,530.065,0.909139],\"11\":[475.452,576.425,0.932442],\"12\":[513.543,480.951,0.926025],\"13\":[521.768,529.98,0.945248],\"14\":[524.624,579.08,0.94791],\"15\":[497.283,404.418,0.962984],\"16\":[505.523,404.381,0.974771],\"17\":[491.709,404.435,0.927352],\"18\":[510.941,404.449,0.675791],\"19\":[535.461,587.393,0.780656],\"20\":[540.953,581.886,0.793023],\"21\":[519.148,581.947,0.869269],\"22\":[475.396,584.696,0.787013],\"23\":[467.254,581.933,0.815815],\"24\":[480.833,579.104,0.836572]}";
	
	//pose2
	//char json[] = "{\"0\":[494.588,431.765,0.93518],\"1\":[497.334,448.178,0.962784],\"2\":[478.19,448.15,0.95741],\"3\":[453.543,464.503,0.932522],\"4\":[437.26,486.412,0.928874],\"5\":[519.14,448.181,0.9567],\"6\":[549.182,467.165,0.945613],\"7\":[559.968,491.787,0.911045],\"8\":[494.652,502.754,0.900178],\"9\":[483.671,502.708,0.908945],\"10\":[472.798,551.853,0.947533],\"11\":[472.696,598.183,0.954826],\"12\":[510.986,502.754,0.918391],\"13\":[521.861,551.8,0.918354],\"14\":[532.636,598.164,0.931327],\"15\":[491.835,428.976,0.976241],\"16\":[499.967,426.404,0.954693],\"17\":[488.998,429.085,0.652026],\"18\":[508.168,429.012,0.900712],\"19\":[535.464,606.474,0.766023],\"20\":[543.637,603.662,0.783899],\"21\":[527.357,600.931,0.835575],\"22\":[469.932,606.427,0.789223],\"23\":[464.44,603.666,0.81762],\"24\":[478.101,598.317,0.83825]}";
	
	//pose3
	//char json[] = "{\"0\":[497.36,420.916,0.945927],\"1\":[500.049,445.304,0.968386],\"2\":[480.944,445.339,0.954535],\"3\":[467.126,470.004,0.987531],\"4\":[445.508,497.215,0.956415],\"5\":[521.788,442.751,0.955725],\"6\":[538.169,469.924,0.946767],\"7\":[554.572,497.216,0.914131],\"8\":[500.006,499.975,0.903445],\"9\":[488.971,499.962,0.923315],\"10\":[486.236,546.341,0.902087],\"11\":[483.65,587.414,0.904753],\"12\":[513.663,497.349,0.9452],\"13\":[519.085,546.362,0.951672],\"14\":[527.319,590.053,0.913701],\"15\":[494.555,418.121,0.935527],\"16\":[502.773,418.128,0.953265],\"17\":[491.74,420.871,0.752306],\"18\":[510.889,420.831,0.919342],\"19\":[527.267,598.291,0.727665],\"20\":[535.422,598.227,0.806002],\"21\":[524.607,595.456,0.867682],\"22\":[483.665,600.911,0.771252],\"23\":[475.419,598.235,0.771201],\"24\":[486.415,592.728,0.83857]}";
	
	//pose4
	//char json[] = "{\"0\":[502.76,418.126,0.899353],\"1\":[502.781,434.461,0.931348],\"2\":[483.659,431.802,0.922322],\"3\":[458.992,429.12,0.837712],\"4\":[448.157,429.012,0.855583],\"5\":[521.862,431.857,0.918285],\"6\":[543.712,448.208,0.910818],\"7\":[557.315,469.98,0.907249],\"8\":[505.393,491.762,0.928957],\"9\":[489.135,491.813,0.940645],\"10\":[489.026,535.583,0.908462],\"11\":[489.07,576.445,0.926745],\"12\":[516.402,489.182,0.932547],\"13\":[527.301,535.58,0.942534],\"14\":[530.016,576.343,0.898296],\"15\":[497.35,415.322,0.899427],\"16\":[508.15,415.345,0.917909],\"17\":[491.931,418.066,0.730567],\"18\":[513.585,418.033,0.77061],\"19\":[532.775,587.356,0.741692],\"20\":[538.303,581.978,0.811007],\"21\":[527.247,576.406,0.764185],\"22\":[489.104,584.681,0.732628],\"23\":[480.876,581.989,0.727546],\"24\":[491.769,576.498,0.771297]}";
	
	//pose5
	//char json[] = "{\"0\":[469.973,426.381,0.345541],\"1\":[489.057,442.68,0.800377],\"2\":[491.794,445.36,0.766887],\"3\":[524.55,470.004,0.612964],\"4\":[532.751,491.732,0.616256],\"5\":[483.552,439.894,0.801918],\"6\":[464.481,420.908,0.842297],\"7\":[448.206,401.662,0.827437],\"8\":[510.988,491.819,0.919203],\"9\":[510.858,491.873,0.899175],\"10\":[469.915,530.072,0.917506],\"11\":[489.142,573.78,0.851511],\"12\":[513.728,491.819,0.937895],\"13\":[491.914,540.899,0.871554],\"14\":[546.36,565.511,0.851899],\"15\":[469.949,423.594,0.0984427],\"16\":[472.708,423.561,0.388651],\"17\":[489.075,423.554,0.102412],\"18\":[483.624,426.265,0.511489],\"19\":[530.037,584.63,0.746204],\"20\":[540.879,584.554,0.748681],\"21\":[554.681,565.451,0.795195],\"22\":[529.968,579.26,0.0699909,464.579,584.623,0.770737],\"23\":[467.313,579.231,0.744093],\"24\":[497.264,579.088,0.753991]}";
	
	//pose6
	//char json[] = "{\"0\":[510.814,415.351,0.66452],\"1\":[524.542,428.97,0.886062],\"2\":[532.83,429.14,0.791057],\"3\":[519.063,459.077,0.199623],\"4\":[489.118,431.757,0.174668],\"5\":[513.718,426.373,0.859328],\"6\":[489.138,434.479,0.787672],\"7\":[472.646,431.732,0.553633],\"8\":[524.64,486.355,0.896132],\"9\":[532.754,488.978,0.905772],\"10\":[513.6,529.957,0.941863],\"11\":[524.579,576.403,0.921484],\"12\":[516.423,483.653,0.854493],\"13\":[467.295,489.005,0.89692],\"14\":[478.126,532.749,0.899436],\"15\":[513.534,407.291,0.20677],\"16\":[510.995,409.939,0.713693],\"17\":[521.835,409.917,0.12817],\"18\":[516.473,412.667,0.824139],\"19\":[467.26,546.361,0.689228],\"20\":[470.048,543.758,0.787056],\"21\":[483.565,532.836,0.840528],\"22\":[497.232,587.298,0.760424],\"23\":[500.093,581.909,0.700618],\"24\":[532.688,579.184,0.877618]}";
	
	//pose7
	//char json[] = "{\"0\":[502.737,426.415,0.94573],\"1\":[502.759,442.759,0.951001],\"2\":[480.898,440.025,0.913823],\"3\":[445.394,434.447,0.858227],\"4\":[448.23,412.618,0.794645],\"5\":[524.516,445.344,0.933619],\"6\":[557.366,456.274,0.904071],\"7\":[557.231,478.21,0.874899],\"8\":[497.31,500.027,0.901018],\"9\":[486.243,499.982,0.905934],\"10\":[467.133,546.474,0.904522],\"11\":[448.092,592.808,0.872879],\"12\":[513.631,499.984,0.909603],\"13\":[535.466,543.724,0.905617],\"14\":[557.375,587.279,0.862386],\"15\":[499.957,423.654,0.964594],\"16\":[508.129,426.198,0.974714],\"17\":[494.47,423.685,0.698864],\"18\":[513.621,426.318,0.849182],\"19\":[560.067,598.174,0.725996],\"20\":[568.273,592.849,0.737807],\"21\":[557.4,590.077,0.751791],\"22\":[448.152,603.671,0.699219],\"23\":[439.993,600.959,0.696567],\"24\":[448.139,595.472,0.731342]}";
	
	//pose8
	//char json[] = "{\"0\":[519.127,461.698,0.821283],\"1\":[500.029,472.758,0.855282],\"2\":[502.653,470.014,0.870135],\"3\":[519.149,489.064,0.822576],\"4\":[543.73,513.664,0.89539],\"5\":[499.98,475.392,0.826199],\"6\":[524.51,478.113,0.610418],\"7\":[549.068,461.851,0.684722],\"8\":[478.215,532.701,0.901224],\"9\":[470.084,532.703,0.981727],\"10\":[494.582,576.446,0.856973],\"11\":[439.927,592.871,0.893326],\"12\":[486.347,532.696,0.866084],\"13\":[535.596,549.078,0.919425],\"14\":[519.112,598.323,0.945003],\"15\":[516.438,453.662,0.862274],\"16\":[519.137,453.626,0.310173],\"17\":[505.445,453.655,0.873629],\"18\":[0,0,0],\"19\":[546.361,606.463,0.789193],\"20\":[538.299,601.076,0.755094],\"21\":[513.658,601.01,0.723018],\"22\":[445.348,614.659,0.752217],\"23\":[437.21,609.252,0.744789],\"24\":[431.699,590.073,0.763799]}";
	
	//pose9
	//char json[] = "{\"0\":[497.271,467.212,0.928141],\"1\":[500.056,480.941,0.92484],\"2\":[480.948,480.899,0.914096],\"3\":[464.442,450.857,0.918256],\"4\":[448.226,423.538,0.878041],\"5\":[521.77,478.202,0.903668],\"6\":[540.896,450.89,0.89176],\"7\":[554.6,426.333,0.86178],\"8\":[505.449,540.913,0.87752],\"9\":[489.157,543.69,0.861135],\"10\":[472.672,546.462,0.707465],\"11\":[489.153,581.901,0.664042],\"12\":[516.404,538.187,0.879181],\"13\":[543.669,527.207,0.881014],\"14\":[535.563,576.436,0.884203],\"15\":[491.899,464.454,0.912036],\"16\":[502.689,464.41,0.919082],\"17\":[486.429,467.28,0.432375],\"18\":[510.985,467.189,0.901],\"19\":[546.415,592.737,0.771377],\"20\":[549.115,587.327,0.727329],\"21\":[530.064,579.217,0.788802],\"22\":[497.299,592.834,0.470364],\"23\":[489.123,595.486,0.407147],\"24\":[494.478,584.613,0.539614]}";
	
	//pose10
	char json[] = "{\"0\":[513.722,461.8,0.781999],\"1\":[510.964,467.146,0.934291],\"2\":[486.376,467.169,0.892151],\"3\":[464.564,491.803,0.918278],\"4\":[486.313,513.702,0.81619],\"5\":[535.558,464.467,0.883487],\"6\":[551.923,491.733,0.899989],\"7\":[541,519.117,0.815618],\"8\":[505.517,494.632,0.842991],\"9\":[489.007,497.239,0.873855],\"10\":[475.381,538.142,0.88754],\"11\":[453.607,587.397,0.86359],\"12\":[521.842,494.578,0.840706],\"13\":[546.419,532.734,0.908096],\"14\":[565.57,581.932,0.930882],\"15\":[510.94,456.285,0.781218],\"16\":[519.074,456.362,0.722178],\"17\":[502.769,450.935,0.763882],\"18\":[527.226,453.597,0.428379],\"19\":[576.499,595.541,0.845548],\"20\":[581.966,590.095,0.814312],\"21\":[562.779,590.075,0.831653],\"22\":[459.07,600.982,0.605614],\"23\":[448.16,600.916,0.622795],\"24\":[453.532,590.035,0.732911]}";

	Document document; 
	std::cout << json << "\n";
#if 0
	// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
	if (document.Parse(json).HasParseError()) {
		
		return 1;
	}
#else
	// In-situ parsing, decode strings directly in the source string. Source must be string.
	char buffer[sizeof(json)];
	//char buffer1[sizeof(json3)];
	memcpy(buffer, json, sizeof(json));
	if (document.ParseInsitu(buffer).HasParseError())
	{
		return 1;
	}
#endif
	// 2. Access values in document. 
	assert(document.IsObject());

	{
		const Value& zero = document["0"]; 
		assert(zero.IsArray());

		points[count_points].key = count_points;
		points[count_points].x = zero[0].GetFloat();
		points[count_points].y = zero[1].GetFloat();
	

	}

	{
		const Value& one = document["1"];
		assert(one.IsArray());

		points[count_points].key = count_points;
		points[count_points].x = one[0].GetFloat();
		points[count_points].y = one[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& two = document["2"]; 
		assert(two.IsArray());
		int size = two.Size();

		points[count_points].key = count_points;
		points[count_points].x = two[0].GetFloat();
		points[count_points].y = two[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& three = document["3"];
		assert(three.IsArray());
		int size = three.Size();

		points[count_points].key = count_points;
		points[count_points].x = three[0].GetFloat();
		points[count_points].y = three[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& four = document["4"]; 
		assert(four.IsArray());
		int size = four.Size();

		points[count_points].key = count_points;
		points[count_points].x = four[0].GetFloat();
		points[count_points].y = four[1].GetFloat();
		count_points++;
		

	}

	{

		const Value& five = document["5"]; 
		assert(five.IsArray());
		int size = five.Size();

		points[count_points].key = count_points;
		points[count_points].x = five[0].GetFloat();
		points[count_points].y = five[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& six = document["6"]; 
		assert(six.IsArray());
		int size = six.Size();

		points[count_points].key = count_points;
		points[count_points].x = six[0].GetFloat();
		points[count_points].y = six[1].GetFloat();
		count_points++;

	}

	{

		const Value& seven = document["7"]; 
		assert(seven.IsArray());
		int size = seven.Size();

		points[count_points].key = count_points;
		points[count_points].x = seven[0].GetFloat();
		points[count_points].y = seven[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& eight = document["8"];
		assert(eight.IsArray());
		int size = eight.Size();

		points[count_points].key = count_points;
		points[count_points].x = eight[0].GetFloat();
		points[count_points].y = eight[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& nine = document["9"]; 
		assert(nine.IsArray());
		int size = nine.Size();

		points[count_points].key = count_points;
		points[count_points].x = nine[0].GetFloat();
		points[count_points].y = nine[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& ten = document["10"]; 
		assert(ten.IsArray());
		int size = ten.Size();

		points[count_points].key = count_points;
		points[count_points].x = ten[0].GetFloat();
		points[count_points].y = ten[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& eleven = document["11"];
		assert(eleven.IsArray());
		int size = eleven.Size();

		points[count_points].key = count_points;
		points[count_points].x = eleven[0].GetFloat();
		points[count_points].y = eleven[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& twelve = document["12"]; 
		assert(twelve.IsArray());
		int size = twelve.Size();

		points[count_points].key = count_points;
		points[count_points].x = twelve[0].GetFloat();
		points[count_points].y = twelve[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& thirteen = document["13"]; 
		assert(thirteen.IsArray());
		int size = thirteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = thirteen[0].GetFloat();
		points[count_points].y = thirteen[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& fourteen = document["14"]; 
		assert(fourteen.IsArray());
		int size = fourteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = fourteen[0].GetFloat();
		points[count_points].y = fourteen[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& fifteen = document["15"]; 
		assert(fifteen.IsArray());
		int size = fifteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = fifteen[0].GetFloat();
		points[count_points].y = fifteen[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& sixteen = document["16"];
		int size = sixteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = sixteen[0].GetFloat();
		points[count_points].y = sixteen[1].GetFloat();
		count_points++;
		
	
	}

	{

		const Value& seventeen = document["17"];
		assert(seventeen.IsArray());
		int size = seventeen.Size();

		points[count_points].key = count_points;
		points[count_points].x = seventeen[0].GetFloat();
		points[count_points].y = seventeen[1].GetFloat();
		count_points++;
	}

	{

		const Value& eighteen = document["18"]; 
		assert(eighteen.IsArray());
		int size = eighteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = eighteen[0].GetFloat();
		points[count_points].y = eighteen[1].GetFloat();
		count_points++;
	}

	{

		const Value& nineteen = document["19"];
		assert(nineteen.IsArray());
		int size = nineteen.Size();

		points[count_points].key = count_points;
		points[count_points].x = nineteen[0].GetFloat();
		points[count_points].y = nineteen[1].GetFloat();
		count_points++;
	
	}

	{

		const Value& twenty = document["20"]; 
		assert(twenty.IsArray());
		int size = twenty.Size();

		points[count_points].key = count_points;
		points[count_points].x = twenty[0].GetFloat();
		points[count_points].y = twenty[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& twentyone = document["21"]; 
		assert(twentyone.IsArray());
		int size = twentyone.Size();

		points[count_points].key = count_points;
		points[count_points].x = twentyone[0].GetFloat();
		points[count_points].y = twentyone[1].GetFloat();
		count_points++;
		
	}

	{

		const Value& twentytwo = document["22"]; 
		assert(twentytwo.IsArray());
		int size = twentytwo.Size();

		points[count_points].key = count_points;
		points[count_points].x = twentytwo[0].GetFloat();
		points[count_points].y = twentytwo[1].GetFloat();
		count_points++;
		

	}

	{

		const Value& twentythree = document["23"];
		assert(twentythree.IsArray());
		int size = twentythree.Size();

		points[count_points].key = count_points;
		points[count_points].x = twentythree[0].GetFloat();
		points[count_points].y = twentythree[1].GetFloat();
		count_points++;

	}

	{

		const Value& twentyfour = document["24"]; 
		assert(twentyfour.IsArray());
		int size = twentyfour.Size();

		points[count_points].key = count_points;
		points[count_points].x = twentyfour[0].GetFloat();
		points[count_points].y = twentyfour[1].GetFloat();
		count_points++;

	}

	return 3;
}

int main(int argc, char **argv)
{
	glutInitWindowSize(700, 700);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInit(&argc, argv);

	glutCreateWindow(myProgramName);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glClearColor(0.1, 0.3, 0.6, 0.0);  /* Blue background */

	myCgContext = cgCreateContext();
	checkForCgError("creating context");
	cgGLSetDebugMode(CG_FALSE);
	cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);

	myCgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	cgGLSetOptimalOptions(myCgVertexProfile);
	checkForCgError("selecting vertex profile");

	myCgVertexProgram =
		cgCreateProgramFromFile(
			myCgContext,              /* Cg runtime context */
			CG_SOURCE,                /* Program in human-readable form */
			myVertexProgramFileName,  /* Name of file containing program */
			myCgVertexProfile,        /* Profile: OpenGL ARB vertex program */
			myVertexProgramName,      /* Entry function name */
			NULL);                    /* No extra compiler options */
	checkForCgError("creating vertex program from file");
	cgGLLoadProgram(myCgVertexProgram);
	checkForCgError("loading vertex program");

	myCgVertexParam_constantColor =
		cgGetNamedParameter(myCgVertexProgram, "constantColor"/*"modelViewProj"*/);
	checkForCgError("could not get constantColor parameter");

	myCgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(myCgFragmentProfile);
	checkForCgError("selecting fragment profile");

	myCgFragmentProgram =
		cgCreateProgramFromFile(
			myCgContext,                /* Cg runtime context */
			CG_SOURCE,                  /* Program in human-readable form */
			myFragmentProgramFileName,  /* Name of file containing program */
			myCgFragmentProfile,        /* Profile: OpenGL ARB vertex program */
			myFragmentProgramName,      /* Entry function name */
			NULL);                      /* No extra compiler options */
	checkForCgError("creating fragment program from file");
	cgGLLoadProgram(myCgFragmentProgram);
	checkForCgError("loading fragment program");

	glutMainLoop();
	return 0;
}

static void display(void)
{
	std::vector< glm::vec3 > vertices;
	bool res = loadOBJ("C:\\Users\\Kusuma\\Desktop\\Computer Graphics Assignments\\autorigging_project\\poses\\pose10.obj", vertices);
	int pose = 10;
	float maxx = -1;
	float maxy = -1;
	float minx = 100;
	float miny = 100;
	float sumx = 0;
	float sumy = 0;
	float sumz = 0;
	for (int i = 0; i < vertices.size(); i++) {
		sumx += vertices[i].x;
		sumy += vertices[i].y;
		sumz += vertices[i].z;
		if (vertices[i].x > maxx) {
			maxx = vertices[i].x / 4;
		}
		if (vertices[i].y > maxy) {
			maxy = vertices[i].y / 4;
		}
		if (vertices[i].x < minx) {
			minx = vertices[i].x / 4;
		}
		if (vertices[i].y < miny) {
			miny = vertices[i].y / 4;
		}
	}
	float avgx = sumx / vertices.size();
	float avgy = sumy / vertices.size();
	float avgz = sumz / vertices.size();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	cgGLBindProgram(myCgVertexProgram);
	checkForCgError("binding vertex program");

	cgGLEnableProfile(myCgVertexProfile);
	checkForCgError("enabling vertex profile");

	int j = json_parse();
	if (j == 3) {
		printf("Valid\n");
	}

	cgGLBindProgram(myCgFragmentProgram);
	checkForCgError("binding fragment program");

	cgGLEnableProfile(myCgFragmentProfile);
	checkForCgError("enabling fragment profile");
	
	float pminx = 1000;
	float pminy = 1000;
	float pmaxx = -1;
	float pmaxy = -1;
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < vertices.size(); i += 3) {
		cgSetParameter4f(myCgVertexParam_constantColor, 1.0, 0.8, 0.0, 0.6);
		cgUpdateProgramParameters(myCgVertexProgram);
		glBegin(GL_TRIANGLES);
		glVertex3f(vertices[i].x/4, vertices[i].y/4, vertices[i].z/4);
		glVertex3f(vertices[i + 1].x/4, vertices[i + 1].y/4, vertices[i + 1].z/4);
		glVertex3f(vertices[i + 2].x/4, vertices[i + 2].y/4, vertices[i + 2].z/4);
		glEnd();
	}

	initialization();
	
	//printf("%f\t %f\n", offsets[pose].offx, offsets[pose].offy);
	// rendering skeleton
	for (int p = 0; p <= count_points; p++)
	{
		if (points[p].x > pmaxx) {
			pmaxx = points[p].x / 500;
		}
		if (points[p].y > pmaxy) {
			pmaxy = points[p].y / 500;
		}
		if (points[p].x < pminx) {
			pminx = points[p].x / 500;
		}
		if (points[p].y < pminy) {
			pminy = points[p].y / 500;
		}
		
		cgSetParameter4f(myCgVertexParam_constantColor, 0.0, 0.0, 0.0, 1.0);
		cgUpdateProgramParameters(myCgVertexProgram);
		glBegin(GL_POINTS);
		glVertex2f(((points[p].x /600) + offsets[pose].offx), ((1-points[p].y/440) + offsets[pose].offy));
		glEnd();
	}

	cgSetParameter4f(myCgVertexParam_constantColor, 1.0, 1.0, 0.0, 1.0);
	//printf("%d", count_points);

	glBegin(GL_LINES);
	glVertex3f(((points[0].x / 600) + offsets[pose].offx), ((1 - points[0].y / 440) + offsets[pose].offy),avgz);
	glVertex3f(((points[14].x / 600) + offsets[pose].offx), ((1 - points[14].y / 440) + offsets[pose].offy),avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(((points[14].x / 600) + offsets[pose].offx), ((1 - points[14].y / 440) + offsets[pose].offy),avgz);
	glVertex3f(((points[16].x / 600) + offsets[pose].offx), ((1 - points[16].y / 440) + offsets[pose].offy),avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[0].x / 600) + offsets[pose].offx, (1 - points[0].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[15].x / 600) + offsets[pose].offx, (1 - points[15].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[15].x / 600) + offsets[pose].offx, (1 - points[15].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[17].x / 600) + offsets[pose].offx, (1 - points[17].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[0].x / 600) + offsets[pose].offx, (1 - points[0].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[1].x / 600) + offsets[pose].offx, (1 - points[1].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[1].x / 600) + offsets[pose].offx, (1 - points[1].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[2].x / 600) + offsets[pose].offx, (1 - points[2].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[2].x / 600) + offsets[pose].offx, (1 - points[2].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[3].x / 600) + offsets[pose].offx, (1 - points[3].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[0].x / 600) + offsets[pose].offx, (1 - points[0].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[4].x / 600) + offsets[pose].offx, (1 - points[4].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[4].x / 600) + offsets[pose].offx, (1 - points[4].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[5].x / 600) + offsets[pose].offx, (1 - points[5].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[5].x / 600) + offsets[pose].offx, (1 - points[5].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[6].x / 600) + offsets[pose].offx, (1 - points[6].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[0].x / 600) + offsets[pose].offx, (1 - points[0].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[7].x / 600) + offsets[pose].offx, (1 - points[7].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[7].x / 600) + offsets[pose].offx, (1 - points[7].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[8].x / 600) + offsets[pose].offx, (1 - points[8].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[8].x / 600) + offsets[pose].offx, (1 - points[8].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[9].x / 600) + offsets[pose].offx, (1 - points[9].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[9].x / 600) + offsets[pose].offx, (1 - points[9].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[10].x / 600) + offsets[pose].offx, (1 - points[10].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[10].x / 600) + offsets[pose].offx, (1 - points[10].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[23].x / 600) + offsets[pose].offx, (1 - points[23].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[10].x / 600) + offsets[pose].offx, (1 - points[10].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[21].x / 600) + offsets[pose].offx, (1 - points[21].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[21].x / 600) + offsets[pose].offx, (1 - points[21].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[22].x / 600) + offsets[pose].offx, (1 - points[22].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[7].x / 600) + offsets[pose].offx, (1 - points[7].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[11].x / 600) + offsets[pose].offx, (1 - points[11].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[11].x / 600) + offsets[pose].offx, (1 - points[11].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[12].x / 600) + offsets[pose].offx, (1 - points[12].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[12].x / 600) + offsets[pose].offx, (1 - points[12].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[13].x / 600) + offsets[pose].offx, (1 - points[13].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[13].x / 600) + offsets[pose].offx, (1 - points[13].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[20].x / 600) + offsets[pose].offx, (1 - points[20].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[13].x / 600) + offsets[pose].offx, (1 - points[13].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[18].x / 600) + offsets[pose].offx, (1 - points[18].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f((points[18].x / 600) + offsets[pose].offx, (1 - points[18].y / 440) + offsets[pose].offy,avgz);
	glVertex3f((points[19].x / 600) + offsets[pose].offx, (1 - points[19].y / 440) + offsets[pose].offy,avgz);
	glEnd();
	
	cgGLDisableProfile(myCgVertexProfile);
	checkForCgError("disabling vertex profile");

	cgGLDisableProfile(myCgFragmentProfile);
	checkForCgError("disabling fragment profile");

	glutSwapBuffers();
}

static void keyboard(unsigned char c, int x, int y)
{
	switch (c) {
	case 27:  
		cgDestroyProgram(myCgVertexProgram);
		cgDestroyProgram(myCgFragmentProgram);
		cgDestroyContext(myCgContext);
		exit(0);
		break;
	}
}
