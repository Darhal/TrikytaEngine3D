#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <time.h>
#include <unordered_map>
#include <bitset>

#include <Core/Context/Extensions.hpp>
#include <Core/Window/Window.hpp>
#include <Core/Misc/Maths/Maths.hpp>
#include <Core/Misc/Utils/Image.hpp>
#include <Core/Context/Context.hpp>
#include <Core/FileSystem/Directory/Directory.hpp>
#include <Core/DataStructure/PackedArray/RefPackedArray.hpp>
#include <Core/DataStructure/HashMap/HashMap.hpp>
#include <Core/TaskSystem/TaskManager/TaskManager.hpp>
#include <Core/DataStructure/PackedArray/PackedArray.hpp>

#include <RenderAPI/Shader/ShaderProgram.hpp>
#include <RenderAPI/Shader/Shader.hpp>
#include <RenderAPI/VertexArray/VAO.hpp>
#include <RenderAPI/VertexBuffer/VBO.hpp>
#include <RenderAPI/Texture/Texture.hpp>
#include <RenderAPI/General/GLContext.hpp>
#include <RenderAPI/RenderBuffer/RBO.hpp>
#include <RenderAPI/FrameBuffer/FBO.hpp>
#include <RenderAPI/GlobalState/GLState.hpp>


#include <Core/DataStructure/Tuple/Tuple.hpp>
#include "ShaderValidator.hpp"
#include <Core/DataStructure/StringBuffer/StringBuffer.hpp>
#include <Core/DataStructure/FixedString/FixedString.hpp>

#include <Core/ECS/ECS/ECS.hpp>
#include <Core/DataStructure/Bitset/Bitset.hpp>
#include <Core/DataStructure/Utils/Utils.hpp>

#include <Renderer/Backend/ICommandBuffer/ICommandBuffer.hpp>
#include <Renderer/Backend/Commands/Commands.hpp>
#include <Renderer/Backend/RenderState/RenderState.hpp>
#include <Core/Misc/Singleton/Singleton.hpp>
#include <Renderer/MeshLoader/Model/Model.hpp>
#include <Renderer/MeshLoader/MeshLoader.hpp>
#include "ShaderValidator.hpp"

using namespace TRE;
/*
	const uint32 bucket_count = 10;
	const uint32 cmds_count = 5'000;
	ICommandBuffer cmd_buffer = ICommandBuffer();
	CommandBucket& bucket = cmd_buffer.CreateBucket();
	INIT_BENCHMARK;

	for (uint32 i = 0; i < bucket_count; i++) {
		for (uint32 j = 0; j < cmds_count; j++) {
			Commands::DrawCmd* cmd = bucket.SubmitCommand<Commands::DrawCmd>(i, std::rand() % 30);
			cmd->start = i;
			cmd->end = i + 1;
			cmd->mode = Primitive::TRIANGLES;
		}
	}

	bucket.End();

	cmd_buffer.DispatchCommands();
*/

#define INIT_BENCHMARK std::chrono::time_point<std::chrono::high_resolution_clock> start, end; std::chrono::microseconds duration;

#define BENCHMARK(name, bloc_of_code) \
	start = std::chrono::high_resolution_clock::now(); \
	bloc_of_code; \
	end = std::chrono::high_resolution_clock::now();\
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); \
	std::cout << "\nExecution of '" << name << "' took : " << duration.count() << " microsecond(s)" << std::endl; \

void HandleEvent(float dt, Mat4f& projecton, Camera& camera, const Event& e);

float vertices[] = {
	-0.5f, -0.5f, -0.5f, 
	 0.5f, -0.5f, -0.5f,  
	 0.5f,  0.5f, -0.5f,  
	 0.5f,  0.5f, -0.5f, 
	-0.5f,  0.5f, -0.5f,  
	-0.5f, -0.5f, -0.5f,  

	-0.5f, -0.5f,  0.5f,  
	 0.5f, -0.5f,  0.5f,  
	 0.5f,  0.5f,  0.5f, 
	 0.5f,  0.5f,  0.5f,  
	-0.5f,  0.5f,  0.5f,  
	-0.5f, -0.5f,  0.5f, 

	-0.5f,  0.5f,  0.5f,  
	-0.5f,  0.5f, -0.5f,  
	-0.5f, -0.5f, -0.5f,  
	-0.5f, -0.5f, -0.5f,  
	-0.5f, -0.5f,  0.5f,  
	-0.5f,  0.5f,  0.5f,  

	 0.5f,  0.5f,  0.5f,  
	 0.5f,  0.5f, -0.5f,  
	 0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f,  0.5f,  
	 0.5f,  0.5f,  0.5f,  

	-0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f, -0.5f,  
	 0.5f, -0.5f,  0.5f,  
	 0.5f, -0.5f,  0.5f,  
	-0.5f, -0.5f,  0.5f,  
	-0.5f, -0.5f, -0.5f,  

	-0.5f,  0.5f, -0.5f,  
	 0.5f,  0.5f, -0.5f,  
	 0.5f,  0.5f,  0.5f,  
	 0.5f,  0.5f,  0.5f,  
	-0.5f,  0.5f,  0.5f,  
	-0.5f,  0.5f, -0.5f,  
};

float deltaTime = 0.5f;
double  frameRate = 30;
bool start = false;
int32 speed = 1;
const unsigned int SCR_WIDTH = 1900 / 2;
const unsigned int SCR_HEIGHT = 1000 / 2;

float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

