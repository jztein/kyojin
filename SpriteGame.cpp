#include "pch.h"
#include "SpriteGame.h" 


#include <DirectXMath.h>    

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace DirectX;
using namespace Windows::Graphics::Display;
using namespace DXCore;


SpriteGame::SpriteGame()
{
	gamestate = Menu;
	time_passed = 0;
}

void SpriteGame::CreateDeviceIndependentResources()
{
	DirectXBase::CreateDeviceIndependentResources();

}

void SpriteGame::LoadLevel(Platform::String^ level_xml)
{
	if (level != NULL)
	{
		delete level;
	}

	BasicLoader^ loader = ref new BasicLoader(m_d3dDevice.Get(), m_wicFactory.Get());
	level = new Level();
	std::wstring level_xmlW(level_xml->Begin());
	std::string level_xmlA(level_xmlW.begin(), level_xmlW.end());
	level->Load("Level Data\\" + level_xmlA, m_spriteBatch, loader);

	//spaceship->health = 100;
	//spaceship->Reset();

	runner->health = 100;
	runner->Reset();

	//spaceship->SetTarget(float2(0.f, m_windowBounds.Height / 2.f));
	runner->SetTarget(float2(0.f, m_windowBounds.Height / 2.f));
	level->SetWindowDependentProperties(m_windowBounds);
	 
	time_passed = 0;
	score = 0;
	last_explosion = 0;
}

void SpriteGame::CreateDeviceResources()
{
	DirectXBase::CreateDeviceResources();

	m_spriteBatch = ref new SpriteBatch();
	unsigned int capacity = 15000;

	m_spriteBatch->Initialize(
		m_d3dDevice.Get(),
		capacity
		);

	// Load the sprite textures.

	BasicLoader^ loader = ref new BasicLoader(m_d3dDevice.Get(), m_wicFactory.Get());


	loader->LoadTexture(
		"Assets\\GameObjects\\spotlight.png",
		&m_player_spot,
		nullptr
		);

	m_spriteBatch->AddTexture(m_player_spot.Get());

	loader->LoadTexture(
		"Assets\\GameObjects\\ida.dds",
		&m_asteroid,
		nullptr
		);
	m_spriteBatch->AddTexture(m_asteroid.Get());

	/*
	loader->LoadTexture(
		"Assets\\GameObjects\\SpaceShip.png",
		&m_player,
		nullptr
		);
	m_spriteBatch->AddTexture(m_player.Get());
	//*/

	loader->LoadTexture(
		"Assets\\GameObjects\\particle.png",
		&m_particle,
		nullptr
		);
	m_spriteBatch->AddTexture(m_particle.Get());

	loader->LoadTexture(
		"Assets\\GameObjects\\debug.png",
		&m_debug_point,
		nullptr
		);
	m_spriteBatch->AddTexture(m_debug_point.Get());
	loader->LoadTexture(
		"Assets\\GameObjects\\bad_health.png",
		&m_background,
		nullptr
		);
	m_spriteBatch->AddTexture(m_background.Get());
	 
	loader->LoadTexture(
		"Assets\\MyAssets\\ground.png",
		&m_ground,
		nullptr
		);
	m_spriteBatch->AddTexture(m_ground.Get());

	runner = new Runner();

	rocketFuel = new RocketFire();

	runner->SetVel(float2(0, 0));
	runner->SetRot(0);
	runner->SetRotVel(0);
	runner->SetScale(float2(1.0, 1.0));

	runner->_projectile = m_particle;

	runner->loadTexture(m_spriteBatch, loader);

	bad_health_background = new FlashingBackground();
	bad_health_background->SetTexture(m_background);



}

void SpriteGame::CreateWindowSizeDependentResources()
{
	DirectXBase::CreateWindowSizeDependentResources();

	runner->SetWindowSize(m_windowBounds);

	runner->SetPos(float2(0.f, m_windowBounds.Height / 2.0f));

	//runner->SetTexture(m_player);
	rocketFuel->SetTexture(m_particle);
	runner->setForwardTriangleCollisionGeometry(runner->GetTopLeft(), runner->GetBottomRight());
	runner->AddChild(float2(runner->textureSize.Width / 2.0f*runner->GetScale().x, runner->textureSize.Height / 2.0f*runner->GetScale().y), rocketFuel);

	rocketFuel->SetPos(runner->GetPos());
	 
	if (level != NULL)
		level->background->SetWindowSize(m_windowBounds);


 
	bad_health_background->SetWindowSize(m_windowBounds);

}

