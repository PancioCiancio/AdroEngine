//
// Created by apant on 07/07/2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Volk/volk.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Graphics
{
public:
	Graphics() = delete;

	/// Mostly used to group swapchain images, views and framebuffers
	/// @todo Presentation should be separated from base Graphics
	struct PerFrame
	{
		std::vector<VkImage>       images;
		std::vector<VkImageView>   image_views;
		std::vector<VkFramebuffer> framebuffers;
	};

	/// Uniform buffer data aligned
	struct PerFrameData
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};
};

#endif //GRAPHICS_H
