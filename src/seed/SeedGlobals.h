#pragma once

#include <vector>
#include <unordered_map>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
using namespace std;

class SeedSprite;

typedef unordered_map<string, SeedSprite*>	SpriteMap;
typedef vector<SeedSprite*>					SpriteVect;
