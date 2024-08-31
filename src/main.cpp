#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutOff;

    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 treePosition = glm::vec3(20.0,-13.0,8.0);
    float treeScale = 5.5f;
    glm::vec3 pumpkinPosition = glm::vec3(8.0,-10.0,14.0);
    float pumpkinScale = 0.04f;
    glm::vec3 batPosition = glm::vec3(26.0,27.0,0.0);
    float batScale = 1.2f;
    glm::vec3 moonPosition = glm::vec3(-6.0,29.0,0.0);
    float moonScale = 2.0f;
    glm::vec3 groundPosition = glm::vec3(0.0,-16.0,0.0);
    float groundScale = 10.0f;
    DirectionalLight directionalLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    // build and compile shaders
    // -------------------------
    Shader treeShader("resources/shaders/tree.vs", "resources/shaders/tree.fs");
    Shader batShader("resources/shaders/bat.vs", "resources/shaders/bat.fs");
    Shader moonShader("resources/shaders/moon.vs", "resources/shaders/moon.fs");
    Shader pumpkinShader("resources/shaders/pumpkin.vs", "resources/shaders/pumpkin.fs");
    Shader groundShader("resources/shaders/ground.vs", "resources/shaders/ground.fs");

    // load models
    // -----------
    Model treeModel("resources/objects/tree/uploads_files_855516_Tree.obj");
    treeModel.SetShaderTextureNamePrefix("material.");

    Model pumpkinModel("resources/objects/bundeva/Pumpkin.obj");
    pumpkinModel.SetShaderTextureNamePrefix("material.");

    Model batModel("resources/objects/bat/Bat.obj");
    batModel.SetShaderTextureNamePrefix("material.");

    Model groundModel("resources/objects/ground/terrain.obj");
    groundModel.SetShaderTextureNamePrefix("material.");

    Model moonModel("resources/objects/moon/Moon.obj");
    moonModel.SetShaderTextureNamePrefix("material.");

    DirectionalLight& directionalLight = programState->directionalLight;
    directionalLight.direction = glm::vec3(-10.0f, -5.0f, -2.0f);
    directionalLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    directionalLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    directionalLight.specular = glm::vec3(2.5);

    //blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //face culling

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        // bat shader
        batShader.use();
        batShader.setVec3("directionalLight.direction",directionalLight.direction);
        batShader.setVec3("directionalLight.ambient",directionalLight.ambient);
        batShader.setVec3("directionalLight.diffuse",directionalLight.diffuse);
        batShader.setVec3("directionalLight.specular",directionalLight.specular);

        batShader.setVec3("viewPosition", programState->camera.Position);
        batShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        batShader.setMat4("projection", projection);
        batShader.setMat4("view", view);

        //render bat models

        float time = glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3((programState->batPosition.x)*cos(time),programState->batPosition.y,(programState->batPosition.x)*sin(time)));
        model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model,glm::vec3(programState->batScale));
        batShader.setMat4("model", model);
        batModel.Draw(batShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-20.0f*cos(time),14.0f,2.0f*sin(time)));
        model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model,glm::vec3(programState->batScale));
        batShader.setMat4("model", model);
        batModel.Draw(batShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-35.0f*cos(time),20.0f,0.0f*sin(time)));
        model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model,glm::vec3(programState->batScale));
        batShader.setMat4("model", model);
        batModel.Draw(batShader);

        // moon shader
        moonShader.use();
        moonShader.setVec3("directionalLight.direction",glm::vec3(-1.0f,-0.5f,-1.0f));
        moonShader.setVec3("directionalLight.ambient",glm::vec3(0.1f,0.1f,0.1f));
        moonShader.setVec3("directionalLight.diffuse",glm::vec3(0.9f,0.7f,0.5f));
        moonShader.setVec3("directionalLight.specular",glm::vec3(0.05f,0.05f,0.05f));

        moonShader.setVec3("viewPosition", programState->camera.Position);
        moonShader.setFloat("material.shininess", 256.0f);
        moonShader.setFloat("material.specular", 1.0f);


        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);

        moonShader.setFloat("alpha",0.5f);

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(programState->moonPosition));
        model = glm::rotate(model, glm::radians(float(20 * (glfwGetTime()))), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model,glm::vec3(programState->moonScale));
        moonShader.setMat4("model", model);
        moonModel.Draw(moonShader);

        // tree shader
        treeShader.use();
        treeShader.setVec3("directionalLight.direction",glm::vec3(-1.0f,-0.5f,-1.0f));
        treeShader.setVec3("directionalLight.ambient",glm::vec3(0.1f,0.1f,0.1f));
        treeShader.setVec3("directionalLight.diffuse",glm::vec3(0.9f,0.7f,0.5f));
        treeShader.setVec3("directionalLight.specular",glm::vec3(0.05f,0.05f,0.05f));

        treeShader.setVec3("viewPosition", programState->camera.Position);
        treeShader.setFloat("material.shininess", 32.0f);

        treeShader.setMat4("projection", projection);
        treeShader.setMat4("view", view);

        unsigned int diffuseTextureID = TextureFromFile("Tree_Bark_lambert_BaseColor.png", "resources/objects/tree");
        unsigned int heightTextureID = TextureFromFile("Tree_Bark_lambert_Height.png", "resources/objects/tree");
        unsigned int normalTextureID = TextureFromFile("Tree_Bark_lambert_Normal.png", "resources/objects/tree");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
        treeShader.setInt("material.texture_diffuse1", 0); 

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, heightTextureID);
        treeShader.setInt("material.texture_height1", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normalTextureID);
        treeShader.setInt("normal.texture_height1", 2);



        //render tree model
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(programState->treePosition));
        model = glm::scale(model,glm::vec3(programState->treeScale));
        treeShader.setMat4("model", model);
        treeModel.Draw(treeShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-29.0f,-12.0f,6.0f));
        model = glm::scale(model,glm::vec3(4.5f));
        treeShader.setMat4("model", model);
        treeModel.Draw(treeShader);

        //ground shader
        groundShader.use();
        groundShader.setVec3("directionalLight.direction",glm::vec3(-1.0f,-0.5f,-1.0f));
        groundShader.setVec3("directionalLight.ambient",glm::vec3(0.1f,0.1f,0.1f));
        groundShader.setVec3("directionalLight.diffuse",glm::vec3(0.9f,0.7f,0.5f));
        groundShader.setVec3("directionalLight.specular",glm::vec3(0.05f,0.05f,0.05f));

        groundShader.setVec3("viewPosition", programState->camera.Position);
        groundShader.setFloat("material.shininess", 32.0f);

        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);

        diffuseTextureID = TextureFromFile("iceland1_t.png", "resources/objects/ground");
        unsigned int specularTextureID = TextureFromFile("specular.png", "resources/objects/ground");
        /*
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
        groundShader.setInt("material.texture_diffuse1", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTextureID);
        groundShader.setInt("material.texture_specular1", 1);*/

        //render ground model

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(programState->groundPosition));
        //model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, glm::radians(-50.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model,glm::vec3(programState->groundScale));
        groundShader.setMat4("model", model);
        groundModel.Draw(moonShader);



        pumpkinShader.use();
        pumpkinShader.setVec3("directionalLight.direction",glm::vec3(-1.0f,-0.5f,-1.0f));
        pumpkinShader.setVec3("directionalLight.ambient",glm::vec3(0.1f,0.1f,0.1f));
        pumpkinShader.setVec3("directionalLight.diffuse",glm::vec3(0.9f,0.7f,0.5f));
        pumpkinShader.setVec3("directionalLight.specular",glm::vec3(0.05f,0.05f,0.05f));
        pumpkinShader.setInt("material.texture_diffuse1", 0);
        pumpkinShader.setInt("material.texture_specular1", 1);
        pumpkinShader.setInt("material.texture_normal1", 2);
        pumpkinShader.setInt("material.texture_emissive1", 3);

        pumpkinShader.setFloat("alpha",0.9f);


        pumpkinShader.setVec3("viewPosition", programState->camera.Position);
        pumpkinShader.setFloat("material.shininess", 32.0f);

        pumpkinShader.setMat4("projection", projection);
        pumpkinShader.setMat4("view", view);

        diffuseTextureID = TextureFromFile("Pumpkin_diff_sketfab.jpg", "resources/objects/bundeva");
        unsigned int emissiveTextureID = TextureFromFile("Pumpkin_lum_Sketchfab.jpg", "resources/objects/bundeva");
        normalTextureID = TextureFromFile("Pumpkin_nrml.jpg", "resources/objects/bundeva");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
        pumpkinShader.setInt("material.texture_diffuse1", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, emissiveTextureID);
        pumpkinShader.setInt("material.texture_emissive1", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normalTextureID);
        pumpkinShader.setInt("normal.texture_height1", 2);


        //render pumpkin model
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(programState->pumpkinPosition));
        //model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model,glm::vec3(programState->pumpkinScale));
        pumpkinShader.setMat4("model", model);
        pumpkinModel.Draw(pumpkinShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-34.0f,-8.0f,10.0f));
        model = glm::rotate(model, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model,glm::vec3(programState->pumpkinScale));
        pumpkinShader.setMat4("model", model);
        pumpkinModel.Draw(pumpkinShader);

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
