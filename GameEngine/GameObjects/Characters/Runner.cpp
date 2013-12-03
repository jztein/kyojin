

#include "pch.h"
#include "Runner.h"

using namespace Windows::System;
using namespace Windows::UI::Core;

const float RUNNER_GRAVITY = 1000.0f;
const float RUNNER_JUMP_VEL = -2000.0f;
const float RUNNER_GROUND_HEIGHT = 100.0f;
const float RUNNER_TMP_WIN_BOUNDS = 768.f;
int RUNNER_ANIM_TIMER[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 11 };

Runner::Runner()
{
	bool m_boosting = false;
	TargetPos = float2(-1, -1);
	vel = float2(0, 0);
	accel.y = 1000;
	weapons_level = WeaponsLevel::single;

	pos = float2(150.f, 500.f);

	m_boosting_count = 0;

	m_lastPointerPos = float2(0.f, 0.f);
	m_jumping = true;
	m_animTimer = 0;
	m_texturePtr = NULL;
}
void Runner::Reset(){
	health = 100;
	TargetPos = (-1.f, -1.f);
	_dead = false;
	weapons_level = WeaponsLevel::single;
	keys_down = 0;

	pos = float2(150.f, 500.f);
	m_lastPointerPos = float2(0.f, 0.f);
	m_jumping = true;
	m_animTimer = 0;
}
void Runner::Shoot()
{


	AudioManager::AudioEngineInstance.StopSoundEffect(AudioEngine::SoundEvent::Shoot);
	AudioManager::AudioEngineInstance.PlaySoundEffect(AudioEngine::SoundEvent::Shoot);

	FireBall data(false);
	FireBall data2(false);
	FireBall data3(false);

	switch (weapons_level)
	{
	case WeaponsLevel::single:
		 
		data.SetPos(pos);
		data.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data.SetScale(float2(1.0f, 1.0f));
		data.SetTexture(_projectile);
		data.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(), data.GetPos());
		data.SetWindowSize(_windowRect);
		bullets.push_back(data);
		break;
	case WeaponsLevel::single_medium: 
		data.SetPos(pos);
		data.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data.SetScale(float2(2.0f, 2.0f));
		data.SetTexture(_projectile);
		data.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(), data.GetPos());
		data.SetWindowSize(_windowRect);
		bullets.push_back(data);
		break;
	case WeaponsLevel::triple: 

		data.SetPos(pos);
		data.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data.SetScale(float2(1.0f, 1.0f));
		data.SetTexture(_projectile);
		data.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(), data.GetPos());
		data.SetWindowSize(_windowRect);
		bullets.push_back(data);
		 
		data2.SetPos(float2(pos.x - textureSize.Width / 2.f*scale.x, pos.y - textureSize.Height / 2.f*scale.y + 20));
		data2.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data2.SetScale(float2(1.0f, 1.0f));
		data2.SetTexture(_projectile);
		data2.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(),  data2.GetPos());
		data2.SetWindowSize(_windowRect);
		bullets.push_back(data2);


		data3.SetPos(float2(pos.x - textureSize.Width / 2.f*scale.x, pos.y + textureSize.Height / 2.f*scale.y - 20));
		data3.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data3.SetScale(float2(1.0f, 1.0f));
		data3.SetTexture(_projectile);
		data3.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(),  data3.GetPos());
		data3.SetWindowSize(_windowRect);
		bullets.push_back(data3);
		break;
	case WeaponsLevel::triple_medium:

		data.SetPos(pos);
		data.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data.SetScale(float2(2.0f, 2.0f));
		data.SetTexture(_projectile);
		data.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(),  data.GetPos());
		data.SetWindowSize(_windowRect);
		bullets.push_back(data);

		data2.SetPos(float2(pos.x - textureSize.Width / 2.f*scale.x, pos.y - textureSize.Height / 2.f*scale.y + 20));
		data2.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data2.SetScale(float2(2.0f, 2.0f));
		data2.SetTexture(_projectile);
		data2.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(), data2.GetPos());
		data2.SetWindowSize(_windowRect);
		bullets.push_back(data2);

		 
		data3.SetPos(float2(pos.x - textureSize.Width / 2.f*scale.x, pos.y + textureSize.Height / 2.f*scale.y - 20));
		data3.vel = float2(1000.0f* cos(rot), -1000.0f*sin(rot));
		data3.SetScale(float2(2.0f, 2.0f));
		data3.SetTexture(_projectile);
		data3.setCollisionGeometryForParticle(float2(20, 20)*data.GetScale(),  data3.GetPos());
		data3.SetWindowSize(_windowRect);
		bullets.push_back(data3);

	}
}

