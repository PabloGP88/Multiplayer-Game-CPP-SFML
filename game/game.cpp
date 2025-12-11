//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//
#include "game.h"
#include "utils.h"

Game::Game(int localPlayer)
	: collisionManager(1280.f, 960.f), decoration(1280.f, 960.f), ui(uiFont),
	localId(localPlayer)

{
	// Initialise the background texture and sprite.
	if (backgroundTexture.loadFromFile("Assets/tileGrass1.png"))
	{
		backgroundTexture.setRepeated(true);
	}
	else
	{
		Utils::printMsg("Could not load texture: Assets/tileGrass1.png");
	}
	// Replace placeholder texture with a proper background texture, now that we have it.
	background.setTexture(backgroundTexture);
	background.setTextureRect(sf::IntRect({0, 0}, {1280, 960}));


	// --------------------------------------

	tanks[localId] = std::make_unique<Tank>("blue");
	tanks[localId]->position = {640, 480};

	// Set default tank position to be the centre of the window.
	tanks[localId]->position = {640, 480};

	sf::Vector2<float> size = {960.f, 720.f}; // camera size, I'm using same as window

	camera.setSize(size);
	camera.setCenter(tanks[localId]->position);

	// Loading Fonts

	if (!uiFont.openFromFile("Assets/MomoTrustDisplay-Regular.ttf")) // Use your font file
	{
		Utils::printMsg("No font", error);
	}

	ui = gameUI(uiFont);
}

void Game::AddTank(int tankId, std::string tankColour) {
	tanks[tankId] = std::make_unique<Tank>(tankColour);
	tanks[tankId]->position = {640, 480};

	if (tankId == localId) {
		camera.setCenter(tanks[localId]->position);
	}
	Utils::printMsg("Added tank " + std::to_string(tankId) + " with color: " + tankColour, success);
}

void Game::HandleEvents(const std::optional<sf::Event> event, int tankId)
{
	// ------------------------------------------------------------------

	if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
		if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
			tanks[tankId]->isMoving.forward = true;
			tanks[tankId]->isMoving.backward = false;
		}
		else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
			tanks[tankId]->isMoving.forward = false;
			tanks[tankId]->isMoving.backward = true;
		}
		if (keyPressed->scancode == sf::Keyboard::Scancode::A) {
			tanks[tankId]->isMoving.left = true;
			tanks[tankId]->isMoving.right = false;
		}
		else if (keyPressed->scancode == sf::Keyboard::Scancode::D) {
			tanks[tankId]->isMoving.left = false;
			tanks[tankId]->isMoving.right = true;
		}

		// ------------------------------------------------------------------ Barrel Rotation

		if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
			tanks[tankId]->isAiming.right = true;
			tanks[tankId]->isAiming.left = false;
		} else if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
		{
			tanks[tankId]->isAiming.left = true;
			tanks[tankId]->isAiming.right = false;
		}

		if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
		{
			if (tanks[tankId]->getAmmo() > 0)
				tanks[tankId]->wantsToShoot = true;
			Utils::printMsg("Buddy wants to shoot");
		}

	}

	// Handle key release events passed from window.
	else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
		if (keyReleased->scancode == sf::Keyboard::Scancode::W)
			tanks[tankId]->isMoving.forward = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::S)
			tanks[tankId]->isMoving.backward = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::A)
			tanks[tankId]->isMoving.left = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::D)
			tanks[tankId]->isMoving.right = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::Right)
			tanks[tankId]->isAiming.right = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::Left)
			tanks[tankId]->isAiming.left = false;
		if (keyReleased->scancode == sf::Keyboard::Scancode::Space)
			tanks[tankId]->wantsToShoot = false;
	}

}

void Game::Update(float dt)
{
	collisionManager.ClearDynamicColliders();

	for (auto& [id, tank] : tanks) {

		if (id == localId)
		{
			// Local tank
			tank->Update(dt, collisionManager);
		} else
		{
			// Interpolate Remote Tanks
			InterpolateRemoteTanks(collisionManager, dt, id);
		}

		tank->UpdateBullets(dt, collisionManager);
	}

	// Check bullet collisions against all tanks

	for (auto& [shooterId, shooterTank] : tanks) {
		for (auto& bullet : shooterTank->bullets) {
			if (!bullet->IsActive()) continue;

			// Check against all other tanks
			for (auto& [targetId, targetTank] : tanks) {
				if (targetId == shooterId) continue; // Don't hit yourself
				if (!targetTank->IsAlive()) continue;

				bullet->CheckTankCollision(targetTank.get());
			}
		}

	}


	camera.setCenter(tanks[localId]->position);

	UpdatePickups(dt);

	for (auto& ammoBox : ammoBoxes)
	{
	 	if (tanks[localId]->CheckPickupCollision(ammoBox.get()))
	 	{
	 		// 0 is ammo box
			OnPickupCollected(ammoBox->GetPickupId(), 0);
	 	}
	}

	for (auto& healthKit : healthKits)
	{
		if (tanks[localId]->CheckPickupCollision(healthKit.get()))
		{
			// 1 is health kit
			OnPickupCollected(healthKit->GetPickupId(), 1);
		}
	}

	ui.Update(*tanks[localId]);

}

