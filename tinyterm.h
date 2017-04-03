//#pragma once


#include <stdlib.h>


// Please use a numeric type, others will not work properly
// (char, int, etc... will work)
#ifndef TERM_CHAR_TYPE
#define TERM_CHAR_TYPE unsigned char
#endif

#ifndef TERM_NULL
#define TERM_NULL (void*)0
#endif

#define TERM_NO_ERROR 0
#define TERM_OUT_OF_BOUNDS 1
#define TERM_INVALID_ARGUMENT 2


#ifndef TERM_DEFINED
#define TERM_DEFINED
typedef struct termChar
{
	TERM_CHAR_TYPE character;
	unsigned char fr;
	unsigned char fg;
	unsigned char fb;

	unsigned char br;
	unsigned char bg;
	unsigned char bb;

} termChar;

typedef struct termScreen
{
	termChar* chars;
	int width;
	int height;
	int size;

} termScreen;


// Stores color as 3 bytes
// R-G-B 
typedef struct termImage
{
	// RGB data (24-bit data)
	unsigned char* data;
	int data_size;
	int width;
	int height;
} termImage;


// Could be memory-optimized further
typedef struct termBitImage
{
	//1 bit images (Tho they end up being 8-bit)
	char* data;
	int data_size;
	int width;
	int height;
} termBitImage;

typedef struct termFont
{
	termBitImage* characters;
	int totalCharacters;

	int width;
	int height;

} termFont;



// WARNING: This is a macro! img must be a termImage and glId a GLuint
#define term_upload_gl_image(img, glId) \
{ \
glBindTexture(GL_TEXTURE_2D, (GLuint)glId); \
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, *img.width, *img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, *img.data); \
glGenerateMipmap(GL_TEXTURE_2D); \
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); \
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); \
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); \
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); \
glBindTexture(GL_TEXTURE_2D, 0); \
} 


int term_set_char(termScreen* sc, int x, int y, termChar nChar);
termChar* term_get_char(termScreen* sc, int x, int y);
int term_render_image(termScreen* scr, termImage* outImage, termFont* font, int blending);
int term_render_image_sdl(termScreen* scr, char* pixels, int pitch, termFont* font, int blending);
termFont term_load_font(char* data, int data_size, int width, int charW, int charH, int alphaChannel);
termImage term_create_image(int w, int h);
char* term_get_printable(termScreen* scr);
termChar term_create_char(TERM_CHAR_TYPE character, char fr, char fg, char fb, char br, char bg, char bb);
termBitImage term_create_bit_image(char* data, int data_size, int width);
int term_define_font_character(int characterN, termFont* f, termBitImage img);
termFont term_create_font(int charW, int charH, int characterCount);
termScreen term_create_screen(int w, int h);

// The termFont will keep existing but all the data will be removed
void term_free_font(termFont* font);
// The termImage will keep existing but all the data will be removed
void term_free_image(termImage* img);
// The bitImage will keep existing but all the data will be removed
void term_free_bimage(termBitImage* img);
// The termScreen will keep existing but all the data will be removed
void term_free_screen(termScreen* scr);
#endif

#ifdef TERM_INCLUDE_DEFINITION
/* Definitions */



// Creates a empty screen
termScreen term_create_screen(int w, int h)
{
	termScreen out;
	out.width = w;
	out.height = h;
	out.size = w * h;
	out.chars = (termChar*)calloc(w * h, sizeof(termChar));

	return out;
}

// Sets char on termScreen "sc" at "x, y" to char "nChar"
// Returns error code to be parsed using term_get_error()
int term_set_char(termScreen* sc, int x, int y, termChar nChar)
{
	int i = sc->width * y + x;
	if (i < sc->size && i >= 0)
	{
		sc->chars[i] = nChar;
		return TERM_NO_ERROR;
	}
	else
	{
		return TERM_OUT_OF_BOUNDS;
	}
}

// Gets char on termScreen "sc" at "x, y"
// If any error occurs returns null
termChar* term_get_char(termScreen* sc, int x, int y)
{
	int i = sc->width * y + x;
	if (i < sc->size && i >= 0)
	{
		return &sc->chars[i];
	}
	else
	{
		return NULL;
	}
}

// Creates empty termFont with "characterCount" characters,
// and each character is "charW * charH" in size
termFont term_create_font(int charW, int charH, int characterCount)
{
	termFont out;
	out.width = charW;
	out.height = charH;
	out.totalCharacters = characterCount;
	out.characters = (termBitImage*)calloc(characterCount, sizeof(termBitImage));
	return out;
}

