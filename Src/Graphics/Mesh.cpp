//
// Created by apant on 20/07/2025.
//

#include "Mesh.h"

#include <VkApp.h>

#include <stdexcept>
#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

void Mesh::Load(const char* file_path, Batch* batch)
{
	const struct aiScene* scene = aiImportFile(
		file_path,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipUVs |
		aiProcess_GenNormals);

	if (!scene)
	{
		throw std::runtime_error("Failed to load mesh");
	}

	uint32_t vertices_count = 0;
	QueryVerticesCount(
		scene,
		&vertices_count);

	batch->position.reserve(vertices_count);
	batch->color.reserve(vertices_count);
	batch->normals.reserve(vertices_count);

	uint32_t indeces_count = 0;
	QueryIndicesCount(
		scene,
		&indeces_count);

	batch->indices.reserve(indeces_count);

	QueryVertecesPosition(
		scene,
		batch->position);

	QueryVertecesNormal(
		scene,
		batch->normals);

	QueryIndices(
		scene,
		batch->indices);

	aiReleaseImport(scene);
}

void Mesh::QueryVerticesCount(
	const aiScene* scene,
	uint32_t*      vertices_count)
{
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		*vertices_count += scene->mMeshes[i]->mNumVertices;
	}
}

void Mesh::QueryVertecesPosition(
	const aiScene*          scene,
	std::vector<glm::vec3>& positions)
{
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			const aiVector3D& position = scene->mMeshes[i]->mVertices[j];
			positions.push_back(glm::vec3(position.x, position.y, position.z));
		}
	}
}

void Mesh::QueryVertecesNormal(
	const aiScene*          scene,
	std::vector<glm::vec3>& normals)
{
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			const aiVector3D& normal = scene->mMeshes[i]->mNormals[j];
			normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
		}
	}
}

void Mesh::QueryIndicesCount(
	const aiScene* scene,
	uint32_t*      indeces_count)
{
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		*indeces_count += scene->mMeshes[i]->mNumFaces * 3;
	}
}

void Mesh::QueryIndices(
	const aiScene*         scene,
	std::vector<uint32_t>& indices)
{
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		for (size_t j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
		{
			const aiFace& face = scene->mMeshes[i]->mFaces[j];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}
}