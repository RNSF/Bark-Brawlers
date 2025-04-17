module;


#define STB_IMAGE_IMPLEMENTATION

// stdlib
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>
#include <utility>
#include <chrono>
#include "common.hpp"
#include <numbers>

// internal
#include "../ext/stb_image/stb_image.h"
#include <glm/trigonometric.hpp>
#include <glm/fwd.hpp>


export module RenderSystem;

import ECS;
import Components;
import Entity;
import Transform;
import Vector2;
import Color;
import Rect;
import Tiling;
import Singletons;
import Polygon;
import Animation;
import RenderHelpers;


// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex {
	glm::vec3 position;
	glm::vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
	glm::vec3 position;
	glm::vec2 texcoord;
};


export class RenderSystem {
	Entity player1Entity;
	Entity player2Entity;

	float EMPHASIS_TIMER_START = 2;
	float EMPHASIS_FACTOR_START = 20;

	float starEmphasisTimer = EMPHASIS_TIMER_START;
	float emphasisFactor = EMPHASIS_FACTOR_START;

	float starEmphasisTimerP2 = EMPHASIS_TIMER_START;
	float emphasisFactorP2 = EMPHASIS_FACTOR_START;

	float speed = EMPHASIS_FACTOR_START/EMPHASIS_TIMER_START;
	
public:
	std::array<GLuint,		(int)TEXTURE_ASSET_ID::COUNT>		textureGlHandles;
	std::array<glm::ivec2,  (int)TEXTURE_ASSET_ID::COUNT>		textureDimensions;
	std::array<GLuint,		(int)GEOMETRY_BUFFER_ID::COUNT>		vertexBuffers;
	std::array<GLuint,		(int)GEOMETRY_BUFFER_ID::COUNT>		indexBuffers;
	std::array<GLuint,		(int)EFFECT_ASSET_ID::COUNT>		effects;


	struct ShaderCompiler {
		std::string shaderPath = "";
		std::string compileString = "";
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<ShaderCompiler, (int)EFFECT_ASSET_ID::COUNT> shaderCompilers = {
		ShaderCompiler(shaderPath("coloured"), ""),
		ShaderCompiler(shaderPath("textured"), ""),
		ShaderCompiler(shaderPath("textured"), "#define OUTLINE 1"),
		ShaderCompiler(shaderPath("textured_instanced"), ""),
		ShaderCompiler(shaderPath("textured_instanced"), "#define OUTLINE 1"),
		ShaderCompiler(shaderPath("screen"), ""),
	};

	glm::mat3 projectionMatrix;

	void setPlayer1(Entity entity) {
		player1Entity = entity;
	}

	void setPlayer2(Entity entity) {
		player2Entity = entity;
	}

	void drawHUD(float delta) {
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);

		
		Camera hudCamera;
		hudCamera.size = { (float)w, (float)h };
		hudCamera.zoom = 2.0f;  