void SpriteGame::Update(float timeTotal, float timeDelta)
{
	time_passed += timeDelta;

	level->background->Update(timeDelta);
	level->foreground->Update(timeDelta);
	bad_health_background->Update(timeDelta);
	runner->Update(timeDelta, m_windowBounds, timeTotal);

	for (auto object = level->all_gameobjects.begin(); object != level->all_gameobjects.end(); object++) // update level objects
		(*object)->Update(timeDelta);

	for (auto object = m_explosionData.begin(); object != m_explosionData.end(); object++) // update explosions
		object->Update(timeDelta);

	if (gamestate == GameState::LevelWon &&   timeTotal - last_explosion > .1)
	{
		Explosion data;
		data.SetLifeTime(100);
		data.SetPos(float2(RandFloat(0, m_windowBounds.Width), RandFloat(0, m_windowBounds.Height)));
		data.color = float4(RandFloat(0, 1), RandFloat(0, 1), RandFloat(0, 1), 1.f);
		data.SetScale(float2(30.0f, 30.0f));
		data.SetTexture(m_particle);
		data.SetWindowSize(m_windowBounds);
		m_explosionData.push_back(data);
		last_explosion = timeTotal;
	}
	if (gamestate == GameState::LevelLost &&   timeTotal - last_explosion > .3)
	{
		Explosion data;
		data.SetLifeTime(100);
		data.SetPos(runner->GetPos());
		data.color = float4(1.f, 0.3f, .0f, 1.f);
		data.SetScale(float2(30.0f, 30.0f));
		data.SetTexture(m_particle);
		data.SetWindowSize(m_windowBounds);
		m_explosionData.push_back(data);
		last_explosion = timeTotal;
	}

	if (gamestate == GameState::Playing) //if playing , create new level objects
	{
		level->Update(timeTotal, timeDelta, m_windowBounds);

		DetectCollisions();
		HandleCollisions();

		if (!runner->IsAlive())
			gamestate = GameState::GameOver;
	}

	RemoveDead();

}
void SpriteGame::DetectCollisions(){

	colliding.clear();
	for (auto pobject = level->all_gameobjects.begin(); pobject != level->all_gameobjects.end(); pobject++)  // process all player collisions
	{
		if (dynamic_cast<Enemy*>((*pobject)) != NULL) // if it is an enemy we need to add the bullets 
		{
			Enemy* en = dynamic_cast<Enemy*>((*pobject));
			for (auto bullet = en->bullets.begin(); bullet != en->bullets.end(); bullet++) // add enemy bullets to the collision list
			{

				if (dist(bullet->GetPos(), runner->GetPos()) < runner->textureSize.Width*runner->GetScale().x*2.0) // prune collision list based on distance
				{
					if (PolygonCollision(bullet->getCollisionGeometry(), runner->getCollisionGeometry())) //checkCollision
						colliding.push_back(pair<GameObject*, GameObject*>(runner, &(*bullet)));
				}
			}
		}
		if (dist((*pobject)->GetPos(), runner->GetPos()) < runner->textureSize.Width*runner->GetScale().x*2.0) // prune collision list based on distance
		{
			if (PolygonCollision((*pobject)->getCollisionGeometry(), runner->getCollisionGeometry())) //checkCollision
				colliding.push_back(pair<GameObject*, GameObject*>(runner, *pobject));

		}
	}

	for (auto pobject = level->all_gameobjects.begin(); pobject != level->all_gameobjects.end(); pobject++) // process all enemy collisions
	{
		for (auto bullet = runner->bullets.begin(); bullet != runner->bullets.end(); bullet++) //against player bullets
		{
			if (dynamic_cast<GamePlayElement*>((*pobject)) == NULL)  // We only want to hit Enemies & asteroids, not gameplay stuff
			{
				if (dist((*pobject)->GetPos(), bullet->GetPos()) < (*pobject)->textureSize.Width*(*pobject)->GetScale().x*2.0) // prune collision list based on distance
				{
					if (PolygonCollision((*pobject)->getCollisionGeometry(), bullet->getCollisionGeometry())) //checkCollision
						colliding.push_back(pair<GameObject*, GameObject*>(&(*bullet),*pobject));

				}
			}
		}
	}

}
void SpriteGame::HandleCollisions()
{
	for (auto c = colliding.begin(); c != colliding.end(); c++)
	{

		ImpactResult res1 = c->first->ProcessHit(c->second->GetImpactSize());

		ImpactResult res2 = c->second->ProcessHit(c->first->GetImpactSize());

		if (res1 == ImpactResult::score || res2 == ImpactResult::score)
			score += 1;

		if (res1 == ImpactResult::explosion || res1 == ImpactResult::bigexplosion  )
		{
			Explosion data;
			data.SetPos(c->first->GetPos());
			

			if (res1 == ImpactResult::explosion  )
			{
				data.SetScale(float2(20.f, 20.f));
				data.SetLifeTime(10);
			}
			else
			{
				data.SetScale(float2(30.f, 30.f));
				data.SetLifeTime(100);
			}
			data.SetTexture(m_particle);
			data.SetWindowSize(m_windowBounds);
			m_explosionData.push_back(data);
		}

		if (res2 == ImpactResult::explosion || res2 == ImpactResult::bigexplosion  )
		{
			Explosion data;
			data.SetPos(c->first->GetPos());


			if (res2 == ImpactResult::explosion)
			{
				data.SetScale(float2(20.f, 20.f));
				data.SetLifeTime(10);
			}
			else
			{
				data.SetScale(float2(30.f, 30.f));
				data.SetLifeTime(100);
			}
			data.SetTexture(m_particle);
			data.SetWindowSize(m_windowBounds);
			m_explosionData.push_back(data);
		}

		if (res1 == ImpactResult::finish || res2 == ImpactResult::finish)
		{
			gamestate = GameState::LevelComplete;
		}
		if (res1 == ImpactResult::health || res2 == ImpactResult::health)
		{
			runner->health = 100;
		}
		if (res1 == ImpactResult::weapon_upgrade || res2 == ImpactResult::weapon_upgrade)
		{
			runner->UpgradeWeapons();
		}
	}

}
void SpriteGame::RemoveDead()
{
	for (auto pobject = level->all_gameobjects.begin(); pobject != level->all_gameobjects.end(); pobject++)
	{
		if (!(*pobject)->IsAlive())
		{
			delete *(pobject);
			pobject = level->all_gameobjects.erase(pobject);

			if (pobject == level->all_gameobjects.end())
				break;
		}
	}

	for (auto pobject = m_explosionData.begin(); pobject != m_explosionData.end(); pobject++)
	{
		if (!pobject->IsAlive())
		{
			pobject = m_explosionData.erase(pobject);

			if (pobject == m_explosionData.end())
				break;
		}
	}
}

void SpriteGame::Render()
{
	m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), reinterpret_cast<float*>(&D2D1::ColorF(D2D1::ColorF::AntiqueWhite)));

	m_spriteBatch->Begin();

	level->background->Draw(m_spriteBatch);
	level->foreground->Draw(m_spriteBatch);

	m_spriteBatch->Draw(
		m_ground.Get(),
		m_ground_pos,
		PositionUnits::DIPs,
		float2(m_windowBounds.Width, 30.0f),
		SizeUnits::DIPs,
		float4(1.0f, 1.0f, 1.0f, 1.0f),
		0.0f
	);

	for (auto object = level->all_gameobjects.begin(); object != level->all_gameobjects.end(); object++)
		(*object)->Draw(m_spriteBatch);
	
	if (runner->IsAlive())
		runner->Draw(m_spriteBatch);

	if (runner->health < 30 && runner->health>0 && gamestate == GameState::Playing)
		bad_health_background->Draw(m_spriteBatch);


	for (auto particle = m_explosionData.begin(); particle != m_explosionData.end(); particle++)
		particle->Draw(m_spriteBatch);

	m_spriteBatch->End();
}





