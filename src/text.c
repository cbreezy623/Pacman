#include "text.h"

#include <SDL/SDL_gfxPrimitives.h>

#include "imageloader.h"

static SDL_Surface *get_surface(char c);
static SDL_Color get_color(TextColor color);
static Uint32 get_pixel(SDL_Surface *surface, int x, int y);
static void set_fontimage_color(SDL_Surface *image, TextColor color);
static void replace_pixel(SDL_Surface *image, SDL_Color newColor);

//TODO: read readme below for details.
int to_argb(int a, int r, int g, int b);

static TextColor textColor;

static SDL_Surface *char_fonts[NUM_FONTS][NUM_CHAR_IMAGES];
static SDL_Surface *num_fonts [NUM_FONTS][NUM_NUM_IMAGES];
static SDL_Surface *spec_fonts[NUM_FONTS][NUM_SPEC_IMAGES];

void load_text(void)
{
	//imageloader.c has already loaded 1 copy of a white coloured font
	//clone the image for each of our font colours, then set the white to that font
	//let imageloader dispose of the original images, and make sure we dipose of ours in the dispose function

	textColor = WhiteText;

	for (int i = 0; i < NUM_CHAR_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			char_fonts[c][i] = clone_image(get_char_images()[i]);
			set_fontimage_color(char_fonts[c][i], c);
		}
	}

	for (int i = 0; i < NUM_NUM_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			num_fonts[c][i] = clone_image(get_num_images()[i]);
			set_fontimage_color(num_fonts[c][i], c);
		}
	}

	for (int i = 0; i < NUM_SPEC_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			spec_fonts[c][i] = clone_image(get_spec_images()[i]);
			set_fontimage_color(spec_fonts[c][i], c);
		}
	}
}

void set_text_color(TextColor col)
{
	textColor = col;
}

bool is_valid(char c)
{
	bool isChar = c >= 'A' && c <= 'Z';
	bool isNum  = c >= '0' && c <= '9';
	bool isSpec = c == ' ' || c == '!' || c == '.' || c == '-' || c == '/' || c == '@' || c == '"';

	return isChar || isNum || isSpec;
}

bool is_valid_string(const char *text)
{
	const char *str = text;
	char c;

	while ((c = *text++))
	{
		if (!is_valid(c)) 
		{
			printf("invalid char: '%c' in string \"%s\"\n", c, str);
			return false;
		}
	}

	return true;
}


void dispose_text(void)
{
	for (int i = 0; i < NUM_CHAR_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			SDL_FreeSurface(char_fonts[c][i]);
		}
	}

	for (int i = 0; i < NUM_NUM_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			SDL_FreeSurface(num_fonts[c][i]);
		}
	}

	for (int i = 0; i < NUM_SPEC_IMAGES; i++)
	{
		for (int c = 0; c < NUM_FONTS; c++)
		{
			SDL_FreeSurface(spec_fonts[c][i]);
		}
	}
}

void draw_text(SDL_Surface *surface, const char *text, int x, int y)
{
	if (!is_valid_string(text))
	{
		printf("The string contains invalid characters: %s\n", text);
		printf("aborting\n");
		exit(1);
	}
	
	SDL_Rect location;
	location.x = x;
	location.y = y;

	char c;

	while ((c = *text++))
	{
		SDL_Surface *charSurface = get_surface(c);

		SDL_BlitSurface(charSurface, NULL, surface, &location);
		location.x = location.x + charSurface->w;
	}
}

void draw_numtext_coord(SDL_Surface *surface, const char *text, int x, int y)
{
	//a text square is 2 actual pixels
	draw_text(surface, text, x * 16, y * 16 - 2);
}

void draw_text_coord(SDL_Surface *surface, const char *text, int x, int y)
{
	draw_text(surface, text, x * 16, y * 16);
}

//TODO: either use this or remove it. Made it non-static so there wouldn't be a warning.
int to_argb(int a, int r, int g, int b)
{
	int argb = (a << 24);
	argb = argb | (r << 16);
	argb = argb | (g << 8);
	argb = argb | (b);

	return argb;
}

static void replace_pixel(SDL_Surface *image, SDL_Color newColor)
{
	for (int y = 0; y < image->h; y++)
	{
		for (int x = 0; x < image->w; x++)
		{
			Uint32 pixel = get_pixel(image, x, y);
			SDL_Color c;
			SDL_GetRGBA(pixel, image->format, &c.r, &c.g, &c.b, &c.unused);

			if (c.unused == 255) pixelRGBA(image, x, y, newColor.r, newColor.g, newColor.b, newColor.unused);
		}
	}
}

static Uint32 get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

static void set_fontimage_color(SDL_Surface *image, TextColor color)
{
	replace_pixel(image, get_color(color));
}

static SDL_Surface *get_surface(char c)
{
	//if char, get the chara image
	if ('A' <= c && c <= 'Z') return char_fonts[textColor][c - 'A'];

	//if number, get the number image
	if ('0' <= c && c <= '9') return num_fonts[textColor][c - '0'];

	//if special, get the special image
	switch (c)
	{
        case ' ': return spec_fonts[textColor][0];
        case '.': return spec_fonts[textColor][1];
        case '-': return spec_fonts[textColor][2];
        case '@': return spec_fonts[textColor][3];
        case '!': return spec_fonts[textColor][4];
        case '/': return spec_fonts[textColor][5];
        case '"': return spec_fonts[textColor][6];
	}

	printf("error getting text surface with char %c\n", c);
	printf("aborting\n");
	exit(1);
}

static SDL_Color get_color(TextColor color)
{
	switch (color)
	{
		case WhiteText: 	return (SDL_Color) { 223, 223, 255, 255 };
		case YellowText:	return (SDL_Color) { 255, 255,   0, 255 };
		case PeachText:		return (SDL_Color) { 255, 185, 175, 255 };
		case RedText: 		return (SDL_Color) { 255,   0,   0, 255 };
		case PinkText: 		return (SDL_Color) { 255, 185, 255, 255 };
		case CyanText: 		return (SDL_Color) {   0, 255, 255, 255 };
		case OrangeText: 	return (SDL_Color) { 255, 185,  81, 255 };
	}

	printf("error color: %d\naborting\n", color);
	exit(1);
}
