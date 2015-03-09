#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

class Texture {
public:
	Texture();
	~Texture();

	void create(int widthIn, int heightIn, bool useDepth, GLuint internalFormat, GLuint textureFormat, GLuint dataType);
	void destroy();
	int getWidth() const;
	int getHeight() const;
	void bind();
	void unbind();
	void unbindTexture();
	void unbindDepth();
	GLuint getTexture() const;
	GLuint getFBO() const;
	GLuint getDepth() const;

private:
	int width, height;
	GLuint fbo, tex, depth;
};

#endif