void Runner::SetTarget(float2 newTarget){
	TargetPos = newTarget;
}
void Runner::UpgradeWeapons()
{
	if (weapons_level < 4)
		weapons_level = WeaponsLevel((int) weapons_level + 1);
}
void Runner::Update(float timeDelta)
{
	++m_animTimer;

	// don't go past bottom
	if (pos.y > RUNNER_TMP_WIN_BOUNDS - RUNNER_GROUND_HEIGHT)
	{
		m_jumping = false;
		vel.y = 0.0f;
		pos.y = RUNNER_TMP_WIN_BOUNDS - RUNNER_GROUND_HEIGHT;
	}

	if (m_jumping)
	{
		vel.y = vel.y + RUNNER_GRAVITY * timeDelta;
	}
	else // running
	{
		// get texture of current animation frame
		if (m_animTimer < RUNNER_ANIM_TIMER[0])
			m_texturePtr = m_texture2.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[1])
			m_texturePtr = m_texture3.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[2])
			m_texturePtr = m_texture4.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[3])
			m_texturePtr = m_texture5.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[4])
			m_texturePtr = m_texture6.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[5])
			m_texturePtr = m_texture7.Get();//*/
		else if (m_animTimer < RUNNER_ANIM_TIMER[6])
			m_texturePtr = m_texture8.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[7])
			m_texturePtr = m_texture9.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[8])
			m_texturePtr = m_texture10.Get();
		else
			m_texturePtr = m_texture1.Get();

		if (m_animTimer > RUNNER_ANIM_TIMER[9])
			m_animTimer = 0;
	}

	float2 prevPos = pos;
	float  prevRot = rot;
	
	pos = pos + vel * timeDelta;

	KeepInBounds();
	UpdateChildren(timeDelta);
	UpdateCollisionGeometry(prevPos, pos, prevRot - rot);

}

void Runner::Update(float timeDelta, Windows::Foundation::Rect windowBounds, float timeTotal)
{
	m_animTimer++;

	_windowRect = windowBounds;

	// don't go past bottom
	if (pos.y > windowBounds.Height - RUNNER_GROUND_HEIGHT)
	{
		m_jumping = false;
		vel.y = 0.0f;
		pos.y = windowBounds.Height - RUNNER_GROUND_HEIGHT;
	}

	if (m_jumping)
	{
		// v = u + at
		vel.y = vel.y + RUNNER_GRAVITY * timeDelta;
	}
	else // running
	{
		// get texture of current animation frame
		if (m_animTimer < RUNNER_ANIM_TIMER[0])
			m_texturePtr = m_texture2.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[1])
			m_texturePtr = m_texture3.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[2])
			m_texturePtr = m_texture4.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[3])
			m_texturePtr = m_texture5.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[4])
			m_texturePtr = m_texture6.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[5])
			m_texturePtr = m_texture7.Get();//*/
		else if (m_animTimer < RUNNER_ANIM_TIMER[6])
			m_texturePtr = m_texture8.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[7])
			m_texturePtr = m_texture9.Get();
		else if (m_animTimer < RUNNER_ANIM_TIMER[8])
			m_texturePtr = m_texture10.Get();
		else
			m_texturePtr = m_texture1.Get();

		if (m_animTimer > RUNNER_ANIM_TIMER[9])
			m_animTimer = 0;
	}

	float2 prevPos = pos;
	float  prevRot = rot;

	pos = pos + vel * timeDelta;

	KeepInBounds();
	UpdateChildren(timeDelta);
	UpdateCollisionGeometry(prevPos, pos, prevRot - rot);
}


void Runner::KeepInBounds()
{

	if (pos.x > _windowRect.Width - textureSize.Width / 2.0f * scale.x)
	{
		pos.x = _windowRect.Width - textureSize.Width / 2.0f * scale.x;
		accel.x = 0;
		vel.x = 0;
	}
	if (pos.y > _windowRect.Height - textureSize.Height / 2.0f * scale.y)
	{
		pos.y = _windowRect.Height - textureSize.Height / 2.0f * scale.y;

		vel.y = 0;
	}
	if (pos.x < textureSize.Width / 2.0f * scale.x)
	{
		pos.x = textureSize.Width / 2.0f * scale.x;
		accel.x = 0;
		vel.x = 0;
	}
	if (pos.y < textureSize.Height / 2.0f * scale.y)
	{
		pos.y = textureSize.Height / 2.0f * scale.y;

		vel.y = 0;
	}
}