// Defines the "characterN" character of font "f" to termBitImage "img"
// Returns error coded
int term_define_font_character(int characterN, termFont* f, termBitImage img)
{
	if (img.width != f->width || img.height != f->height)
	{
		return TERM_INVALID_ARGUMENT;
	}

	if (characterN < f->totalCharacters && characterN >= 0)
	{
		f->characters[characterN] = img;
		return TERM_NO_ERROR;
	}
	else
	{
		return TERM_OUT_OF_BOUNDS;
	}
}



termBitImage term_create_bit_image(char* data, int data_size, int width)
{
	termBitImage out;
	out.data = data;
	out.data_size = data_size;
	out.width = width;
	out.height = data_size / width;
	return out;
}

termChar term_create_char(TERM_CHAR_TYPE character, char fr, char fg, char fb, char br, char bg, char bb)
{
	termChar out;
	out.character = character;
	out.fr = fr;
	out.fg = fg;
	out.fb = fb;
	out.br = br;
	out.bg = bg;
	out.bb = bb;
	return out;
}

// Generates a printable (non-color) string of scr
// To be fed into printf or similar
char* term_get_printable(termScreen* scr)
{
	int x = 0;
	int y = 0;
	int i = 0;

	int endLineCount = scr->height;

	char* out = (char*)calloc(scr->size + endLineCount + 1, sizeof(char));

	for (y = 0; y < scr->height; y++)
	{
		i = scr->width * y + x;
		for (x = 0; x < scr->width; x++)
		{
			i = scr->width * y + x;
			// Only output if printable, otherwise output space
			// May or may not correspond with graphic output
			if (scr->chars[i].character >= 32 && scr->chars[i].character < 255)
			{
				out[i] = scr->chars[i].character;
			}
			else
			{
				out[i] = ' ';
			}
		}
		out[i] = '\n';
	}


	out[scr->size + endLineCount + 1] = 0;
	return out;

}

termImage term_create_image(int w, int h)
{
	termImage out;
	out.data = (unsigned char*)calloc(w * h * 3, sizeof(char));
	out.data_size = w * h * 3;
	out.width = w;
	out.height = h;
	return out;
}

termFont term_load_font(char* data, int data_size, int width, int charW, int charH, int alphaChannel)
{
	termFont out;
	out.width = charW;
	out.height = charH;

	int height = 0;
	if (alphaChannel == 0)
	{
		height = data_size / 3 / width;
	}
	else
	{
		height = data_size / 4 / width;
	}

	out.totalCharacters = (width / charW) * (height / charH);

	out.characters = (termBitImage*)calloc(out.totalCharacters, sizeof(termBitImage));

	int charactersPerRow = width / charW;
	int charactersPerColumn = height / charH;

	int i = 0;


	for (int y = 0; y < charactersPerColumn; y++)
	{
		for (int x = 0; x < charactersPerRow; x++)
		{
			termBitImage tbi = term_create_bit_image(NULL, charW * charH, charW);

			char* slice = (char*)malloc(charW * charH * sizeof(char));
			int xOff = x * charW;
			int yOff = y * charH;

			tbi.data = slice;
			tbi.width = charW;
			tbi.height = charH;
			tbi.data_size = charW * charH * sizeof(char);

			for (int sX = 0; sX < charW; sX++)
			{
				for (int sY = 0; sY < charH; sY++)
				{
					if (alphaChannel)
					{
						int i = sY * charW + sX;
						int ii = ((yOff + sY) * width + (sX + xOff)) * 4;
						slice[i] = data[ii];
					}
					else
					{
						int i = sY * charW + sX;
						int ii = ((yOff + sY) * width + (sX + xOff)) * 3;
						slice[i] = data[ii];
					}
				}
			}

			out.characters[i] = tbi;
			i++;
		}
	}

	return out;
}