		// Draw player icons 
		if (ecs.healths.has(player1Entity)) {
			Player& player1 = ecs.players.get(player1Entity);
			auto& player1Health = ecs.healths.get(player1Entity);
			Vector2 p1Pos = { -(float)w, (float)h / 2 };
			drawSpriteIndexed(
				getPlayerSprite(player1),
				glm::ivec2(34, 1),
				0,  
				p1Pos + Vector2(85, 130) * 1.3,
				Vector2(8.f, 8.f),
				0.2f,
				Color::white(),
				Vector2::unitCenter(),
				hudCamera);

			for (int i = 0; i < player1Health.health; i++) {
				drawSprite(TEXTURE_ASSET_ID::HEART,
					{ -(float)w + 200 + (64 * i), (float)h - 31},
					Vector2::one() * 4,
					0.0f,
					Color::white(),
					Vector2::unitCenter(),
					hudCamera);
			}

			for (int i = player1Health.health; i < 10; i++) {
				drawSprite(TEXTURE_ASSET_ID::HEART,
			{ -(float)w + 200 + (64 * i), (float)h - 31},
					Vector2::one() * 4,
					0.0f,
					Color::black(),
					Vector2::unitCenter(),
					hudCamera);
			}

			for (int i = 0; i < gameState.gameScore.player1Wins; i++) {
				if (gameState.gameScore.emphasizeP1Won && starEmphasisTimer > 0) {
					starEmphasisTimer -= delta;

					emphasisFactor = std::lerp(emphasisFactor , 1, speed * delta);

					if (i == gameState.gameScore.player1Wins - 1) {
						drawSprite(TEXTURE_ASSET_ID::STAR,
						{ -(float)w + 35 + (i * 70), -(float)h + 37},
						Vector2::one() * 4 * emphasisFactor,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
					} else {
						drawSprite(TEXTURE_ASSET_ID::STAR,
						{ -(float)w + 35 + (i * 70), -(float)h + 37},
						Vector2::one() * 4,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
					}
				} else {
					gameState.gameScore.emphasizeP1Won = false;
					starEmphasisTimer = EMPHASIS_TIMER_START;
					emphasisFactor = EMPHASIS_FACTOR_START;

					drawSprite(TEXTURE_ASSET_ID::STAR,
						{ -(float)w + 35 + (i * 70), -(float)h + 37},
						Vector2::one() * 4,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
				}
			}

			for (int i = gameState.gameScore.player1Wins; i < MAX_ROUNDS; i++) {
				drawSprite(TEXTURE_ASSET_ID::STAR,
				{ -(float)w + 35 + (i * 70), -(float)h + 37},
				Vector2::one() * 4,
				0.0f,
				Color::black(),
				Vector2::unitCenter(),
				hudCamera);
			}
		}
		if (ecs.healths.has(player2Entity)) {
			Player& player2 = ecs.players.get(player2Entity);
			auto& player2Health = ecs.healths.get(player2Entity);
			Vector2 p2Pos = { (float)w, (float)h / 2 };
			drawSpriteIndexed(
				getPlayerSprite(player2),
				glm::ivec2(34, 1),
				0,  
				p2Pos + Vector2(-85, 130) * 1.3,
				Vector2(-8.f, 8.f), 
				-0.2f,
				Color::white(),
				Vector2::unitCenter(),
				hudCamera);


			for (int i = 0; i < player2Health.health; i++) {
				drawSprite(TEXTURE_ASSET_ID::HEART,
					{ (float)w - 220 - (64 * i), (float)h - 31 },
					Vector2::one() * 4,
					0.0f,
					Color::white(),
					Vector2::unitCenter(),
					hudCamera);
			}

			// draw the lost hearts
			for (int i = player2Health.health; i < 10; i++) {
				drawSprite(TEXTURE_ASSET_ID::HEART,
					{ (float)w - 220 - (64 * i), (float)h - 31 },
					Vector2::one() * 4,
					0.0f,
					Color::black(),
					Vector2::unitCenter(),
					hudCamera);
			}

			for (int i = 0; i < gameState.gameScore.player2Wins; i++) {
				if (gameState.gameScore.emphasizeP2Won && starEmphasisTimerP2 > 0) {
					starEmphasisTimerP2 -= delta;

					emphasisFactorP2 = std::lerp(emphasisFactorP2 , 1, speed * delta);

					if (i == gameState.gameScore.player2Wins - 1) {
						drawSprite(TEXTURE_ASSET_ID::STAR,
							{ (float)w - 35 - (i * 70), -(float)h + 37 },
						Vector2::one() * 4 * emphasisFactorP2,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
					} else {
						drawSprite(TEXTURE_ASSET_ID::STAR,
						{ (float)w - 35 - (i * 70), -(float)h + 37 },
						Vector2::one() * 4,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
					}
				} else {
					gameState.gameScore.emphasizeP2Won = false;
					starEmphasisTimerP2 = EMPHASIS_TIMER_START;
					emphasisFactorP2 = EMPHASIS_FACTOR_START;

					drawSprite(TEXTURE_ASSET_ID::STAR,
						{ (float)w - 35 - (i * 70), -(float)h + 37 },
						Vector2::one() * 4,
						0.0f,
						Color::white(),
						Vector2::unitCenter(),
						hudCamera);
				}
			}

			for (int i = gameState.gameScore.player2Wins; i < MAX_ROUNDS; i++) {
				drawSprite(TEXTURE_ASSET_ID::STAR,
		{ (float)w - 35 - (i * 70), -(float)h + 37 },
				Vector2::one() * 4,
				0.0f,
				Color::black(),
				Vector2::unitCenter(),
				hudCamera);
			}
		}
	}

	GLFWwindow* window;

	// Screen texture handles
	GLuint frameBuffer;
	GLuint offScreenRenderBufferColor;
	GLuint offScreenRenderBufferDepth;

	GLuint vao;


	const std::array<std::string, (int) TEXTURE_ASSET_ID::COUNT> texturePaths = {
		texturesPath("jeff.png"),
		texturesPath("FighterTest.png"),
		texturesPath("dawg.png"),
		texturesPath("ForestAssets.png"),
		texturesPath("backgrounds/ForestBackgroundDesaturated.png"),
		texturesPath("1x1.png"),
		texturesPath("winner.png"),
		texturesPath("round-over.png"),
		texturesPath("10pxCircleParticle.png"),
		

		// WEAPONS
		texturesPath("sword.png"),
		texturesPath("Arrow.png"),
		texturesPath("Bow.png"),
		texturesPath("staff.png"),

		// TILESETS
		tilesetsPath("black_tile.png"),
		tilesetsPath("GrassTileset.png"),
		tilesetsPath("GrassTileset2.png"),
		tilesetsPath("black_bricks.png"),
		tilesetsPath("black_bricks2.png"),
		tilesetsPath("sandstone_bricks.png"),
		tilesetsPath("wooden_planks.png"),
		tilesetsPath("dry_dirt.png"),
		tilesetsPath("dark_dirt.png"),
		tilesetsPath("cave.png"),
		tilesetsPath("red_bricks.png"),


		// BACKGROUNDS
		texturesPath("backgrounds/forest.png"),
		texturesPath("backgrounds/caves.png"),
		texturesPath("backgrounds/town.png"),

		texturesPath("backgrounds/charbackgroundnewnewnew.png"),
		texturesPath("backgrounds/arenaselectbackground2.png"),
		texturesPath("backgrounds/title_banner.png"),
		texturesPath("backgrounds/gameoverbackground.png"),

		// FONTS
		texturesPath("fonts/BoreBlasters.png"),
		texturesPath("fonts/smiley5x5.png"),

		// MISC
		texturesPath("heart.png"),
		texturesPath("star.png"),
		texturesPath("48pxCircle.png"),
		texturesPath("inputs.png"),

		// CLASSES
		texturesPath("knight_dawg_red.png"),
		texturesPath("knight_dawg_blue.png"),
		texturesPath("archer_dawg_brown.png"),
		texturesPath("archer_dawg_green.png"),
		texturesPath("wizard_dawg_red.png"), 
		texturesPath("wizard_dawg_purple.png"), 
	};

	bool glHasErrors() {
		GLenum error = glGetError();

		if (error == GL_NO_ERROR) return false;

		while (error != GL_NO_ERROR)
		{
			const char* error_str = "";
			switch (error)
			{
			case GL_INVALID_OPERATION:
				error_str = "INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error_str = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error_str = "INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error_str = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error_str = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			}

			fprintf(stderr, "OpenGL: %s", error_str);
			error = glGetError();
			assert(false);
		}

		return true;
	}


	//// Render initialization
	bool init(GLFWwindow* window_arg) {
		this->window = window_arg;

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // vsync

		// Load OpenGL function pointers
		const int is_fine = gl3w_init();
		assert(is_fine == 0);

		// Create a frame buffer
		frameBuffer = 0;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glHasErrors();

		// For some high DPI displays (ex. Retina Display on Macbooks)
		// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
		int frameBuffer_width_px, frameBuffer_height_px;
		int WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX;
		glfwGetWindowSize(window, &WINDOW_WIDTH_PX, &WINDOW_HEIGHT_PX);
		glfwGetFramebufferSize(window, &frameBuffer_width_px, &frameBuffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
		if (frameBuffer_width_px != WINDOW_WIDTH_PX)
		{
			printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
			printf("glfwGetFramebufferSize = %d,%d\n", frameBuffer_width_px, frameBuffer_height_px);
			printf("requested window width,height = %d,%d\n", WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
		}

		// Hint: Ask your TA for how to setup pretty OpenGL error callbacks. 
		// This can not be done in mac os, so do not enable
		// it unless you are on Linux or Windows. You will need to change the window creation
		// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
		// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

		// We are not really using VAO's but without at least one bound we will crash in
		// some systems.
		
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glHasErrors();

		initScreenTexture();
		initializeGlTextures();
		initializeGlEffects();
		initializeGlGeometryBuffers();

		

		return true;
	}

	void initializeGlTextures() {
		glGenTextures((GLsizei)textureGlHandles.size(), textureGlHandles.data());

		for (glm::uint i = 0; i < texturePaths.size(); i++)
		{
			const std::string& path = texturePaths[i];
			glm::ivec2& dimensions = textureDimensions[i];

			stbi_uc* data;
			data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

			if (data == NULL)
			{
				const std::string message = "Could not load the file " + path + ".";
				fprintf(stderr, "%s", message.c_str());
				assert(false);
			} else {
				const std::string message = "Loaded texture from " + path + ".";
				std::cout << message << std::endl;
			}
			glBindTexture(GL_TEXTURE_2D, textureGlHandles[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glHasErrors();
			stbi_image_free(data);
		}
		glHasErrors();
	}

	void initializeGlEffects() {
		for (glm::uint i = 0; i < shaderCompilers.size(); i++)
		{
			ShaderCompiler shaderCompiler = shaderCompilers[i];
			const std::string vertex_shader_name = shaderCompiler.shaderPath + ".vs.glsl";
			const std::string fragment_shader_name = shaderCompiler.shaderPath + ".fs.glsl";

			bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, shaderCompiler.compileString, effects[i]);
			assert(is_valid && (GLuint)effects[i] != 0);

			std::cout << "Initialized effect: " << shaderCompiler.shaderPath << std::endl;
		}
	}

	//// One could merge the following two functions as a template function...
	template <class T>
	void bindVBOandIBO(glm::uint gid, std::vector<T> vertices, std::vector<uint16_t> indices) {
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[gid]);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glHasErrors();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffers[gid]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
		glHasErrors();
	}

	void initializeGlGeometryBuffers() {
		// Vertex Buffer creation.
		glGenBuffers((GLsizei)vertexBuffers.size(), vertexBuffers.data());
		// Index Buffer creation.
		glGenBuffers((GLsizei)indexBuffers.size(), indexBuffers.data());

		//////////////////////
		// Initialize sprite
		// The position corresponds to the center of the texture.
		std::vector<TexturedVertex> textured_vertices(4);
		std::vector<glm::vec3> textured_vertices_positions = POLYGONS[(int)GEOMETRY_BUFFER_ID::SPRITE].getRenderVerticies();
		textured_vertices[0].position = textured_vertices_positions[0];
		textured_vertices[1].position = textured_vertices_positions[1];
		textured_vertices[2].position = textured_vertices_positions[2];
		textured_vertices[3].position = textured_vertices_positions[3];
		textured_vertices[0].texcoord = { 0.f, 1.f };
		textured_vertices[1].texcoord = { 1.f, 1.f };
		textured_vertices[2].texcoord = { 1.f, 0.f };
		textured_vertices[3].texcoord = { 0.f, 0.f };

		//// Counterclockwise as it's the default OpenGL front winding direction.
		const std::vector<uint16_t> textured_indices = POLYGONS[(int)GEOMETRY_BUFFER_ID::SPRITE].getRenderIndices();
		bindVBOandIBO((glm::uint) GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);


		/////////////////////////////////////////////////////
		//// Initialize rest of polygons:

		for (unsigned int i = (int)GEOMETRY_BUFFER_ID::SPRITE + 1; i < (int) GEOMETRY_BUFFER_ID::COUNT; i++) {
			bindVBOandIBO(i, POLYGONS[i].getRenderVerticies(), POLYGONS[i].getRenderIndices());
		}
		
	}

	~RenderSystem() {
		// Don't need to free gl resources since they last for as long as the program,
		// but it's polite to clean after yourself.
		glDeleteBuffers((GLsizei)vertexBuffers.size(), vertexBuffers.data());
		glDeleteBuffers((GLsizei)indexBuffers.size(), indexBuffers.data());
		glDeleteTextures((GLsizei)textureGlHandles.size(), textureGlHandles.data());
		glDeleteTextures(1, &offScreenRenderBufferColor);
		glDeleteRenderbuffers(1, &offScreenRenderBufferDepth);
		glHasErrors();

		for (glm::uint i = 0; i < effects.size(); i++) {
			glDeleteProgram(effects[i]);
		}
		// delete allocated resources
		glDeleteFramebuffers(1, &frameBuffer);
		glHasErrors();
	}

	//// Initialize the screen texture from a standard sprite
	bool initScreenTexture() {
		// create a single entry

		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &framebuffer_width, &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

		glGenTextures(1, &offScreenRenderBufferColor);
		glBindTexture(GL_TEXTURE_2D, offScreenRenderBufferColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glHasErrors();

		glGenRenderbuffers(1, &offScreenRenderBufferDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, offScreenRenderBufferDepth);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, offScreenRenderBufferColor, 0);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offScreenRenderBufferDepth);
		glHasErrors();

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		return true;
	}

	bool compileShader(GLuint shader) {
		glCompileShader(shader);
		glHasErrors();
		GLint success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint log_len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetShaderInfoLog(shader, log_len, &log_len, log.data());
			glDeleteShader(shader);

			glHasErrors();

			fprintf(stderr, "GLSL: %s", log.data());
			return false;
		}

		return true;
	}

	bool loadEffectFromFile(
		const std::string& vs_path, const std::string& fs_path, const std::string& compileString, GLuint& out_program) {
		// Opening files
		std::ifstream vs_is(vs_path);
		std::ifstream fs_is(fs_path);
		if (!vs_is.good() || !fs_is.good())
		{
			fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
			assert(false);
			return false;
		}

		// Reading sources
		std::stringstream vs_ss, fs_ss;
		vs_ss << vs_is.rdbuf();
		fs_ss << fs_is.rdbuf();
		std::string vs_str = vs_ss.str();
		std::string fs_str = fs_ss.str();

		// Add in special compiler defines and such in the compileString
		if (compileString.size() > 0) {
			assert(vs_str.starts_with("#version 330") && "Version is not defined on first line of shader!");
			assert(fs_str.starts_with("#version 330") && "Version is not defined on first line of shader!");
			vs_str = vs_str.insert(vs_str.find("\n") + 2, compileString + "\n"); // insert after first line where #version is defined
			fs_str = fs_str.insert(fs_str.find("\n") + 2, compileString + "\n"); // insert after first line where #version is defined
		}

		const char* vs_src = vs_str.c_str();
		const char* fs_src = fs_str.c_str();

		GLsizei vs_len = (GLsizei)vs_str.size();
		GLsizei fs_len = (GLsizei)fs_str.size();

		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vs_src, &vs_len);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fs_src, &fs_len);
		glHasErrors();

		// Compiling
		if (!compileShader(vertex))
		{
			fprintf(stderr, "Vertex compilation failed");
			assert(false);
			return false;
		}
		if (!compileShader(fragment))
		{
			fprintf(stderr, "Fragment compilation failed");
			assert(false);
			return false;
		}

		// Linking
		out_program = glCreateProgram();
		glAttachShader(out_program, vertex);
		glAttachShader(out_program, fragment);
		glLinkProgram(out_program);
		glHasErrors();

		{
			GLint is_linked = GL_FALSE;
			glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
			if (is_linked == GL_FALSE)
			{
				GLint log_len;
				glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
				std::vector<char> log(log_len);
				glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
				glHasErrors();

				fprintf(stderr, "Link error: %s", log.data());
				assert(false);
				return false;
			}
		}

		// No need to carry this around. Keeping these objects is only useful if we recycle
		// the same shaders over and over, which we don't, so no need and this is simpler.
		glDetachShader(out_program, vertex);
		glDetachShader(out_program, fragment);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glHasErrors();

		return true;
	}


	void drawDebugBox(Entity entity, Camera camera, TEXTURE_ASSET_ID drawAround, Vector2 scale = Vector2::one()) {
		Transform& entityTransform = ecs.transforms.get(entity);

		Vector2 dimensions = scale * Vector2(textureDimensions[(int)drawAround]) * Rect::unit().size();
		Transform transform;

		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
		// Apply the camera transformation so the line moves correctly with the scene
		transform.matrix /= camera.getTransform(windowSize).matrix;

		// Center the line at the entity�s position
		glm::vec2 translation = entityTransform.getTranslation();
		// For the right edge, subtract an extra half width:
		transform.translate(entityTransform.getTranslation() - glm::vec2{0.f, 5.f});
		transform.scale(dimensions);

		Rect section = Rect::unit();
		Color modulate = Color::red();
		TEXTURE_ASSET_ID textureId = TEXTURE_ASSET_ID::PIXEL;


		// Setting shaders
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
		glUseProgram(program);
		glHasErrors();

		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

		// Setting vertex and index buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();

		// texture-mapped entities - use data location as in the vertex buffer
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		glHasErrors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				glm::vec3)); // note the stride to skip the preceeding vertex position
		glVertexAttribDivisor(in_texcoord_loc, 0);

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glHasErrors();

		GLuint texture_id = textureGlHandles[(int)textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();



		// Getting uniform locations for glUniform* calls
		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		glUniform4fv(color_uloc, 1, (float*)&modulate);
		glHasErrors();

		// Get number of indices from index buffer, which has elements uint16_t
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glHasErrors();

		GLsizei num_indices = size / sizeof(uint16_t);
		// GLsizei num_triangles = num_indices / 3;

		GLint currProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.matrix);
		glHasErrors();

		GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projectionMatrix);
		glHasErrors();

