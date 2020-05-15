#include "Sonido.h"
#include <stdio.h>
#include<irrKlang/irrKlang.h>
using namespace irrklang;

Sonido::Sonido()
{	
	// start the sound engine with default parameters
	estado = false;
}

void Sonido::Reproduce()
{
	if (!estado) {
		engine = createIrrKlangDevice();
		if (!engine)
			printf("\nError al cargar motor de audio.\n");

		// play some sound stream, looped
		engine->play2D("over-and-over.mp3", true);
		estado = true;
	}
}

void Sonido::Deten()
{
	if (estado)
	{
		engine->drop();
		estado = false;
	}
}

Sonido::~Sonido()
{
}