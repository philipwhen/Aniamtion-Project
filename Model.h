#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"

GLint TextureFromFile(const char* path, string directory);
enum state
{
	running, walking, jumping
}robotState;

class Model
{
public:
	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	glm::mat4 baseRotationMatrix=glm::mat4(),currentRotationMatrix= glm::mat4();
	GLfloat currentAngle,targetAngle, lastUpdate, angularSpeed, currentOffset, targetOffset, linearSpeed;
	GLfloat phaseStartTime, phaseDuration;
	GLint numOfPhases,currentPhaseIndex;
	glm::vec3 baseTranslate,rotationAxle, translateAxle;
	bool reverseMovement=false;//left move first, so right side is reversed movement
	bool angularMovement=true;

	GLfloat *keyFrameDatar,*keyFrameDataw,*keyFrameDataj; //a pointer point to key frame angle vector
	static GLfloat cycleDuration, cycleStartTime;

	Model(GLchar* path, GLuint objectID, glm::vec3 initialPosition=glm::vec3(0.0f, 0.0f, 0.0f), GLfloat initialRotationDegree=0,glm::vec3 rotationAxle= glm::vec3(0.0f, 0.0f, 1.0f),glm::vec3 movingAxle=glm::vec3(0.0f, 1.0f, 0.0f))
	{
		this->currentAngle = initialRotationDegree;
		this->baseTranslate = initialPosition;
		this->objectID = objectID;
		this->rotationAxle = rotationAxle;
		this->translateAxle = movingAxle;
		this->currentOffset = 0;
		baseRotationMatrix = glm::rotate(glm::mat4(), glm::radians(currentAngle), translateAxle);
		baseRotationMatrix = glm::translate(baseRotationMatrix,baseTranslate);
		this->loadModel(path);
	}

	// Draws the model, and all its meshes
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

	void initKeyFrameInfo(bool reversed, GLfloat *inputKeyFramer,GLfloat *inputKeyFramew, GLfloat *inputKeyFramej, GLint numerOfPhases, bool angularMovement=true){
		this->reverseMovement = reversed;
		this->keyFrameDatar = inputKeyFramer;
		this->keyFrameDataw = inputKeyFramew;
		this->keyFrameDataj = inputKeyFramej;
		this->numOfPhases = numerOfPhases;
		this->angularMovement = angularMovement;
		this->InitTransformMatrix();
	}

	void InitTransformMatrix() {
		if (angularMovement) {
			currentRotationMatrix = glm::rotate(baseRotationMatrix, glm::radians(currentAngle), rotationAxle);
		}
		else {
			currentRotationMatrix = glm::translate(baseRotationMatrix, currentOffset*translateAxle);
		}
	}

	void startMoving() {
		this->lastUpdate = glfwGetTime();
		this->currentPhaseIndex = reverseMovement?(numOfPhases/2+1):1;
		this->phaseStartTime = lastUpdate;
		this->phaseDuration = cycleDuration / numOfPhases;
		if (angularMovement) { 
			this->angularSpeed = (keyFrameDatar[currentPhaseIndex] - currentAngle) / phaseDuration; 
		}
		else { this->linearSpeed = (keyFrameDatar[currentPhaseIndex] - currentOffset) / phaseDuration; }
	}

	void frameUpdate() {
		//if phase changing
		if (glfwGetTime() - phaseStartTime >= phaseDuration) {
			currentPhaseIndex += 1;
			//a new cycle starts
			if(currentPhaseIndex >= numOfPhases) {
				currentPhaseIndex = 0;	
			}
			phaseStartTime = glfwGetTime();
			if (angularMovement) {
				if (robotState == running) {
					targetAngle = keyFrameDatar[currentPhaseIndex];
				}
				else if (robotState == walking) {
					targetAngle = keyFrameDataw[currentPhaseIndex];
				}
				else {
					targetAngle = keyFrameDataj[currentPhaseIndex];
				}
				
				angularSpeed = (targetAngle - currentAngle) / phaseDuration;
			}
			else {
				if (robotState == running) {
					targetOffset = keyFrameDatar[currentPhaseIndex];
				}
				else if (robotState == walking) {
					targetOffset =keyFrameDataw[currentPhaseIndex];
				}
				else{ targetOffset = keyFrameDataj[currentPhaseIndex]; }
				linearSpeed = (targetOffset - currentOffset) / phaseDuration;
			}
		}

		currentAngle += angularSpeed*(glfwGetTime() - lastUpdate);
		currentOffset += linearSpeed*(glfwGetTime() - lastUpdate);
		lastUpdate = glfwGetTime();

		if (angularMovement) {
			currentRotationMatrix = glm::rotate(baseRotationMatrix, glm::radians(currentAngle), rotationAxle);
		}
		else{
			currentRotationMatrix = glm::translate(baseRotationMatrix, currentOffset*translateAxle); 
		}
	}

private:
	/*  Model Data  */
	GLuint objectID;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										/*  Functions   */
										// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string path)
	{
		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));

		// Process ASSIMP's root node recursively
		this->processNode(scene->mRootNode, scene);
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Data to fill
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							  // Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// Normals
			/*vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;*/
			vertex.Normal = glm::vec3();
			// Texture Coordinates
			if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}
		// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// Process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN

			// 1. Diffuse maps
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. Specular maps
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		// Return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures, objectID);
	}

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // If texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}
};


GLfloat Model::cycleDuration;
GLfloat Model::cycleStartTime;


GLint TextureFromFile(const char* path, string directory)
{
	//Generate texture ID and load texture data 
	string filename = string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	//std::cout << "width is: " << width << "   height is: " << height << std::endl;
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}