#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "cLoadModels.h"
#include "FlyCam.h"
#include "cShaderCompiler.h"
#include "cVAOManager.h"
#include "cScene.h"
#include "IPlayer.h"
#include "cPlayer.h"
#include "cAiEnemy.h"
#include "cLightManager.h"
#include "cLightMover.h"
#include "cPhysicsUpdated.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

float RandomFloat(float min, float max) {
    float range = max - min;
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / range);
}

int RandomInt(int min, int max) {
    return min + rand() % (max - min + 1); // Including max
}

void GenerateRandomCoordinates(float& x, float& y, float& z) {
    x = RandomFloat(-3000.0f, 3000.0f);
    y = RandomFloat(-6000.0f, 6000.0f);
    z = RandomFloat(-16000.0f, 16000.0f);       // Range taken from screenshots of camLocation
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (3.3 core profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Refer cShaderCompiler class for more info.
    cShaderCompiler shader;
    GLuint shaderProgram = shader.CompileShader();

    // Import Scene
    cScene scene;
    scene.CreateScene("D:/Graphics1/PhysicsMidterm/Question1/sceneFileWithNormals.txt");
    //scene.ExportMaterial(shaderProgram, scene.numberOfMeshesToLoad);            // Considering number of materials = number of meshes to load

    GLuint VAO, VBO;
    cVAOManager VAOManager;
    
    int numberOfVerticesToRenderForAllModels = 0;

    cLightManager lightManager;
    lightManager.LoadLights("D:/Graphics1/PhysicsMidterm/Question1/lightsFile.txt");
    // Camera Initialization
    FlyCam flyCam(800, 600, glm::vec3(-29794.0f, -624.0f, -85037.0f), 90.0f);
    flyCam.camSpeed = 1000.0f;

    cLightMover lightMover(lightManager, flyCam, 2);


    float deltaTime = 0;
    float startTime, endTime;

    for (int modelIndex = 0; modelIndex != scene.numberOfMeshesToLoad; modelIndex++) {
        scene.pModels[modelIndex].pTransformedVertices = new cLoadModels::sTransformedVertex[scene.pModels[modelIndex].numberOfVertices];
    
        glm::mat4 model = scene.pModels[modelIndex].CreateModelMatrix(shaderProgram, scene.pModels[modelIndex]);      // Creation of model matrix with arguements passed in sceneFile.txt
        scene.pModels[modelIndex].GenerateTransformedVertices(model);
    }   // Used for initializing the pTransformedVertices, Nothing new xD

    // Starting physics
    cPhysicsUpdated physicsEngine(scene);
    //physicsEngine.StartPhysics(scene);

    startTime = glfwGetTime();
    //cPlayer bunny(scene.pModels[0]);
    //bunny.SetSpeed(0.06f);

    //cPlayer dragon(scene.pModels[2]);
    //dragon.SetSpeed(0.06f);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    std::vector<cPlayer*> asteroids;

    for (int i = 2; i != scene.pModels.size(); i++) {
        cPlayer asteroid(scene.pModels[i]);
        asteroids.push_back(&asteroid);
    }

    int randomNumber = 2;
    float x, y, z;  // Random coordinates for asteroids
    int randomIndex = 0;         // so that I can get random asteroids to collide with spaceship

    // Render loop
    while (!glfwWindowShouldClose(window)) {

        glm::vec3 collisionPoint;

        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        glfwGetWindowSize(window, &flyCam.camWidth, &flyCam.camHeight);

        std::string cameraPositionInTitle = "Camera Location: (" + to_string(flyCam.camLocation.x) + ", " + to_string(flyCam.camLocation.y) + ", " + to_string(flyCam.camLocation.z) + ") Camera Speed: " + to_string(flyCam.camSpeed) + " Camera Roll: " + to_string(flyCam.camRoll) + " Cam yaw: " + to_string(flyCam.camYaw) + " Cam pitch: " + to_string(flyCam.camPitch);
        glfwSetWindowTitle(window, cameraPositionInTitle.c_str());


        VAO = VAOManager.BindVAOVBO(VBO, scene.pModels.size(), scene.pModels);
        for (int index = 0; index != scene.pModels.size(); index++) {
            numberOfVerticesToRenderForAllModels += scene.pModels[index].numberOfVerticesToRender;
        }               // Moved here because I want multiple models and number of meshes to load is variable!

        asteroids.resize(scene.pModels.size() - 1);


        endTime = glfwGetTime();
        deltaTime = endTime - startTime;
        //std::cout << deltaTime << " Time passed" << std::endl;
        

        //flyCam.camControls(window);
        flyCam.cameraMatrix(45.0f, 0.1f, 1000000.0f, shaderProgram, "camMatrix", window);

        // Rendering commands here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glEnable(GL_DEPTH_TEST);   // Enable depth testing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader program and draw the triangle
        glUseProgram(shaderProgram);

        //lightMover.MoveForward();
        //lightMover.MoveUp();
        //lightMover.MoveRight();

        // ------------------------------------------------------------------------------------------------------------------------------
        // You can call movement functions from light mover class for lights here now and then call turn on lights function of light manager
        lightManager.TurnOnLights(shaderProgram, 2);
        // ------------------------------------------------------------------------------------------------------------------------------
        // You can create player objects here and make them move from here
        //bunny.MoveBackward();

        //dragon.MoveForward();
        // ------------------------------------------------------------------------------------------------------------------------------

        glBindVertexArray(VAO);
        shader.SetSceneView(window);        // Press 1, 2 or 3 for different viewModes like wireframe, fill or point
        
        
        randomNumber = rand() % 7 + 2;
            
        //if (deltaTime == 1.1 * deltaTime) {
        //}

        for (int index = 0; index != 1; index++) {

            glm::mat4 model = scene.pModels[index].CreateModelMatrix(shaderProgram, scene.pModels[index]);      // Creation of model matrix with arguements passed in sceneFile.txt
            scene.pModels[index].GenerateTransformedVertices(model);            // this is here because collisions want transformed vertices

            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, scene.pModels[index].numberOfVerticesToRender);
        }

        
        
        int offset = scene.pModels[1].numberOfVerticesToRender + scene.pModels[0].numberOfVerticesToRender;
        for (int index = 2; index != scene.pModels.size(); index++) {

            glm::mat4 model = scene.pModels[index].CreateModelMatrix(shaderProgram, scene.pModels[index]);      // Creation of model matrix with arguements passed in sceneFile.txt
            scene.pModels[index].GenerateTransformedVertices(model);            // this is here because collisions want transformed vertices

            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, offset, scene.pModels[index].numberOfVerticesToRender);
            offset += scene.pModels[index].numberOfVerticesToRender;
        }

        if (deltaTime >= 0.1f) {
            startTime = endTime;
            GenerateRandomCoordinates(x, y, z);

            scene.pModels[randomNumber].pMeshTransform.x = x - 65915;
            scene.pModels[randomNumber].pMeshTransform.y = y;
            scene.pModels[randomNumber].pMeshTransform.z = z;

            scene.pModels.push_back(scene.pModels[randomNumber]);

            cPlayer asteroid(scene.pModels[randomNumber]);
            //asteroid.SetSpeed(1000.0f);
            //asteroid.MoveRight();
            asteroids.push_back(&asteroid);
        }

        randomIndex = RandomInt(0, asteroids.size() - 1);
        for (int i = 2; i != scene.pModels.size(); i++) {

            if (randomIndex % 2 == 0) {
                scene.pModels[i].pMeshTransform.x += 100.0f;
            }
            else {
                scene.pModels[i].pMeshTransform.x += 300.0f;
            }
            //asteroids[i]->MoveRight();
            //asteroids[i]->SetSpeed(1000.0f);
        }

        if (physicsEngine.CheckCollision(shaderProgram, scene)) {
            for (int i = 1; i < scene.pModels.size(); ++i) { // Start from 1 to avoid comparing model 0 with itself
                if (physicsEngine.CheckAABBCollision(physicsEngine.pAABB[0], physicsEngine.pAABB[i])) {
                    cPhysicsUpdated::sAABBAABB_Collision collision;
                    collision.collisionPoint = physicsEngine.GetAABBCollisionPoint(physicsEngine.pAABB[0], physicsEngine.pAABB[i]);
                    collision.collidingAABBs[0] = &physicsEngine.pAABB[0];
                    collision.collidingAABBs[1] = &physicsEngine.pAABB[i];

                    // Adjust explosion model transform to match the collision point
                    scene.pModels[1].pMeshTransform.x = collision.collisionPoint.x-3400;
                    scene.pModels[1].pMeshTransform.y = collision.collisionPoint.y;
                    scene.pModels[1].pMeshTransform.z = collision.collisionPoint.z;
                    scene.pModels[1].pMeshTransform.xScale = 200.0f;
                    scene.pModels[1].pMeshTransform.yScale = 200.0f;
                    scene.pModels[1].pMeshTransform.zScale = 200.0f;

                    // Add to vector of collisions
                    physicsEngine.vecCollidingAABBs.push_back(collision);

                    // Render the explosion at the collision point

                    physicsEngine.HandleColllision(shaderProgram, scene);
                }
            }
        }


        //std::cout << deltaTime << ": ";

        
        //for (int i = 0; i != physicsEngine.vecCollidingTriangles.size(); i++) {
        //}
        // Swap buffers and poll IO events (keys pressed/released, mouse moved, etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // Cleanup
    VAOManager.VAOVBOCleanup();
    
    // Terminate GLFW
    glfwTerminate();
    return 0;
}