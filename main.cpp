#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <thread>
#include <png++/png.hpp>

unsigned imageWidth = 1920;
unsigned imageHeight = 1080;
unsigned threadCount = 4;

struct Bubble {
	unsigned x, y;
	unsigned radius, color;
};

std::random_device randomDevice;
std::default_random_engine rEngine(randomDevice());

std::vector<Bubble> bubbles;

float distance(unsigned ax, unsigned ay, unsigned bx, unsigned by)
{
	return sqrt((ax-bx)*(ax-bx) + (ay-by)*(ay-by));
}

int collision(unsigned x, unsigned y, unsigned radius)
{
	for(int i = 0;i < bubbles.size();++ i)
		if(distance(x, y, bubbles[i].x, bubbles[i].y) <= radius + bubbles[i].radius)
			return i;

	return -1;
}

void fillBubbles(unsigned tries, unsigned size, unsigned color)
{
	std::uniform_int_distribution<unsigned> distWidth(0, imageWidth);
	std::uniform_int_distribution<unsigned> distHeight(0, imageHeight);

	for(unsigned i = 0;i < tries;++ i)
	{
		unsigned x = distWidth(rEngine), y = distHeight(rEngine);

		if(collision(x, y, size) == -1)
			bubbles.push_back({x, y, size, color});
	}
}

void renderRow(png::gray_pixel* data, unsigned row)
{
	for(unsigned x = 0;x < imageWidth;++ x)
	{
		int c = collision(x, row, 0);
		if(c == -1)
			data[x] = png::gray_pixel(0);
		else
			data[x] = png::gray_pixel(bubbles[c].color);
	}
}

int main()
{
	unsigned size = 130, color = 0;
	const unsigned totalPixels = imageHeight * imageWidth;

	while(size >= 3)
	{
		unsigned avrCirVolume = 3.14 * size * size;

		unsigned tries = (totalPixels/avrCirVolume)*20;
		fillBubbles(tries, size, color);

		if(size > 20)
			color += 5;
		else
			color += 50;

		size /= 2;
	}

	fillBubbles(totalPixels / 2, 1, 250);

	std::cout << "Done filling, rendering the image..." << std::endl;
	png::image<png::gray_pixel> image(imageWidth, imageHeight);

	png::gray_pixel** rows = new png::gray_pixel*[threadCount];
	for(unsigned i = 0;i < threadCount;++ i)
		rows[i] = new png::gray_pixel[imageWidth];

	std::thread* threads = new std::thread[threadCount];
	for(png::uint_32 y = 0;y < imageHeight/threadCount;++ y)
	{
		for(unsigned i = 0;i < threadCount;++ i)
			threads[i] = std::thread(renderRow, rows[i], y*threadCount+i);

		for(unsigned i = 0;i < threadCount;++ i)
		{
			threads[i].join();
			for(unsigned x = 0;x < imageWidth;++ x)
				image[y*threadCount+i][x] = rows[i][x];
		}
	}

	image.write("output.png");
	return 0;
}