		//GLuint scale_loc = glGetUniformLocation(currProgram, "scale");
		//glUniformMatrix3fv(scale_loc, 1, GL_FALSE, (float*)&projectionMatrix);
		//glHasErrors();

		glm::vec2 sectionTopLeft = section.topLeft();
		glm::vec2 sectionSize = section.size();

		GLuint texcoord_offset_loc = glGetUniformLocation(currProgram, "texcoord_offset");
		glUniform2fv(texcoord_offset_loc, 1, (float*)&sectionTopLeft);
		glHasErrors();

		GLuint texcoord_scale_loc = glGetUniformLocation(currProgram, "texcoord_scale");
		glUniform2fv(texcoord_scale_loc, 1, (float*)&sectionSize);
		glHasErrors();




		// Drawing of num_indices/3 triangles specified in the index buffer
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		glHasErrors();
	}


	void drawLine(Vector2 startPosition, Vector2 endPosition, float thickness, Color color, Camera camera) {
		drawSprite(TEXTURE_ASSET_ID::PIXEL,
			startPosition,
			Vector2(startPosition.distanceTo(endPosition), thickness),
			(endPosition - startPosition).angle(),
			color,
			Vector2(0, 0.5),
			camera);
	}


	void drawGrid(const Camera& camera) {
		auto& tilemap = ecs.tilemaps.components[0];
		auto& transform = ecs.transforms.get(ecs.tilemaps.entities[0]);
		Vector2 tileSize = tilemap.tileSize;
		Vector2 offset = transform.getTranslation();
		Color gridColor = Color(0.0f, 0.0f, 0.0f, 0.3f);

		// Draw vertical lines
		for (int x = 0; x <= TILEMAP_W; x++) {
			float xPos = x * tileSize.x + offset.x;
			Vector2 linePos = Vector2(xPos, offset.y);
			drawSprite(TEXTURE_ASSET_ID::PIXEL,
				linePos,
				Vector2(1, TILEMAP_H * tileSize.y),
				0.0f,
				gridColor,
				Vector2::zero(),
				camera);
		}

		// Draw horizontal lines
		for (int y = 0; y <= TILEMAP_H; y++) {
			float yPos = y * tileSize.y + offset.y;
			Vector2 linePos = Vector2(offset.x, yPos);
			drawSprite(TEXTURE_ASSET_ID::PIXEL,
				linePos,
				Vector2(TILEMAP_W * tileSize.x, 1),
				0.0f,
				gridColor,
				Vector2::zero(),
				camera);
		}
	}

	
	void drawPolygon(GEOMETRY_BUFFER_ID polygonId, Vector2 position, Vector2 scale, float rotation, Color color = Color::red(), Camera camera = Camera()) {
		assert((int) polygonId >= (int)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE);

		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);

		Transform transform;
		transform.matrix /= camera.getTransform(windowSize).matrix;
		transform.translate(position);
		transform.rotate(rotation);
		transform.scale(scale);



