
#pragma once
#include "ControllableCharacter.h"
using namespace DXCore;
using namespace DXCore::BasicSprites;

enum HoriWeaponsLevel
{
	horisingle =1,
	horisingle_medium = 2,
	horitriple = 3,
	horitriple_medium = 4

};
class HorizontalSliderPlayer :public ControllableCharacter
{
	int keys_down; 
	float2 TargetPos;
	HoriWeaponsLevel weapons_level;
public:
	float z;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> spot_texture;
	 
	void Update(float timeDelta);
	void KeepInBounds();

	void Shoot();
	void UpgradeWeapons();
	void Reset();
	void SetTarget(float2 newTarget);
	HorizontalSliderPlayer();
	void ProcessKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);
	void ProcessKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);
	void ProcessPointerPressed(Windows::UI::Input::PointerPoint^ pt);
	void ProcessPointerReleased(Windows::UI::Input::PointerPoint^ pt);
	void ProcessPointerMoved(Windows::UI::Input::PointerPoint^ pt); 

};