// Interpolation stuff
void Game::InterpolateRemoteTanks(CollisionManager& collisionManager, float dt, int tankID)
{
	if (previousStates.find(tankID) == previousStates.end() || targetStates.find(tankID) == targetStates.end())
	{
		// Theres no data yet
		return;
	}

	auto& prevState = previousStates[tankID];
	auto& targetState = targetStates[tankID];
	auto& clock = interpClocks[tankID];

	float elapsedTime = clock.getElapsedTime().asSeconds();
	float currentTime = std::min(elapsedTime / INTERP_TIME, 1.0f);

	// Linear lerp position
	tanks[tankID]->position = prevState.position + (targetState.position - prevState.position) * currentTime;
	// Lerp for angles
	tanks[tankID]->bodyRotation = findLerpAngle(prevState.bodyRotation, targetState.bodyRotation, currentTime);
	tanks[tankID]->barrelRotation = findLerpAngle(prevState.barrelRotation, targetState.barrelRotation, currentTime);

	tanks[tankID]->Update(dt, collisionManager);
}

void Game::AddNetworkTankState(int tankID, const GameStateMessage::PlayerState& state)
{
	if (targetStates.find(tankID) != targetStates.end()) {
		previousStates[tankID] = targetStates[tankID];
	}

	RemoteTankData newState;
	newState.position = {state.x, state.y};
	newState.bodyRotation = sf::degrees(state.rotationBody);
	newState.barrelRotation = sf::degrees(state.rotationBarrel);
	targetStates[tankID] = newState;

	// Restart interpolation timer
	interpClocks[tankID].restart();
}


sf::Angle Game::findLerpAngle(sf::Angle angle1, sf::Angle angle2, float t)
{
	// Formula by shaunlebron - https://gist.github.com/shaunlebron/8832585#file-anglelerp-js-L9

	float a0 = angle1.asRadians();
	float a1 = angle2.asRadians();

	// Find shortest angle distance
	float pi = 3.1416;
	float max = 2.0f * pi;

	// fmod works like % but is used like this in cpp for decimals
	float da = fmod(a1 - a0, max);
	float shortestDist = fmod(2.0f * da, max) - da;

	// Lerp and return
	float shortestAng = a0 + shortestDist * t;
	return sf::radians(shortestAng);
}


void Game::NetworkUpdate(float dt, int idTank, TankMessage data) {
	if (tanks.find(idTank) == tanks.end()) {
		// Map player IDs to colors dynamically
		std::vector<std::string> colors = {"blue", "red", "green", "black"};
		std::string color = colors[idTank % colors.size()];
		AddTank(idTank, color);
	}

	Tank* remoteTank = tanks[idTank].get();
	remoteTank->position = {data.x, data.y};
	remoteTank->bodyRotation = sf::degrees(data.rotationBody);
	remoteTank->barrelRotation = sf::degrees(data.rotationBarrel);
	remoteTank->Update(dt, collisionManager);
}


void Game::Render(sf::RenderWindow& window)
{
	window.clear(sf::Color(70, 130, 180));

	window.setView(camera); // set the window to use the camera as viewport

	window.draw(background);

	decoration.Render(window);
	for (const auto& obstacle : obstacles)
	{
		obstacle->Render(window, false);
	}
	//
	RenderPickups(window);

	for (auto& [id, tank] : tanks) {
		tank->Render(window);
		tank->RenderBullets(window);
	}

	window.setView(window.getDefaultView());
	ui.Draw(window);
}

TankMessage Game::GetNetworkUpdate(int id) {
	return {
		tanks[id]->position.x,
		tanks[id]->position.y,
		tanks[id]->bodyRotation.asDegrees(),
		tanks[id]->barrelRotation.asDegrees(),
		tanks[id]->wantsToShoot,
		tanks[id]->IsAlive()
	};
}

void Game::CreateObstacles()
{
	int numRocks = 10;
	int minSize = 3;
	int maxSize = 6;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> posX(0.f, 800.f);
	std::uniform_real_distribution<float> posY(0.f, 700);
	std::uniform_int_distribution<int> sizeDist(minSize, maxSize);

	for (int i = 0; i < numRocks; i++)
	{
		sf::Vector2f pos = { posX(gen), posY(gen) };
		int size = sizeDist(gen);
		sf::Vector2f scale(size, size);

		auto rock = std::make_unique<obstacle>("Assets/Rock.png", pos,
					  sf::Vector2f(0,0), sf::Vector2f(0,0), scale);

		collisionManager.AddStaticCollider(rock->GetBounds());

		std::uniform_int_distribution<int> brightness(200, 255); // Range for lightness

		sf::Color tint(brightness(gen), brightness(gen), brightness(gen));

		rock->sprite.setColor(tint);

		obstacles.push_back(std::move(rock));
	}
}

void Game::CreatePickups(PickUpMessage& msg)
{
	ammoBoxes.clear();
	healthKits.clear();

	for (auto& pickupData : msg.pickUps)
	{
		sf::Vector2f pos = {pickupData.x, pickupData.y};
		if (pickupData.pickUpType == 0)
		{
			// Create Ammo Box
			auto ammo = std::make_unique<class ammoBox>(
				pos,
				WORLD_WIDTH,
				WORLD_HEIGHT
			);
			ammo->SetPickupId(pickupData.pickUpId);
			ammoBoxes.push_back(std::move(ammo));

		} else
		{
			// Create HealthKjt

			auto health = std::make_unique<class healthKit>(
				pos,
				WORLD_WIDTH,
				WORLD_HEIGHT
			);
			health->SetPickupId(pickupData.pickUpId);
			healthKits.push_back(std::move(health));
		}
	}
}

void Game::UpdatePickups(float dt)
{
	for (auto& ammoBox : ammoBoxes)
	{
		ammoBox->Update(dt);
	}

	for (auto& healthKit : healthKits)
	{
		healthKit->Update(dt);
	}
}

void Game::RenderPickups(sf::RenderWindow& window)
{
	for (const auto& ammoBox : ammoBoxes)
	{
		ammoBox->Render(window);
	}

	for (const auto& healthKit : healthKits)
	{
		healthKit->Render(window);
	}
}
