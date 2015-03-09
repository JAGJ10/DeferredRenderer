#include "Texture.h"

Texture::Texture() : fbo(0) {}

Texture::~Texture() {
	if (fbo != 0) {
		destroy();
	}
}

void Texture::create(int widthIn, int heightIn, bool useDepth, GLuint internalFormat, GLuint textureFormat, GLuint dataType) {
	width = widthIn;
	height = heightIn;
	//Create texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Empty texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, textureFormat, dataType, nullptr);

	//Unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	//Create fbo
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	if (useDepth) {
		glGenRenderbuffers(1, &depth);
		glBindRenderbuffer(GL_RENDERBUFFER, depth);

		//Set up the depth buffer
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

		//Attach the depth buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else {
		depth = 0;
	}

	//Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Texture::destroy() {
	if (fbo != 0) glDeleteFramebuffers(1, &fbo);
	if (tex != 0) glDeleteTextures(1, &tex);
	if (depth != 0) glDeleteRenderbuffers(1, &depth);

	fbo = 0;
}

int Texture::getWidth() const {
	return width;
}

int Texture::getHeight() const {
	return height;
}

void Texture::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Texture::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Texture::unbindTexture() {
	tex = 0;
}

void Texture::unbindDepth() {
	depth = 0;
}

GLuint Texture::getTexture() const {
	return tex;
}

GLuint Texture::getFBO() const {
	return fbo;
}

GLuint Texture::getDepth() const {
	return depth;
}