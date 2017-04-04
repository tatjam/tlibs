# Tinyterm.h  (NOT FINISHED YET, LACKING SFML OUTPUT AND SOME TESTING)

>Tinyterm is a quick and simple multi-platform, no dependency, library independent RGB terminal rendering engine.

It is as lightweight as it can be only doing basic text rendering, all the rest is for you to manage. (There are no ``println`` functions, you are only given direct access to a character array).

(It's planned to have a termBuffer which will do exactly that, allow printf() into it and similar)

#### Warning!

Rendering functions are not memory safe! If you give a buffer too small to fit the render functions will just silently fail, probably crashing your program. Always make sure your buffers are big enough before feeding into ``term_render_[wathever]``!!

### Features
-----

##### Fast Software Renderer
The renderer is fast and as easy to use as calling a single function.
It will output to a pixel array (either a ``termImage`` or any of the supported formats, read below) that you must create beforehand.
The cost of the rendering is proportional to the size of your ``termScreen``, so keep in mind using huge terminals might cause perfomance issues.

##### Simple to use
The API is designed to be lightweight and not-intrusive, you can use it with pretty much any output library by just writing a few lines of code.

Code Example: ( C with SDL to output )
Note that we are directly writing to the video buffer! 

```C
#include "SDL.h"

#define TERM_INCLUDE_DEFINITION
#include  "tinyterm.h"

int main(int argc, char* args[])
{
	SDL_Surface* screen;
	SDL_Surface* fontSrf;

	// Bit masks for using in the font loading
	Uint32 rmask, gmask, bmask, amask;
	rmask = 0x000000ff;	gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;

	SDL_Init(SDL_INIT_EVERYTHING);

	// We need to set bpp to 24 as we will be directly writing to this from
	// the terminal renderer. If we used another surface for rendering the
	// terminal we could perfectly use a 32 bit video buffer
	screen = SDL_SetVideoMode(256, 256, 24, SDL_SWSURFACE);

	fontSrf = SDL_LoadBMP("test_font.bmp");
	int charWidth = 8; int charHeight = 8;
	// We load our font, read from fontSurface->pixels,
	// the last argument tells the loader that there is no alpha channel
	termFont font = term_load_font(fontSrf->pixels,
		(fontSrf->w * fontSrf->h * fontSrf->format->BytesPerPixel),
		fontSrf->w, charWidth, charHeight, 0);

	termScreen term = term_create_screen(32, 32);

	// We won't need a termImage as we render directly to the SDL_Surface

	// Let's give data to our terminal!
	for (int i = 0; i < 32 * 32; i++)
	{
		// We won't use the term_set_char() functon as we are not indexing
		// using x and y, but we are using i
		termChar* c = &term.chars[i];
		c->character = i % 255;
		// Also set some nice colors
		c->fr = 255 - i; c->fg = i * 2; c->fb = i;
		c->br = i / 3; c->bg = i / 5; c->bb = i;
	}

	// Now lets just render to our screen
	SDL_LockSurface(screen);
	// The 1 at the end means we want to use blending
	term_render_image_sdl(&term, screen->pixels, screen->pitch, &font, 1);
	SDL_UnlockSurface(screen);

	SDL_Flip(screen);

	// Wait a bit before closing
	SDL_Delay(5000);

	return 0;

}
```

Result: 

![Result of the code](http://imgur.com/ikdp08k.png)

### Plans:

- Add a hardware renderer (Either using openGL or making you give the renderer the functions to draw primitives)
- Add termBuffer for easy terminal output with a function similar to printf

## Documentation
------

#### Basic Usage Guide

Every program that uses tinyterm.h will have to create a ``termScreen`` and a ``termFont``, and must have a way to draw a pixel array (SDL, OpenGL, SFML, .png writer, etc...).

In order to create the screen you will have to use the ``termScreen term_create_screen(int w, int h)`` function. It takes two parameters, ``w`` and ``h``, they are the width and height of the terminal in characters.

You must also create a font, you are recommended to use the 
``termFont term_load_font(char* data, int size, int width, int charW, int charH, int alpha)`` function but you can manually create a font by creating yourself the glyphs.

``char* data`` is a pointer to your data in 24 bit color format (RGB) or 32 bit (RGBA) (Data endianness does not really matter as fonts should be black and white).
``int size`` is the size of your char* block. Usually ``width * height * bpp``, bpp being the bytes per pixel of your image (24 or 32).
``int width`` is the width of your font image.
``int charW`` is the width of each character.
``int charH`` is the height of each character.
``int alpha`` should be 1 if you are using an image with alpha channel (RGBA) and 0 if it's RGB

Now everything is ready to do, to set chars you can either use ``int term_set_char(termScreen* sc, int x, int y, termChar nChar)`` if you are using (x, y) coodinates or you can also directly access the termScreen, to do this use ``termChar* scrChar = &term.chars[index]``. ScrChar will point to the character at index and modifying it will change the data on the terminal.


You have multiple ways to render your image. 

##### Using termImage
-----
If you are willing to use OpenGL or the output system you are using does not allow writing to its textures you should use ``int term_render_image(termScreen* scr, termImage* outImage, termFont* font, int blending)``, it will render your screen to a given image (which you must create properly using the ``termImage term_create_image(int w, int h)`` function, make sure your image is big enough!) using the given font.

The data contained by ``termImage`` is simply a char array in R-G-B order.

You can load a ``termImage`` into OpenGL with the macro ``term_upload_gl_image(img, glId)``.
``img`` is a termImage and ``glId`` is the GLuint that you want the texture to use (You must use ``glGenTextures`` to get it!)

##### Direct output to your texture (sdl, sfml, etc...)
------
``void term_render_image_sdl(termScreen* scr, char* pixels, int pitch, termFont* font, int blending)`` will render your image and output to the given char array directly. 

``pitch`` is the length of a row of pixels in bytes. In SDL it's as simple to get as ``surface->pitch``, for SFML or your own texture type you should use ``width * bpp``.

##### Warning!
Keep in mind that if your output buffer is too small the function will probably crash your program!
``term_render_image_sdl`` can only check that your image is wide enough, but can't do anything about height, so please make sure your buffers are big enough!


### Function documentation

###### ``int term_set_char(termScreen* sc, int x, int y, termChar nChar)``
Will set character at (x, y) to char nChar. Returns an error code

###### ``termChar* term_get_char(termScreen* sc, int x, int y)``
Will return the character at (x, y) or null.

###### ``int term_render_image(termScreen* scr, termImage* outImage, termFont* font, int blending)``
Will render a termScreen to a termImage using the given font. Blending will use blending if set to 1 and disable it if set to 0

###### ``int term_render_image_sdl(termScreen* scr, char* pixels, int pitch, termFont* font, int blending)``
Will render a termScreen to a pixel array using the given font. Blending will use blending if set to 1 and disable it if set to 0
``pitch`` is the lenght of a row of pixels in bytes. ``width*bpp``


###### ``termFont term_load_font(char* data, int data_size, int width, int charW, int charH, int alphaChannel)``
Will load a font from a pixel array of either 24/32 bpp data. 
``data_size`` is ``width*height*bpp``. 
``charW`` and ``charH`` are the width and height of each character
``alphaChannel`` should be 1 if your image is 32 bpp, otherwise it should be 0

###### ``termImage term_create_image(int w, int h)``
Creates empty (black) image of given width and height

###### ``char* term_get_printable(termScreen* scr)``
Gets a printable string (for use with printf or similar) of your screen.
May not be accurate to the graphic output as your font can use any glyph order that is not ASCII

###### ``termChar term_create_char(TERM_CHAR_TYPE character, char fr, char fg, char fb, char br, char bg, char bb)``
Creates a character (``TERM_CHAR_TYPE`` is usually char) from given data. f/b[r,g,b] are the foreground and background colors

###### ``termBitImage term_create_bit_image(char* data, int data_size, int width)''
Creates 8bpp image from given data


###### ``int term_define_font_character(int characterN, termFont* f, termBitImage img)``
Manually defines a glyph in a font from bitImage

###### ``termFont term_create_font(int charW, int charH, int characterCount)``
Creates empty font (all characters will be black)

###### ``termScreen term_create_screen(int w, int h)``
Creates empty screen (all characters are 0)

#### Freeing functions:
###### ``void term_free_font(termFont* font)``
###### ``void term_free_image(termImage* img)``
###### ``void term_free_bimage(termBitImage* img)``
###### ``void term_free_screen(termScreen* scr)``

## Changelog:

[RIGHT NOW] V0.9 -> Testing before first release

## Blending Example (Using CowThing's texture pack for Dwarf Fortress: [Tergel](http://www.bay12forums.com/smf/index.php?PHPSESSID=f9f82e2fad9ca6c97947053892c3f5a7&topic=145802.0))

![IMG](http://imgur.com/AFKhomo.png)