// Renders screen to image. If arguments are invalid no operation will be done
// Returns an error code
int term_render_image(termScreen* scr, termImage* outImage, termFont* font, int blending)
{
	if (outImage->width < scr->width * font->width || outImage->height < scr->height * font->height)
	{
		return TERM_INVALID_ARGUMENT;
	}

	for (int x = 0; x < scr->width; x++)
	{
		for (int y = 0; y < scr->height; y++)
		{
			int i = y * scr->width + x;
			termChar* c = &scr->chars[i];




			// Draw glyph
			for (int sX = 0; sX < font->width; sX++)
			{
				for (int sY = 0; sY < font->height; sY++)
				{
					int screenI = (y * font->height + sY) * (scr->width * font->width) + (x * font->width + sX);

					int sI = sY * font->width + sX;

					if (font->totalCharacters > c->character && c->character >= 0)
					{
						if (font->characters[c->character].data[sI] != 0)
						{
							if (blending)
							{
								outImage->data[screenI * 3] = c->fr * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								outImage->data[screenI * 3 + 1] = c->fg * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								outImage->data[screenI * 3 + 2] = c->fb * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
							}
							else
							{
								outImage->data[screenI * 3] = (unsigned char)c->fr;
								outImage->data[screenI * 3 + 1] = (unsigned char)c->fg;
								outImage->data[screenI * 3 + 2] = (unsigned char)c->fb;
							}
						}
						else
						{
							outImage->data[screenI * 3] = (unsigned char)c->br;
							outImage->data[screenI * 3 + 1] = (unsigned char)c->bg;
							outImage->data[screenI * 3 + 2] = (unsigned char)c->bb;
						}
					}
					else
					{
						outImage->data[screenI * 3] = (unsigned char)c->br;
						outImage->data[screenI * 3 + 1] = (unsigned char)c->bg;
						outImage->data[screenI * 3 + 2] = (unsigned char)c->bb;
					}

				}
			}
		}
	}

	return TERM_NO_ERROR;
}


void term_free_font(termFont* font)
{
	for (int i = 0; i < font->totalCharacters; i++)
	{
		term_free_bimage(&font->characters[i]);
		free(&font->characters[i]);
	}
}
void term_free_image(termImage* img)
{
	free(img->data);
}
void term_free_bimage(termBitImage* img)
{
	free(img->data);
}
void term_free_screen(termScreen* scr)
{
	free(scr->chars);
}


// Pretty much the same as term_render_image but outputs directly to a SDL surface (pixel array)
// Requires 3bpp surface
// Make sure, if the surface requires locking, to lock it!
// It can also work with any data structure similar to SDL surfaces
// ByteOrder defaults to big indean but make sure SDL defines it properly!
// Please make sure your surface is big enough so you dont get memory read errors!
int term_render_image_sdl(termScreen* scr, char* pixels, int pitch, termFont* font, int blending)
{

	// pitch / 3 is width in pixels
	// 3 is the bpp
	if (pitch / 3 < scr->width * font->width)
	{
		return TERM_INVALID_ARGUMENT;
	}

	for (int x = 0; x < scr->width; x++)
	{
		for (int y = 0; y < scr->height; y++)
		{
			int i = y * scr->width + x;
			termChar* c = &scr->chars[i];




			// Draw glyph
			for (int sX = 0; sX < font->width; sX++)
			{
				for (int sY = 0; sY < font->height; sY++)
				{

					int sI = sY * font->width + sX;
					unsigned char* dpt = (unsigned char*)pixels + (y * font->height + sY) *
						pitch +
						(x * font->width + sX) *
						3; //3 is the BPP

					unsigned char ccc = font->characters[c->character].data[sI];
					//printf("%u\n", ccc);
					if (font->totalCharacters > c->character && c->character >= 0)
					{
						if (font->characters[c->character].data[sI] != 0)
						{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
							if (blending == 1)
							{
								dpt[2] = c->fb * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								dpt[1] = c->fg * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								dpt[0] = c->fr * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
							}
							else
							{
								dpt[2] = c->fb;
								dpt[1] = c->fg;
								dpt[0] = c->fr;

							}
#else
							if (blending == 1)
							{
								dpt[0] = c->fb * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								dpt[1] = c->fg * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
								dpt[2] = c->fr * ((float)((unsigned char)font->characters[c->character].data[sI]) / 255.f);
							}
							else
							{
								dpt[0] = c->fb;
								dpt[1] = c->fg;
								dpt[2] = c->fr;

							}
#endif			
						}
						else
						{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
							dpt[0] = c->br;
							dpt[1] = c->bg;
							dpt[2] = c->bb;
#else
							dpt[0] = c->bb;
							dpt[1] = c->bg;
							dpt[2] = c->br;
#endif
						}
					}
					else
					{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
						dpt[0] = c->br;
						dpt[1] = c->bg;
						dpt[2] = c->bb;
#else
						dpt[0] = c->bb;
						dpt[1] = c->bg;
						dpt[2] = c->br;
#endif
					}

				}
			}
		}
	}

	return TERM_NO_ERROR;
}
#endif
