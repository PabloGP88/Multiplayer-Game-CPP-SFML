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
		Utils::printMsg("Texture Loaded Successfully!");

		backgroundTexture.setRepeated(true);
	}
	else
	{
		Utils::printMsg("Could not load texture: Assets/tileGrass1.png");
	}
	// Replace placeholder texture with a proper background texture, now that we have it.
	background.setTexture(backgroundTexture);
	background.setTextureRect(sf::IntRect({0, 0}, {1280, 960}));

	// Create Obstacles --------------------------------------------

	//CreateObstacles();

	// Create PickUps --------------------------------------------

	//CreatePickups();

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
		Utils::printMsg("Could not load UI font!", error);
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
			tanks[tankId]->Shoot();
			Utils::printMsg("Bullet Shoot!");
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
	}
}

void Game::Update(float dt)
{
	collisionManager.ClearDynamicColliders();

	for (auto& [id, tank] : tanks) {
		tank->Update(dt, collisionManager);
		tank->UpdateBullets(dt, collisionManager);
	}
	camera.setCenter(tanks[localId]->position);

	//UpdatePickups(dt);

	// for (auto& ammoBox : ammoBoxes)
	// {
	// 	tanks[localId]->CheckPickupCollision(ammoBox.get());
	// }
	//
	// for (auto& healthKit : healthKits)
	// {
	// 	tanks[localId]->CheckPickupCollision(healthKit.get());
	// }

	ui.Update(*tanks[localId]);

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

	// decoration.Render(window);
	// for (const auto& obstacle : obstacles)
	// {
	// 	obstacle->Render(window, false);
	// }
	//
	// RenderPickups(window);

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
		id
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

		std::uniform_int_distribution<int> brightness(150, 255); // Range for lightness

		sf::Color tint(brightness(gen), brightness(gen), brightness(gen));

		rock->sprite.setColor(tint);

		obstacles.push_back(std::move(rock));
	}
}

void Game::CreatePickups()
{
	// Create ammo boxes
	int numAmmoBoxes = 2;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> posX(100.f, WORLD_WIDTH - 100.f);
	std::uniform_real_distribution<float> posY(100.f, WORLD_HEIGHT - 100.f);

	for (int i = 0; i < numAmmoBoxes; i++)
	{
		sf::Vector2f pos = { posX(gen), posY(gen) };

		int ammoAmount = tanks[localId]->getMaxAmmo() / 4;

		auto ammoBox = std::make_unique<class ammoBox>(pos, WORLD_WIDTH, WORLD_HEIGHT, ammoAmount);
		ammoBoxes.push_back(std::move(ammoBox));
	}

	// Create health kits
	int numHealthKits = 2;

	for (int i = 0; i < numHealthKits; i++)
	{
		sf::Vector2f pos = { posX(gen), posY(gen) };

		// Health kits heal 25% of max health
		int healAmount = tanks[localId]->getMaxHealth() / 4;

		auto healthKit = std::make_unique<class healthKit>(pos, WORLD_WIDTH, WORLD_HEIGHT, healAmount);
		healthKits.push_back(std::move(healthKit));
	}

	Utils::printMsg("Created " + std::to_string(numAmmoBoxes) + " ammo boxes and " +
				   std::to_string(numHealthKits) + " health kits", success);
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