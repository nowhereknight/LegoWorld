#pragma once
#include<irrKlang/irrKlang.h>
using namespace irrklang;

class Sonido
{
public:
	Sonido();
	void Reproduce();
	void Stop();
	~Sonido();

private:
	ISoundEngine* engine;
	bool estado;
};