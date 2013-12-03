
#pragma once
#include "ControllableCharacter.h"
using namespace DXCore;
using namespace DXCore::BasicSprites;

enum WeaponsLevel
{
	single =1,
	single_medium = 2,
	triple = 3,
	triple_medium = 4

};
class Runner :public ControllableCharacter
{
public:
	ID3D11Texture2D* m_texturePtr;
	float z;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> spot_texture;
	 
	void Update(float timeDelta);
	void KeepInBounds();

	void Shoot();

	void Jump();
	void loadTexture(BasicSprites::SpriteBatch^ spriteBatch, BasicLoader^ loader);
	void Update(float timeDelta, Windows::Foundation::Rect windowBounds, float timeTotal);
	void Draw(BasicSprites::SpriteBatch^ m_spriteBatch);

	void UpgradeWeapons();
	void Reset();
	void SetTarget(float2 newTarget);
	Runner();
	void ProcessKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);
	void ProcessKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);
	void ProcessPointerPressed(Windows::UI::Input::PointerPoint^ pt);
	void ProcessPointerReleased(Windows::UI::Input::PointerPoint^ pt);
	void ProcessPointerMoved(Windows::UI::Input::PointerPoint^ pt); 

private:
	int keys_down;
	float2 TargetPos;
	WeaponsLevel weapons_level;

	bool m_boosting;
	float m_boosting_count;

	bool m_jumping;
	int m_animTimer;
	float2 m_lastPointerPos;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture1;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture2;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture3;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture4;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture5;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture6;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture7;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture8;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture9;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture10;
};