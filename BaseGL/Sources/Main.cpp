// ----------------------------------------------
// Base code for practical computer graphics 
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "Sampling.cpp"
#include "Texture.cpp"
#include "Render.cpp"

static const std::string SHADER_PATH ("Resources/Shaders/");
static const std::vector<std::string> SKYBOX_TEXTURE = {
    "Resources/skybox/right.jpg",
    "Resources/skybox/left.jpg",
    "Resources/skybox/top.jpg",
    "Resources/skybox/bottom.jpg",
    "Resources/skybox/back.jpg",
    "Resources/skybox/front.jpg",
};

static const std::string DEFAULT_MESH_FILENAME ("Resources/Models/face.off");

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram>
    geometryShader,
    lightingShader,
    directShader,
    directBlurShader,
    indirectShader,
    indirectBlurShader,
    mixerShader,
    skyboxShader;

int draw_buffer = 8;

// Camera control variables
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

ostream& operator<<(ostream& os, const glm::vec3 v) {
    return os << '{' << v.x << ',' << v.y << ',' << v.z << '}';
}
ostream& operator<<(ostream& os, const glm::mat4 m) {
    for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
        os << m[i][j] << " \n"[j==3];
    return os << endl;
}

void clear ();

void printHelp () {
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
   			  << "    * ESC: quit the program" << std::endl;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window. 
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
    return; // NOT supported
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
	// ssaoShader->set("windowSize", glm::vec2(width, height));
    // TODO resize VBO :(
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
	    if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose (windowPtr, true);
        else if (key == GLFW_KEY_H)
            printHelp();
        else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
            draw_buffer = key - GLFW_KEY_0;
    }
	else if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        } 
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Computer Graphics - Practical Assignment", nullptr, nullptr);
	if (!windowPtr) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control 
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}

GLuint gBuffer, gPositionDepth, gNormal, gAlbedo, noiseTex, skyboxMap,
       ssdoFBO, ssdoBlurFBO, ssdoLightingFBO, ssdoIndirectFBO, ssdoIndirectBlurFBO, skyboxFBO,
       ssdoTex, ssdoBlurTex, ssdoLightingTex, ssdoIndirectTex, ssdoIndirectBlurTex, skyboxTex;

void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress)) 
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
    glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	glClearDepthf(1); // specify the background color, used any time the framebuffer is cleared
	// Loads and compile the programmable shader pipeline
	try {
        bool DEBUG = true;
		geometryShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "geometry.vs",
             SHADER_PATH + "geometry.fs");
        if (DEBUG) cout << "geometry OK\n";
		lightingShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "lighting.fs");
        if (DEBUG) cout << "lighting OK\n";
		directShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "direct.fs");
        if (DEBUG) cout << "direct OK\n";
		directBlurShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "blur.fs");
        if (DEBUG) cout << "blur OK\n";
		indirectShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "indirect.fs");
        if (DEBUG) cout << "indirect OK\n";
		indirectBlurShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "blur.fs");
        if (DEBUG) cout << "blur OK\n";
		skyboxShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "skybox.vs",
             SHADER_PATH + "skybox.fs");
        if (DEBUG) cout << "skybox OK\n";
		mixerShader = ShaderProgram::genBasicShaderProgram
            (SHADER_PATH + "pass.vs",
             SHADER_PATH + "mixer.fs");
        if (DEBUG) cout << "mixer OK\n";
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}

	int SCR_WIDTH, SCR_HEIGHT;
	glfwGetWindowSize (windowPtr, &SCR_WIDTH, &SCR_HEIGHT);
    printf("window size: %d %d\n", SCR_WIDTH, SCR_HEIGHT);

    auto kernel = generateKernel(64);
    directShader->use();
    directShader->set("samples", kernel);
    indirectShader->use();
    indirectShader->set("samples", kernel);

    auto noise = generateNoise(16);
    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &noise[0]); // 32 bits
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    skyboxMap = loadCubemap(SKYBOX_TEXTURE);

    // samplers

    lightingShader->use();
    lightingShader->set("gPositionDepth", 0);
    lightingShader->set("gNormal", 1);
    lightingShader->set("gAlbedo", 2);
    directShader->use();
    directShader->set("gPositionDepth", 0);
    directShader->set("gNormal", 1);
    directShader->set("texNoise", 2);
    directShader->set("skybox", 3);
    directBlurShader->use();
    directBlurShader->set("tex", 0);
    indirectShader->use();
    indirectShader->set("gPositionDepth", 0);
    indirectShader->set("gNormal", 1);
    indirectShader->set("texNoise", 2);
    indirectShader->set("texLighting", 3);
    indirectBlurShader->use();
    indirectBlurShader->set("tex", 0);
    skyboxShader->use();
    skyboxShader->set("skybox", 0);
    mixerShader->use();
    mixerShader->set("gPositionDepth", 0);
    mixerShader->set("gNormal", 1);
    mixerShader->set("ssdo", 2);
    mixerShader->set("ssdoBlur", 3);
    mixerShader->set("texLighting", 4);
    mixerShader->set("texIndirectLight", 5);
    mixerShader->set("texIndirectLightBlur", 6);
    mixerShader->set("texSkybox", 7);

    // FBOs

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer); // geometry
        // - Position + linear depth color buffer
        glGenTextures(1, &gPositionDepth);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionDepth, 0);
        // - Normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
        // - Albedo color buffer
        glGenTextures(1, &gAlbedo);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
        // - Create and attach depth buffer (renderbuffer)
        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // - Finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "GBuffer Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint *FBOs[] = {
        &ssdoFBO,
        &ssdoBlurFBO,
        &ssdoLightingFBO,
        &ssdoIndirectFBO,
        &ssdoIndirectBlurFBO,
        &skyboxFBO,
    };
    GLuint *Texs[] = {
        &ssdoTex,
        &ssdoBlurTex,
        &ssdoLightingTex,
        &ssdoIndirectTex,
        &ssdoIndirectBlurTex,
        &skyboxTex,
    };
    for (int i=0; i<6; i++) {
        auto FBO = FBOs[i];
        auto Tex = Texs[i];
        glGenFramebuffers(1, FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, *FBO);
        glGenTextures(1, Tex);
        glBindTexture(GL_TEXTURE_2D, *Tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *Tex, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
}

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	
	// Mesh
	meshPtr = std::make_shared<Mesh> ();
	try {
		MeshLoader::loadOFF (meshFilename, meshPtr);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}
	meshPtr->standardize();
	meshPtr->init ();

	// Adjust the camera to the actual mesh
	cameraPtr->setTranslation (glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);

    geometryShader->use();
    geometryShader->set("NEAR", meshScale / 100);
    geometryShader->set("FAR", 6 * meshScale);
}

