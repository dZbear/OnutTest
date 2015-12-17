#pragma once
#include <vector>
using namespace std;

class SeedView;
class SeedApp
{
public:

	SeedApp();
	virtual ~SeedApp();

	// to be overriden by your "Game Specific" App
	virtual void OnStart() {};
	virtual void OnUpdate() {};
	virtual void OnRender() {};

	void Start();
	void Update();
	void Render();

protected:

	void CreateView(SeedView* in_newView);
	
private:
	
	vector<SeedView*> m_views;

};