int main()
{
	// settings
	TRE::Window window(SCR_WIDTH, SCR_HEIGHT, "Trikyta ENGINE 3 (OpenGL 3.3)", WindowStyle::Resize);
	window.initContext(3, 3);

	printf("- GPU Vendor    	: %s\n", glGetString(GL_VENDOR));
	printf("- Graphics      	: %s\n", glGetString(GL_RENDERER));
	printf("- Version       	: %s\n", glGetString(GL_VERSION));
	printf("- GLSL Version  	: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("- Hardware Threads 	: %d\n", std::thread::hardware_concurrency());

	ShaderValidator("res/Shader/SimpleShader.vs", "res/Shader/SimpleShader.fs");
	ShaderID shader_id = ResourcesManager::Instance().CreateResource<ShaderProgram>(ResourcesTypes::SHADER, 
		Shader("res/Shader/SimpleShader.vs", ShaderType::VERTEX), 
		Shader("res/Shader/SimpleShader.fs", ShaderType::FRAGMENT)
	);
	ShaderValidator("res/Shader/cam.vs", "res/Shader/cam.fs");
	ShaderID shader_id2 = ResourcesManager::Instance().CreateResource<ShaderProgram>(ResourcesTypes::SHADER,
		Shader("res/Shader/cam.vs", ShaderType::VERTEX),
		Shader("res/Shader/cam.fs", ShaderType::FRAGMENT)
		);
	{
		ShaderProgram& shader = ResourcesManager::Instance().Get<ShaderProgram>(ResourcesTypes::SHADER, shader_id);
		shader.LinkProgram();
		shader.Use();
		shader.AddUniform("MVP");
		shader.AddUniform("ProjView");
		shader.AddUniform("model");
	}
	{
		ShaderProgram& shader = ResourcesManager::Instance().Get<ShaderProgram>(ResourcesTypes::SHADER, shader_id2);
		shader.LinkProgram();
		shader.Use();

		shader.AddUniform("MVP");
		shader.AddUniform("ProjView");
		shader.AddUniform("model");
		shader.AddUniform("material.ambient");
		shader.AddUniform("material.diffuse");
		shader.AddUniform("material.specular");
		shader.AddUniform("material.alpha");
		shader.AddUniform("material.shininess");

		shader.AddSamplerSlot("material.specular_tex");
		shader.AddSamplerSlot("material.diffuse_tex");

		shader.SetFloat("material.alpha", 1.f);
		shader.SetFloat("material.shininess", 1.0f);
	}


	ICommandBuffer cmd_buffer = ICommandBuffer();
	CommandBucket& bucket = cmd_buffer.CreateBucket();
	bucket.GetCamera().Position = vec3(0.0f, 0.0f, 3.0f);
	bucket.GetProjectionMatrix() = mat4::perspective((float)bucket.GetCamera().Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.f);
	// bucket.SubmitCommand<Commands::DrawIndexedCmd>(-1); // TODO: DUE TO BUG IN MAP REMOVE THIS AFTER THE BUG IS FIXED!

	ModelData data;
	data.vertices = vertices;
	data.vertexSize = ARRAY_SIZE(vertices);
	Model model(data);
	StaticMesh mesh = model.LoadMesh(shader_id);
	
	MeshLoader loader("res/obj/lowpoly/carrot_box.obj");
	Model carrot  = loader.LoadAsOneObject();
	StaticMesh carrot_mesh = carrot.LoadMesh(shader_id2);

	for (uint32 i = 0; i < 10; i++) {
		carrot_mesh.GetTransformationMatrix() = mat4();
		vec3 pos;
		pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 5.f));
		pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 5.f));
		pos.z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 5.f));
		carrot_mesh.GetTransformationMatrix().translate(pos);
		carrot_mesh.Submit(bucket);
	}
	
	// mesh.Submit(bucket);
	bucket.End();

	Event ev;
	Enable(Capability::DEPTH_TEST);

	while (true) {
		if (window.getEvent(ev)) {
			HandleEvent(deltaTime, bucket.GetProjectionMatrix(), bucket.GetCamera(), ev);
		}

		ClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		Clear(Buffer::COLOR | Buffer::DEPTH);
		
		cmd_buffer.DispatchCommands();
		window.Present();
	}

	getchar();
	return 0;
}


void HandleEvent(float dt, Mat4f& projecton, Camera& camera, const Event& e)
{
	if (e.Type == Event::TE_RESIZE) {
		glViewport(0, 0, e.Window.Width, e.Window.Height);
		projecton = mat4::perspective((float)camera.Zoom, (float)e.Window.Width / (float)e.Window.Height, 0.1f, 300.f);
		printf("DETECTING RESIZE EVENT(%d, %d)\n", e.Window.Width, e.Window.Height);
	} else if (e.Type == Event::TE_KEY_DOWN) {
		switch (e.Key.Code) {
		case Key::Up:
			speed += 1;
			printf("Changing camera speed to %d\n", speed);
			break;
		case Key::Down:
			speed -= 1;
			printf("Changing camera speed to %d\n", speed);
			break;
		case Key::Space:
			start = !start;
			deltaTime = 0;
			break;
		case Key::Z:
			camera.ProcessKeyboard(FORWARD, speed * dt);
			break;
		case Key::S:
			camera.ProcessKeyboard(BACKWARD, speed * dt);
			break;
		case Key::Q:
			camera.ProcessKeyboard(LEFT, speed * dt);
			break;
		case Key::D:
			camera.ProcessKeyboard(RIGHT, speed * dt);
			break;
		default:
			break;
		}
	} else if (e.Type == Event::TE_MOUSE_MOVE) {
		float xpos = (float)e.Mouse.X;
		float ypos = (float)e.Mouse.Y;

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}