void Runner::ProcessKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args){

	if (args->Key == VirtualKey::Right)
	{
		vel.x = 1000;
		m_boosting = true;
	}

	if (args->Key == VirtualKey::Left)
	{
		vel.x = -1100;
		m_boosting = true;
	}

	if (args->Key == VirtualKey::Up)
	{
		Jump();
		//vel.y = -500;
		m_boosting = true;
	}
	if (args->Key == VirtualKey::Down)
	{
		vel.y = 500;
		m_boosting = true;
	}

	if (!args->KeyStatus.WasKeyDown)
		keys_down += 1;

	TargetPos = float2(-1, -1);
}
void Runner::ProcessKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args)
{
	m_boosting = false;

	keys_down -= 1;
	if (keys_down == 0)
		vel = (0.f, 0.f);

	if (args->Key == VirtualKey::Space)
	{
		Shoot();
	}
}

void Runner::ProcessPointerPressed(Windows::UI::Input::PointerPoint^ pt)
{
}

void Runner::ProcessPointerReleased(Windows::UI::Input::PointerPoint^ pt)
{
	m_boosting = false;
	Shoot();
}

void Runner::ProcessPointerMoved(Windows::UI::Input::PointerPoint^ pt)
{
	TargetPos = float2(pt->Position.X, pt->Position.Y);
	m_boosting = true;
	// UP
	if (TargetPos.y < m_lastPointerPos.y)
	{
		Jump();
	}

	m_lastPointerPos = float2(pt->Position.X, pt->Position.Y);
}

void Runner::Jump()
{
	m_jumping = true;
	vel.y = RUNNER_JUMP_VEL;
}

void Runner::loadTexture(BasicSprites::SpriteBatch^ spriteBatch, BasicLoader^ loader)
{
	loader->LoadTexture("Assets\\MyAssets\\volt1.png", &m_texture1, nullptr);
	m_texturePtr = m_texture1.Get();
	spriteBatch->AddTexture(m_texture1.Get());

	loader->LoadTexture("Assets\\MyAssets\\volt2.png", &m_texture2, nullptr);
	spriteBatch->AddTexture(m_texture2.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt3.png", &m_texture3, nullptr);
	spriteBatch->AddTexture(m_texture3.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt4.png", &m_texture4, nullptr);
	spriteBatch->AddTexture(m_texture4.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt5.png", &m_texture5, nullptr);
	spriteBatch->AddTexture(m_texture5.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt6.png", &m_texture6, nullptr);
	spriteBatch->AddTexture(m_texture6.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt7.png", &m_texture7, nullptr);
	spriteBatch->AddTexture(m_texture7.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt8.png", &m_texture8, nullptr);
	spriteBatch->AddTexture(m_texture8.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt9.png", &m_texture9, nullptr);
	spriteBatch->AddTexture(m_texture9.Get());
	loader->LoadTexture("Assets\\MyAssets\\volt10.png", &m_texture10, nullptr);
	spriteBatch->AddTexture(m_texture10.Get());

	SetTexture(m_texture1);


	return;
}

void Runner::Draw(BasicSprites::SpriteBatch^ m_spriteBatch)
{
	float4 color = float4(1.f, 1.f, 1.f, 1.0f);

	for (auto particle = bullets.begin(); particle != bullets.end(); particle++)
	{
		particle->Draw(m_spriteBatch);
	}

	if (Character::hit > 0)
	{
		color = float4(1.f, 0.f, 0.f, 1.f);
		Character::hit--;
	}
	for (auto child = children.begin(); child != children.end(); child++)
	{
		// if rocket fire, only want to draw when boosting
		// i.e. when jumping up or moving left/right
		if (dynamic_cast<RocketFire*>(child->first) != NULL)
		{
			RocketFire* rocket_fire = dynamic_cast<RocketFire*>(child->first);
			if (!m_boosting)
			{
				// this makes rocket fire collapsing less abrupt
				m_boosting_count += 0.25;
				if (m_boosting_count > 1.0f)
				{
					continue;
				}

			}
			else
			{
				m_boosting_count = 0.0f;
			}

			rocket_fire->Draw(m_spriteBatch, 1.0f - m_boosting_count);
			continue;
				
		}
		child->first->Draw(m_spriteBatch);
	}

	m_spriteBatch->Draw(
		m_texturePtr,
		pos,
		PositionUnits::DIPs,
		float2(0.75f, 0.75f) * scale,
		SizeUnits::Normalized,
		color,
		rot
		);
}