void init (const std::string & meshFilename) {
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initScene (meshFilename); // Actual scene to render
}

void clear () {
	cameraPtr.reset ();
	meshPtr.reset ();
	geometryShader.reset ();
	directShader.reset ();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}


// The main rendering call
void render () {
    glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
    glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Geometry
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        geometryShader->use();
        geometryShader->set ("projectionMat", projectionMatrix);
        for (auto mesh: {meshPtr}) { // render meshes
            glm::mat4 modelMatrix = mesh->computeTransformMatrix ();
            glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
            glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
            geometryShader->set ("modelViewMat", modelViewMatrix);
            geometryShader->set ("normalMat", glm::mat3(normalMatrix) );
            mesh->render ();
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Phong shading
    glBindFramebuffer(GL_FRAMEBUFFER, ssdoLightingFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightingShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glm::vec4 lightPos = glm::vec4(0,0,5, 1.);
        glm::vec3 lightColor = {.8, .8, .6};
        auto lightPosView = glm::vec3(viewMatrix * lightPos);
        const GLfloat constant = 1.0;
        const GLfloat linear = 0.09;
        const GLfloat quadratic = 0.032;
        lightingShader->set("light.Position", lightPosView);
        lightingShader->set("light.Color", lightColor);
        lightingShader->set("light.Linear", linear);
        lightingShader->set("light.Quadratic", quadratic);
        renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // SSDO Direct
    glBindFramebuffer(GL_FRAMEBUFFER, ssdoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        directShader->use();
        directShader->set ("projectionMat", projectionMatrix);
        directShader->set ("iViewMat", glm::inverse(viewMatrix));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxMap);
        renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSDO Blur
    glBindFramebuffer(GL_FRAMEBUFFER, ssdoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        directBlurShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssdoTex);
        renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // SSDO Indirect
    glBindFramebuffer(GL_FRAMEBUFFER, ssdoIndirectFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        indirectShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssdoLightingTex);
        // Send kernel + rotation 
        indirectShader->set("projectionMat", projectionMatrix);
        renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSDO Indirect Blur
    glBindFramebuffer(GL_FRAMEBUFFER, ssdoIndirectBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        indirectBlurShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssdoIndirectTex);
        renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Skybox
    glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL);
        skyboxShader->use();
        auto viewM2 = glm::mat4(glm::mat3(viewMatrix)); // no translation
        skyboxShader->set("viewMat", viewM2);
        skyboxShader->set("projectionMat", projectionMatrix);
        // skybox cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxMap);
        renderCube();
        glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 7. Accumulate Light pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mixerShader->use();
    mixerShader->set("mode", draw_buffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionDepth);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ssdoTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssdoBlurTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ssdoLightingTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ssdoIndirectTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ssdoIndirectBlurTex);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, skyboxTex);
    renderQuad();
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->
	
}

void usage (const char * command) {
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv) {
	if (argc > 2)
		usage (argv[0]);
	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)

	while (!glfwWindowShouldClose (windowPtr)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;
}