		// Setting shaders
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::COLOURED];
		glUseProgram(program);
		glHasErrors();

		const GLuint vbo = vertexBuffers[(GLuint)polygonId];
		const GLuint ibo = indexBuffers[(GLuint)polygonId];

		// Setting vertex and index buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();

		// texture-mapped entities - use data location as in the vertex buffer
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		glHasErrors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();




		// Getting uniform locations for glUniform* calls
		GLint color_uloc = glGetUniformLocation(program, "color");
		glUniform4fv(color_uloc, 1, (float*)&color);
		glHasErrors();

		// Get number of indices from index buffer, which has elements uint16_t
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glHasErrors();

		GLsizei num_indices = size / sizeof(uint16_t);
		// GLsizei num_triangles = num_indices / 3;

		GLint currProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.matrix);
		glHasErrors();

		GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projectionMatrix);
		glHasErrors();

		// Drawing of num_indices/3 triangles specified in the index buffer
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		glHasErrors();
	}

	void drawBackground(TEXTURE_ASSET_ID textureId) {
		glDisable(GL_DEPTH_TEST);

		Transform transform;
		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);

		Vector2 position = Vector2(0,0);
		Vector2 scale = Vector2(windowSize.x, windowSize.y);

		transform.translate(position);
		transform.scale(scale);

		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
		glUseProgram(program);
		glHasErrors();

		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(glm::vec3));

		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		glUniform4f(color_uloc, 1.0f, 1.0f, 1.0f, 1.0f);

		glActiveTexture(GL_TEXTURE0);
		GLuint texture_id = textureGlHandles[(int)textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();

		GLuint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.matrix);

		GLuint projection_loc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projectionMatrix);

		GLuint texcoord_offset_loc = glGetUniformLocation(program, "texcoord_offset");
		glm::vec2 texcoord_offset = { 0.0f, 0.0f };
		glUniform2fv(texcoord_offset_loc, 1, (float*)&texcoord_offset);

		GLuint texcoord_scale_loc = glGetUniformLocation(program, "texcoord_scale");
		glm::vec2 texcoord_scale = { 1.0f, 1.0f };
		glUniform2fv(texcoord_scale_loc, 1, (float*)&texcoord_scale);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		glHasErrors();

		glEnable(GL_DEPTH_TEST);
	}


	// M1 [1] Textured geometry
	// M1 [2] Basic 2D transformations
	// rotation is in radians.rend
	// anchor determines where the "position" is on the sprite. An anchor of {0, 0} puts the position at the top left, an anchor of {1, 1} puts the position at the bottom right.  
	// This also determines the point in which the sprite is rotated around.
	// section determines what part of the sprite is drawn. When section is the unit square the entire sprite is drawn. If section is {0, 0, 0.5, 0.5} only the topleft quadrant of the sprite is drawn.
	void drawSprite(TEXTURE_ASSET_ID textureId, Vector2 position, Vector2 scale = Vector2::one(), float rotation = 0.0f, Color modulate = Color::white(), Vector2 anchor = {0, 0}, Camera camera = Camera(), Rect section = Rect::unit(), Color overrideColor = Color::clear()) {
		
	
		// CALCULTE TRANSFORM
		Vector2 dimensions = scale * Vector2(textureDimensions[(int)textureId]) * section.size();
		Transform transform;
		
		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
		transform.matrix /= camera.getTransform(windowSize).matrix;

		transform.translate(position - (dimensions * anchor).rotated(rotation));
		transform.rotate(rotation);
		transform.scale(dimensions);
		
		// SETTING SHADER
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
		glUseProgram(program);
		glHasErrors();
		
		// BIND VBO / IBO
		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();

		// IN POSITION
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		glHasErrors();
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();

		// IN TEXCOORD
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		assert(in_texcoord_loc >= 0);
		glHasErrors();
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				glm::vec3)); // note the stride to skip the preceeding vertex position
		glVertexAttribDivisor(in_texcoord_loc, 0);

		// TEXTURE
		glActiveTexture(GL_TEXTURE0);
		glHasErrors();

		GLuint texture_id = textureGlHandles[(int) textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();
		
		

		// MODULATE
		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		glUniform4fv(color_uloc, 1, (float*)&modulate);
		glHasErrors();


		// OVERRIDE COLOR
		GLint overrideColorLoc = glGetUniformLocation(program, "overrideColor");
		glUniform4fv(overrideColorLoc, 1, (float*)&overrideColor);
		glHasErrors();

		
		
		// TRANSFORM
		GLuint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.matrix);
		glHasErrors();

		// PROJECTION
		GLuint projection_loc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projectionMatrix);
		glHasErrors();


		// TEXCOORD RECT
		glm::vec2 sectionTopLeft = section.topLeft();
		glm::vec2 sectionSize = section.size();

		GLuint texcoord_offset_loc = glGetUniformLocation(program, "texcoord_offset");
		glUniform2fv(texcoord_offset_loc, 1, (float*)&sectionTopLeft);
		glHasErrors();

		GLuint texcoord_scale_loc = glGetUniformLocation(program, "texcoord_scale");
		glUniform2fv(texcoord_scale_loc, 1, (float*)&sectionSize);
		glHasErrors();

		
		

		// DRAW
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);  // Get number of indices from index buffer, which has elements uint16_t
		glHasErrors();
		GLsizei num_indices = size / sizeof(uint16_t);

		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		glHasErrors();
	}


	void drawSpriteOutlined(TEXTURE_ASSET_ID textureId, Vector2 position, Vector2 scale = Vector2::one(), float rotation = 0.0f, Color modulate = Color::white(), Vector2 anchor = { 0, 0 }, Camera camera = Camera(), Rect section = Rect::unit(), Color overrideColor = Color::clear(), Color outlineColor = Color::clear(), float outlineSize = 0.0f) {


		// CALCULTE TRANSFORM
		Vector2 dimensions = scale * Vector2(textureDimensions[(int)textureId]) * section.size();
		Transform transform;

		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
		transform.matrix /= camera.getTransform(windowSize).matrix;

		transform.translate(position - (dimensions * anchor).rotated(rotation));
		transform.rotate(rotation);
		transform.scale(dimensions);

		// SETTING SHADER
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED_OUTLINED];
		glUseProgram(program);
		glHasErrors();

		// BIND VBO / IBO
		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();

		// IN POSITION
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		glHasErrors();
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();

		// IN TEXCOORD
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		assert(in_texcoord_loc >= 0);
		glHasErrors();
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				glm::vec3)); // note the stride to skip the preceeding vertex position
		glVertexAttribDivisor(in_texcoord_loc, 0);

		// TEXTURE
		glActiveTexture(GL_TEXTURE0);
		glHasErrors();

		GLuint texture_id = textureGlHandles[(int)textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();



		// MODULATE
		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		glUniform4fv(color_uloc, 1, (float*)&modulate);
		glHasErrors();


		// OVERRIDE COLOR
		GLint overrideColorLoc = glGetUniformLocation(program, "overrideColor");
		glUniform4fv(overrideColorLoc, 1, (float*)&overrideColor);
		glHasErrors();


		// OUTLINE COLOR
		GLint outlineColorLoc = glGetUniformLocation(program, "outlineColor");
		glUniform4fv(outlineColorLoc, 1, (float*)&outlineColor);
		glHasErrors();

		// OUTLINE SIZE
		GLint outlineSizeLoc = glGetUniformLocation(program, "outlineSize");
		glUniform1fv(outlineSizeLoc, 1, (float*)&outlineSize);
		glHasErrors();

		// TEXTURE SIZE
		vec2 textureSize = textureDimensions[(int)textureId];
		GLint textureSizeLoc = glGetUniformLocation(program, "textureSize");
		glUniform2fv(textureSizeLoc, 1, (float*)&textureSize);
		glHasErrors();


		// TRANSFORM
		GLuint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.matrix);
		glHasErrors();

		// PROJECTION
		GLuint projection_loc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projectionMatrix);
		glHasErrors();


		// TEXCOORD RECT
		glm::vec2 sectionTopLeft = section.topLeft();
		glm::vec2 sectionSize = section.size();

		GLuint texcoord_offset_loc = glGetUniformLocation(program, "texcoord_offset");
		glUniform2fv(texcoord_offset_loc, 1, (float*)&sectionTopLeft);
		glHasErrors();

		GLuint texcoord_scale_loc = glGetUniformLocation(program, "texcoord_scale");
		glUniform2fv(texcoord_scale_loc, 1, (float*)&sectionSize);
		glHasErrors();




		// DRAW
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);  // Get number of indices from index buffer, which has elements uint16_t
		glHasErrors();
		GLsizei num_indices = size / sizeof(uint16_t);

		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		glHasErrors();
	}


	
	void initInstancedRender(InstancedRender& instancedRender) {
		
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED_INSTANCED];
		glUseProgram(program);
		glHasErrors();


		
		glGenBuffers(1, &instancedRender.bases.vbo);
		instancedRender.bases.pushData();

		glGenBuffers(1, &instancedRender.offsets.vbo);
		instancedRender.offsets.pushData();
		
		glGenBuffers(1, &instancedRender.sections.vbo);
		instancedRender.sections.pushData();

		glGenBuffers(1, &instancedRender.colors.vbo);
		instancedRender.colors.pushData();
	}

	
	void destroyInstancedRender(InstancedRender& instancedRender) {
		glDeleteBuffers(1, &instancedRender.bases.vbo);
		glDeleteBuffers(1, &instancedRender.offsets.vbo);
		glDeleteBuffers(1, &instancedRender.sections.vbo);
		glDeleteBuffers(1, &instancedRender.colors.vbo);
	};

	
	void drawSpriteInstanced(TEXTURE_ASSET_ID textureId, InstancedRender& instancedRender, Transform overallTransform, Color overallModulate, Vector2 instanceAnchors, Camera camera = Camera()) {

		
		// SET PROGRAM
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED_INSTANCED];
		glUseProgram(program);
		glHasErrors();

		
		// VERTEX / ARRAY BUFFERS
		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();


		// IN POSITION
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*) 0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();


		// IN TEXCOORD
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(glm::vec3)); // note the stride to skip the preceeding vertex position
		glVertexAttribDivisor(in_texcoord_loc, 0);


		// TEXTURE
		glActiveTexture(GL_TEXTURE0);
		glHasErrors();

		GLuint texture_id = textureGlHandles[(int)textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();
		
		
		// BASIS
		GLint basisLoc = glGetAttribLocation(program, "basis");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.bases.vbo);

		for (unsigned int i = 0; i < 2; i++) {
			glEnableVertexAttribArray(basisLoc + i);
			glVertexAttribPointer(basisLoc + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::mat2), (void*)(i * sizeof(glm::vec2)));
			glVertexAttribDivisor(basisLoc + i, 1);
		}

		glHasErrors();

		// OFFSET
		GLint offsetLoc = glGetAttribLocation(program, "offset");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.offsets.vbo);

		glEnableVertexAttribArray(offsetLoc);
		glVertexAttribPointer(offsetLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
		glVertexAttribDivisor(offsetLoc, 1);
		glHasErrors();
		

		// SECTION
		GLint sectionLoc = glGetAttribLocation(program, "section");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.sections.vbo);

		for (unsigned int i = 0; i < 2; i++) {
			glEnableVertexAttribArray(sectionLoc + i);
			glVertexAttribPointer(sectionLoc + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::mat2), (void*)(i * sizeof(glm::vec2)));
			glVertexAttribDivisor(sectionLoc + i, 1);
		}
		glHasErrors();

		// COLOR
		GLint colorLoc = glGetAttribLocation(program, "modulate");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.colors.vbo);

		glEnableVertexAttribArray(colorLoc);
		glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glVertexAttribDivisor(colorLoc, 1);
		glHasErrors();

		
		// PROJECTION (UNIFORM)
		Transform projection;
		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
		projection.matrix /= camera.getTransform(windowSize).matrix;
		projection.matrix = projectionMatrix * projection.matrix;

		GLint projectionLoc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projectionLoc, 1, GL_FALSE, (float*)&projection);
		glHasErrors();


		// TEXTURE SIZE (UNIFORM)
		GLint textureSizeLoc = glGetUniformLocation(program, "textureSize");
		Vector2 textureSize = Vector2(textureDimensions[(int)textureId]);
		glUniform2fv(textureSizeLoc, 1, (float*)&textureSize);
		glHasErrors();

		// ANCHOR (UNIFORM)
		GLint anchorLoc = glGetUniformLocation(program, "anchor");
		glUniform2fv(anchorLoc, 1, (float*)&instanceAnchors);
		glHasErrors();

		// OVERALL TRANSFORM (UNIFORM)
		GLint overallTransformLoc = glGetUniformLocation(program, "overallTransform");
		glUniformMatrix3fv(overallTransformLoc, 1, GL_FALSE, (float*)&overallTransform);
		glHasErrors();

		// OVERALL MODULATE (UNIFORM)
		GLint overallModulateLoc = glGetUniformLocation(program, "overallModulate");
		glUniform4fv(overallModulateLoc, 1, (float*)&overallModulate);
		glHasErrors();
		

		
		// GET NUM INDICES
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		GLsizei num_indices = size / sizeof(uint16_t);
		glHasErrors();

		// DRAW
		glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, instancedRender.instanceCount());
		glHasErrors();
	}



	void drawSpriteInstancedOutlined(TEXTURE_ASSET_ID textureId, InstancedRender& instancedRender, Transform overallTransform, Color overallModulate, Vector2 instanceAnchors, Camera camera = Camera(), Color outlineColor = Color::clear(), float outlineSize = 0.0f) {


		// SET PROGRAM
		const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED_INSTANCED_OUTLINED];
		glUseProgram(program);
		glHasErrors();


		// VERTEX / ARRAY BUFFERS
		const GLuint vbo = vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		const GLuint ibo = indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glHasErrors();


		// IN POSITION
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();


		// IN TEXCOORD
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(glm::vec3)); // note the stride to skip the preceeding vertex position
		glVertexAttribDivisor(in_texcoord_loc, 0);


		// TEXTURE
		glActiveTexture(GL_TEXTURE0);
		glHasErrors();

		GLuint texture_id = textureGlHandles[(int)textureId];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glHasErrors();


		// BASIS
		GLint basisLoc = glGetAttribLocation(program, "basis");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.bases.vbo);

		for (unsigned int i = 0; i < 2; i++) {
			glEnableVertexAttribArray(basisLoc + i);
			glVertexAttribPointer(basisLoc + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::mat2), (void*)(i * sizeof(glm::vec2)));
			glVertexAttribDivisor(basisLoc + i, 1);
		}

		glHasErrors();

		// OFFSET
		GLint offsetLoc = glGetAttribLocation(program, "offset");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.offsets.vbo);

		glEnableVertexAttribArray(offsetLoc);
		glVertexAttribPointer(offsetLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
		glVertexAttribDivisor(offsetLoc, 1);
		glHasErrors();


		// SECTION
		GLint sectionLoc = glGetAttribLocation(program, "section");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.sections.vbo);

		for (unsigned int i = 0; i < 2; i++) {
			glEnableVertexAttribArray(sectionLoc + i);
			glVertexAttribPointer(sectionLoc + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::mat2), (void*)(i * sizeof(glm::vec2)));
			glVertexAttribDivisor(sectionLoc + i, 1);
		}
		glHasErrors();

		// COLOR
		GLint colorLoc = glGetAttribLocation(program, "modulate");
		glBindBuffer(GL_ARRAY_BUFFER, instancedRender.colors.vbo);

		glEnableVertexAttribArray(colorLoc);
		glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glVertexAttribDivisor(colorLoc, 1);
		glHasErrors();


		// PROJECTION (UNIFORM)
		Transform projection;
		glm::ivec2 windowSize;
		glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
		projection.matrix /= camera.getTransform(windowSize).matrix;
		projection.matrix = projectionMatrix * projection.matrix;

		GLint projectionLoc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projectionLoc, 1, GL_FALSE, (float*)&projection);
		glHasErrors();


		// OUTLINE COLOR (UNIFORM)
		GLint outlineColorLoc = glGetUniformLocation(program, "outlineColor");
		glUniform4fv(outlineColorLoc, 1, (float*)&outlineColor);
		glHasErrors();

		// OUTLINE SIZE (UNIFORM)
		GLint outlineSizeLoc = glGetUniformLocation(program, "outlineSize");
		glUniform1fv(outlineSizeLoc, 1, (float*)&outlineSize);
		glHasErrors();

		

		// TEXTURE SIZE (UNIFORM)
		GLint textureSizeLoc = glGetUniformLocation(program, "textureSize");
		Vector2 textureSize = Vector2(textureDimensions[(int)textureId]);
		glUniform2fv(textureSizeLoc, 1, (float*)&textureSize);
		glHasErrors();

		// ANCHOR (UNIFORM)
		GLint anchorLoc = glGetUniformLocation(program, "anchor");
		glUniform2fv(anchorLoc, 1, (float*)&instanceAnchors);
		glHasErrors();

		// OVERALL TRANSFORM (UNIFORM)
		GLint overallTransformLoc = glGetUniformLocation(program, "overallTransform");
		glUniformMatrix3fv(overallTransformLoc, 1, GL_FALSE, (float*)&overallTransform);
		glHasErrors();

		// OVERALL MODULATE (UNIFORM)
		GLint overallModulateLoc = glGetUniformLocation(program, "overallModulate");
		glUniform4fv(overallModulateLoc, 1, (float*)&overallModulate);
		glHasErrors();



		// GET NUM INDICES
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		GLsizei num_indices = size / sizeof(uint16_t);
		glHasErrors();

		// DRAW
		glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, instancedRender.instanceCount());
		glHasErrors();
	}


	void drawSpriteIndexed(
		TEXTURE_ASSET_ID textureId,
		glm::ivec2 spriteCount, 
		unsigned int spriteIndex,
		Vector2 position, 
		Vector2 scale = Vector2::one(), 
		float rotation = 0.0f, 
		Color modulate = Color::white(),
		Vector2 anchor = { 0, 0 }, 
		Camera camera = Camera(),
		Color overrideColor = Color::clear())
	{
		
		//std::cout << spriteIndex << ": " << subSpritePosition.x << " " << subSpritePosition.y << " count: " << spriteCount.x << " " << spriteCount.y << " scale: " << subSpriteSize.x << " " << subSpriteSize.y << std::endl;
		drawSprite(textureId, position, scale, rotation, modulate, anchor, camera, getSubSpriteRect(spriteCount, spriteIndex), overrideColor);
	}

	void drawGameBackground(const Camera& camera) {

		for (auto& entity : ecs.arenas.entities) {
			Arena& arena = ecs.arenas.get(entity);
			drawSprite(
				arena.getBackgroundTexture(),
				Vector2::zero(),
				Vector2::one(),
				0.0f,
				Color::white(),
				Vector2::unitCenter(),
				camera
			);
		}

		for (auto& entity : ecs.backgroundParticles.entities) {
			drawParticleEntity(entity, camera);
		}

		
	}

	


	// first draw to an intermediate texture,
	// apply the "vignette" texture, when requested
	// then draw the intermediate texture
	void drawToScreen() {
		// Setting shaders
	// get the vignette texture, sprite mesh, and program
		glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
		glHasErrors();

		// Clearing backbuffer
		int w, h;
		glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glDepthRange(0, 10);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glHasErrors();
		// Enabling alpha channel for textures
		glDisable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		// Draw the screen texture on the quad geometry
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
		glBindBuffer(
			GL_ELEMENT_ARRAY_BUFFER,
			indexBuffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
		// indices to the bound GL_ARRAY_BUFFER
		glHasErrors();

		// add the "vignette" effect
		const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];


		// Set the vertex position and vertex texture coordinates (both stored in the
		// same VBO)
		GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glVertexAttribDivisor(in_position_loc, 0);
		glHasErrors();

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, offScreenRenderBufferColor);
		glHasErrors();

		// Draw
		glDrawElements(
			GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
			nullptr); // one triangle = 3 vertices; nullptr indicates that there is
		// no offset from the bound index buffer
		glHasErrors();
		
	}

	// DRAW PARTICLES
	void drawParticleEntity(const Entity& entity, const Camera& camera) {
		
		auto& instancedRender = ecs.instancedRenders.get(entity);
		auto& particleEmitter = ecs.particleEmitters.get(entity);
		auto& transform = ecs.transforms.get(entity);

		drawSpriteInstanced(particleEmitter.textureId, instancedRender, transform, Color::white(), Vector2::unitCenter(), camera);
		
	}

	// DRAWING TILES
	void drawTiles(const Camera& camera) {

		

		for (auto& entity : ecs.tilemaps.entities) {
			auto& tileset = ecs.tilesets.get(entity);
			auto& tilemap = ecs.tilemaps.get(entity);
			auto& transform = ecs.transforms.get(entity);
			auto& instancedRender = ecs.instancedRenders.get(entity);

			drawSpriteInstanced(tileset.texture, instancedRender, transform, Color::white(), Vector2::unitTopLeft(), camera);
		}

		if (camera.debug) {
			for (auto& entity : ecs.pathingGraphs.entities) {

				auto& pathingGraph = ecs.pathingGraphs.get(entity);
				auto& transform = ecs.transforms.get(entity);

				for (auto& walkableCoordinate : pathingGraph.walkableCoordinates) {
					glm::ivec2 coordinate = pathingGraph.getCoordinate(walkableCoordinate);
					Vector2 position = pathingGraph.getNodePosition(coordinate, transform.getTranslation());
					drawSprite(TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX, position, Vector2::one(), 0, Color::magenta(), Vector2::unitCenter(), camera, getSubSpriteRect({ 5, 1 }, 0));

					PathingNode& pathingNode = pathingGraph.getNode(coordinate);

					for (auto& connection : pathingNode.reachableCoordinates) {
						Vector2 position2 = pathingGraph.getNodePosition(connection.coordinate, transform.getTranslation());
						Color color = Color::cyan();
						if (connection.flavor == PATH_EDGE_FLAVOR::FALL_WALK) color = Color::yellow();
						if (connection.flavor == PATH_EDGE_FLAVOR::JUMP) color = Color::red();

						color.a = 0.5;

						drawLine(position, position2, 2, color, camera);
					}
				}

			}
		}
	}

	TEXTURE_ASSET_ID getPlayerSprite(int id, CLASS_ID classId) {
		switch (id) {
		case 1:
			switch (classId) {
			case CLASS_ID::KNIGHT: return TEXTURE_ASSET_ID::KNIGHT_A;
			case CLASS_ID::ARCHER: return TEXTURE_ASSET_ID::ARCHER_A;
			case CLASS_ID::MAGE:	return TEXTURE_ASSET_ID::MAGE_A;
			}
		case 2:
			switch (classId) {
			case CLASS_ID::KNIGHT: return TEXTURE_ASSET_ID::KNIGHT_B;
			case CLASS_ID::ARCHER: return TEXTURE_ASSET_ID::ARCHER_B;
			case CLASS_ID::MAGE:	return TEXTURE_ASSET_ID::MAGE_B;
			}
		}
	}

	TEXTURE_ASSET_ID getPlayerSprite(Player& player) {
		return getPlayerSprite(player.id, player.class_id);
	}

	// DRAWING PLAYERS
	void drawPlayers(const Camera& camera, float delta) {
		for (auto& entity : ecs.players.entities) {
			auto& transform = ecs.transforms.get(entity);
			auto& player = ecs.players.get(entity);
			Dash& dash = ecs.dashes.get(entity);
			PlayerInput& input = ecs.playerInputs.get(entity);

			float walkDirection = input.getAimVector().x;

			auto& kinematic = ecs.kinematics.get(entity);

			// Determine which animation to play based on state
			Animation* nextAnimation = &player.idleAnimation;

			if (player.isDead) {
				nextAnimation = &player.deathAnimation;
				if (!player.deathAnimation.isPlaying) {
					player.deathAnimation.currentFrameIndex = player.deathAnimation.frames.size() - 1;
				}
			}
			else {
				player.deathAnimation.reset();
				player.deathAnimation.play();

				if (kinematic.velocity.y != 0.0f) {  // Falling
					nextAnimation = &player.fallAnimation;
				}
				else if (abs(walkDirection) > 0.1f) {
					nextAnimation = &player.walkAnimation;
				}
			}

			if (player.currentAnimation != nextAnimation) {
				player.currentAnimation = nextAnimation;
				if (nextAnimation == &player.deathAnimation) {
					nextAnimation->reset();
					nextAnimation->play();
				}
			}

			player.currentAnimation->update(delta);

			
			Color overrideColor = player.isHitFlashed() ? Color::white() : Color::clear();
			Knockable knockable = ecs.knockables.get(entity);


			// M1 [3] Key-frame/state interpolation
			// M1 Basic Feature: [1] Simple rendering effects
			// make player darker the more their attack is charged
			const Color gray = Color::white().lerp(Color::black(), 0.5f);
			Color modulate = Color::white();
			Weapon& weapon = ecs.weapons.get(player.weapon);

			

			if (knockable.isKnocked())		modulate = modulate.lerp(gray, 1.3f);
			//else if (weapon.isCharging())	modulate = modulate.lerp(gray, weapon.attackChargeAmount);
			//else if (weapon.isAttacking())  modulate = modulate.lerp(gray, weapon.attackStrength);
			else if (weapon.isAttacking() || weapon.inCooldown())	modulate = modulate.lerp(gray, 1.3f);
			 


			
			float outlineSize = player.isSpecialCharged ? 1.0f : 0.0f;
			Color outlineColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

			if (weapon.attackChargeAmount >= 1.0f) {
				outlineSize = 1.0f;
				outlineColor = Color(1.0f, 0.7f, 0.3f);
			}
			
			// draw player
			drawSpriteOutlined(
				getPlayerSprite(player),
				transform.getTranslation(),
				Vector2(transform.getScale()) * (player.isFacingRight ? Vector2(1, 1) : Vector2(-1, 1)),
				transform.getRotation(),
				modulate,
				Vector2(0.5f, 36.0f / 52.0f),
				camera,
				getSubSpriteRect(glm::ivec2(34, 1), player.currentAnimation->getCurrentFrame()),
				overrideColor,
				outlineColor,
				outlineSize
			);


			// DRAW WEAPON 

			{
				auto& transform = ecs.transforms.get(player.weapon);
				auto& weapon = ecs.weapons.get(player.weapon);

				switch (weapon.id) {
				case WEAPON_ID::SWORD: {
					drawSpriteOutlined(TEXTURE_ASSET_ID::SWORD, transform.getTranslation(), transform.getScale(), transform.getRotation() + std::numbers::pi / 2, modulate, Vector2(0.5, 0.8), camera, Rect::unit(), overrideColor, outlineColor, outlineSize);
				} break;
				case WEAPON_ID::BOW: {

					unsigned int index = weapon.attackChargeAmount == 1.0f ? 3 : (int)(weapon.attackChargeAmount * 4);

					if (weapon.isCharging()) {
						int a[] = { 13, 12, 11, 9 };
						Vector2 pos = transform.getTranslation() + (Vector2::right().rotated(transform.getRotation())) * a[index];
						drawSpriteOutlined(TEXTURE_ASSET_ID::ARROW, pos, transform.getScale(), transform.getRotation(), modulate, Vector2(1.0, 0.5) - Vector2(2, 0) / Vector2(textureDimensions[(int)TEXTURE_ASSET_ID::ARROW]), camera, Rect::unit(), overrideColor, outlineColor, outlineSize);
					}

					
					drawSpriteOutlined(TEXTURE_ASSET_ID::BOW, transform.getTranslation(), transform.getScale(), transform.getRotation(), modulate, Vector2(1.0, 0.5) - Vector2(2, 0) / Vector2(textureDimensions[(int)TEXTURE_ASSET_ID::BOW]), camera, getSubSpriteRect({4, 1}, index), overrideColor, outlineColor, outlineSize);

					
				} break;
				case WEAPON_ID::STAFF: {
					drawSpriteOutlined(TEXTURE_ASSET_ID::STAFF, transform.getTranslation(), transform.getScale(), transform.getRotation() + std::numbers::pi / 2, modulate, Vector2::unitCenter(), camera, Rect::unit(), overrideColor, outlineColor, outlineSize);
					if (weapon.attackChargeAmount > 0.0f)
						drawSprite(TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX, transform.getTranslation() + Vector2::right().rotated(transform.getRotation()) * 9.0f, transform.getScale(), transform.getRotation() + std::numbers::pi / 2, Color::white().lerp(Color::yellow(), 0.2f), Vector2::unitCenter(), camera, getSubSpriteRect(glm::ivec2(5, 1), floor((1 - weapon.attackChargeAmount * 2.0 / 3.0) * 4.9)), overrideColor);
				} break;
				}
			}
			
		}

		// DRAW ARROWS
		for (auto& entity : ecs.arrows.entities) {
			auto& transform = ecs.transforms.get(entity);
			Arrow& arrow = ecs.arrows.get(entity);

			drawSprite(TEXTURE_ASSET_ID::ARROW, transform.getTranslation(), transform.getScale(), transform.getRotation() + arrow.rotation, Color::white(), Vector2(0.85, 0.5), camera);
		}


		// DRAW FIREBALLS
		for (auto& entity : ecs.fireballs.entities) {
			drawParticleEntity(ecs.fireballs.get(entity).particle, camera);
		}


		// DRAW EXPLOSIONS
		for (auto& entity : ecs.explosions.entities) {
			drawParticleEntity(entity, camera);
		}

		for (auto& entity : ecs.explosions.entities) {
			auto& transform = ecs.transforms.get(entity);
			auto& explosion = ecs.explosions.get(entity);

			if (!ecs.attacks.has(explosion.hitbox)) continue;

			drawSprite(TEXTURE_ASSET_ID::CIRCLE_48PX, transform.getTranslation(), Vector2::one(), 0, Color::white(), Vector2::unitCenter(), camera);
		}

		// DRAW AI PATH
		if (camera.debug) {
			if (ecs.pathingGraphs.components.size() > 0) {
				auto& pathingGraphEntity = ecs.pathingGraphs.entities.front();
				auto& pathingGraph = ecs.pathingGraphs.get(pathingGraphEntity);
				auto& pathingGraphTransform = ecs.transforms.get(pathingGraphEntity);
				for (auto& entity : ecs.aiAgents.entities) {
					auto& aiAgent = ecs.aiAgents.get(entity);
					auto& transform = ecs.transforms.get(entity);

					if (aiAgent.path.edges.size() == 0) continue;

					Vector2 pos = pathingGraph.getNodePosition(aiAgent.path.edges.back().coordinate, pathingGraphTransform.getTranslation());
					for (int i = aiAgent.path.edges.size() - 2; i >= 0; i--) {
						auto& edge = aiAgent.path.edges[i];
						Vector2 newPos = pathingGraph.getNodePosition(edge.coordinate, pathingGraphTransform.getTranslation());
						drawLine(pos, newPos, 3, Color::cyan(), camera);
						pos = newPos;
					}

					drawSprite(TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX, pos, Vector2::one(), 0, Color::lime(), Vector2::unitCenter(), camera, getSubSpriteRect({ 5, 1 }, 0));
				}


			}
		}
	}

	// Render our game world
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
	void draw(float delta, GAME_SCREEN game_screen) {
		// Getting size of window
		int w, h;
		glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

		// First render to the custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glHasErrors();

		// clear backbuffer
		glViewport(0, 0, w, h);
		glDepthRange(0.00001, 10);


		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(10.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
		// and alpha blending, one would have to sort
		// sprites back to front
		glHasErrors();

		// update projection matrix
		projectionMatrix = createProjectionMatrix();




		/////////////////////////////////////
		// START DRAWING STUFF HERE
		/////////////////////////////////////


		Camera camera = defaultCamera;
		camera.debug = debug.flags & (int)DEBUG_FLAGS::SHOW_COLLISION_BOXES;

		if (game_screen == GAME_SCREEN::GAME_END_SCREEN) {
			TEXTURE_ASSET_ID playerSprite;

			drawSprite(
				TEXTURE_ASSET_ID::BACKGROUND_GAME_OVER,
				Vector2::zero(),
				Vector2::one(),
				0.0f,
				Color::white(),
				Vector2::unitCenter(),
				camera
			);

			if (gameState.gameScore.player1Wins > gameState.gameScore.player2Wins) {
				playerSprite = getPlayerSprite(1, gameState.player1Options.selectedClass);
			}
			else {
				playerSprite = getPlayerSprite(2, gameState.player2Options.selectedClass);
			}

			drawSpriteIndexed(playerSprite, glm::ivec2(34, 1), 0, Vector2(0, -15), Vector2::one() * 3, 0, Color::white(), Vector2::unitCenter(), camera);

			for (int i = 0; i < gameState.gameScore.player2Wins; i++) {
				drawSprite(
					TEXTURE_ASSET_ID::STAR,
					Vector2(150, (40 - 20 * i)),
					Vector2::one(),
					0.0f,
					Color::white(),
					Vector2::unitCenter(),
					camera
				);
			}

			drawSpriteIndexed(getPlayerSprite(2, gameState.player2Options.selectedClass), glm::ivec2(34, 1), 0, Vector2(150, 70), Vector2(1, 1), 0, Color::white(), Vector2::unitCenter(), camera);

			for (int i = 0; i < gameState.gameScore.player1Wins; i++) {
				drawSprite(
					TEXTURE_ASSET_ID::STAR,
					Vector2(-150, (40 - 20 * i)),
					Vector2::one(),
					0.0f,
					Color::white(),
					Vector2::unitCenter(),
					camera
				);
			}

			drawSpriteIndexed(getPlayerSprite(1, gameState.player1Options.selectedClass), glm::ivec2(34, 1), 0, Vector2(-150, 70), Vector2(1, 1), 0, Color::white(), Vector2::unitCenter(), camera);
		} else if (game_screen == GAME_SCREEN::CHARACTER_SELECT_SCREEN) {

			drawSprite(
				TEXTURE_ASSET_ID::BACKGROUND_CHAR_SELECT,
				Vector2::zero(),
				Vector2::one(),
				0.0f,
				Color::white(),
				Vector2::unitCenter(),
				camera
			);

			

			for (auto& entity : ecs.characterSelects.entities) {
				auto& characterSelect = ecs.characterSelects.get(entity);
				auto& transform = ecs.transforms.get(entity);
				auto& playerInput = ecs.playerInputs.get(entity);
				auto& playerOptions = ecs.playerOptions.get(entity);


				// Draw the current class sprite
				TEXTURE_ASSET_ID spriteId = getPlayerSprite(playerInput.controllerId, playerOptions.selectedClass);

				drawSpriteIndexed(
					spriteId,
					glm::ivec2(34, 1),
					characterSelect.idleAnimation.getCurrentFrame(),  // Use the instance's animation
					transform.getTranslation(),
					Vector2(playerInput.controllerId == 1 ? transform.getScale().x : -transform.getScale().x, transform.getScale().y),
					0.0f,
					characterSelect.hasConfirmed ? Color::green() : Color::white(),
					Vector2(0.5f, 36.0f / 48.0f),
					camera
				);

				if (playerOptions.playerType == PLAYER_TYPE_ID::HUMAN) {
					drawSprite(TEXTURE_ASSET_ID::INPUTS, transform.getTranslation() + Vector2::right() * 50, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::RIGHT)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, transform.getTranslation() + Vector2::left()  * 50, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::LEFT)));
				}

				if (playerInput.controllerId == 2) {
					drawSprite(TEXTURE_ASSET_ID::INPUTS, transform.getTranslation() + Vector2(73, -88), Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({34, 24}, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::UP)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, transform.getTranslation() + Vector2(73, -88) + Vector2::down() * 15, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({34, 24}, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::DOWN)));
				}

				Vector2 inputPos = transform.getTranslation() + Vector2(-50 + 32, 46);
				Vector2 spacing = Vector2(13, 15);

				
				if (playerOptions.playerType == PLAYER_TYPE_ID::HUMAN) {
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 0 + Vector2::left() * spacing * 3, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::UP)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 0 + Vector2::left() * spacing * 2, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::LEFT)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 0 + Vector2::left() * spacing * 1, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::DOWN)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 0 + Vector2::left() * spacing * 0, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::RIGHT)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 1, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::JUMP)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 2, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::ATTACK)));
					drawSprite(TEXTURE_ASSET_ID::INPUTS, inputPos + Vector2::down() * spacing * 3, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(playerInput.controllerId, INPUT_ID::SPECIAL)));
				}

				for (auto& textEntity : characterSelect.controlTexts) {
					ecs.texts.get(textEntity).color = playerOptions.playerType == PLAYER_TYPE_ID::HUMAN ? Color::black() : Color::clear();
				}
			}
		}
		else if (game_screen == GAME_SCREEN::ARENA_SELECT_SCREEN) {
			drawSprite(
				TEXTURE_ASSET_ID::BACKGROUND_ARENA_SELECT,
				Vector2::zero(),
				Vector2::one(),
				0.0f,
				Color::white(),
				Vector2::unitCenter(),
				camera
			);

			drawSprite(getArenaBackgroundFromTheme(gameState.arenaTheme), Vector2::zero(), Vector2::one(), 0, Color::white(), Vector2::unitCenter(), camera, Rect(0.25, 0.25, 0.75, 0.75));

			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::right() * 150 + Vector2::up() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({34, 24}, getInputSpriteIndex(1, INPUT_ID::RIGHT)));
			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::left() * 150 + Vector2::up() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(1, INPUT_ID::LEFT)));
			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::right() * 150 + Vector2::down() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(2, INPUT_ID::RIGHT)));
			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::left() * 150 + Vector2::down() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(2, INPUT_ID::LEFT)));

			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::down() * 100 + Vector2::left() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(1, INPUT_ID::JUMP)));
			drawSprite(TEXTURE_ASSET_ID::INPUTS, Vector2::down() * 100 + Vector2::right() * 8, Vector2::one(), 0.0f, Color::white(), Vector2::unitCenter(), camera, getSubSpriteRect({ 34, 24 }, getInputSpriteIndex(2, INPUT_ID::JUMP)));
		}
		else if (gameState.currentState == GAME_SCREEN_ID::DRAWING_MAIN ||
			gameState.currentState == GAME_SCREEN_ID::DRAWING_BACKGROUND) {
			drawGameBackground(camera);
			drawTiles(camera);
			drawGrid(camera);
		}
		else if (gameState.currentState == GAME_SCREEN_ID::PLAYING) {
			drawGameBackground(camera);
			drawTiles(camera);
			drawPlayers(camera, delta);
			drawHUD(delta);
		}
		else if (game_screen == GAME_SCREEN::TITLE_SCREEN) {
			drawGameBackground(camera);
			drawTiles(camera);
			drawPlayers(camera, delta);

			drawBottomChars(camera);
			drawSprite(
				TEXTURE_ASSET_ID::BACKGROUND_TITLE_BANNER,
				Vector2(0, -65),
				Vector2(0.35, 0.27),
				0.0f,
				Color::white(),
				Vector2::unitCenter(),
				camera
			);
		}

		

		if (camera.debug) {
			for (auto& entity : ecs.collisionBoxes.entities) {
				auto& transform = ecs.transforms.get(entity);
				auto& collisionBox = ecs.collisionBoxes.get(entity);

				Transform drawTransform = Transform(transform);
				drawTransform.translate(collisionBox.bounds.topLeft());
				drawTransform.scale(collisionBox.bounds.size());

				drawSprite(TEXTURE_ASSET_ID::PIXEL, drawTransform.getTranslation(), drawTransform.getScale(), 0.0f, Color(1.0f, 0.0f, 0.0f, 0.5f), Vector2::unitTopLeft(), camera);
			}

			for (auto& entity : ecs.collisionPolygons.entities) {
				auto& transform = ecs.transforms.get(entity);
				auto& collisionPolygon = ecs.collisionPolygons.get(entity);

				drawPolygon(collisionPolygon.polygonId, transform.getTranslation(), transform.getScale(), transform.getRotation(), Color(1.0f, 0.0f, 0.0f, 0.5f), camera);
			}
		}

			// DRAWING TEXT
		if (ecs.texts.has(gameState.gameOverText)) {
			drawText(camera, gameState.gameOverText);
		} else {
			for (auto& entity : ecs.texts.entities) {
				drawText(camera, entity);
			}
		}

			// float timeSec = ((uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000.0f;
			// drawPolygon(GEOMETRY_BUFFER_ID::SWORD, Vector2::zero(), Vector2(40.0f, 20.0f), timeSec, Color::blue(), camera);




			//drawPolygon(GEOMETRY_BUFFER_ID::SQUARE, Vector2::zero(), Vector2::one() * 50, 0, Color(1.0f, 0.0f, 0.0f, 0.5f), camera);

			/////////////////////////////////////
			// STOP DRAWING HERE
			/////////////////////////////////////

			drawToScreen();

			// flicker-free display with a double buffer
			glfwSwapBuffers(window);
			glHasErrors();
		}

		void drawBottomChars(const Camera& camera)
		{
			float h = 102;
			float w = 205;
			float spacing = 53;
			float scale = 2.3f;

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::MAGE_A,
				glm::ivec2(34, 1),
				0,
				Vector2(-w, h) * 1.0,
				Vector2(scale, scale),
				0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::ARCHER_A,
				glm::ivec2(34, 1),
				0,
				Vector2(-(w - spacing), h) * 1.0,
				Vector2(scale, scale),
				0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::KNIGHT_A,
				glm::ivec2(34, 1),
				0,
				Vector2(-(w - 2*spacing), h) * 1.0,
				Vector2(scale, scale),
				0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::MAGE_B,
				glm::ivec2(34, 1),
				0,
				Vector2(w, h) * 1.0,
				Vector2(-scale, scale),
				-0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::ARCHER_B,
				glm::ivec2(34, 1),
				0,
				Vector2(w - spacing, h) * 1.0,
				Vector2(-scale, scale),
				-0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

			drawSpriteIndexed(
				TEXTURE_ASSET_ID::KNIGHT_B,
				glm::ivec2(34, 1),
				0,
				Vector2(w - 2*spacing, h) * 1.0,
				Vector2(-scale, scale),
				-0.1f,
				Color::white(),
				Vector2::unitCenter(),
				camera);

		}

		glm::mat3 createProjectionMatrix() {
			// fake projection matrix, scaled to window coordinates
			float left = 0.f;
			float top = 0.f;

			glm::ivec2 windowSize;
			glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);

			float right = (float)windowSize.x;
			float bottom = (float)windowSize.y;

			float sx = 2.f / (right - left);
			float sy = 2.f / (top - bottom);
			float tx = -(right + left) / (right - left);
			float ty = -(top + bottom) / (top - bottom);

			return {
				{ sx, 0.f, 0.f},
				{0.f,  sy, 0.f},
				{ tx,  ty, 1.f}
			};
		}

	void drawText(Camera camera, Entity entity) {
		auto& transform = ecs.transforms.get(entity);
		auto& text = ecs.texts.get(entity);
		auto font = AtlasFont::fromId(text.fontId);
		auto& instancedRender = ecs.instancedRenders.get(entity);

		Vector2 charSize = Vector2::one() / Vector2(font.glyphCount) * textureDimensions[(int)font.textureId] - font.marginSize.size();
		Vector2 stringSize = charSize * Vector2(text.str.length(), 1) + text.charSpacing * (text.str.length() - 1);

		if (text.isPendingUpdate) {
			for (unsigned int i = 0; i < text.str.length(); i++) {
				wchar_t id = text.str[i] - font.firstCharacter;
				Rect subSpriteRect = getSubSpriteRect(font.glyphCount, id);
				instancedRender.offsets.data[i] = (Vector2::right() * charSize.x + text.charSpacing) * i - font.marginSize.topLeft();
				instancedRender.sections.data[i] = subSpriteRect;
			}

			for (unsigned int i = text.str.length(); i < instancedRender.instanceCount(); i++) {
				instancedRender.sections.data[i] = { 0, 0, 0, 0 };
			}

			instancedRender.offsets.pushData();
			instancedRender.sections.pushData();

			text.isPendingUpdate = false;
		}

		auto drawTransform = transform;
		drawTransform.translate(-stringSize * text.anchor);

		if (text.outlineSize > 0.0f && text.outlineColor.a > 0.0f)
			drawSpriteInstancedOutlined(font.textureId, instancedRender, drawTransform, text.color, Vector2::unitTopLeft(), camera, text.outlineColor, text.outlineSize);
		else
			drawSpriteInstanced(font.textureId, instancedRender, drawTransform, text.color, Vector2::unitTopLeft(), camera);
	}
};
